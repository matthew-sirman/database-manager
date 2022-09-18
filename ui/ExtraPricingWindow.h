#ifndef EXTRAPRICINGWINDOW_H
#define EXTRAPRICINGWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/database/ExtraPriceManager.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"
#include "AddExtraPriceWindow.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QInputEvent>

namespace Ui {
    class ExtraPricingWindow;
}
class ExtraPricingWindow : public QDialog {
    Q_OBJECT
public:
    explicit ExtraPricingWindow(Client* client, QWidget* parent = nullptr);

    void update(Client* client);

    void setComboboxCallback(std::function<void(DynamicComboBox*)>);

    void updateSource();
private:
    Ui::ExtraPricingWindow* ui = nullptr;

    QWidget* extraPricingScroll;

    DynamicComboBox* extraPriceComboBox;

    ComboboxComponentDataSource<ExtraPrice> extraPriceSource;
};

#endif // EXTRAPRICINGWINDOW_H
