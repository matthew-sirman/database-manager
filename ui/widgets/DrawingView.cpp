//
// Created by matthew on 17/07/2020.
//

#include "DrawingView.h"

DrawingView::DrawingView(QWidget *parent) : QGraphicsView(parent) {
    numberOfBars = 0;
    barWidths = {0, 0};
    barSpacings = {0};

    for (std::optional<Drawing::Lap> &lap : lapCache) {
        lap = std::nullopt;
    }
}

DrawingView::~DrawingView() {
    delete leftLapHint, rightLapHint, topLapHint, bottomLapHint;
    delete widthDimension, lengthDimension;
    delete widthSumTextItem;
    for (QGraphicsProxyWidget *spacingProxy : spacingProxies) {
        delete spacingProxy;
    }
    for (QGraphicsProxyWidget *barProxy : barProxies) {
        delete barProxy;
    }
    for (WidgetDimensionLine *spacingDim : spacingDimensions) {
        delete spacingDim;
    }
    for (WidgetDimensionLine *barDim : barDimensions) {
        delete barDim;
    }
    for (QGraphicsRectItem *region : matSectionRects) {
        delete region;
    }
    for (ImpactPadGraphicsItem *impactPad : impactPadRegions) {
        delete impactPad;
    }
    for (BlankSpaceGraphicsItem* blankSpace : blankSpaceRegions) {
        delete blankSpace;
    }
    for (ExtraApertureGraphicsItem* extraAperture : extraApertureRegions) {
        delete extraAperture;
    }
    for (DamBarGraphicsItem* damBar : damBarRegions) {
        delete damBar;
    }
    delete centreHoleSet;
    delete deflectorSet;
    delete divertorSet;
}

void DrawingView::setDrawing(Drawing &d) {
    this->drawing = &d;
    barSpacings = d.allBarSpacings();
    barWidths = d.allBarWidths();
    numberOfBars = d.numberOfBars();

    switch (drawing->tensionType()) {
        case Drawing::SIDE:
            lapCache[0] = drawing->sidelap(Drawing::LEFT);
            lapCache[1] = drawing->sidelap(Drawing::RIGHT);
            lapCache[2] = drawing->overlap(Drawing::LEFT);
            lapCache[3] = drawing->overlap(Drawing::RIGHT);
            break;
        case Drawing::END:
            lapCache[2] = drawing->sidelap(Drawing::LEFT);
            lapCache[3] = drawing->sidelap(Drawing::RIGHT);
            lapCache[0] = drawing->overlap(Drawing::LEFT);
            lapCache[1] = drawing->overlap(Drawing::RIGHT);
            break;
    }

    this->drawing->addUpdateCallback([this]() { setRedrawRequired(); });
}

void DrawingView::setNumberOfBars(unsigned bars) {
    if (bars > numberOfBars) {
        for (; numberOfBars < bars; numberOfBars++) {
            barSpacings.push_back(0);
            barWidths.insert(barWidths.end() - 1, 0);
        }
    }
    if (bars < numberOfBars) {
        float last = barWidths.back();
        for (; numberOfBars > bars; numberOfBars--) {
            barSpacings.pop_back();
            barWidths.pop_back();
        }
        barWidths.back() = last;
    }

    numberOfBars = bars;

    setRedrawRequired();
}

void DrawingView::enableLaps() {
    if (!lapsDisabled) {
        return;
    }

    switch (drawing->tensionType()) {
        case Drawing::SIDE:
            if (lapCache[0].has_value()) {
                drawing->setSidelap(Drawing::LEFT, lapCache[0].value());
            }
            if (lapCache[1].has_value()) {
                drawing->setSidelap(Drawing::RIGHT, lapCache[1].value());
            }
            if (lapCache[2].has_value()) {
                drawing->setOverlap(Drawing::LEFT, lapCache[2].value());
            }
            if (lapCache[3].has_value()) {
                drawing->setOverlap(Drawing::RIGHT, lapCache[3].value());
            }
            break;
        case Drawing::END:
            if (lapCache[2].has_value()) {
                drawing->setSidelap(Drawing::LEFT, lapCache[2].value());
            }
            if (lapCache[3].has_value()) {
                drawing->setSidelap(Drawing::RIGHT, lapCache[3].value());
            }
            if (lapCache[0].has_value()) {
                drawing->setOverlap(Drawing::LEFT, lapCache[0].value());
            }
            if (lapCache[1].has_value()) {
                drawing->setOverlap(Drawing::RIGHT, lapCache[1].value());
            }
            break;
    }

    lapsDisabled = false;

    setRedrawRequired();
}

void DrawingView::disableLaps() {
    if (lapsDisabled) {
        return;
    }

    switch (drawing->tensionType()) {
        case Drawing::SIDE:
            lapCache[0] = drawing->sidelap(Drawing::LEFT);
            lapCache[1] = drawing->sidelap(Drawing::RIGHT);
            lapCache[2] = drawing->overlap(Drawing::LEFT);
            lapCache[3] = drawing->overlap(Drawing::RIGHT);
            break;
        case Drawing::END:
            lapCache[2] = drawing->sidelap(Drawing::LEFT);
            lapCache[3] = drawing->sidelap(Drawing::RIGHT);
            lapCache[0] = drawing->overlap(Drawing::LEFT);
            lapCache[1] = drawing->overlap(Drawing::RIGHT);
            break;
    }

    drawing->removeOverlap(Drawing::LEFT);
    drawing->removeOverlap(Drawing::RIGHT);
    drawing->removeSidelap(Drawing::LEFT);
    drawing->removeSidelap(Drawing::RIGHT);

    lapsDisabled = true;

    setRedrawRequired();
}

void DrawingView::setRedrawRequired() {
    refreshRequired = true;
}

void DrawingView::setInspector(Inspector *inspector) {
    this->inspector = inspector;
}

void DrawingView::paintEvent(QPaintEvent *event) {
    QGraphicsScene *graphicsScene = scene();

    if (graphicsScene) {
        if (refreshRequired) {
            redrawScene();
            refreshRequired = false;
        }

        graphicsScene->setSceneRect(0, 0, viewport()->width(), viewport()->height());
        fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
    }

    update();

    QGraphicsView::paintEvent(event);
}

void DrawingView::resizeEvent(QResizeEvent *event) {
    setRedrawRequired();
    QGraphicsView::resizeEvent(event);
}

void DrawingView::contextMenuEvent(QContextMenuEvent *event) {
    if (addPartState != AddPartState::NONE) {
        addPartState = AddPartState::NONE;
        QApplication::restoreOverrideCursor();
    } else {
        if (drawingBorderRect) {
            if (drawingBorderRect->contains(event->pos())) {
                for (ImpactPadGraphicsItem *impactPad : impactPadRegions) {
                    if (impactPad->contains(event->pos())) {
                        goto base_event_handler;
                    }
                }
                for (BlankSpaceGraphicsItem* blankSpace : blankSpaceRegions) {
                    if (blankSpace->contains(event->pos())) {
                        goto base_event_handler;
                    }
                }
                for (ExtraApertureGraphicsItem* extraAperture : extraApertureRegions) {
                    if (extraAperture->contains(event->pos())) {
                        goto base_event_handler;
                    }
                }
                for (DamBarGraphicsItem* damBar : damBarRegions) {
                    if (damBar->contains(event->pos())) {
                        goto base_event_handler;
                    }
                }
                if (centreHoleSet) {
                    if (centreHoleSet->contains(event->pos())) {
                        goto base_event_handler;
                    }
                }
                if (deflectorSet) {
                    if (deflectorSet->contains(event->pos())) {
                        goto base_event_handler;
                    }
                }
                if (divertorSet) {
                    if (divertorSet->contains(event->pos())) {
                        goto base_event_handler;
                    }
                }

                QMenu *menu = new QMenu();

                menu->addAction("Add Impact Pad", [this]() {
                    updateSnapLines();
                    impactPadRegionSelector = new QRubberBand(QRubberBand::Rectangle, this);
                    QApplication::setOverrideCursor(Qt::CrossCursor);
                    addPartState = AddPartState::ADD_IMPACT_PAD;
                });
                menu->addAction("Add Blank Space", [this]() {
                    updateSnapLines();
                    blankSpaceRegionSelector = new QRubberBand(QRubberBand::Rectangle, this);
                    QApplication::setOverrideCursor(Qt::CrossCursor);
                    addPartState = AddPartState::ADD_BLANK_SPACE;
                });
                menu->addAction("Add Extra Aperture", [this]() {
                    updateSnapLines();
                    extraApertureRegionSelector = new QRubberBand(QRubberBand::Rectangle, this);
                    QApplication::setOverrideCursor(Qt::CrossCursor);
                    addPartState = AddPartState::ADD_EXTRA_APERTURE;
});
                menu->addAction("Add Dam Bar", [this]() {
                    updateSnapLines();
                    damBarRegionSelector = new QRubberBand(QRubberBand::Rectangle, this);
                    QApplication::setOverrideCursor(Qt::CrossCursor);
                    addPartState = AddPartState::ADD_DAM_BAR;
                    });
                menu->addAction("Add Centre Holes", [this]() {
                    updateCentreSnapLines();
                    QApplication::setOverrideCursor(Qt::CrossCursor);
                    addPartState = AddPartState::ADD_CENTRE_HOLES;
                });
                menu->addAction("Add Deflectors", [this]() {
                    updateCentreSnapLines();
                    QApplication::setOverrideCursor(Qt::CrossCursor);
                    addPartState = AddPartState::ADD_DEFLECTORS;
                });
                menu->addAction("Add Divertors", [this]() {
                    QApplication::setOverrideCursor(Qt::CrossCursor);
                    addPartState = AddPartState::ADD_DIVERTORS;
                });

                menu->popup(event->globalPos());

                QWidget::connect(menu, &QMenu::triggered, [menu](QAction *) { menu->deleteLater(); });
            }
        }
    }

    base_event_handler:
    QGraphicsView::contextMenuEvent(event);
}

void DrawingView::mousePressEvent(QMouseEvent *event) {
    if (drawingBorderRect && drawingBorderRect->rect().contains(event->pos()) && event->button() == Qt::LeftButton) {
            switch (addPartState) {
            case AddPartState::ADD_IMPACT_PAD:
                if (impactPadRegionSelector) {
                    impactPadRegionSelector->setGeometry(QRect(snapPoint(event->pos()), QSize()));
                    impactPadAnchorPoint = snapPoint(event->pos());
                    impactPadRegionSelector->show();
                }
                break;
            case AddPartState::ADD_BLANK_SPACE:
                if (blankSpaceRegionSelector) {
                    blankSpaceRegionSelector->setGeometry(QRect(snapPoint(event->pos()), QSize()));
                    blankSpaceAnchorPoint = snapPoint(event->pos());
                    blankSpaceRegionSelector->show();
                }
                break;
            case AddPartState::ADD_EXTRA_APERTURE:
                if (extraApertureRegionSelector) {
                    extraApertureRegionSelector->setGeometry(QRect(snapPoint(event->pos()), QSize()));
                    extraApertureAnchorPoint = snapPoint(event->pos());
                    extraApertureRegionSelector->show();
                }
                break;
            case AddPartState::ADD_DAM_BAR:
                if (damBarRegionSelector) {
                    damBarRegionSelector->setGeometry(QRect(snapPoint(event->pos()), QSize()));
                    damBarAnchorPoint = snapPoint(event->pos());
                    damBarRegionSelector->show();
                }
                break;
            case AddPartState::ADD_CENTRE_HOLES: {
                Drawing::CentreHole hole;

                QPointF holeCentre = snapPointToCentreF(event->pos());
                hole.pos.x = ((holeCentre.x() - drawingBorderRect->rect().left()) / drawingBorderRect->rect().width()) *
                    drawing->width();
                hole.pos.y = ((holeCentre.y() - drawingBorderRect->rect().top()) / drawingBorderRect->rect().height()) *
                    drawing->length();
                if (centreHoleSet->empty()) {
                    // Most popular aperture for centre holes (20RL30)
                    hole.setAperture(DrawingComponentManager<Aperture>::findComponentByID(632));
                }
                centreHoleSet->addItem(std::move(hole));

                setRedrawRequired();

                break;
            }
            case AddPartState::ADD_DEFLECTORS: {
                Drawing::Deflector deflector;

                QPointF deflectorCentre = snapPointToCentreF(event->pos());
                deflector.pos.x =
                    ((deflectorCentre.x() - drawingBorderRect->rect().left()) / drawingBorderRect->rect().width()) *
                    drawing->width();
                deflector.pos.y =
                    ((deflectorCentre.y() - drawingBorderRect->rect().top()) / drawingBorderRect->rect().height()) *
                    drawing->length();
                if (deflectorSet->empty()) {
                    deflector.setMaterial(DrawingComponentManager<Material>::findComponentByID(29));
                }

                deflectorSet->addItem(std::move(deflector));

                setRedrawRequired();

                break;
            }
            case AddPartState::ADD_DIVERTORS: {
                Drawing::Divertor divertor;

                divertor.verticalPosition =
                    ((event->pos().y() - drawingBorderRect->rect().top()) / drawingBorderRect->rect().height()) *
                    drawing->length();
                divertor.side =
                    event->pos().x() < drawingBorderRect->rect().center().x() ? Drawing::LEFT : Drawing::RIGHT;
                if (divertorSet->empty()) {
                    divertor.setMaterial(DrawingComponentManager<Material>::findComponentByID(29));
                }

                divertorSet->addItem(std::move(divertor));

                setRedrawRequired();

                break;
            }
            default:
                break;
            }
        }
        QGraphicsView::mousePressEvent(event);
}

void DrawingView::mouseMoveEvent(QMouseEvent *event) {
    if (impactPadRegionSelector && addPartState == AddPartState::ADD_IMPACT_PAD &&
        drawingBorderRect->rect().contains(event->pos())) {
        QPoint startPoint = impactPadAnchorPoint;
        QPoint endPoint = snapPoint(event->pos());

        QPoint topLeft(std::min(startPoint.x(), endPoint.x()), std::min(startPoint.y(), endPoint.y())),
                bottomRight(std::max(startPoint.x(), endPoint.x()), std::max(startPoint.y(), endPoint.y()));

        impactPadRegionSelector->setGeometry(QRect(topLeft, bottomRight));
    }
    if (blankSpaceRegionSelector && addPartState == AddPartState::ADD_BLANK_SPACE &&
        drawingBorderRect->rect().contains(event->pos())) {
        QPoint startPoint = blankSpaceAnchorPoint;
        QPoint endPoint = snapPoint(event->pos());

        QPoint topLeft(std::min(startPoint.x(), endPoint.x()), std::min(startPoint.y(), endPoint.y())),
            bottomRight(std::max(startPoint.x(), endPoint.x()), std::max(startPoint.y(), endPoint.y()));

        blankSpaceRegionSelector->setGeometry(QRect(topLeft, bottomRight));
    }
    if (extraApertureRegionSelector && addPartState == AddPartState::ADD_EXTRA_APERTURE &&
        drawingBorderRect->rect().contains(event->pos())) {
        QPoint startPoint = extraApertureAnchorPoint;
        QPoint endPoint = snapPoint(event->pos());

        QPoint topLeft(std::min(startPoint.x(), endPoint.x()), std::min(startPoint.y(), endPoint.y())),
            bottomRight(std::max(startPoint.x(), endPoint.x()), std::max(startPoint.y(), endPoint.y()));

        extraApertureRegionSelector->setGeometry(QRect(topLeft, bottomRight));
    }
    if (damBarRegionSelector && addPartState == AddPartState::ADD_DAM_BAR &&
        drawingBorderRect->rect().contains(event->pos())) {
        QPoint startPoint = damBarAnchorPoint;
        QPoint endPoint = snapPoint(event->pos());

        QPoint topLeft(std::min(startPoint.x(), endPoint.x()), std::min(startPoint.y(), endPoint.y())),
            bottomRight(std::max(startPoint.x(), endPoint.x()), std::max(startPoint.y(), endPoint.y()));

        damBarRegionSelector->setGeometry(QRect(topLeft, bottomRight));
    }
    QGraphicsView::mouseMoveEvent(event);
}

void DrawingView::mouseReleaseEvent(QMouseEvent *event) {
    if (impactPadRegionSelector && addPartState == AddPartState::ADD_IMPACT_PAD) {
        if (event->button() == Qt::LeftButton) {
            QRectF impactPadRect = QRectF(impactPadRegionSelector->pos(), impactPadRegionSelector->size());
            impactPadRect.setTopLeft(snapPointF(impactPadRect.topLeft()));
            impactPadRect.setBottomRight(snapPointF(impactPadRect.bottomRight()));

            Drawing::ImpactPad pad;

            pad.pos.x =
                    ((impactPadRect.left() - drawingBorderRect->rect().left()) / drawingBorderRect->rect().width()) *
                    drawing->width();
            pad.pos.y = ((impactPadRect.top() - drawingBorderRect->rect().top()) / drawingBorderRect->rect().height()) *
                        drawing->length();
            pad.width = (impactPadRect.width() / drawingBorderRect->rect().width()) * drawing->width();
            pad.length = (impactPadRect.height() / drawingBorderRect->rect().height()) * drawing->length();

            pad.setMaterial(DrawingComponentManager<Material>::findComponentByID(29));
            pad.setAperture(DrawingComponentManager<Aperture>::findComponentByID(1));

            drawing->addImpactPad(pad);
            addPartState = AddPartState::NONE;
            QApplication::restoreOverrideCursor();
        }
        delete impactPadRegionSelector;
        impactPadRegionSelector = nullptr;
    }
    if (blankSpaceRegionSelector && addPartState == AddPartState::ADD_BLANK_SPACE) {
        if (event->button() == Qt::LeftButton) {
            QRectF blankSpaceRect = QRectF(blankSpaceRegionSelector->pos(), blankSpaceRegionSelector->size());
            blankSpaceRect.setTopLeft(snapPointF(blankSpaceRect.topLeft()));
            blankSpaceRect.setBottomRight(snapPointF(blankSpaceRect.bottomRight()));

            Drawing::BlankSpace space;

            space.pos.x =
                ((blankSpaceRect.left() - drawingBorderRect->rect().left()) / drawingBorderRect->rect().width()) *
                drawing->width();
            space.pos.y = ((blankSpaceRect.top() - drawingBorderRect->rect().top()) / drawingBorderRect->rect().height()) *
                drawing->length();
            space.width = (blankSpaceRect.width() / drawingBorderRect->rect().width()) * drawing->width();
            space.length = (blankSpaceRect.height() / drawingBorderRect->rect().height()) * drawing->length();


            drawing->addBlankSpace(space);
            addPartState = AddPartState::NONE;
            QApplication::restoreOverrideCursor();
        }
        delete blankSpaceRegionSelector;
        blankSpaceRegionSelector = nullptr;
    }
    if (extraApertureRegionSelector && addPartState == AddPartState::ADD_EXTRA_APERTURE) {
        if (event->button() == Qt::LeftButton) {
            QRectF extraApertureRect = QRectF(extraApertureRegionSelector->pos(), extraApertureRegionSelector->size());
            extraApertureRect.setTopLeft(snapPointF(extraApertureRect.topLeft()));
            extraApertureRect.setBottomRight(snapPointF(extraApertureRect.bottomRight()));

            Drawing::ExtraAperture aperture;

            aperture.pos.x =
                ((extraApertureRect.left() - drawingBorderRect->rect().left()) / drawingBorderRect->rect().width()) *
                drawing->width();
            aperture.pos.y = ((extraApertureRect.top() - drawingBorderRect->rect().top()) / drawingBorderRect->rect().height()) *
                drawing->length();
            aperture.width = (extraApertureRect.width() / drawingBorderRect->rect().width()) * drawing->width();
            aperture.length = (extraApertureRect.height() / drawingBorderRect->rect().height()) * drawing->length();


            drawing->addExtraAperture(aperture);
            addPartState = AddPartState::NONE;
            QApplication::restoreOverrideCursor();
        }
        delete extraApertureRegionSelector;
        extraApertureRegionSelector = nullptr;
    }
    if (damBarRegionSelector && addPartState == AddPartState::ADD_DAM_BAR) {
        if (event->button() == Qt::LeftButton) {
            QRectF damBarRect = QRectF(damBarRegionSelector->pos(), damBarRegionSelector->size());
            damBarRect.setTopLeft(snapPointF(damBarRect.topLeft()));
            damBarRect.setBottomRight(snapPointF(damBarRect.bottomRight()));

            Drawing::DamBar bar;

            bar.pos.x =
                ((damBarRect.left() - drawingBorderRect->rect().left()) / drawingBorderRect->rect().width()) *
                drawing->width();
            bar.pos.y = ((damBarRect.top() - drawingBorderRect->rect().top()) / drawingBorderRect->rect().height()) *
                drawing->length();
            bar.width = (damBarRect.width() / drawingBorderRect->rect().width()) * drawing->width();
            bar.length = (damBarRect.height() / drawingBorderRect->rect().height()) * drawing->length();
            bar.setMaterial(DrawingComponentManager<Material>::findComponentByID(29));

            drawing->addDamBar(bar);
            addPartState = AddPartState::NONE;
            QApplication::restoreOverrideCursor();
        }
        delete damBarRegionSelector;
        damBarRegionSelector = nullptr;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void DrawingView::redrawScene() {
    QGraphicsScene *graphicsScene = scene();

    if (graphicsScene && drawing) {
        double width = drawing->width(), length = drawing->length();

        if (width != 0 && length != 0) {
            std::optional<Drawing::Lap> leftLap, rightLap, topLap, bottomLap;

            double sceneWidth = viewport()->width(), sceneHeight = viewport()->height();
            double sceneAspect = sceneWidth / sceneHeight;

            switch (drawing->tensionType()) {
                case Drawing::SIDE:
                    leftLap = drawing->sidelap(Drawing::LEFT);
                    rightLap = drawing->sidelap(Drawing::RIGHT);
                    topLap = drawing->overlap(Drawing::LEFT);
                    bottomLap = drawing->overlap(Drawing::RIGHT);
                    break;
                case Drawing::END:
                    topLap = drawing->sidelap(Drawing::LEFT);
                    bottomLap = drawing->sidelap(Drawing::RIGHT);
                    leftLap = drawing->overlap(Drawing::LEFT);
                    rightLap = drawing->overlap(Drawing::RIGHT);
                    break;
            }

            struct {
                double rLeft = 0;
                double rCentre = 0;
                double rRight = 0;

                inline double total() const {
                    return rLeft + rCentre + rRight;
                }
            } widthDim, lengthDim;

            widthDim.rCentre = width;
            lengthDim.rCentre = length;

            if (leftLap.has_value()) {
                widthDim.rLeft = leftLap->width == 0 ? defaultLapSize : leftLap->width;
            }
            if (rightLap.has_value()) {
                widthDim.rRight = rightLap->width == 0 ? defaultLapSize : rightLap->width;
            }
            if (topLap.has_value()) {
                lengthDim.rLeft = topLap->width == 0 ? defaultLapSize : topLap->width;
            }
            if (bottomLap.has_value()) {
                lengthDim.rRight = bottomLap->width == 0 ? defaultLapSize : bottomLap->width;
            }

            double pWidth, pLength;

            if (widthDim.total() / sceneWidth > lengthDim.total() / sceneHeight) {
                pWidth = maxMatDimensionPercentage;
                pLength = maxMatDimensionPercentage * (lengthDim.total() / widthDim.total()) * sceneAspect;
            } else {
                // sceneAspect = 1.0 / sceneAspect;
                pLength = maxMatDimensionPercentage;
                pWidth = maxMatDimensionPercentage * (widthDim.total() / lengthDim.total()) / sceneAspect;
            }

            QRectF matBoundingRegion;
            matBoundingRegion.setTopLeft(QPointF(
                    (0.5 - pWidth * (0.5 - widthDim.rLeft / widthDim.total())) * sceneWidth,
                    (0.5 - pLength * (0.5 - lengthDim.rLeft / lengthDim.total())) * sceneHeight
            ));
            matBoundingRegion.setBottomRight(QPointF(
                    (0.5 + pWidth * (0.5 - widthDim.rRight / widthDim.total())) * sceneWidth,
                    (0.5 + pLength * (0.5 - lengthDim.rRight / lengthDim.total())) * sceneHeight
            ));

            if (drawingBorderRect) {
                drawingBorderRect->setRect(matBoundingRegion);
            } else {
                drawingBorderRect = graphicsScene->addRect(matBoundingRegion);
            }

            QLineF leftWidthExtender(
                    QPointF(matBoundingRegion.left(), (0.5 * (1.0 - pLength)) * sceneHeight),
                    QPointF(matBoundingRegion.left(),
                            (0.5 * (1.0 - pLength) - dimensionLineOffset * sceneAspect) * sceneHeight));
            QLineF rightWidthExtender(
                    QPointF(matBoundingRegion.right(), (0.5 * (1.0 - pLength)) * sceneHeight),
                    QPointF(matBoundingRegion.right(),
                            (0.5 * (1.0 - pLength) - dimensionLineOffset * sceneAspect) * sceneHeight));

            if (widthDimension) {
                widthDimension->setBounds(QRectF(leftWidthExtender.p2(), rightWidthExtender.p1()));
                widthDimension->setLabel(("Width: " + to_str(width)).c_str());
            } else {
                widthDimension = new DimensionLine(QRectF(leftWidthExtender.p2(), rightWidthExtender.p1()),
                                                   DimensionLine::HORIZONTAL, ("Width: " + to_str(width)).c_str());
                graphicsScene->addItem(widthDimension);
            }

            QLineF topLengthExtender(
                    QPointF((0.5 * (1.0 - pWidth)) * sceneWidth, matBoundingRegion.top()),
                    QPointF((0.5 * (1.0 - pWidth) - dimensionLineOffset) * sceneWidth, matBoundingRegion.top()));
            QLineF bottomLengthExtender(
                    QPointF((0.5 * (1.0 - pWidth)) * sceneWidth, matBoundingRegion.bottom()),
                    QPointF((0.5 * (1.0 - pWidth) - dimensionLineOffset) * sceneWidth, matBoundingRegion.bottom()));

            if (lengthDimension) {
                lengthDimension->setBounds(QRectF(topLengthExtender.p2(), bottomLengthExtender.p1()));
                lengthDimension->setLabel(("Length: " + to_str(length)).c_str());
            } else {
                lengthDimension = new DimensionLine(QRectF(topLengthExtender.p2(), bottomLengthExtender.p1()),
                                                    DimensionLine::VERTICAL, ("Length: " + to_str(length)).c_str());
                graphicsScene->addItem(lengthDimension);
            }

            QColor unused = QColor(220, 220, 220), unusedHighlight = QColor(150, 255, 180),
                    used = QColor(94, 255, 137);

            QRectF leftLapRegion;
            leftLapRegion.setTopRight(matBoundingRegion.topLeft());
            leftLapRegion.setBottomRight(matBoundingRegion.bottomLeft());
            if (leftLap.has_value()) {
                leftLapRegion.setLeft((0.5 * (1.0 - pWidth)) * sceneWidth);
            } else {
                leftLapRegion.setLeft(leftLapRegion.right() - sceneWidth * lapHintWidth);
            }
            if (leftLapHint) {
                leftLapHint->setBounds(leftLapRegion);
                leftLapHint->setLap(leftLap);
            } else {
                leftLapHint = new AddLapWidget(graphicsScene, leftLapRegion, DimensionLine::VERTICAL, leftLap, unused,
                                               unusedHighlight, used);
                leftLapHint->setActivationCallback([this](bool active) { lapActivationCallback(active, 0); });
                leftLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) { lapChangedCallback(lap, 0); });
                graphicsScene->addItem(leftLapHint);
            }
            leftLapHint->setVisible(!lapsDisabled);
            leftLapHint->setUsed(leftLap.has_value());

            QRectF rightLapRegion;
            rightLapRegion.setTopLeft(matBoundingRegion.topRight());
            rightLapRegion.setBottomLeft(matBoundingRegion.bottomRight());
            if (rightLap.has_value()) {
                rightLapRegion.setRight((0.5 * (1.0 + pWidth)) * sceneWidth);
            } else {
                rightLapRegion.setRight(rightLapRegion.left() + sceneWidth * lapHintWidth);
            }
            if (rightLapHint) {
                rightLapHint->setBounds(rightLapRegion);
                rightLapHint->setLap(rightLap);
            } else {
                rightLapHint = new AddLapWidget(graphicsScene, rightLapRegion, DimensionLine::VERTICAL, rightLap,
                                                unused,
                                                unusedHighlight, used);
                rightLapHint->setActivationCallback([this](bool active) { lapActivationCallback(active, 1); });
                rightLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) { lapChangedCallback(lap, 1); });
                graphicsScene->addItem(rightLapHint);
            }
            rightLapHint->setVisible(!lapsDisabled);
            rightLapHint->setUsed(rightLap.has_value());

            QRectF topLapRegion;
            topLapRegion.setBottomLeft(matBoundingRegion.topLeft());
            topLapRegion.setBottomRight(matBoundingRegion.topRight());
            if (topLap.has_value()) {
                topLapRegion.setTop((0.5 * (1.0 - pLength)) * sceneHeight);
            } else {
                topLapRegion.setTop(topLapRegion.bottom() - sceneWidth * lapHintWidth);
            }
            if (topLapHint) {
                topLapHint->setBounds(topLapRegion);
                topLapHint->setLap(topLap);
            } else {
                topLapHint = new AddLapWidget(graphicsScene, topLapRegion, DimensionLine::HORIZONTAL, topLap, unused,
                                              unusedHighlight, used);
                topLapHint->setActivationCallback([this](bool active) { lapActivationCallback(active, 2); });
                topLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) { lapChangedCallback(lap, 2); });
                graphicsScene->addItem(topLapHint);
            }
            topLapHint->setVisible(!lapsDisabled);
            topLapHint->setUsed(topLap.has_value());

            QRectF bottomLapRegion;
            bottomLapRegion.setTopLeft(matBoundingRegion.bottomLeft());
            bottomLapRegion.setTopRight(matBoundingRegion.bottomRight());
            if (bottomLap.has_value()) {
                bottomLapRegion.setBottom((0.5 * (1.0 + pLength)) * sceneHeight);
            } else {
                bottomLapRegion.setBottom(bottomLapRegion.top() + sceneWidth * lapHintWidth);
            }
            if (bottomLapHint) {
                bottomLapHint->setBounds(bottomLapRegion);
                bottomLapHint->setLap(bottomLap);
            } else {
                bottomLapHint = new AddLapWidget(graphicsScene, bottomLapRegion, DimensionLine::HORIZONTAL, bottomLap,
                                                 unused,
                                                 unusedHighlight, used);
                bottomLapHint->setActivationCallback([this](bool active) { lapActivationCallback(active, 3); });
                bottomLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) { lapChangedCallback(lap, 3); });
                graphicsScene->addItem(bottomLapHint);
            }
            bottomLapHint->setVisible(!lapsDisabled);
            bottomLapHint->setUsed(bottomLap.has_value());

            updateProxies();

            std::vector<double> apertureRegionEndpoints;
            apertureRegionEndpoints.push_back((barWidths.front() == 0) ? defaultBarSize : drawing->leftMargin());

            double defaultSpacing = width / (numberOfBars + 1.0);

            double currentMatPosition = 0;

            for (unsigned bar = 0; bar < numberOfBars; bar++) {
                currentMatPosition += (barSpacings[bar] == 0) ? defaultSpacing : barSpacings[bar];

                double barWidth = (barWidths[bar + 1] == 0) ? defaultBarSize : barWidths[bar + 1];
                apertureRegionEndpoints.push_back(currentMatPosition - barWidth / 2);
                apertureRegionEndpoints.push_back(currentMatPosition + barWidth / 2);
            }

            apertureRegionEndpoints.push_back(width - ((barWidths.back() == 0) ? defaultBarSize : barWidths.back()));

            for (unsigned apertureRegion = 0; apertureRegion < apertureRegionEndpoints.size() / 2; apertureRegion++) {
                double start = apertureRegionEndpoints[2 * apertureRegion];
                double end = apertureRegionEndpoints[2 * apertureRegion + 1];

                QRectF region;
                region.setTopLeft(QPointF(
                        (start / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                        (defaultHorizontalBarSize / lengthDim.rCentre) * matBoundingRegion.height() +
                        matBoundingRegion.top()
                ));
                region.setBottomRight(QPointF(
                        (end / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                        (1 - (defaultHorizontalBarSize / lengthDim.rCentre)) * matBoundingRegion.height() +
                        matBoundingRegion.top()
                ));

                matSectionRects[apertureRegion]->setRect(region);
            }

            double spacingPosition = 0;

            for (unsigned spacingDimension = 0; spacingDimension <= numberOfBars; spacingDimension++) {
                double nextSpacingPosition = spacingPosition +
                                             ((barSpacings[spacingDimension] == 0) ? defaultSpacing
                                                                                   : barSpacings[spacingDimension]);

                spacingDimensions[spacingDimension]->setBounds(QRectF(
                        QPointF(
                                (spacingPosition / widthDim.rCentre) * matBoundingRegion.width() +
                                matBoundingRegion.left(),
                                matBoundingRegion.height() * (barSpacingDimensionHeight - barDimensionHeight / 2) +
                                matBoundingRegion.top()),
                        QPointF(
                                (nextSpacingPosition / widthDim.rCentre) * matBoundingRegion.width() +
                                matBoundingRegion.left(),
                                matBoundingRegion.height() * (barSpacingDimensionHeight + barDimensionHeight / 2) +
                                matBoundingRegion.top())
                ));

                spacingPosition = nextSpacingPosition;
            }

            std::stringstream widthSumText;
            widthSumText << "Spacings currently add to: "
                         << std::accumulate(barSpacings.begin(), barSpacings.end(), 0.0f) << "mm";
            if (widthSumTextItem) {
                widthSumTextItem->setPlainText(widthSumText.str().c_str());
            } else {
                widthSumTextItem = graphicsScene->addText(widthSumText.str().c_str());
            }

            std::vector<double> barEndpoints = apertureRegionEndpoints;
            barEndpoints.insert(barEndpoints.begin(), 0);
            barEndpoints.push_back(widthDim.rCentre);

            for (unsigned bar = 0; bar < numberOfBars + 2; bar++) {
                barDimensions[bar]->setBounds(QRectF(
                        QPointF(
                                (barEndpoints[2 * bar] / widthDim.rCentre) * matBoundingRegion.width() +
                                matBoundingRegion.left(),
                                matBoundingRegion.height() * (barWidthDimensionHeight - barDimensionHeight / 2) +
                                matBoundingRegion.top()),
                        QPointF((barEndpoints[2 * bar + 1] / widthDim.rCentre) * matBoundingRegion.width() +
                                matBoundingRegion.left(),
                                matBoundingRegion.height() * (barWidthDimensionHeight + barDimensionHeight / 2) +
                                matBoundingRegion.top())
                ));
            }

            if (impactPadRegions.size() != drawing->impactPads().size()) {
                for (ImpactPadGraphicsItem *region : impactPadRegions) {
                    graphicsScene->removeItem(region);
                    delete region;
                }
                impactPadRegions.clear();

                for (unsigned i = 0; i < drawing->impactPads().size(); i++) {
                    ImpactPadGraphicsItem *impactPad = new AreaGraphicsItem<Drawing::ImpactPad, Material, Aperture>(QRectF(), drawing->impactPad(i),
                                                                                 inspector);
                    impactPad->setToolTip("Impact Pad");
                    impactPad->setRemoveFunction([this, i, impactPad, graphicsScene]() {
                        drawing->removeImpactPad(impactPad->get());
                        graphicsScene->removeItem(impactPad);
                        impactPadRegions.erase(std::find(impactPadRegions.begin(), impactPadRegions.end(), impactPad));
                    });
                    graphicsScene->addItem(impactPad);
                    impactPadRegions.push_back(impactPad);
                }
            }

            //for (unsigned i = 0; i < impactPadRegions.size(); i++) {
            for (ImpactPadGraphicsItem *ipGraphics : impactPadRegions) {
                const Drawing::ImpactPad& impactPad = ipGraphics->get();
                ipGraphics->setBounds(QRectF(
                        QPointF(matBoundingRegion.left() + (impactPad.pos.x / width) * matBoundingRegion.width(),
                                matBoundingRegion.top() + (impactPad.pos.y / length) * matBoundingRegion.height()),
                        QSizeF((impactPad.width / width) * matBoundingRegion.width(),
                               (impactPad.length / length) * matBoundingRegion.height())
                ));
            }
            if (blankSpaceRegions.size() != drawing->blankSpaces().size()) {
                for (BlankSpaceGraphicsItem* region : blankSpaceRegions) {
                    graphicsScene->removeItem(region);
                    delete region;
                }
                blankSpaceRegions.clear();

                for (unsigned i = 0; i < drawing->blankSpaces().size(); i++) {
                    BlankSpaceGraphicsItem* blankSpace = new BlankSpaceGraphicsItem(QRectF(), drawing->blankSpace(i),
                        inspector);
                    blankSpace->setToolTip("Blank Space");
                    blankSpace->setRemoveFunction([this, i, blankSpace, graphicsScene]() {
                        drawing->removeBlankSpace(blankSpace->get());
                        graphicsScene->removeItem(blankSpace);
                        blankSpaceRegions.erase(std::find(blankSpaceRegions.begin(), blankSpaceRegions.end(), blankSpace));
                        });
                    graphicsScene->addItem(blankSpace);
                    blankSpaceRegions.push_back(blankSpace);
                }
            }

            //for (unsigned i = 0; i < blankSpaceRegions.size(); i++) {
            for (BlankSpaceGraphicsItem *bsGraphics : blankSpaceRegions) {
                const Drawing::BlankSpace& blankSpace = bsGraphics->get();
                bsGraphics->setBounds(QRectF(
                    QPointF(matBoundingRegion.left() + (blankSpace.pos.x / width) * matBoundingRegion.width(),
                            matBoundingRegion.top() + (blankSpace.pos.y / length) * matBoundingRegion.height()),
                    QSizeF((blankSpace.width / width) * matBoundingRegion.width(),
                           (blankSpace.length / length) * matBoundingRegion.height())
                ));
            }

            if (extraApertureRegions.size() != drawing->extraApertures().size()) {
                for (ExtraApertureGraphicsItem* region : extraApertureRegions) {
                    graphicsScene->removeItem(region);
                    delete region;
                }
                extraApertureRegions.clear();

                for (unsigned i = 0; i < drawing->extraApertures().size(); i++) {
                    ExtraApertureGraphicsItem* extraAperture = new AreaGraphicsItem<Drawing::ExtraAperture, Aperture>(QRectF(), drawing->extraAperture(i),
                        inspector);
                    extraAperture->setToolTip("Extra Aperture");
                    extraAperture->setRemoveFunction([this, i, extraAperture, graphicsScene]() {
                        drawing->removeExtraAperture(extraAperture->get());
                        graphicsScene->removeItem(extraAperture);
                        extraApertureRegions.erase(std::find(extraApertureRegions.begin(), extraApertureRegions.end(), extraAperture));
                        });
                    graphicsScene->addItem(extraAperture);
                    extraApertureRegions.push_back(extraAperture);
                }
            }

            //for (unsigned i = 0; i < extraApertureRegions.size(); i++) {
            for (ExtraApertureGraphicsItem* eaGraphics : extraApertureRegions) {
                const Drawing::ExtraAperture& extraAperture = eaGraphics->get();
                eaGraphics->setBounds(QRectF(
                    QPointF(matBoundingRegion.left() + (extraAperture.pos.x / width) * matBoundingRegion.width(),
                            matBoundingRegion.top() + (extraAperture.pos.y / length) * matBoundingRegion.height()),
                    QSizeF((extraAperture.width / width) * matBoundingRegion.width(),
                           (extraAperture.length / length) * matBoundingRegion.height())
                ));
            }

            if (damBarRegions.size() != drawing->damBars().size()) {
                for (DamBarGraphicsItem* region : damBarRegions) {
                    graphicsScene->removeItem(region);
                    delete region;
                }
                damBarRegions.clear();

                for (unsigned i = 0; i < drawing->damBars().size(); i++) {
                    DamBarGraphicsItem* damBar = new AreaGraphicsItem<Drawing::DamBar, Material>(QRectF(),
                        drawing->damBar(i), inspector);
                    damBar->setToolTip("Dam Bar");
                    damBar->setRemoveFunction([this, i, damBar, graphicsScene]() {
                        graphicsScene->removeItem(damBar);
                        damBarRegions.erase(std::find(damBarRegions.begin(), damBarRegions.end(), damBar));
                        drawing->removeDamBar(damBar->get());
                        });
                    graphicsScene->addItem(damBar);
                    damBarRegions.push_back(damBar);
                }
            }

            for (DamBarGraphicsItem* region : damBarRegions) {
                const Drawing::DamBar& damBar = region->get();
                region->setBounds(QRectF(
                    QPointF(matBoundingRegion.left() + (damBar.pos.x / width) * matBoundingRegion.width(),
                        matBoundingRegion.top() + (damBar.pos.y / length) * matBoundingRegion.height()),
                    QSizeF((damBar.width / width) * matBoundingRegion.width(),
                        (damBar.length / length) * matBoundingRegion.height())
                ));
            }

            if (centreHoleSet) {
                centreHoleSet->setBounds(matBoundingRegion, width, length);
            } else {
                centreHoleSet = new CentreHoleSetGraphicsItem(drawing->centreHoles(), matBoundingRegion, width, length, inspector);
                centreHoleSet->setRemoveFunction(std::bind(&Drawing::removeCentreHole, drawing, std::placeholders::_1));
                graphicsScene->addItem(centreHoleSet);
            }

            if (deflectorSet) {
                deflectorSet->setBounds(matBoundingRegion, width, length);
            } else {
                deflectorSet = new DeflectorSetGraphicsItem(drawing->deflectors(), matBoundingRegion, width, length, inspector);
                deflectorSet->setRemoveFunction(std::bind(&Drawing::removeDeflector, drawing, std::placeholders::_1));
                graphicsScene->addItem(deflectorSet);
            }

            if (divertorSet) {
                divertorSet->setBounds(matBoundingRegion, width, length);
            } else {
                divertorSet = new DivertorSetGraphicsItem(drawing->divertors(), matBoundingRegion, width, length, inspector);
                divertorSet->setRemoveFunction(std::bind(&Drawing::removeDivertor, drawing, std::placeholders::_1));
                graphicsScene->addItem(divertorSet);
            }
        }
    }
}

void DrawingView::updateProxies() {
    QGraphicsScene *graphicsScene = scene();

    if (spacingProxies.size() <= numberOfBars) {
        for (unsigned spacing = spacingProxies.size(); spacing <= numberOfBars; spacing++) {
            QDoubleSpinBox *spacingInput = new QDoubleSpinBox();
            spacingInput->setDecimals(1);
            spacingInput->setMinimum(0);
            spacingInput->setMaximum(32767);
            spacingInput->setValue(barSpacings[spacing]);

            connect(spacingInput, qOverload<double>(&QDoubleSpinBox::valueChanged), [this, spacing](double d) {
                barSpacings[spacing] = (float) d;
            });
            connect(spacingInput, &QDoubleSpinBox::editingFinished, [this, spacing]() {
                if (barSpacings[numberOfBars - spacing] == 0) {
                    qobject_cast<QDoubleSpinBox *>(spacingProxies[numberOfBars - spacing]->widget())->setValue(
                            barSpacings[spacing]);
                }
                drawing->setBars(barSpacings, barWidths);
            });

            QGraphicsProxyWidget *spacingProxy = graphicsScene->addWidget(spacingInput);
            spacingProxy->setZValue(1);

            spacingProxies.push_back(spacingProxy);

            WidgetDimensionLine *dimensionLine = new WidgetDimensionLine(QRectF(), DimensionLine::HORIZONTAL,
                                                                         spacingProxy);
            spacingDimensions.push_back(dimensionLine);

            graphicsScene->addItem(dimensionLine);

            matSectionRects.push_back(graphicsScene->addRect(QRectF()));
        }
    } else {
        while (spacingProxies.size() > numberOfBars + 1) {
            graphicsScene->removeItem(spacingProxies.back());
            spacingProxies.pop_back();
            graphicsScene->removeItem(spacingDimensions.back());
            spacingDimensions.pop_back();

            graphicsScene->removeItem(matSectionRects.back());
            matSectionRects.pop_back();
        }
    }

    if (barProxies.size() < numberOfBars + 2) {
        for (unsigned bar = barProxies.size(); bar < numberOfBars + 2; bar++) {
            QDoubleSpinBox *barInput = new QDoubleSpinBox();
            barInput->setDecimals(1);
            barInput->setMinimum(0);
            barInput->setMaximum(32767);
            barInput->setValue(barWidths[bar]);

            connect(barInput, qOverload<double>(&QDoubleSpinBox::valueChanged), [this, bar](double d) {
                barWidths[bar] = (float) d;
            });
            connect(barInput, &QDoubleSpinBox::editingFinished, [this, bar]() {
                if (bar == 0 && barWidths.back() == 0) {
                    qobject_cast<QDoubleSpinBox *>(barProxies.back()->widget())->setValue(barWidths[0]);
                } else if (bar < numberOfBars) {
                    for (unsigned fillBar = 1; fillBar < numberOfBars + 1; fillBar++) {
                        if (barWidths[fillBar] == 0) {
                            qobject_cast<QDoubleSpinBox *>(barProxies[fillBar]->widget())->setValue(barWidths[bar]);
                        }
                    }
                }
                drawing->setBars(barSpacings, barWidths);
            });

            QGraphicsProxyWidget *barProxy = graphicsScene->addWidget(barInput);
            barProxy->setZValue(1);

            barProxies.push_back(barProxy);

            WidgetDimensionLine *dimensionLine = new WidgetDimensionLine(QRectF(), DimensionLine::HORIZONTAL,
                                                                         barProxy);
            barDimensions.push_back(dimensionLine);

            graphicsScene->addItem(dimensionLine);
        }
    } else {
        while (barProxies.size() > numberOfBars + 2) {
            graphicsScene->removeItem(barProxies.back());
            barProxies.pop_back();
            graphicsScene->removeItem(barDimensions.back());
            barDimensions.pop_back();
        }
    }

    QGraphicsWidget *precedingTabOrderItem = spacingProxies.front();

    for (unsigned i = 1; i <= numberOfBars; i++) {
        QGraphicsWidget::setTabOrder(precedingTabOrderItem, spacingProxies[i]);
        precedingTabOrderItem = spacingProxies[i];
    }

    for (unsigned i = 0; i < numberOfBars + 2; i++) {
        QGraphicsWidget::setTabOrder(precedingTabOrderItem, barProxies[i]);
        precedingTabOrderItem = barProxies[i];
    }
}

void DrawingView::lapActivationCallback(bool active, unsigned lapIndex) {
    Drawing::Side side = (Drawing::Side) (lapIndex % 2);

    switch (drawing->tensionType()) {
        case Drawing::SIDE:
            if (active) {
                if (!lapCache[lapIndex].has_value()) {
                    lapCache[lapIndex] = Drawing::Lap();
                }
                if (lapIndex < 2) {
                    drawing->setSidelap(side, lapCache[lapIndex].value());
                } else {
                    drawing->setOverlap(side, lapCache[lapIndex].value());
                }
            } else {
                if (lapIndex < 2) {
                    drawing->removeSidelap(side);
                } else {
                    drawing->removeOverlap(side);
                }
            }
            break;
        case Drawing::END:
            if (active) {
                if (!lapCache[lapIndex].has_value()) {
                    lapCache[lapIndex] = Drawing::Lap();
                }
                if (lapIndex < 2) {
                    drawing->setOverlap(side, lapCache[lapIndex].value());
                } else {
                    drawing->setSidelap(side, lapCache[lapIndex].value());
                }
            } else {
                if (lapIndex < 2) {
                    drawing->removeOverlap(side);
                } else {
                    drawing->removeSidelap(side);
                }
            }
            break;
    }
}

void DrawingView::lapChangedCallback(const Drawing::Lap &lap, unsigned lapIndex) {
    Drawing::Side side = (Drawing::Side) (lapIndex % 2);

    lapCache[lapIndex] = lap;

    switch (drawing->tensionType()) {
        case Drawing::SIDE:
            if (lapIndex < 2) {
                drawing->setSidelap(side, lap);
            } else {
                drawing->setOverlap(side, lap);
            }
            break;
        case Drawing::END:
            if (lapIndex < 2) {
                drawing->setOverlap(side, lap);
            } else {
                drawing->setSidelap(side, lap);
            }
            break;
    }
}

void DrawingView::updateSnapLines() {
    hSnapLines.clear();
    vSnapLines.clear();

    if (drawingBorderRect) {
        QRectF border = drawingBorderRect->rect();
        hSnapLines.push_back(border.top());
        hSnapLines.push_back(border.bottom());
        vSnapLines.push_back(border.left());
        vSnapLines.push_back(border.right());
    }

    for (const QGraphicsRectItem *sectionRect : matSectionRects) {
        QRectF border = sectionRect->rect();
        hSnapLines.push_back(border.top());
        hSnapLines.push_back(border.bottom());
        vSnapLines.push_back(border.left());
        vSnapLines.push_back(border.right());
    }
}

void DrawingView::updateCentreSnapLines() {
    centreVSnapLines.clear();

    for (unsigned i = 0; i < matSectionRects.size() - 1; i++) {
        centreVSnapLines.push_back((matSectionRects[i]->rect().right() + matSectionRects[i + 1]->rect().left()) / 2.0);
    }
}

QPoint DrawingView::snapPoint(const QPoint &point) const {
    const int snapThreshold = 10;

    QPoint snapped = point;

    for (double line : hSnapLines) {
        if (abs(line - point.y()) <= snapThreshold) {
            snapped.setY(line);
            break;
        }
    }

    for (double line : vSnapLines) {
        if (abs(line - point.x()) <= snapThreshold) {
            snapped.setX(line);
            break;
        }
    }

    return snapped;
}

QPointF DrawingView::snapPointF(const QPointF &point) const {
    const double snapThreshold = 10;

    QPointF snapped = point;

    for (double line : hSnapLines) {
        if (abs(line - point.y()) <= snapThreshold) {
            snapped.setY(line);
            break;
        }
    }

    for (double line : vSnapLines) {
        if (abs(line - point.x()) <= snapThreshold) {
            snapped.setX(line);
            break;
        }
    }

    return snapped;
}

QPointF DrawingView::snapPointToCentreF(const QPointF &point) const {
    const double snapThreshold = 10;

    QPointF snapped = point;

    for (double line : centreVSnapLines) {
        if (abs(line - point.x()) <= snapThreshold) {
            snapped.setX(line);
            break;
        }
    }

    return snapped;
}