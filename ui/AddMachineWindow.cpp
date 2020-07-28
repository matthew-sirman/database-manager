//
// Created by matthew on 27/07/2020.
//

#include "AddMachineWindow.h"
#include "../build/ui_AddMachineWindow.h"

AddMachineWindow::AddMachineWindow(Client *client, QWidget *parent)
	: QDialog(parent), ui(new Ui::AddMachineWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	connect(this, &QDialog::finished, [this, client](int result) {
		switch ((DialogCode)result) {
			case DialogCode::Accepted: {
				ComponentInsert insert;
				insert.setComponentData<ComponentInsert::MachineData>({ ui->manufacturerInput->text().toStdString(), ui->modelInput->text().toStdString() });
				unsigned bufferSize = insert.serialisedSize();
				void *buffer = alloca(bufferSize);
				insert.serialise(buffer);
				client->addMessageToSendQueue(buffer, bufferSize);
				break;
			}
			case DialogCode::Rejected:
				break;
		}
	});
}