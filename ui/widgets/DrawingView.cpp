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
        graphicsScene->setSceneRect(-(sceneWidth / 2), -(sceneHeight / 2), sceneWidth, sceneHeight);
        fitInView(graphicsScene->sceneRect(), Qt::KeepAspectRatio);
    }

    QGraphicsView::paintEvent(event);
}

void DrawingView::redrawScene() {
    QGraphicsScene *graphicsScene = scene();

    if (graphicsScene && drawing) {
        graphicsScene->clear();

        float width = drawing->width(), length = drawing->length();

        if (width != 0 && length != 0) {
            float w, h;
            if (width / (float) size().width() > length / (float) size().height()) {
                w = sceneWidth * maxMatDimensionPercentage, h = sceneHeight * maxMatDimensionPercentage *
                                                                (length / width);
            } else {
                w = sceneWidth * maxMatDimensionPercentage * (width / length), h = sceneHeight *
                                                                                   maxMatDimensionPercentage;
            }
            graphicsScene->addRect(-w / 2, -h / 2, w, h);

            graphicsScene->addItem(new DimensionLine(QRectF(-w / 2, -h / 2 - sceneHeight * dimensionLineOffset,
                                                            w, sceneHeight * dimensionLineOffset),
                                                     DimensionLine::HORIZONTAL,
                                                     ("Width: " + to_str(width)).c_str()));
            graphicsScene->addItem(new DimensionLine(QRectF(-w / 2 - sceneWidth * dimensionLineOffset, -h / 2,
                                                            sceneWidth * dimensionLineOffset, h),
                                                     DimensionLine::VERTICAL,
                                                     ("Length: " + to_str(length)).c_str()));

            float startPoint = 0;//w * matSectionInsetW / 2;

            for (unsigned bar = 0; bar <= numberOfBars; bar++) {
                float barSpacing = w * (barSpacings[bar] / width);
                if (barSpacing == 0) {
                    barSpacing = w / (float) (numberOfBars + 1);
                }
                float bx = MIN(startPoint - w / 2, w * (1 - matSectionInsetW) / 2);
                float by = -h / 2;
                float bw = MIN(barSpacing, w / 2 - bx);
                float bh = h;
                startPoint += bw;

                QDoubleSpinBox *spacingInput = new QDoubleSpinBox();
                spacingInput->setDecimals(1);
                spacingInput->setMinimum(0);
                spacingInput->setMaximum(width);
                spacingInput->setValue(barSpacings[bar]);

                connect(spacingInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
                        [this, bar](double d) { barSpacings[bar] = (float) d; });

                QGraphicsProxyWidget *spacingProxy = graphicsScene->addWidget(spacingInput);
                spacingProxy->setScale(2);
                spacingProxy->setZValue(1);

                graphicsScene->addItem(new WidgetDimensionLine(
                        QRectF(bx,by + bh * (1 - barSpacingDimensionHeight),
                               bw,bh * barDimensionHeight),
                        DimensionLine::HORIZONTAL, spacingProxy));

                QDoubleSpinBox *barInput = new QDoubleSpinBox();
                barInput->setDecimals(1);
                barInput->setMinimum(0);
                barInput->setMaximum(width);
                barInput->setValue(barWidths[bar]);

                connect(barInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
                        [this, bar](double d) { barWidths[bar] = (float) d; });

                QGraphicsProxyWidget *barProxy = graphicsScene->addWidget(barInput);
                barProxy->setScale(2);
                barProxy->setZValue(1);

                graphicsScene->addItem(new WidgetDimensionLine(
                        QRectF(bx - (float)(bar != 0) * w * matSectionInsetW / 2,
                                by + bh * (1 - barWidthDimensionHeight),
                                w * matSectionInsetW, bh * barDimensionHeight),
                        DimensionLine::HORIZONTAL, barProxy));

                bx += w * (float)((bar == 0) + 1) * matSectionInsetW / 2;
                by += h * matSectionInsetH / 2;
                bw -= w * (float)((bar == 0) + (bar == numberOfBars) + 2) / 2.0f * matSectionInsetW;
                bh -= h * matSectionInsetH;

                graphicsScene->addRect(bx, by, bw, bh);
            }

            QDoubleSpinBox *barInput = new QDoubleSpinBox();
            barInput->setDecimals(1);
            barInput->setMinimum(0);
            barInput->setMaximum(width);
            barInput->setValue(barWidths.back());

            connect(barInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
                    [this](double d) { barWidths.back() = (float) d; });

            QGraphicsProxyWidget *barProxy = graphicsScene->addWidget(barInput);
            barProxy->setScale(2);
            barProxy->setZValue(1);

            graphicsScene->addItem(new WidgetDimensionLine(
                    QRectF(w / 2 - w * matSectionInsetW, -h / 2 + h * (1 - barWidthDimensionHeight),
                           w * matSectionInsetW, h * barDimensionHeight),
                    DimensionLine::HORIZONTAL, barProxy));
        }
    }

    update();
}

#pragma clang diagnostic pop