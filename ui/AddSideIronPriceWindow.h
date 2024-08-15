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
    class AddSideIronPriceWindow;
}

class SideIronPricingWindow;

/// <summary>
/// AddSideIronPriceWindow inherits QDialog
/// Opens a new window in which a new side iron price can be defined, based on generic parameters, then added to the database.
/// </summary>
class AddSideIronPriceWindow : public QDialog {
    Q_OBJECT
public:
    /// <summary>
    /// Opens a new dialog for adding a new side iron price to the database.
    /// </summary>
    /// <param name="client">The network client to update the database through.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit AddSideIronPriceWindow(Client *client, QWidget* parent = nullptr); // New
    /// <summary>
    /// Opens a new dialog for editing or removing a side iron price from the database.
    /// </summary>
    /// <param name="client">The network client to update the database through.</param>
    /// <param name="priceMode">Indicates whether the price is being edited or deleted.</param>
    /// <param name="price">The current price of the side iron.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit AddSideIronPriceWindow(Client* client, ComponentInsert::PriceMode priceMode, const SideIronPrice& price, QWidget* parent = nullptr); // update/remove
private:
    Ui::AddSideIronPriceWindow* ui = nullptr;
};