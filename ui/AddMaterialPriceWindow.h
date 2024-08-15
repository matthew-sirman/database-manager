#pragma once

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>

namespace Ui {
    class AddMaterialPriceWindow;
}

class MaterialPricingWindow;

/// <summary>
/// AddMaterialPriceWindow inherits QDialog
/// Opens a new window in which a new material price can be defined, then added to the database.
/// </summary>
class AddMaterialPriceWindow : public QDialog {
    Q_OBJECT
public:
    /// <summary>
    /// Opens a dialog to allow a new material price to be added to the database.
    /// </summary>
    /// <param name="client">The network client to send the new material price to the database through.</param>
    /// <param name="caller">The pricing window that called for the new material, to update the prices shown.</param>
    /// <param name="handle">The handle of the material the price will be attached to.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit AddMaterialPriceWindow(Client* client, MaterialPricingWindow* caller, int handle, QWidget* parent = nullptr); // New
    /// <summary>
    /// Opens a dialog to allow a material price to be changed or removed from the database.
    /// </summary>
    /// <param name="client">The network client to send the new material price to the database through.</param>
    /// <param name="caller">The pricing window that called for the new material, to update the prices shown.</param>
    /// <param name="handle">The handle of the material the price will be attached to.</param>
    /// <param name="priceMode">Indicates whether this dialog is to edit or remove.</param>
    /// <param name="pricing">The current price of the material.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit AddMaterialPriceWindow(Client* client, MaterialPricingWindow* caller, int handle, ComponentInsert::PriceMode priceMode, Material::MaterialPrice pricing, QWidget* parent = nullptr); // Update and Remove
private:
    Ui::AddMaterialPriceWindow* ui = nullptr;

    ComboboxComponentDataSource<Material> materialSource;

    DynamicComboBox* materialNameComboBox;
};