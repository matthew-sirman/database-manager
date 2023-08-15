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

class AddMaterialPriceWindow : public QDialog {
    Q_OBJECT
public:
    explicit AddMaterialPriceWindow(Client* client, MaterialPricingWindow* caller, int handle, QWidget* parent = nullptr); // New
    explicit AddMaterialPriceWindow(Client* client, MaterialPricingWindow* caller, int handle, ComponentInsert::PriceMode priceMode, std::tuple<float, float, float, MaterialPricingType> pricing, QWidget* parent = nullptr); // Update and Remove
private:
    Ui::AddMaterialPriceWindow* ui = nullptr;

    ComboboxComponentDataSource<Material> materialSource;

    DynamicComboBox* materialNameComboBox;
};