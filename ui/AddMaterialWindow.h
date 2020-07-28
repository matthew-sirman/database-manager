//
// Created by matthew on 27/07/2020.
//

#ifndef DATABASE_MANAGER_ADDMATERIALWINDOW_H
#define DATABASE_MANAGER_ADDMATERIALWINDOW_H

#include <QDialog>

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"

namespace Ui {
	class AddMaterialWindow;
}

class AddMaterialWindow : public QDialog {
	Q_OBJECT

public:
	AddMaterialWindow(Client *client, QWidget *parent = nullptr);

private:
	Ui::AddMaterialWindow *ui;
};

#endif //DATABASE_MANAGER_ADDMATERIALWINDOW_H