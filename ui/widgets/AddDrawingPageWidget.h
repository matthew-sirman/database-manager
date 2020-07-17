//
// Created by matthew on 14/07/2020.
//

#ifndef DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
#define DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H

#include <QWidget>
#include <QGraphicsScene>

#include "../../include/database/Drawing.h"

namespace Ui {
    class AddDrawingPageWidget;
}

class AddDrawingPageWidget : public QWidget {
    Q_OBJECT

public:
    // TODO: Constructor from drawing (clone)

    explicit AddDrawingPageWidget(QWidget *parent = nullptr);

    ~AddDrawingPageWidget() override;

    void setupComboboxSources();

    void setupActivators();

    void setupDrawingUpdateConnections();

private:
    Ui::AddDrawingPageWidget *ui = nullptr;

    ComboboxComponentDataSource<Product> productSource;
    ComboboxComponentDataSource<Aperture> apertureSource;
    ComboboxComponentDataSource<Material> materialSource;
    ComboboxComponentDataSource<SideIron> sideIronSource;
    ComboboxComponentDataSource<Machine> machineSource;
    ComboboxComponentDataSource<MachineDeck> machineDeckSource;

    QGraphicsScene *visualsScene = nullptr;

    Drawing drawing;

    const QRegExp drawingNumberRegex = QRegExp("^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3}[a-zA-Z]?)$");

private slots:
    void capitaliseLineEdit(const QString &text);
};


#endif //DATABASE_MANAGER_ADDDRAWINGPAGEWIDGET_H
