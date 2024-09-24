//
// Created by matthew on 17/07/2020.
//

#ifndef DATABASE_MANAGER_DIMENSIONLINE_H
#define DATABASE_MANAGER_DIMENSIONLINE_H

#include <QGraphicsLineItem>
#include <QPainter>
#include <QTextItem>
#include <QGraphicsProxyWidget>

/// <summary>
/// DimensionLine inherits QGraphicsItem
/// Draws a Labelled line, used for the width and length lines on the drawing.
/// </summary>
class DimensionLine : public QGraphicsItem {
public:
    /// <summary>
    /// An enum representing whether the dimension line should be vertical or horizontal.
    /// </summary>
    enum Orientation {
        /// <summary>
        /// The dimension line is vertical.
        /// </summary>
        VERTICAL,
        /// <summary>
        /// The dimension line is vertical.
        /// </summary>
        HORIZONTAL
    };

    /// <summary>
    /// Constructs a new dimension line.
    /// </summary>
    /// <param name="dimensionBounds">The area to draw the line.</param>
    /// <param name="orientation">The orientation of the line and text.</param>
    /// <param name="label">The label of the line.</param>
    /// <param name="labelFontSize">The font size of the label.</param>
    DimensionLine(QRectF dimensionBounds, Orientation orientation, const QString &label = QString(), int labelFontSize=10);

    /// <summary>
    /// Paints the dimension line.
    /// </summary>
    /// <param name="painter">Painter that paints the line.</param>
    /// <param name="option">Unused.</param>
    /// <param name="widget">Unused.</param>
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// <summary>
    /// Getter for the bounds of this object.
    /// </summary>
    /// <returns>The bounding area.</returns>
    [[nodiscard]] QRectF boundingRect() const override;

    /// <summary>
    /// Setter for the bounds of this object.
    /// </summary>
    /// <param name="newBounds">The new bounding area.</param>
    void setBounds(QRectF newBounds);

    /// <summary>
    /// Setter for the label.
    /// </summary>
    /// <param name="newLabel">Updated label.</param>
    void setLabel(const QString &newLabel);

private:
    QRectF bounds;
    Orientation orientation;
    QString label;
    int labelFontSize;
};

/// <summary>
/// WidgetDimensionLine inherits QGraphicsItem
/// Similar to DimensionLine, but uses a proxy widget and has no option for text.
/// </summary>
class WidgetDimensionLine : public QGraphicsItem {
public:
    /// <summary>
    /// Constructs a new WidgetDimensionLine.
    /// </summary>
    /// <param name="dimensionBounds">The bounding rect.</param>
    /// <param name="orientation">The orientation of the line.</param>
    /// <param name="proxy">The proxy widget.</param>
    WidgetDimensionLine(QRectF dimensionBounds, DimensionLine::Orientation orientation, QGraphicsProxyWidget *proxy);

    /// <summary>
    /// Paints this object using a provided painter.
    /// </summary>
    /// <param name="painter">Painter used to draw</param>
    /// <param name="option">unused.</param>
    /// <param name="widget">unused.</param>
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// <summary>
    /// Getter for the bounding rect.
    /// </summary>
    /// <returns>The bounding rect.</returns>
    [[nodiscard]] QRectF boundingRect() const override;

    /// <summary>
    /// Sett for the bounding rect.
    /// </summary>
    /// <param name="newBounds">The new bounding rect.</param>
    void setBounds(QRectF newBounds);

private:
    QRectF bounds;
    DimensionLine::Orientation orientation;
    QGraphicsProxyWidget *proxy = nullptr;
};


#endif //DATABASE_MANAGER_DIMENSIONLINE_H
