#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 18/07/2020.
//

#include "AddLapWidget.h"

AddLapWidget::AddLapWidget(QGraphicsScene *scene, QRectF hoverBounds, DimensionLine::Orientation orientation,
                           const std::optional<Drawing::Lap> &currentLap, const QColor &unusedColour,
                           const QColor &unusedHighlightColour,
                           const QColor &usedColour) {
    this->hoverBounds = hoverBounds;
    this->unusedColour = unusedColour;
    this->unusedHighlightColour = unusedHighlightColour;
    this->usedColour = usedColour;
    this->orientation = orientation;

    hovering = false;

    if (currentLap.has_value()) {
        lap = currentLap.value();
    } else {
        lap = Drawing::Lap();
    }
    used = false;

    setAcceptHoverEvents(true);

    widthInput = new QDoubleSpinBox();
    widthInput->setDecimals(1);
    widthInput->setMinimum(0);
    widthInput->setMaximum(32767);
    widthInput->setValue(lap.width);

    QWidget::connect(widthInput, &QDoubleSpinBox::editingFinished, [this]() {
        lap.width = (float) widthInput->value();
        if (lapChangedCallback) {
            lapChangedCallback(lap);
        }
    });

    widthProxy = scene->addWidget(widthInput);
    widthProxy->setZValue(1);

    QRectF dimensionPosition;
    switch (orientation) {
        case DimensionLine::VERTICAL:
            dimensionPosition = QRectF(hoverBounds.left(),
                                       hoverBounds.bottom() -
                                       hoverBounds.height() * (verticalEditPosition + editSize / 2),
                                       hoverBounds.width(), hoverBounds.height() * editSize);
            widthDimension = new WidgetDimensionLine(dimensionPosition, DimensionLine::HORIZONTAL, widthProxy);
            break;
        case DimensionLine::HORIZONTAL:
            dimensionPosition = QRectF(hoverBounds.left() + hoverBounds.width() * (horizontalEditPosition + editSize / 2),
                                       hoverBounds.top(),
                                       hoverBounds.width() * editSize, hoverBounds.height());
            widthDimension = new WidgetDimensionLine(dimensionPosition, DimensionLine::VERTICAL, widthProxy);
            break;
    }
    widthDimension->setZValue(0.5);

    widthProxy->setVisible(used);
    widthDimension->setVisible(used);

    scene->addItem(widthDimension);

    DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
    materialSource.updateSource();

    setToolTip("Click to add overlap");
}

void AddLapWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->save();

    painter->setPen(Qt::PenStyle::NoPen);
    if (!hovering && !used) {
        painter->setBrush(QBrush(unusedColour));
    }
    if (hovering && !used) {
        painter->setBrush(QBrush(unusedHighlightColour));
    }
    if (used) {
        painter->setBrush(QBrush(usedColour));
    }
    painter->drawRect(hoverBounds);

    painter->restore();
}

QRectF AddLapWidget::boundingRect() const {
    return hoverBounds;
}

void AddLapWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    hovering = true;
    QGraphicsItem::hoverEnterEvent(event);
}

void AddLapWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    hovering = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void AddLapWidget::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem::mousePressEvent(event);
    if (event->button() == Qt::LeftButton && !used) {
        setUsed(true);
    }
}

void AddLapWidget::setActivationCallback(const std::function<void(bool)> &callback) {
    activateCallback = callback;
}

void AddLapWidget::setLapChangedCallback(const std::function<void(const Drawing::Lap &)> &callback) {
    lapChangedCallback = callback;
}

void AddLapWidget::setBounds(QRectF bounds) {
    hoverBounds = bounds;

    QRectF dimensionPosition;
    switch (orientation) {
        case DimensionLine::VERTICAL:
            dimensionPosition = QRectF(hoverBounds.left(),
                                       hoverBounds.bottom() -
                                       hoverBounds.height() * (verticalEditPosition + editSize / 2),
                                       hoverBounds.width(), hoverBounds.height() * editSize);
            break;
        case DimensionLine::HORIZONTAL:
            dimensionPosition = QRectF(hoverBounds.left() + hoverBounds.width() * (horizontalEditPosition + editSize / 2),
                                       hoverBounds.top(),
                                       hoverBounds.width() * editSize, hoverBounds.height());
            break;
    }
    widthDimension->setBounds(dimensionPosition);
}

void AddLapWidget::setLap(const std::optional<Drawing::Lap> &newLap) {
    if (newLap.has_value()) {
        lap = newLap.value();
    } else {
        lap = Drawing::Lap();
    }

    widthInput->setValue(lap.width);

    setUsed(newLap.has_value());
}

void AddLapWidget::setUsed(bool val) {
    if (used == val) {
        return;
    }

    used = val;

    widthProxy->setVisible(used);
    widthDimension->setVisible(used);

    if (used) {
        setToolTip("");
    } else {
        setToolTip("Click to add overlap");
    }

    if (activateCallback) {
        activateCallback(used);
    }
}

void AddLapWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    if (used) {
        QMenu *menu = new QMenu();

        QMenu *materialSubMenu;

        switch (lap.attachmentType) {
            case LapAttachment::INTEGRAL:
                materialSubMenu = menu->addMenu("Make Bonded");
                break;
            case LapAttachment::BONDED:
                menu->addAction("Make Integral", [this]() {
                    lap.attachmentType = LapAttachment::INTEGRAL;
                    if (lapChangedCallback) {
                        lapChangedCallback(lap);
                    }
                });

                materialSubMenu = menu->addMenu("Material");
                break;
        }

        for (const ComboboxDataElement &mat : materialSource) {
            QAction *action = materialSubMenu->addAction(mat.text.c_str(), [this, mat]() {
                if (mat.index.has_value()) {
                    lap.setMaterial(mat.index.value());
                    lap.attachmentType = LapAttachment::BONDED;
                    if (lapChangedCallback) {
                        lapChangedCallback(lap);
                    }
                }
            });

            if (mat.index.has_value()) {
                if (mat.index.value() == lap.material().handle()) {
                    QFont font = action->font();
                    font.setBold(true);
                    font.setUnderline(true);
                    action->setFont(font);
                }
            }
        }

        menu->addAction("Remove overlap",
                        [this]() { setUsed(false); }, Qt::Key_Delete);
        menu->popup(event->screenPos());
    }
    QGraphicsItem::contextMenuEvent(event);
}

#pragma clang diagnostic pop