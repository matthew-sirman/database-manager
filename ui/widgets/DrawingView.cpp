#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
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

    this->drawing->addUpdateCallback([this]() { redrawScene(); });
}

void DrawingView::setNumberOfBars(unsigned int bars) {
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

    redrawScene();
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

    redrawScene();
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

    redrawScene();
}

void DrawingView::paintEvent(QPaintEvent *event) {
    QGraphicsScene *graphicsScene = scene();

    if (graphicsScene) {
        graphicsScene->setSceneRect(0, 0, sceneSize, sceneSize);
        fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
    }

    QGraphicsView::paintEvent(event);
}

void DrawingView::redrawScene() {
    QGraphicsScene *graphicsScene = scene();

    if (graphicsScene && drawing) {
        double width = drawing->width(), length = drawing->length();

        if (width != 0 && length != 0) {
            // double regionWidth = size().width(), regionHeight = size().height();

            std::optional<Drawing::Lap> leftLap, rightLap, topLap, bottomLap;

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
                widthDim.rLeft = leftLap->width == 0 ? defaultLapSize * sceneSize : leftLap->width;
            }
            if (rightLap.has_value()) {
                widthDim.rRight = rightLap->width == 0 ? defaultLapSize * sceneSize : rightLap->width;
            }
            if (topLap.has_value()) {
                lengthDim.rLeft = topLap->width == 0 ? defaultLapSize * sceneSize : topLap->width;
            }
            if (bottomLap.has_value()) {
                lengthDim.rRight = bottomLap->width == 0 ? defaultLapSize * sceneSize : bottomLap->width;
            }

            double pWidth, pLength;

            if (widthDim.total() / size().width() > lengthDim.total() / size().height()) {
                pWidth = maxMatDimensionPercentage;
                pLength = maxMatDimensionPercentage * (lengthDim.total() / widthDim.total());
            } else {
                pLength = maxMatDimensionPercentage;
                pWidth = maxMatDimensionPercentage * (widthDim.total() / lengthDim.total());
            }

            QRectF matBoundingRegion;
            matBoundingRegion.setTopLeft(QPointF(
                (0.5 - pWidth * (0.5 - widthDim.rLeft / widthDim.total())) * sceneSize,
                (0.5 - pLength * (0.5 - lengthDim.rLeft / lengthDim.total())) * sceneSize
            ));
            matBoundingRegion.setBottomRight(QPointF(
                (0.5 + pWidth * (0.5 - widthDim.rRight / widthDim.total())) *sceneSize,
                (0.5 + pLength * (0.5 - lengthDim.rRight / lengthDim.total())) *sceneSize
            ));

            if (drawingBorderRect) {
                drawingBorderRect->setRect(matBoundingRegion);
            } else {
                drawingBorderRect = graphicsScene->addRect(matBoundingRegion);
            }

            QLineF leftWidthExtender(
                QPointF(matBoundingRegion.left(), (0.5 * (1.0 - pLength)) *sceneSize),
                QPointF(matBoundingRegion.left(), (0.5 * (1.0 - pLength) - dimensionLineOffset) *sceneSize));
            QLineF rightWidthExtender(
                QPointF(matBoundingRegion.right(), (0.5 * (1.0 - pLength)) *sceneSize),
                QPointF(matBoundingRegion.right(), (0.5 * (1.0 - pLength) - dimensionLineOffset) *sceneSize));

            if (widthDimension) {
                widthDimension->setBounds(QRectF(leftWidthExtender.p2(), rightWidthExtender.p1()));
                widthDimension->setLabel(("Width: " + to_str(width)).c_str());
            } else {
                widthDimension = new DimensionLine(QRectF(leftWidthExtender.p2(), rightWidthExtender.p1()),
                    DimensionLine::HORIZONTAL, ("Width: " + to_str(width)).c_str());
                graphicsScene->addItem(widthDimension);
            }

            QLineF topLengthExtender(
                QPointF((0.5 * (1.0 - pWidth)) *sceneSize, matBoundingRegion.top()),
                QPointF((0.5 * (1.0 - pWidth) - dimensionLineOffset) *sceneSize, matBoundingRegion.top()));
            QLineF bottomLengthExtender(
                QPointF((0.5 * (1.0 - pWidth)) *sceneSize, matBoundingRegion.bottom()),
                QPointF((0.5 * (1.0 - pWidth) - dimensionLineOffset) *sceneSize, matBoundingRegion.bottom()));

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
                leftLapRegion.setLeft((0.5 * (1.0 - pWidth)) *sceneSize);
            } else {
                leftLapRegion.setLeft(leftLapRegion.right() - sceneSize * lapHintWidth);
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

            QRectF rightLapRegion;
            rightLapRegion.setTopLeft(matBoundingRegion.topRight());
            rightLapRegion.setBottomLeft(matBoundingRegion.bottomRight());
            if (rightLap.has_value()) {
                rightLapRegion.setRight((0.5 * (1.0 + pWidth)) * sceneSize);
            } else {
                rightLapRegion.setRight(rightLapRegion.left() + sceneSize * lapHintWidth);
            }
            if (rightLapHint) {
                rightLapHint->setBounds(rightLapRegion);
                rightLapHint->setLap(rightLap);
            } else {
                rightLapHint = new AddLapWidget(graphicsScene, rightLapRegion, DimensionLine::VERTICAL, rightLap, unused,
                    unusedHighlight, used);
                rightLapHint->setActivationCallback([this](bool active) { lapActivationCallback(active, 1); });
                rightLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) { lapChangedCallback(lap, 1); });
                graphicsScene->addItem(rightLapHint);
            }
            rightLapHint->setVisible(!lapsDisabled);

            QRectF topLapRegion;
            topLapRegion.setBottomLeft(matBoundingRegion.topLeft());
            topLapRegion.setBottomRight(matBoundingRegion.topRight());
            if (topLap.has_value()) {
                topLapRegion.setTop((0.5 * (1.0 - pLength)) *sceneSize);
            } else {
                topLapRegion.setTop(topLapRegion.bottom() - sceneSize * lapHintWidth);
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

            QRectF bottomLapRegion;
            bottomLapRegion.setTopLeft(matBoundingRegion.bottomLeft());
            bottomLapRegion.setTopRight(matBoundingRegion.bottomRight());
            if (bottomLap.has_value()) {
                bottomLapRegion.setBottom((0.5 * (1.0 + pLength)) *sceneSize);
            } else {
                bottomLapRegion.setBottom(bottomLapRegion.top() + sceneSize * lapHintWidth);
            }
            if (bottomLapHint) {
                bottomLapHint->setBounds(bottomLapRegion);
                bottomLapHint->setLap(bottomLap);
            } else {
                bottomLapHint = new AddLapWidget(graphicsScene, bottomLapRegion, DimensionLine::HORIZONTAL, bottomLap, unused,
                    unusedHighlight, used);
                bottomLapHint->setActivationCallback([this](bool active) { lapActivationCallback(active, 3); });
                bottomLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) { lapChangedCallback(lap, 3); });
                graphicsScene->addItem(bottomLapHint);
            }
            bottomLapHint->setVisible(!lapsDisabled);

            updateProxies();

            std::vector<double> apertureRegionEndpoints;
            apertureRegionEndpoints.push_back((barWidths.front() == 0) ? defaultBarSize : drawing->leftBar());

            double defaultSpacing = width / (numberOfBars + 1);

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
                    matSectionInset * matBoundingRegion.height() + matBoundingRegion.top()
                ));
                region.setBottomRight(QPointF(
                    (end / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                    (1 - matSectionInset) * matBoundingRegion.height() + matBoundingRegion.top()
                ));

                matSectionRects[apertureRegion]->setRect(region);
            }

            double spacingPosition = 0;

            for (unsigned spacingDimension = 0; spacingDimension <= numberOfBars; spacingDimension++) {
                double nextSpacingPosition = spacingPosition + 
                    ((barSpacings[spacingDimension] == 0) ? defaultSpacing : barSpacings[spacingDimension]);

                spacingDimensions[spacingDimension]->setBounds(QRectF(
                    QPointF(
                        (spacingPosition / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                        matBoundingRegion.height() * (barSpacingDimensionHeight - barDimensionHeight / 2) + matBoundingRegion.top()),
                    QPointF(
                        (nextSpacingPosition / widthDim.rCentre) * matBoundingRegion.width() + matBoundingRegion.left(),
                        matBoundingRegion.height() *(barSpacingDimensionHeight + barDimensionHeight / 2) + matBoundingRegion.top())
                ));

                spacingPosition = nextSpacingPosition;
            }

            std::vector<double> barEndpoints = apertureRegionEndpoints;
            barEndpoints.insert(barEndpoints.begin(), 0);
            barEndpoints.push_back(widthDim.rCentre);

            for (unsigned bar = 0; bar < numberOfBars + 2; bar++) {
                barDimensions[bar]->setBounds(QRectF(
                    QPointF(
                        (barEndpoints[2 * bar] / widthDim.rCentre) *matBoundingRegion.width() + matBoundingRegion.left(), 
                        matBoundingRegion.height() *(barWidthDimensionHeight - barDimensionHeight / 2) + matBoundingRegion.top()),
                    QPointF((barEndpoints[2 * bar + 1] / widthDim.rCentre) *matBoundingRegion.width() + matBoundingRegion.left(),
                        matBoundingRegion.height() *(barWidthDimensionHeight + barDimensionHeight / 2) + matBoundingRegion.top())
                ));
            }
        }
        update();
    }
}

void DrawingView::updateProxies() {
    QGraphicsScene *graphicsScene = scene();

    if (spacingProxies.size() <= numberOfBars) {
        for (unsigned spacing = spacingProxies.size(); spacing <= numberOfBars; spacing++) {
            QDoubleSpinBox *spacingInput = new QDoubleSpinBox();
            spacingInput->setDecimals(1);
            spacingInput->setMinimum(0);
            spacingInput->setMaximum(drawing->width());
            spacingInput->setValue(barSpacings[spacing]);

            connect(spacingInput, qOverload<double>(&QDoubleSpinBox::valueChanged), [this, spacing](double d) {
                barSpacings[spacing] = (float)d;
            });
            connect(spacingInput, &QDoubleSpinBox::editingFinished, [this]() {
                drawing->setBars(barSpacings, barWidths);
            });

            QGraphicsProxyWidget *spacingProxy = graphicsScene->addWidget(spacingInput);
            spacingProxy->setScale(2);
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
            barInput->setMaximum(drawing->width());
            barInput->setValue(barWidths[bar]);

            connect(barInput, qOverload<double>(&QDoubleSpinBox::valueChanged), [this, bar](double d) {
                barWidths[bar] = (float)d;
            });
            connect(barInput, &QDoubleSpinBox::editingFinished, [this]() {
                drawing->setBars(barSpacings, barWidths);
            });

            QGraphicsProxyWidget *barProxy = graphicsScene->addWidget(barInput);
            barProxy->setScale(2);
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

#pragma clang diagnostic pop