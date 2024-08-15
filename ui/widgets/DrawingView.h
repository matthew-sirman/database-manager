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
#include "addons/AreaGraphicsItem.h"
#include "addons/GroupGraphicsItem.h"
#include "../../include/database/Drawing.h"

/// <summary>
/// DrawingView inherits QGraphicsView
/// DrawingView displays and allows editing a drawing
/// </summary>
class DrawingView : public QGraphicsView {
    Q_OBJECT

public:
    /// <summary>
    /// Constructs a new DrawingView.
    /// </summary>
    /// <param name="parent">This widgets parent.</param>
    explicit DrawingView(QWidget *parent = nullptr);

    /// <summary>
    /// Default destructor with cleanup.
    /// </summary>
    ~DrawingView() override;

    /// <summary>
    /// Specifies drawing to render.
    /// </summary>
    /// <param name="d">The drawing to render.</param>
    void setDrawing(Drawing &d);

    /// <summary>
    /// Sets the number of bars.
    /// </summary>
    /// <param name="bars">The new number of bars.</param>
    void setNumberOfBars(unsigned bars);

    /// <summary>
    /// Enables the laps on the drawing.
    /// </summary>
    void enableLaps();

    /// <summary>
    /// Disables the laps on the drawing.
    /// </summary>
    void disableLaps();

    /// <summary>
    /// Makes the drawing get redrawn ASAP.
    /// </summary>
    void setRedrawRequired();

    /// <summary>
    /// Sets the insepctor for more details.
    /// </summary>
    /// <param name="inspector">The inespector to set.</param>
    void setInspector(Inspector *inspector);

protected:
    /// <summary>
    /// Overridden paint function to redraw scene if required. Propogates event.
    /// </summary>
    /// <param name="event">The triggering event</param>
    void paintEvent(QPaintEvent *event) override;

    /// <summary>
    /// overriden resize event to force a redraw. Propogates event.
    /// </summary>
    /// <param name="event">The triggering event</param>
    void resizeEvent(QResizeEvent *event) override;

    /// <summary>
    /// Overriden context menu event for custom context menu. Propogates event.
    /// </summary>
    /// <param name="event">The triggering event</param>
    void contextMenuEvent(QContextMenuEvent *event) override;

    /// <summary>
    /// Overriden to allow for additions to drawings. Propogates event.
    /// </summary>
    /// <param name="event">The triggering event</param>
    void mousePressEvent(QMouseEvent *event) override;

    /// <summary>
    /// Overriden mouse move event to snap drawing zones on drawing. Propogates event.
    /// </summary>
    /// <param name="event">The triggering event</param>
    void mouseMoveEvent(QMouseEvent *event) override;

    /// <summary>
    /// Overriden mouse release event. Propogates event.
    /// </summary>
    /// <param name="event">The triggering event</param>
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
    typedef AreaGraphicsItem<Drawing::ImpactPad, Material, Aperture> ImpactPadGraphicsItem;
    typedef AreaGraphicsItem<Drawing::BlankSpace> BlankSpaceGraphicsItem;
    typedef AreaGraphicsItem<Drawing::ExtraAperture, Aperture> ExtraApertureGraphicsItem;
    typedef AreaGraphicsItem<Drawing::DamBar, Material> DamBarGraphicsItem;
    std::vector<ImpactPadGraphicsItem*> impactPadRegions;
    std::vector<BlankSpaceGraphicsItem*> blankSpaceRegions;
    std::vector<ExtraApertureGraphicsItem*> extraApertureRegions;
    std::vector<DamBarGraphicsItem*> damBarRegions;
    typedef GroupGraphicsItem<Drawing::CentreHole, Aperture> CentreHoleSetGraphicsItem;
    typedef GroupGraphicsItem<Drawing::Deflector, Material> DeflectorSetGraphicsItem;
    typedef GroupGraphicsItem<Drawing::Divertor, Material> DivertorSetGraphicsItem;
    CentreHoleSetGraphicsItem* centreHoleSet = nullptr;
    DeflectorSetGraphicsItem *deflectorSet = nullptr;
    DivertorSetGraphicsItem *divertorSet = nullptr;

    std::vector<double> hSnapLines, vSnapLines, centreVSnapLines;

    enum class AddPartState {
        NONE,
        ADD_IMPACT_PAD,
        ADD_BLANK_SPACE,
        ADD_EXTRA_APERTURE,
        ADD_DAM_BAR,
        ADD_CENTRE_HOLES,
        ADD_DEFLECTORS,
        ADD_DIVERTORS
    };

    QRubberBand *impactPadRegionSelector = nullptr;
    QRubberBand* blankSpaceRegionSelector = nullptr;
    QRubberBand* extraApertureRegionSelector = nullptr;
    QRubberBand* damBarRegionSelector = nullptr;
    QPoint impactPadAnchorPoint;
    QPoint blankSpaceAnchorPoint;
    QPoint extraApertureAnchorPoint;
    QPoint damBarAnchorPoint;
    AddPartState addPartState = AddPartState::NONE;

    bool lapsDisabled = false;

    bool refreshRequired = true;
};


#endif //DATABASE_MANAGER_DRAWINGVIEW_H
