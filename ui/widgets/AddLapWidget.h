//
// Created by matthew on 18/07/2020.
//

#ifndef DATABASE_MANAGER_ADDLAPWIDGET_H
#define DATABASE_MANAGER_ADDLAPWIDGET_H

#include <QGraphicsLineItem>
#include <QPainter>
#include <QTextItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QGraphicsScene>
#include <QDoubleSpinBox>
#include <QWidgetAction>
#include <QComboBox>

#include "DimensionLine.h"
#include "DynamicComboBox.h"
#include "../../include/database/Drawing.h"

/// <summary>
/// A widget that displays a lap on the drawing.
/// </summary>
class AddLapWidget : public QGraphicsItem {
public:
    /// <summary>
    /// Displays a lap onto the drawing.
    /// </summary>
    /// <param name="scene">The scene to drawn upon.</param>
    /// <param name="hoverBounds">The area the lap occupies.</param>
    /// <param name="orientation">Sets the orientation of the lap.</param>
    /// <param name="currentLap">The lap this widget is based upon, or the default lap.</param>
    /// <param name="unusedColour">The colour to paint when unused.</param>
    /// <param name="unusedHighlightColour">The colour to paint when unused and hovered.</param>
    /// <param name="usedColour">The colour to use while in use.</param>
    explicit AddLapWidget(QGraphicsScene *scene, QRectF hoverBounds, DimensionLine::Orientation orientation, const std::optional<Drawing::Lap> &currentLap = std::nullopt,
            const QColor &unusedColour = Qt::gray, const QColor &unusedHighlightColour = Qt::lightGray, const QColor &usedColour = Qt::green);

    /// <summary>
    /// Destroys widget and relevant pointers.
    /// </summary>
    ~AddLapWidget() override;

    /// <summary>
    /// paints this widget.
    /// </summary>
    /// <param name="painter">The painter to use to draw.</param>
    /// <param name="option">Unused</param>
    /// <param name="widget">Unused</param>
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// <summary>
    /// Getter for the area this lap occupies.
    /// </summary>
    /// <returns>The laps area.</returns>
    [[nodiscard]] QRectF boundingRect() const override;

    /// <summary>
    /// Sets a callback action for activation behaviour.
    /// </summary>
    /// <param name="callback">The callback to set, taking a bool of whether it has been activated or deactivated.</param>
    void setActivationCallback(const std::function<void(bool)> &callback);

    /// <summary>
    /// Sets the callback for a lap being changed.
    /// </summary>
    /// <param name="callback">A callback function, taking the new lap.</param>
    void setLapChangedCallback(const std::function<void(const Drawing::Lap &)> &callback);

    /// <summary>
    /// Sets the bounding rect for the lap.
    /// </summary>
    /// <param name="bounds">The new bounding rect</param>
    void setBounds(QRectF bounds);

    /// <summary>
    /// Sets the lap to a new lap, or clears the lap and resets it to the default lap.
    /// </summary>
    /// <param name="newLap">The lap to replace, or std::nullopt</param>
    void setLap(const std::optional<Drawing::Lap> &newLap);

    /// <summary>
    /// Sets whether or not the lap is in use.
    /// </summary>
    /// <param name="val">The new activation of the lap widget.</param>
    void setUsed(bool val);

protected:
    /// <summary>
    /// Overloads hover enter event for painting. Propogates event.
    /// </summary>
    /// <param name="event">The causing event.</param>
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

    /// <summary>
    /// Overloads hover leave event for painting. Propogates event.
    /// </summary>
    /// <param name="event">The causing event.</param>
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    /// <summary>
    /// Overloads press event for painting and lap activation. Propogates event.
    /// </summary>
    /// <param name="event">The causing event.</param>
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /// <summary>
    /// Overloads context menu event for custom options. Propogates event.
    /// </summary>
    /// <param name="event">The causing event.</param>
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    QRectF hoverBounds;
    QColor unusedColour, unusedHighlightColour, usedColour;
    bool hovering, used;
    DimensionLine::Orientation orientation;

    Drawing::Lap lap;

    std::function<void(bool)> activateCallback = nullptr;
    std::function<void(const Drawing::Lap &)> lapChangedCallback = nullptr;

    QDoubleSpinBox *widthInput = nullptr;
    QGraphicsProxyWidget *widthProxy = nullptr;
    WidgetDimensionLine *widthDimension = nullptr;

    const float horizontalEditPosition = 0.3;
    const float verticalEditPosition = 0.3;
    const float editSize = 0.03;

    ComboboxComponentDataSource<Material> materialSource;
};


#endif //DATABASE_MANAGER_ADDLAPWIDGET_H
