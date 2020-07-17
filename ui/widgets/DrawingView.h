//
// Created by matthew on 17/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGVIEW_H
#define DATABASE_MANAGER_DRAWINGVIEW_H

#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QSpinBox>

#include "DimensionLine.h"
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

    const float sceneWidth = 2000, sceneHeight = 2000;
    const float maxMatDimensionPercentage = 0.9;
    const float matSectionInsetW = 0.03, matSectionInsetH = 0.08;
    const float dimensionLineOffset = 0.03;
    const float barSpacingDimensionHeight = 0.4;
    const float barWidthDimensionHeight = 0.2;
    const float barDimensionHeight = 0.1;

    void redrawScene();
};


#endif //DATABASE_MANAGER_DRAWINGVIEW_H
