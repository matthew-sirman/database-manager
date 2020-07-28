//
// Created by matthew on 27/07/2020.
//

#ifndef DATABASE_MANAGER_ADDMACHINEWINDOW_H
#define DATABASE_MANAGER_ADDMACHINEWINDOW_H

#include <QDialog>

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"

namespace Ui {
	class AddMachineWindow;
}

class AddMachineWindow : public QDialog {
	Q_OBJECT

public:
	AddMachineWindow(Client *client, QWidget *parent = nullptr);

private:
	Ui::AddMachineWindow *ui;
};

#endif //DATABASE_MANAGER_ADDMACHINEWINDOW_H