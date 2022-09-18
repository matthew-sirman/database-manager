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
    class AddExtraPriceWindow;
}

class ExtraPricingWindow;

class AddExtraPriceWindow : public QDialog {
    Q_OBJECT
public:
    explicit AddExtraPriceWindow(Client* client, ExtraPricingWindow* caller, ExtraPrice& price, int index, QWidget* parent = nullptr); // Update
private:
    Ui::AddExtraPriceWindow* ui = nullptr;
};