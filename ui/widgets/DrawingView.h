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

    void enableLaps();

    void disableLaps();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Drawing *drawing = nullptr;

    unsigned numberOfBars;
    std::vector<float> barSpacings;
    std::vector<float> barWidths;

    const double sceneSize = 2000;
    const double maxMatDimensionPercentage = 0.9;
    const double matSectionInset = 0.03;
    const double dimensionLineOffset = 0.05;
    const double barSpacingDimensionHeight = 0.7;
    const double barWidthDimensionHeight = 0.8;
    const double barDimensionHeight = 0.1;
    const double lapHintWidth = 0.02;
    const double defaultLapSize = 0.01;
    const double defaultBarSize = 50;

    std::optional<Drawing::Lap> lapCache[4];

    void redrawScene();

    void updateProxies();

    void lapActivationCallback(bool active, unsigned lapIndex);

    void lapChangedCallback(const Drawing::Lap &lap, unsigned lapIndex);

    QGraphicsRectItem *drawingBorderRect = nullptr;
    AddLapWidget *leftLapHint = nullptr, *rightLapHint = nullptr, *topLapHint = nullptr, *bottomLapHint = nullptr;
    DimensionLine *widthDimension = nullptr, *lengthDimension = nullptr;
    std::vector<QGraphicsProxyWidget *> spacingProxies, barProxies;
    std::vector<WidgetDimensionLine *> spacingDimensions, barDimensions;
    std::vector<QGraphicsRectItem *> matSectionRects;

    bool lapsDisabled = false;
};


#endif //DATABASE_MANAGER_DRAWINGVIEW_H
