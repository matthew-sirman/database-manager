//
// Created by matthew on 26/07/2020.
//

#ifndef DATABASE_MANAGER_ADDAPERTUREWINDOW_H
#define DATABASE_MANAGER_ADDAPERTUREWINDOW_H

#include <QDialog>

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"

namespace Ui {
	class AddApertureWindow;
}

/// <summary>
/// AddApertureWindow inherits QDialog
/// Dialog that lets the user create a new aperture.
/// </summary>
class AddApertureWindow : public QDialog {
	Q_OBJECT

public:
	/// <summary>
	/// Creates a new dialog, that, if accepted, will create a new aperture in the database.
	/// </summary>
	/// <param name="client">The TCP client for updating the database.</param>
	/// <param name="parent">The parent of this widget for qt's management.</param>
	explicit AddApertureWindow(Client *client, QWidget *parent = nullptr);

private:
	Ui::AddApertureWindow *ui = nullptr;

	ComboboxComponentDataSource<ApertureShape> apertureShapeSource;
	ComboboxComponentDataSource<Aperture> apertureSource;

};

#endif //DATABASE_MANAGER_ADDAPERTUREWINDOW_H