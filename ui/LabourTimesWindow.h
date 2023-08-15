#ifndef LABOURTIMESWINDOW_H
#define LABOURTIMESWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/database/ExtraPriceManager.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"
#include "AddLabourTimesWindow.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QInputEvent>

namespace Ui {
    class LabourTimesWindow;
}
class LabourTimesWindow : public QDialog {
    Q_OBJECT
public:

    LabourTimesWindow(Client* client, QWidget* parent = nullptr);

    void update(Client* client);

    void setComboboxCallback(std::function<void(DynamicComboBox*)>);
private:
    Ui::LabourTimesWindow* ui = nullptr;

    QWidget* labourTimesScroll;

    DynamicComboBox* labourTimesComboBox;

    ComboboxComponentDataSource<LabourTime> labourTimesSource;
    
};

#endif // LABOURTIMESWINDOW_H