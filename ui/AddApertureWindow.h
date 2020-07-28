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

class AddApertureWindow : public QDialog {
	Q_OBJECT

public:
	explicit AddApertureWindow(Client *client, QWidget *parent = nullptr);

private:
	Ui::AddApertureWindow *ui = nullptr;

	ComboboxComponentDataSource<ApertureShape> apertureShapeSource;

};

#endif //DATABASE_MANAGER_ADDAPERTUREWINDOW_H