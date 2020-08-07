//
// Created by matthew on 17/07/2020.
//

#ifndef DATABASE_MANAGER_DIMENSIONLINE_H
#define DATABASE_MANAGER_DIMENSIONLINE_H

#include <QGraphicsLineItem>
#include <QPainter>
#include <QTextItem>
#include <QGraphicsProxyWidget>

class DimensionLine : public QGraphicsItem {
public:
    enum Orientation {
        VERTICAL,
        HORIZONTAL
    };

    DimensionLine(QRectF dimensionBounds, Orientation orientation, const QString &label = QString(), int labelFontSize=10);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    [[nodiscard]] QRectF boundingRect() const override;

    void setBounds(QRectF newBounds);

    void setLabel(const QString &newLabel);

private:
    QRectF bounds;
    Orientation orientation;
    QString label;
    int labelFontSize;
};

class WidgetDimensionLine : public QGraphicsItem {
public:
    WidgetDimensionLine(QRectF dimensionBounds, DimensionLine::Orientation orientation, QGraphicsProxyWidget *proxy);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    [[nodiscard]] QRectF boundingRect() const override;

    void setBounds(QRectF newBounds);

private:
    QRectF bounds;
    DimensionLine::Orientation orientation;
    QGraphicsProxyWidget *proxy = nullptr;
};


#endif //DATABASE_MANAGER_DIMENSIONLINE_H
