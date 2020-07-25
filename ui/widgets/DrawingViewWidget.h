//
// Created by matthew on 14/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGVIEWWIDGET_H
#define DATABASE_MANAGER_DRAWINGVIEWWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QPdfDocument>
#include <QPdfView>

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

    void setUpdateDrawingCallback(const std::function<void()> &callback);

private:
    Ui::DrawingViewWidget *ui;

    const Drawing *drawing;

    std::function<void()> updateDrawingCallback = nullptr;

    QPdfView *pdfViewer = nullptr;
    QPdfDocument *pdfDocument = nullptr;
};


#endif //DATABASE_MANAGER_DRAWINGVIEWWIDGET_H
