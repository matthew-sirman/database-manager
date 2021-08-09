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

class AddSideIronPriceWindow : public QDialog {
    Q_OBJECT
public:
    explicit AddSideIronPriceWindow(Client* client, SideIronPricingWindow* caller, int index, QWidget* parent = nullptr); // New
    explicit AddSideIronPriceWindow(Client* client, SideIronPricingWindow* caller, int index, ComponentInsert::PriceMode priceMode, SideIronPrice sideIronPrice, std::tuple<unsigned, float, float, unsigned, bool> pricing, QWidget* parent = nullptr); // Update and Remove
private:
    Ui::AddSideIronPriceWindow* ui = nullptr;

    ComboboxComponentDataSource<SideIronPrice> sideIronPriceSource;

    DynamicComboBox* sideIronNameComboBox;
};