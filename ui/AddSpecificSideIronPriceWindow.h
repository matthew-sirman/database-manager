#ifndef ADDSPECIFICSIDEIRONPRICEWINDOW_H
#define ADDSPECIFICSIDEIRONPRICEWINDOW_H

#include <qdialog.h>

#include "../include/database/DatabaseManager.h"
#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"

namespace Ui {
class AddSpecificSideIronPriceWindow;
}

/// <summary>
/// AddSpecificSideIronPriceWindow inherits QDialog
/// Opens a new window to edit or remove prices for specific side irons.
/// </summary>
class AddSpecificSideIronPriceWindow : public QDialog {
    Q_OBJECT
   public:
    /// <summary>
    /// Creates a dialog to edit, or remove specific side iron prices through.
    /// </summary>
    /// <param name="client">Network client to update the database
    /// through.</param> <param name="sideIron">The side iron the price is
    /// attached to.</param> <param name="priceMode">Enum indicating whether the
    /// price is being edited, or removed. Update is invalid, as the prices
    /// technically already exists, as nulls.</param> <param name="parent">The
    /// parent of this widget.</param>
    explicit AddSpecificSideIronPriceWindow(
        Client* client, SideIron* sideIron,
        ComponentInsert::PriceMode priceMode, QWidget* parent = nullptr);

   private:
    Ui::AddSpecificSideIronPriceWindow* ui = nullptr;

    DynamicComboBox* sideIronComboBox;

    ComboboxComponentDataSource<SideIron> sideIronSource;
};

#endif  // ADDSPECIFICSIDEIRONPRICEWINDOW_H