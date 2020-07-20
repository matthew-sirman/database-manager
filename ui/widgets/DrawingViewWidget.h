//
// Created by matthew on 14/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGVIEWWIDGET_H
#define DATABASE_MANAGER_DRAWINGVIEWWIDGET_H

#include <QWidget>
#include <QMessageBox>

#include "../../include/database/Drawing.h"

namespace Ui {
    class DrawingViewWidget;
}

class DrawingViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit DrawingViewWidget(const Drawing &drawing, QWidget *parent = nullptr);

    ~DrawingViewWidget() override;

    void updateFields();

private:
    Ui::DrawingViewWidget *ui;

    const Drawing *drawing;

//    QPdfView *pdfViewer= nullptr;
};


#endif //DATABASE_MANAGER_DRAWINGVIEWWIDGET_H
