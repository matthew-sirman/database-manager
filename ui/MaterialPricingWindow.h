#ifndef MATERIALPRICINGWINDOW_H
#define MATERIALPRICINGWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "AddMaterialPriceWindow.h"
#include "widgets/DynamicComboBox.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QInputEvent>

namespace Ui {
    class MaterialPricingWindow;
}

class MaterialPricingWindow : public QDialog {
    Q_OBJECT
public:
    explicit MaterialPricingWindow(Client* client, QWidget* parent = nullptr);

    void update(Client* client);

    void setComboboxCallback(std::function<void(DynamicComboBox*)>);

    void updateSource();
private:
    Ui::MaterialPricingWindow* ui = nullptr;

    QWidget* materialPricingScroll;

    DynamicComboBox* materialComboBox;

    ComboboxComponentDataSource<Material> materialSource;
};

#endif // MATERIALPRICINGWINDOW_H
