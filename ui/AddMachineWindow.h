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

/// <summary>
/// AddMachineWindow inherits QDialog
/// Opens a new window in which a new machine can be defined, then added to the database.
/// </summary>
class AddMachineWindow : public QDialog {
	Q_OBJECT

public:
	/// <summary>
	/// Constructs a new window for the addition of machines.
	/// </summary>
	/// <param name="client">The network client to update the database through the server.</param>
	/// <param name="parent">The parent of this widget.</param>
	AddMachineWindow(Client *client, QWidget *parent = nullptr);

private:
	Ui::AddMachineWindow *ui;
};

#endif //DATABASE_MANAGER_ADDMACHINEWINDOW_H