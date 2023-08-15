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
    class AddLabourTimesWindow;
}

class LabourTimesWindow;

class AddLabourTimesWindow : public QDialog {
    Q_OBJECT
public:
    explicit AddLabourTimesWindow(Client* client, LabourTime& times, int index, QWidget* parent = nullptr); // Update
private:
    Ui::AddLabourTimesWindow* ui = nullptr;
signals:
    void updateParent(QString l);
};