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
    explicit AddSideIronPriceWindow(Client *client, QWidget* parent = nullptr); // New
    explicit AddSideIronPriceWindow(Client* client, ComponentInsert::PriceMode priceMode, const SideIronPrice& price, QWidget* parent = nullptr); // update/remove
private:
    Ui::AddSideIronPriceWindow* ui = nullptr;
};