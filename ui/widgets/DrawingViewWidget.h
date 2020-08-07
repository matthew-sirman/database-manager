//
// Created by matthew on 14/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGVIEWWIDGET_H
#define DATABASE_MANAGER_DRAWINGVIEWWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QPdfDocument>
#include <QPdfView>
#include <QDesktopServices>

#include <filesystem>

#include "AddDrawingPageWidget.h"
#include "../../include/database/Drawing.h"

#define PUNCH_PDF_LOCATION "T:/Drawings/2. Rubber Screen Cloths/Punch Program PDF's"

namespace Ui {
    class DrawingViewWidget;
}

class DrawingViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit DrawingViewWidget(const Drawing &drawing, QWidget *parent = nullptr);

    ~DrawingViewWidget() override;

    void updateFields();

    void setChangeDrawingCallback(const std::function<void(AddDrawingPageWidget::AddDrawingMode)> &callback);

private:
    Ui::DrawingViewWidget *ui;

    const Drawing *drawing;

    std::function<void(AddDrawingPageWidget::AddDrawingMode)> changeDrawingCallback = nullptr;

    QPdfView *pdfViewer = nullptr;
    QPdfDocument *pdfDocument = nullptr;

    static std::filesystem::path punchProgramPathForDrawing(const std::string &drawingNumber);
};


#endif //DATABASE_MANAGER_DRAWINGVIEWWIDGET_H
