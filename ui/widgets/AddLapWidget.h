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

class AddLapWidget : public QGraphicsItem {
public:
    explicit AddLapWidget(QGraphicsScene *scene, QRectF hoverBounds, DimensionLine::Orientation orientation, const std::optional<Drawing::Lap> &currentLap = std::nullopt,
            const QColor &unusedColour = Qt::gray, const QColor &unusedHighlightColour = Qt::lightGray, const QColor &usedColour = Qt::green);

    ~AddLapWidget() override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    [[nodiscard]] QRectF boundingRect() const override;

    void setActivationCallback(const std::function<void(bool)> &callback);

    void setLapChangedCallback(const std::function<void(const Drawing::Lap &)> &callback);

    void setBounds(QRectF bounds);

    void setLap(const std::optional<Drawing::Lap> &newLap);

    void setUsed(bool val);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

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
