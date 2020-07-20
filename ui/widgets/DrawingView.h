//
// Created by matthew on 17/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGVIEW_H
#define DATABASE_MANAGER_DRAWINGVIEW_H

#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QSpinBox>

#include "DimensionLine.h"
#include "AddLapWidget.h"
#include "../../include/database/Drawing.h"

class DrawingView : public QGraphicsView {
    Q_OBJECT

public:
    explicit DrawingView(QWidget *parent = nullptr);

    ~DrawingView() override;

    void setDrawing(Drawing &d);

    void setNumberOfBars(unsigned bars);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Drawing *drawing = nullptr;

    unsigned numberOfBars;
    std::vector<float> barSpacings;
    std::vector<float> barWidths;

    const float sceneSize = 2000;
    const float maxMatDimensionPercentage = 0.9;
    const float matSectionInset = 0.03;
    const float dimensionLineOffset = 0.05;
    const float barSpacingDimensionHeight = 0.4;
    const float barWidthDimensionHeight = 0.2;
    const float barDimensionHeight = 0.1;
    const float lapHintWidth = 0.02;
    const float defaultLapSize = 0.01;

    std::optional<Drawing::Lap> lapCache[4];

    void redrawScene();

    QGraphicsRectItem *drawingBorderRect = nullptr;
    AddLapWidget *leftLapHint = nullptr, *rightLapHint = nullptr, *topLapHint = nullptr, *bottomLapHint = nullptr;
    DimensionLine *widthDimension = nullptr, *lengthDimension = nullptr;
    std::vector<QGraphicsProxyWidget *> spacingProxies, barProxies;
    std::vector<WidgetDimensionLine *> spacingDimensions, barDimensions;
    std::vector<QGraphicsRectItem *> matSectionRects;
};


#endif //DATABASE_MANAGER_DRAWINGVIEW_H
