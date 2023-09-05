#ifndef SIDEIRONPRICINGWINDOW_H
#define SIDEIRONPRICINGWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "AddSideIronPriceWindow.h"
#include "widgets/DynamicComboBox.h"

#include <QDialog>

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QFormLayout>
#include <QInputEvent>
#include <QCheckBox>

namespace Ui {
    class SideIronPricingWindow;
}

class SideIronPricingWindow : public QDialog {
    Q_OBJECT
public:
    explicit SideIronPricingWindow(Client* client, QWidget* parent = nullptr);

    void update();
private:
    Ui::SideIronPricingWindow* ui = nullptr;

    QWidget* sideIronPricingScroll;
    Client* client;
};
#endif // SIDEIRONPRICINGWINDOW_H
