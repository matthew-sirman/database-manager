//
// Created by matthew on 27/07/2020.
//

#ifndef DATABASE_MANAGER_ADDSIDEIRONWINDOW_H
#define DATABASE_MANAGER_ADDSIDEIRONWINDOW_H

#include <QDialog>
#include <QFileDialog>

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"

namespace Ui {
	class AddSideIronWindow;
}

class AddSideIronWindow : public QDialog {
	Q_OBJECT

public:
	AddSideIronWindow(Client *client, QWidget *parent = nullptr);

private:
	Ui::AddSideIronWindow *ui;
};

#endif //DATABASE_MANAGER_ADDSIDEIRONWINDOW_H
