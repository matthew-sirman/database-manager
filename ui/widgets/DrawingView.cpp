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

void DrawingView::paintEvent(QPaintEvent *event) {
    QGraphicsScene *graphicsScene = scene();

    if (graphicsScene) {
        graphicsScene->setSceneRect(-(sceneSize / 2), -(sceneSize / 2), sceneSize, sceneSize);
        fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
    }

    QGraphicsView::paintEvent(event);
}

void DrawingView::redrawScene() {
    QGraphicsScene *graphicsScene = scene();

    if (graphicsScene && drawing) {
//        graphicsScene->clear();

        float width = drawing->width(), length = drawing->length();

        if (width != 0 && length != 0) {
            float totalWidth = width, totalLength = length;
            std::optional<Drawing::Lap> leftLap, rightLap, topLap, bottomLap;
            float leftLapDrawWidth, rightLapDrawWidth, topLapDrawWidth, bottomLapDrawWidth;

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
            }

            if (leftLap.has_value()) {
                leftLapDrawWidth = leftLap->width == 0 ? defaultLapSize * sceneSize : leftLap->width;
                totalWidth += leftLapDrawWidth;
            }
            if (rightLap.has_value()) {
                rightLapDrawWidth = rightLap->width == 0 ? defaultLapSize * sceneSize : rightLap->width;
                totalWidth += rightLapDrawWidth;
            }
            if (topLap.has_value()) {
                topLapDrawWidth = topLap->width == 0 ? defaultLapSize * sceneSize : topLap->width;
                totalLength += topLapDrawWidth;
            }
            if (bottomLap.has_value()) {
                bottomLapDrawWidth = bottomLap->width == 0 ? defaultLapSize * sceneSize : bottomLap->width;
                totalLength += bottomLapDrawWidth;
            }

            QRectF matRect;
            QRectF totalBounds;
            if (totalWidth / (float) size().width() > totalLength / (float) size().height()) {
                totalBounds.setLeft(-(sceneSize * maxMatDimensionPercentage) / 2);
                totalBounds.setTop(-(sceneSize * maxMatDimensionPercentage * (totalLength / totalWidth)) / 2);
                totalBounds.setWidth(sceneSize * maxMatDimensionPercentage);
                totalBounds.setHeight(totalBounds.width() * (totalLength / totalWidth));

                if (leftLap.has_value()) {
                    matRect.setLeft(totalBounds.width() * (leftLapDrawWidth / totalWidth - 0.5));
                } else {
                    matRect.setLeft(totalBounds.left());
                }
                if (topLap.has_value()) {
                    matRect.setTop(totalBounds.height() * (topLapDrawWidth / totalLength - 0.5));
                } else {
                    matRect.setTop(totalBounds.top());
                }
                matRect.setWidth(totalBounds.width() * (width / totalWidth));
                matRect.setHeight(matRect.width() * (length / width));
            } else {
                totalBounds.setLeft(-(sceneSize * maxMatDimensionPercentage * (totalWidth / totalLength)) / 2);
                totalBounds.setTop(-(sceneSize * maxMatDimensionPercentage) / 2);
                totalBounds.setHeight(sceneSize * maxMatDimensionPercentage);
                totalBounds.setWidth(totalBounds.height() * (totalWidth / totalLength));

                if (leftLap.has_value()) {
                    matRect.setLeft(totalBounds.width() * (leftLapDrawWidth / totalWidth - 0.5));
                } else {
                    matRect.setLeft(totalBounds.left());
                }
                if (topLap.has_value()) {
                    matRect.setTop(totalBounds.height() * (topLapDrawWidth / totalLength - 0.5));
                } else {
                    matRect.setTop(totalBounds.top());
                }
                matRect.setHeight(totalBounds.height() * (length / totalLength));
                matRect.setWidth(matRect.height() * (width / length));
            }

            if (drawingBorderRect) {
                drawingBorderRect->setRect(matRect);
            } else {
                drawingBorderRect = graphicsScene->addRect(matRect);
            }

            QColor unused = QColor(220, 220, 220), unusedHighlight = QColor(150, 255, 180),
                    used = QColor(94, 255, 137);

            QRectF lap;

            lap.setTopRight(matRect.topLeft());
            lap.setBottomRight(matRect.bottomLeft());
            if (leftLap.has_value()) {
                lap.setLeft(totalBounds.left());
            } else {
                lap.setLeft(lap.right() - sceneSize * lapHintWidth);
            }
            if (leftLapHint) {
                leftLapHint->setBounds(lap);
                leftLapHint->setLap(leftLap);
            } else {
                leftLapHint = new AddLapWidget(graphicsScene, lap, DimensionLine::VERTICAL, leftLap, unused,
                                               unusedHighlight, used);
                leftLapHint->setActivationCallback([this](bool active) {
                    switch (drawing->tensionType()) {
                        case Drawing::SIDE:
                            if (active) {
                                if (!lapCache[0].has_value()) {
                                    lapCache[0] = Drawing::Lap();
                                }
                                drawing->setSidelap(Drawing::LEFT, lapCache[0].value());
                            } else {
                                drawing->removeSidelap(Drawing::LEFT);
                            }
                            break;
                        case Drawing::END:
                            if (active) {
                                if (!lapCache[0].has_value()) {
                                    lapCache[0] = Drawing::Lap();
                                }
                                drawing->setOverlap(Drawing::LEFT, lapCache[0].value());
                            } else {
                                drawing->removeOverlap(Drawing::LEFT);
                            }
                            break;
                    }
                });
                leftLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) {
                    lapCache[0] = lap;
                    switch (drawing->tensionType()) {
                        case Drawing::SIDE:
                            drawing->setSidelap(Drawing::LEFT, lap);
                            break;
                        case Drawing::END:
                            drawing->setOverlap(Drawing::LEFT, lap);
                            break;
                    }
                });
                graphicsScene->addItem(leftLapHint);
            }

            lap.setTopLeft(matRect.topRight());
            lap.setBottomLeft(matRect.bottomRight());
            if (rightLap.has_value()) {
                lap.setRight(totalBounds.right());
            } else {
                lap.setRight(lap.left() + sceneSize * lapHintWidth);
            }
            if (rightLapHint) {
                rightLapHint->setBounds(lap);
                rightLapHint->setLap(rightLap);
            } else {
                rightLapHint = new AddLapWidget(graphicsScene, lap, DimensionLine::VERTICAL, rightLap, unused,
                                                unusedHighlight, used);
                rightLapHint->setActivationCallback([this](bool active) {
                    switch (drawing->tensionType()) {
                        case Drawing::SIDE:
                            if (active) {
                                if (!lapCache[1].has_value()) {
                                    lapCache[1] = Drawing::Lap();
                                }
                                drawing->setSidelap(Drawing::RIGHT, lapCache[1].value());
                            } else {
                                drawing->removeSidelap(Drawing::RIGHT);
                            }
                            break;
                        case Drawing::END:
                            if (active) {
                                if (!lapCache[1].has_value()) {
                                    lapCache[1] = Drawing::Lap();
                                }
                                drawing->setOverlap(Drawing::RIGHT, lapCache[1].value());
                            } else {
                                drawing->removeOverlap(Drawing::RIGHT);
                            }
                            break;
                    }
                });
                rightLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) {
                    lapCache[1] = lap;
                    switch (drawing->tensionType()) {
                        case Drawing::SIDE:
                            drawing->setSidelap(Drawing::RIGHT, lap);
                            break;
                        case Drawing::END:
                            drawing->setOverlap(Drawing::RIGHT, lap);
                            break;
                    }
                });
                graphicsScene->addItem(rightLapHint);
            }

            lap.setBottomLeft(matRect.topLeft());
            lap.setBottomRight(matRect.topRight());
            if (topLap.has_value()) {
                lap.setTop(totalBounds.top());
            } else {
                lap.setTop(lap.bottom() - sceneSize * lapHintWidth);
            }
            if (topLapHint) {
                topLapHint->setBounds(lap);
                topLapHint->setLap(topLap);
            } else {
                topLapHint = new AddLapWidget(graphicsScene, lap, DimensionLine::HORIZONTAL, topLap, unused,
                                              unusedHighlight, used);
                topLapHint->setActivationCallback([this](bool active) {
                    switch (drawing->tensionType()) {
                        case Drawing::SIDE:
                            if (active) {
                                if (!lapCache[2].has_value()) {
                                    lapCache[2] = Drawing::Lap();
                                }
                                drawing->setOverlap(Drawing::LEFT, lapCache[2].value());
                            } else {
                                drawing->removeOverlap(Drawing::LEFT);
                            }
                            break;
                        case Drawing::END:
                            if (active) {
                                if (!lapCache[2].has_value()) {
                                    lapCache[2] = Drawing::Lap();
                                }
                                drawing->setSidelap(Drawing::LEFT, lapCache[2].value());
                            } else {
                                drawing->removeSidelap(Drawing::LEFT);
                            }
                            break;
                    }
                });
                topLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) {
                    lapCache[2] = lap;
                    switch (drawing->tensionType()) {
                        case Drawing::SIDE:
                            drawing->setOverlap(Drawing::LEFT, lap);
                            break;
                        case Drawing::END:
                            drawing->setSidelap(Drawing::LEFT, lap);
                            break;
                    }
                });
                graphicsScene->addItem(topLapHint);
            }

            lap.setTopLeft(matRect.bottomLeft());
            lap.setTopRight(matRect.bottomRight());
            if (bottomLap.has_value()) {
                lap.setBottom(totalBounds.bottom());
            } else {
                lap.setBottom(lap.top() + sceneSize * lapHintWidth);
            }
            if (bottomLapHint) {
                bottomLapHint->setBounds(lap);
                bottomLapHint->setLap(bottomLap);
            } else {
                bottomLapHint = new AddLapWidget(graphicsScene, lap, DimensionLine::HORIZONTAL, bottomLap, unused,
                                                 unusedHighlight, used);
                bottomLapHint->setActivationCallback([this](bool active) {
                    switch (drawing->tensionType()) {
                        case Drawing::SIDE:
                            if (active) {
                                if (!lapCache[3].has_value()) {
                                    lapCache[3] = Drawing::Lap();
                                }
                                drawing->setOverlap(Drawing::RIGHT, lapCache[3].value());
                            } else {
                                drawing->removeOverlap(Drawing::RIGHT);
                            }
                            break;
                        case Drawing::END:
                            if (active) {
                                if (!lapCache[3].has_value()) {
                                    lapCache[3] = Drawing::Lap();
                                }
                                drawing->setSidelap(Drawing::RIGHT, lapCache[3].value());
                            } else {
                                drawing->removeSidelap(Drawing::RIGHT);
                            }
                            break;
                    }
                });
                bottomLapHint->setLapChangedCallback([this](const Drawing::Lap &lap) {
                    lapCache[3] = lap;
                    switch (drawing->tensionType()) {
                        case Drawing::SIDE:
                            drawing->setOverlap(Drawing::RIGHT, lap);
                            break;
                        case Drawing::END:
                            drawing->setSidelap(Drawing::RIGHT, lap);
                            break;
                    }
                });
                graphicsScene->addItem(bottomLapHint);
            }

            QRectF widthDimensionBounds = QRectF(matRect.left(), totalBounds.top() - sceneSize * dimensionLineOffset,
                                                 matRect.width(), sceneSize * dimensionLineOffset);
            QRectF lengthDimensionBounds = QRectF(totalBounds.left() - sceneSize * dimensionLineOffset, matRect.top(),
                                                  sceneSize * dimensionLineOffset, matRect.height());

            if (widthDimension) {
                widthDimension->setBounds(widthDimensionBounds);
                widthDimension->setLabel(("Width: " + to_str(width)).c_str());
            } else {
                widthDimension = new DimensionLine(widthDimensionBounds, DimensionLine::HORIZONTAL,
                                                   ("Width: " + to_str(width)).c_str());
                graphicsScene->addItem(widthDimension);
            }
            if (lengthDimension) {
                lengthDimension->setBounds(lengthDimensionBounds);
                lengthDimension->setLabel(("Length: " + to_str(length)).c_str());
            } else {
                lengthDimension = new DimensionLine(lengthDimensionBounds, DimensionLine::VERTICAL,
                                                    ("Length: " + to_str(length)).c_str());
                graphicsScene->addItem(lengthDimension);
            }

            if (spacingProxies.size() <= numberOfBars) {
                for (unsigned spacing = spacingProxies.size(); spacing <= numberOfBars; spacing++) {
                    QDoubleSpinBox *spacingInput = new QDoubleSpinBox();
                    spacingInput->setDecimals(1);
                    spacingInput->setMinimum(0);
                    spacingInput->setMaximum(width);
                    spacingInput->setValue(barSpacings[spacing]);

                    connect(spacingInput, qOverload<double>(&QDoubleSpinBox::valueChanged),[this, spacing](double d) {
                        barSpacings[spacing] = (float) d;
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
                    barInput->setMaximum(width);
                    barInput->setValue(barWidths[bar]);

                    connect(barInput, qOverload<double>(&QDoubleSpinBox::valueChanged),[this, bar](double d) {
                        barWidths[bar] = (float) d;
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

            float startPoint = 0;

            for (unsigned bar = 0; bar <= numberOfBars; bar++) {
                float barSpacing = (float) matRect.width() * (barSpacings[bar] / width);
                if (barSpacing == 0) {
                    barSpacing = (float) matRect.width() / (float) (numberOfBars + 1);
                }
                float bx = MIN((float) matRect.left() + startPoint,
                               (float) matRect.right() - sceneSize * matSectionInset);
                float by = matRect.top();
                float bw = MIN(barSpacing, matRect.right() - bx);
                float bh = matRect.height();
                startPoint += bw;

                spacingDimensions[bar]->setBounds(
                        QRectF(bx, by + bh * (1 - barSpacingDimensionHeight), bw, bh * barDimensionHeight));
                barDimensions[bar]->setBounds(QRectF(bx - (float) (bar != 0) * sceneSize * matSectionInset / 2,
                                                     by + bh * (1 - barWidthDimensionHeight),
                                                     sceneSize * matSectionInset, bh * barDimensionHeight));

                bx += (float) ((bar == 0) + 1) * sceneSize * matSectionInset / 2;
                by += sceneSize * matSectionInset;
                bw -= (float) ((bar == 0) + (bar == numberOfBars) + 2) / 2.0f * sceneSize * matSectionInset;
                bh -= sceneSize * matSectionInset * 2;

                matSectionRects[bar]->setRect(bx, by, bw, bh);
            }

            barDimensions.back()->setBounds(QRectF(matRect.right() - sceneSize * matSectionInset,
                                                   matRect.top() + matRect.height() * (1 - barWidthDimensionHeight),
                                                   sceneSize * matSectionInset, matRect.height() * barDimensionHeight));
        }
        update();
    }
}

#pragma clang diagnostic pop