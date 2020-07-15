//
// Created by matthew on 14/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGVIEWWIDGET_H
#define DATABASE_MANAGER_DRAWINGVIEWWIDGET_H

#include <QWidget>

namespace Ui {
    class DrawingViewWidget;
}

class DrawingViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit DrawingViewWidget(QWidget *parent = nullptr);

    ~DrawingViewWidget() override;

private:
    Ui::DrawingViewWidget *ui;
};


#endif //DATABASE_MANAGER_DRAWINGVIEWWIDGET_H
