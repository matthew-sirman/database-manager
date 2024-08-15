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

/// <summary>
/// AddSideIronWindow inherits QDialog
/// Opens a dialog where a new side iron can be defined, then added to the database.
/// </summary>
class AddSideIronWindow : public QDialog {
	Q_OBJECT

public:
	/// <summary>
	/// Constructs a dialog to add a new side iron.
	/// </summary>
	/// <param name="client">Network client to update database through.</param>
	/// <param name="parent">The parent of this widget.</param>
	AddSideIronWindow(Client *client, QWidget *parent = nullptr);

private:
	Ui::AddSideIronWindow *ui;
};

#endif //DATABASE_MANAGER_ADDSIDEIRONWINDOW_H
