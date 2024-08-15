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

/// <summary>
/// AddMaterialWindow inherits QDialog
/// Opens a new window in which a new material can be defined, then added to the database.
/// </summary>
class AddMaterialWindow : public QDialog {
	Q_OBJECT

public:
	/// <summary>
	/// Constructs a new window for the addition of materials.
	/// </summary>
	/// <param name="client">The network client to update the database through the server.</param>
	/// <param name="parent">The parent of this widget.</param>
	AddMaterialWindow(Client *client, QWidget *parent = nullptr);

private:
	Ui::AddMaterialWindow *ui;
};

#endif //DATABASE_MANAGER_ADDMATERIALWINDOW_H