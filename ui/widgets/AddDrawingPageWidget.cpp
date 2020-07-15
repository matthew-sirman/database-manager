//
// Created by matthew on 14/07/2020.
//

#include "AddDrawingPageWidget.h"
#include "../build/ui_AddDrawingPageWidget.h"

AddDrawingPageWidget::AddDrawingPageWidget(QWidget *parent)
        : QWidget(parent), ui(new Ui::AddDrawingPageWidget) {
    ui->setupUi(this);
}

AddDrawingPageWidget::~AddDrawingPageWidget() {

}
