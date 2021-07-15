//
// Created by matthew on 17/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGVIEW_H
#define DATABASE_MANAGER_DRAWINGVIEW_H

#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QSpinBox>
#include <QApplication>

#include "Inspector.h"
#include "DimensionLine.h"
#include "AddLapWidget.h"
#include "addons/ImpactPadGraphicsItem.h"
#include "addons/BlankSpaceGraphicsItem.h"
#include "addons/DamBarGraphicsItem.h"
#include "addons/CentreHoleSetGraphicsItem.h"
#include "addons/DivertorSetGraphicsItem.h"
#include "addons/DeflectorSetGraphicsItem.h"
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

    void setRedrawRequired();

    void setInspector(Inspector *inspector);

protected:
    void paintEvent(QPaintEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Drawing *drawing = nullptr;
    Inspector *inspector = nullptr;

    unsigned numberOfBars;
    std::vector<float> barSpacings;
    std::vector<float> barWidths;

    // const double sceneSize = 2000;
    const double maxMatDimensionPercentage = 0.8;
    const double defaultHorizontalBarSize = 45;
    const double dimensionLineOffset = 0.03;
    const double barSpacingDimensionHeight = 0.7;
    const double barWidthDimensionHeight = 0.8;
    const double barDimensionHeight = 0.1;
    const double lapHintWidth = 0.012;
    const double defaultLapSize = 40;
    const double defaultBarSize = 50;

    std::optional<Drawing::Lap> lapCache[4];

    void redrawScene();

    void updateProxies();

    void lapActivationCallback(bool active, unsigned lapIndex);

    void lapChangedCallback(const Drawing::Lap &lap, unsigned lapIndex);

    void updateSnapLines();

    void updateCentreSnapLines();

    QPoint snapPoint(const QPoint &point) const;

    QPointF snapPointF(const QPointF &point) const;

    QPointF snapPointToCentreF(const QPointF &point) const;

    QGraphicsRectItem *drawingBorderRect = nullptr;
    AddLapWidget *leftLapHint = nullptr, *rightLapHint = nullptr, *topLapHint = nullptr, *bottomLapHint = nullptr;
    DimensionLine *widthDimension = nullptr, *lengthDimension = nullptr;
    QGraphicsTextItem *widthSumTextItem = nullptr;
    std::vector<QGraphicsProxyWidget *> spacingProxies, barProxies;
    std::vector<WidgetDimensionLine *> spacingDimensions, barDimensions;
    std::vector<QGraphicsRectItem *> matSectionRects;
    std::vector<ImpactPadGraphicsItem *> impactPadRegions;
    std::vector<BlankSpaceGraphicsItem *> blankSpaceReigons;
    std::vector<DamBarGraphicsItem*> damBarReigons;
    CentreHoleSetGraphicsItem *centreHoleSet = nullptr;
    DeflectorSetGraphicsItem *deflectorSet = nullptr;
    DivertorSetGraphicsItem *divertorSet = nullptr;

    std::vector<double> hSnapLines, vSnapLines, centreVSnapLines;

    enum class AddPartState {
        NONE,
        ADD_IMPACT_PAD,
        ADD_BLANK_SPACE,
        ADD_DAM_BAR,
        ADD_CENTRE_HOLES,
        ADD_DEFLECTORS,
        ADD_DIVERTORS
    };

    QRubberBand *impactPadRegionSelector = nullptr;
    QRubberBand *blankSpaceReigonSelector = nullptr;
    QRubberBand* damBarReigonSelector = nullptr;
    QPoint impactPadAnchorPoint;
    QPoint blankSpaceAnchorPoint;
    QPoint damBarAnchorPoint;
    AddPartState addPartState = AddPartState::NONE;

    bool lapsDisabled = false;

    bool refreshRequired = true;
};


#endif //DATABASE_MANAGER_DRAWINGVIEW_H
