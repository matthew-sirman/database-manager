//
// Created by matthew on 14/07/2020.
//

#ifndef DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
#define DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H

#include <QWidget>

namespace Ui {
    class AddDrawingPageWidget;
}

class AddDrawingPageWidget : public QWidget {
    Q_OBJECT

public:
    explicit AddDrawingPageWidget(QWidget *parent = nullptr);

    ~AddDrawingPageWidget() override;

private:
    Ui::AddDrawingPageWidget *ui = nullptr;
};


#endif //DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
