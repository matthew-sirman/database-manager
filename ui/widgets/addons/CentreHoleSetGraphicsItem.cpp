//
// Created by matthew on 10/08/2020.
//

#include "CentreHoleSetGraphicsItem.h"

CentreHoleSetGraphicsItem::CentreHoleSetGraphicsItem(const QRectF &bounds, float matWidth, float matLength,
                                                     Inspector *inspector)
        : inspector(inspector) {
    this->matBounds = bounds;
    this->matWidth = matWidth;
    this->matLength = matLength;

    setAcceptHoverEvents(true);

    static std::unordered_map<unsigned char, unsigned char> shapeOrder = {
    {1, 1},
    {2, 4},
    {3, 2},
    {4, 3},
    {5, 5},
    {6, 0}
    };

    std::function<bool(const Aperture&, const Aperture&)> apertureComparator = [](const Aperture& a, const Aperture& b) {
        if (a.apertureShapeID != b.apertureShapeID) {
            return shapeOrder[a.apertureShapeID] < shapeOrder[b.apertureShapeID];
        }
        return a.width < b.width;
    };

    DrawingComponentManager<Aperture>::addCallback([this, apertureComparator]() {
        apertureSource.updateSource();
        apertureSource.sort(apertureComparator);
});

    apertureSource.updateSource();
    apertureSource.sort(apertureComparator);
}

QRectF CentreHoleSetGraphicsItem::boundingRect() const {
    return matBounds;
}

void CentreHoleSetGraphicsItem::setBounds(const QRectF &bounds, float width, float length) {
    matBounds = bounds;
    matWidth = width;
    matLength = length;
}

void CentreHoleSetGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->save();

    for (Drawing::CentreHole *hole : holes) {
        Aperture ap = DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID);
        QRectF verticalHoleBounds = QRectF(
                QPointF(matBounds.left() +
                        ((hole->pos.x - ap.width / 2) / matWidth) * matBounds.width(),
                        matBounds.top() +
                        ((hole->pos.y - ap.length / 2) / matLength) * matBounds.height()),
                QSizeF((ap.width / matWidth) * matBounds.width(),
                       (ap.length / matLength) * matBounds.height())
        );
        QRectF horizontalHoleBounds = QRectF(
        QPointF(matBounds.left() +
                ((hole->pos.x - ap.length / 2) / matLength) * matBounds.height(),
                matBounds.top() +
                ((hole->pos.y - ap.width / 2) / matWidth) * matBounds.width()),
        QSizeF((ap.length / matLength) * matBounds.height(),
               (ap.width / matWidth) * matBounds.width())
        );

        painter->setPen(Qt::red);
        ApertureShape shape = DrawingComponentManager<ApertureShape>::findComponentByID(ap.apertureShapeID);
        if (shape.shape == "SQ" || shape.shape == "SL") {
            painter->drawRect(verticalHoleBounds);
            painter->setPen(Qt::DashDotLine);
            painter->drawLine(QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.top()),
                              QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.bottom()));
            painter->drawLine(QPointF(verticalHoleBounds.left(), verticalHoleBounds.center().y()),
                              QPointF(verticalHoleBounds.right(), verticalHoleBounds.center().y()));
        } else if (shape.shape == "ST") {
            painter->drawRect(horizontalHoleBounds);
            painter->setPen(Qt::DashDotLine);
            painter->drawLine(QPointF(horizontalHoleBounds.center().x(), horizontalHoleBounds.top()),
                              QPointF(horizontalHoleBounds.center().x(), horizontalHoleBounds.bottom()));
            painter->drawLine(QPointF(horizontalHoleBounds.left(), horizontalHoleBounds.center().y()),
                              QPointF(horizontalHoleBounds.right(), horizontalHoleBounds.center().y()));
        } else if (shape.shape == "DIA") {
            painter->setRenderHint(QPainter::Antialiasing);
            QPainterPath roundedRectPath;
            double radius = (std::min(ap.width, ap.length) / 2.0) / matWidth * matBounds.width();
            roundedRectPath.addRoundedRect(verticalHoleBounds, radius, radius);
            painter->drawPath(roundedRectPath);
            painter->setPen(Qt::DashDotLine);
            painter->drawLine(QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.top()),
                              QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.bottom()));
            painter->drawLine(QPointF(verticalHoleBounds.left(), verticalHoleBounds.center().y()),
                              QPointF(verticalHoleBounds.right(), verticalHoleBounds.center().y()));
        } else if (shape.shape == "RL") {
            painter->setRenderHint(QPainter::Antialiasing);
            QPainterPath roundedRectPath;
            double radius = (std::min(ap.width, ap.length) / 2.0) / matWidth * matBounds.width();
            roundedRectPath.addRoundedRect(verticalHoleBounds, radius, radius);
            painter->drawPath(roundedRectPath);
            painter->setPen(Qt::DashDotLine);
            painter->drawLine(QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.top()),
                              QPointF(verticalHoleBounds.center().x(), verticalHoleBounds.bottom()));
            painter->drawLine(QPointF(verticalHoleBounds.left(), verticalHoleBounds.center().y()),
                              QPointF(verticalHoleBounds.right(), verticalHoleBounds.center().y()));
        } else if (shape.shape == "RT") {
            painter->setRenderHint(QPainter::Antialiasing);
            QPainterPath roundedRectPath;
            double radius = (std::min(ap.width, ap.length) / 2.0) / matWidth * matBounds.width();
            roundedRectPath.addRoundedRect(horizontalHoleBounds, radius, radius);
            painter->drawPath(roundedRectPath);
            painter->setPen(Qt::DashDotLine);
            painter->drawLine(QPointF(horizontalHoleBounds.center().x(), horizontalHoleBounds.top()),
                              QPointF(horizontalHoleBounds.center().x(), horizontalHoleBounds.bottom()));
            painter->drawLine(QPointF(horizontalHoleBounds.left(), horizontalHoleBounds.center().y()),
                              QPointF(horizontalHoleBounds.right(), horizontalHoleBounds.center().y()));
        }


    }


    painter->restore();
}

void CentreHoleSetGraphicsItem::addCentreHole(Drawing::CentreHole &hole) {
    if (!DrawingComponentManager<Aperture>::validComponentID(apertureID))
        apertureID = hole.apertureID;
    holes.push_back(&hole);
    holeAcquireIDs.push_back(-1);
}

void CentreHoleSetGraphicsItem::clearCentreHoles() {
    holes.clear();
    holeAcquireIDs.clear();
}

Aperture CentreHoleSetGraphicsItem::currentAperture() const {
    return DrawingComponentManager<Aperture>::findComponentByID(apertureID);
}

bool CentreHoleSetGraphicsItem::contains(const QPointF &point) const {
    for (Drawing::CentreHole *hole : holes) {
        QRectF holeBounds = QRectF(
                QPointF(matBounds.left() +
                        ((hole->pos.x - DrawingComponentManager<Aperture>::findComponentByID(apertureID).width / 2) / matWidth) * matBounds.width(),
                        matBounds.top() +
                        ((hole->pos.y - DrawingComponentManager<Aperture>::findComponentByID(apertureID).length / 2) / matLength) * matBounds.height()),
                QSizeF((DrawingComponentManager<Aperture>::findComponentByID(apertureID).width / matWidth) * matBounds.width(),
                       (DrawingComponentManager<Aperture>::findComponentByID(apertureID).length / matLength) * matBounds.height())
        );
        if (holeBounds.contains(point)) {
            return true;
        }
    }
    return false;
}

void CentreHoleSetGraphicsItem::setRemoveFunction(const std::function<void(const Drawing::CentreHole &)> &remove) {
    removeFunction = remove;
}

void CentreHoleSetGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        for (unsigned i = 0; i < holes.size(); i++) {
            Drawing::CentreHole *hole = holes[i];
            QRectF holeBounds = QRectF(
                    QPointF(matBounds.left() +
                            ((hole->pos.x - DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).width / 2) / matWidth) * matBounds.width(),
                            matBounds.top() +
                            ((hole->pos.y - DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).length / 2) / matLength) * matBounds.height()),
                    QSizeF((DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).width / matWidth) * matBounds.width(),
                           (DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).length / matLength) * matBounds.height())
            );

            if (holeBounds.contains(event->pos())) {
                if (inspector) {
                    if (inspector->currentOwner() != holeAcquireIDs[i]) {
                        holeAcquireIDs[i] = inspector->acquire();
                        inspector->addFloatField("X Position:", hole->pos.x, 1, 0);
                        inspector->addFloatField("Y Position:", hole->pos.y, 1, 0);
                        inspector->addComponentField<Aperture>("Aperture:", [this](const Aperture& aperture) {
                            apertureID = aperture.componentID();
                            for (Drawing::CentreHole *hole : holes) {
                                hole->apertureID = aperture.componentID();
                            }
                        }, apertureSource, DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).handle());
                    }
                }
                break;
            }
        }
    }

    QGraphicsItem::mousePressEvent(event);
}

void CentreHoleSetGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    for (Drawing::CentreHole *hole : holes) {
        QRectF holeBounds = QRectF(
                QPointF(matBounds.left() +
                        ((hole->pos.x - DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).width / 2) / matWidth) * matBounds.width(),
                        matBounds.top() +
                        ((hole->pos.y - DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).length / 2) / matLength) * matBounds.height()),
                QSizeF((DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).width / matWidth) * matBounds.width(),
                       (DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).length / matLength) * matBounds.height())
        );
        if (holeBounds.contains(event->pos())) {
            if (inspector) {
                inspector->expand();
            }
            break;
        }
    }

    QGraphicsItem::mouseDoubleClickEvent(event);
}

void CentreHoleSetGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    for (Drawing::CentreHole *hole : holes) {
        QRectF holeBounds = QRectF(
                QPointF(matBounds.left() +
                        ((hole->pos.x - DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).width / 2) / matWidth) * matBounds.width(),
                        matBounds.top() +
                        ((hole->pos.y - DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).length / 2) / matLength) * matBounds.height()),
                QSizeF((DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).width / matWidth) * matBounds.width(),
                       (DrawingComponentManager<Aperture>::findComponentByID(hole->apertureID).length / matLength) * matBounds.height())
        );
        if (holeBounds.contains(event->pos())) {
            QMenu *menu = new QMenu();

            menu->addAction("Remove", [this, hole]() {
                if (removeFunction) {
                    removeFunction(*hole);
                }
            }, Qt::Key_Delete);

            menu->popup(event->screenPos());

            QWidget::connect(menu, &QMenu::triggered, [menu](QAction *) { menu->deleteLater(); });
            break;
        }
    }

    QGraphicsItem::contextMenuEvent(event);
}

bool CentreHoleSetGraphicsItem::empty() const {
    return holes.empty();
}