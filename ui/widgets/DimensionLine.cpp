//
// Created by matthew on 17/07/2020.
//

#include "DimensionLine.h"

DimensionLine::DimensionLine(QRectF dimensionBounds, DimensionLine::Orientation orientation, const QString &label, int labelFontSize) {
    this->bounds = dimensionBounds;
    this->orientation = orientation;
    this->label = label;
    this->labelFontSize = labelFontSize;
}

void DimensionLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    switch (orientation) {
        case VERTICAL: {
            painter->save();
            painter->setPen(Qt::SolidLine);
            painter->drawLine(bounds.topLeft(), bounds.topRight());
            painter->drawLine(bounds.bottomLeft(), bounds.bottomRight());

            painter->setPen(Qt::DashLine);
            painter->drawLine((bounds.topLeft() + bounds.topRight()) / 2,
                              (bounds.bottomLeft() + bounds.bottomRight()) / 2);

            painter->setPen(Qt::SolidLine);
            painter->translate(bounds.center());
            painter->rotate(-90);
            QFont font = painter->font();
            font.setPointSize(labelFontSize);
            painter->setFont(font);
            painter->drawText(-QFontMetrics(font).size(Qt::TextSingleLine, label).width() / 2, 0, label);
            painter->restore();
            break;
        }
        case HORIZONTAL: {
            painter->save();
            painter->setPen(Qt::SolidLine);
            painter->drawLine(bounds.topLeft(), bounds.bottomLeft());
            painter->drawLine(bounds.topRight(), bounds.bottomRight());

            painter->setPen(Qt::DashLine);
            painter->drawLine((bounds.topLeft() + bounds.bottomLeft()) / 2,
                              (bounds.topRight() + bounds.bottomRight()) / 2);

            painter->setPen(Qt::SolidLine);
            painter->translate(bounds.center());
            QFont font = painter->font();
            font.setPointSize(labelFontSize);
            painter->setFont(font);
            painter->drawText(-QFontMetrics(font).size(Qt::TextSingleLine, label).width() / 2, 0, label);
            painter->restore();
            break;
        }
    }
}

QRectF DimensionLine::boundingRect() const {
    return bounds;
}

WidgetDimensionLine::WidgetDimensionLine(QRectF dimensionBounds, DimensionLine::Orientation orientation,
                                         QGraphicsProxyWidget *proxy) {
    this->bounds = dimensionBounds;
    this->orientation = orientation;
    this->proxy = proxy;
}

void WidgetDimensionLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    switch (orientation) {
        case DimensionLine::VERTICAL: {
            painter->save();
            painter->setPen(Qt::SolidLine);
            painter->drawLine(bounds.topLeft(), bounds.topRight());
            painter->drawLine(bounds.bottomLeft(), bounds.bottomRight());

            painter->setPen(Qt::DashLine);
            painter->drawLine((bounds.topLeft() + bounds.topRight()) / 2,
                              (bounds.bottomLeft() + bounds.bottomRight()) / 2);

            proxy->setRotation(-90);
            proxy->setPos(bounds.center() - QPointF(proxy->size().width() / 2, proxy->size().height() * 2));

            painter->restore();
            break;
        }
        case DimensionLine::HORIZONTAL: {
            painter->save();
            painter->setPen(Qt::SolidLine);
            painter->drawLine(bounds.topLeft(), bounds.bottomLeft());
            painter->drawLine(bounds.topRight(), bounds.bottomRight());

            painter->setPen(Qt::DashLine);
            painter->drawLine((bounds.topLeft() + bounds.bottomLeft()) / 2,
                              (bounds.topRight() + bounds.bottomRight()) / 2);

            proxy->setPos(bounds.center() - QPointF(proxy->size().width() / 2, proxy->size().height() * 2));

            painter->restore();
            break;
        }
    }
}

QRectF WidgetDimensionLine::boundingRect() const {
    return bounds;
}
