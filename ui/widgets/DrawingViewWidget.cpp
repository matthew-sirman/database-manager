//
// Created by matthew on 14/07/2020.
//

#include "DrawingViewWidget.h"
#include "../build/ui_DrawingViewWidget.h"

DrawingViewWidget::DrawingViewWidget(QWidget *parent)
        : QWidget(parent), ui(new Ui::DrawingViewWidget()) {
    ui->setupUi(this);
}

DrawingViewWidget::~DrawingViewWidget() {

}
