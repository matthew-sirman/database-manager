#ifndef ADDSPECIFICSIDEIRONPRICEWINDOW_H
#define ADDSPECIFICSIDEIRONPRICEWINDOW_H

#include <qdialog.h>

#include "../include/networking/Client.h"
#include "../include/database/DatabaseManager.h"
#include "widgets/DynamicComboBox.h"
#include "../include/database/DatabaseQuery.h"

namespace Ui {
    class AddSpecificSideIronPriceWindow;
}

class AddSpecificSideIronPriceWindow : public QDialog {
    Q_OBJECT
public:
    explicit AddSpecificSideIronPriceWindow(Client* client, SideIron* sideIron, ComponentInsert::PriceMode priceMode, QWidget* parent = nullptr);
private:
    Ui::AddSpecificSideIronPriceWindow* ui = nullptr;

    DynamicComboBox* sideIronComboBox;

    ComboboxComponentDataSource<SideIron> sideIronSource;
signals:
    void updateParent();
};

#endif // ADDSPECIFICSIDEIRONPRICEWINDOW_H