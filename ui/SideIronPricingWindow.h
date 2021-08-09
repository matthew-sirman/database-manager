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
#include <QFormLayout>
#include <QInputEvent>

namespace Ui {
    class SideIronPricingWindow;
}

class SideIronPricingWindow : public QDialog {
    Q_OBJECT
public:
    explicit SideIronPricingWindow(Client* client, QWidget* parent = nullptr);

    void update(Client* client, int index);

    void setComboboxCallback(std::function<void(DynamicComboBox*)>);
private:
    Ui::SideIronPricingWindow* ui = nullptr;

    QWidget* sideIronPricingScroll;

    DynamicComboBox* sideIronComboBox;

    ComboboxComponentDataSource<SideIronPrice> sideIronPriceSource;
};
#endif // SIDEIRONPRICINGWINDOW_H
