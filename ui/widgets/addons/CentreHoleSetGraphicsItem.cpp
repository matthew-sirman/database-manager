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
        QRectF holeBounds = QRectF(
                QPointF(matBounds.left() +
                        ((hole->pos.x - hole->centreHoleShape.width / 2) / matWidth) * matBounds.width(),
                        matBounds.top() +
                        ((hole->pos.y - hole->centreHoleShape.length / 2) / matLength) * matBounds.height()),
                QSizeF((hole->centreHoleShape.width / matWidth) * matBounds.width(),
                       (hole->centreHoleShape.length / matLength) * matBounds.height())
        );

        painter->setPen(Qt::red);

        if (!hole->centreHoleShape.rounded) {
            painter->drawRect(holeBounds);
            painter->setPen(Qt::DashDotLine);
            painter->drawLine(QPointF(holeBounds.center().x(), holeBounds.top()),
                              QPointF(holeBounds.center().x(), holeBounds.bottom()));
            painter->drawLine(QPointF(holeBounds.left(), holeBounds.center().y()),
                              QPointF(holeBounds.right(), holeBounds.center().y()));
        } else {
            painter->setRenderHint(QPainter::Antialiasing);
            QPainterPath roundedRectPath;
            double radius = (std::min(hole->centreHoleShape.width, hole->centreHoleShape.length) / 2.0) / matWidth * matBounds.width();
            roundedRectPath.addRoundedRect(holeBounds, radius, radius);
            painter->drawPath(roundedRectPath);
            painter->setPen(Qt::DashDotLine);
            painter->drawLine(QPointF(holeBounds.center().x(), holeBounds.top()),
                              QPointF(holeBounds.center().x(), holeBounds.bottom()));
            painter->drawLine(QPointF(holeBounds.left(), holeBounds.center().y()),
                              QPointF(holeBounds.right(), holeBounds.center().y()));
        }
    }

    painter->restore();
}

void CentreHoleSetGraphicsItem::addCentreHole(Drawing::CentreHole &hole) {
    holes.push_back(&hole);
    holeAcquireIDs.push_back(-1);
}

void CentreHoleSetGraphicsItem::clearCentreHoles() {
    holes.clear();
    holeAcquireIDs.clear();
}

Drawing::CentreHole::Shape CentreHoleSetGraphicsItem::currentShape() const {
    return currentHoleShape;
}

bool CentreHoleSetGraphicsItem::contains(const QPointF &point) const {
    for (Drawing::CentreHole *hole : holes) {
        QRectF holeBounds = QRectF(
                QPointF(matBounds.left() +
                        ((hole->pos.x - hole->centreHoleShape.width / 2) / matWidth) * matBounds.width(),
                        matBounds.top() +
                        ((hole->pos.y - hole->centreHoleShape.length / 2) / matLength) * matBounds.height()),
                QSizeF((hole->centreHoleShape.width / matWidth) * matBounds.width(),
                       (hole->centreHoleShape.length / matLength) * matBounds.height())
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
                            ((hole->pos.x - hole->centreHoleShape.width / 2) / matWidth) * matBounds.width(),
                            matBounds.top() +
                            ((hole->pos.y - hole->centreHoleShape.length / 2) / matLength) * matBounds.height()),
                    QSizeF((hole->centreHoleShape.width / matWidth) * matBounds.width(),
                           (hole->centreHoleShape.length / matLength) * matBounds.height())
            );

            if (holeBounds.contains(event->pos())) {
                if (inspector) {
                    if (inspector->currentOwner() != holeAcquireIDs[i]) {
                        holeAcquireIDs[i] = inspector->acquire();
                        inspector->addFloatField("Width:", [this](float value) {
                            std::for_each(holes.begin(), holes.end(),
                                          [value](Drawing::CentreHole *hole) { hole->centreHoleShape.width = value; });
                            currentHoleShape.width = value;
                        }, currentHoleShape.width, 1, 0);
                        inspector->addFloatField("Length:", [this](float value) {
                            std::for_each(holes.begin(), holes.end(),
                                          [value](Drawing::CentreHole *hole) { hole->centreHoleShape.length = value; });
                            currentHoleShape.length = value;
                        }, currentHoleShape.length, 1, 0);
                        inspector->addFloatField("X Position:", hole->pos.x, 1, 0);
                        inspector->addFloatField("Y Position:", hole->pos.y, 1, 0);
                        inspector->addBooleanField("Rounded:", [this](bool value) {
                            std::for_each(holes.begin(), holes.end(),
                                          [value](Drawing::CentreHole *hole) { hole->centreHoleShape.rounded = value; });
                            currentHoleShape.rounded = value;
                        }, currentHoleShape.rounded);
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
                        ((hole->pos.x - hole->centreHoleShape.width / 2) / matWidth) * matBounds.width(),
                        matBounds.top() +
                        ((hole->pos.y - hole->centreHoleShape.length / 2) / matLength) * matBounds.height()),
                QSizeF((hole->centreHoleShape.width / matWidth) * matBounds.width(),
                       (hole->centreHoleShape.length / matLength) * matBounds.height())
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
                        ((hole->pos.x - hole->centreHoleShape.width / 2) / matWidth) * matBounds.width(),
                        matBounds.top() +
                        ((hole->pos.y - hole->centreHoleShape.length / 2) / matLength) * matBounds.height()),
                QSizeF((hole->centreHoleShape.width / matWidth) * matBounds.width(),
                       (hole->centreHoleShape.length / matLength) * matBounds.height())
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
