//
// Created by matthew on 27/07/2020.
//

#include "AddMaterialWindow.h"
#include "../build/ui_AddMaterialWindow.h"

AddMaterialWindow::AddMaterialWindow(Client *client, QWidget *parent)
	: QDialog(parent), ui(new Ui::AddMaterialWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	connect(this, &QDialog::finished, [this, client](int result) {
		switch ((DialogCode)result) {
			case DialogCode::Accepted: {
				ComponentInsert insert;
				insert.setComponentData<ComponentInsert::MaterialData>({ 
					ui->materialInput->currentText().toStdString(), 
					(unsigned) ui->hardnessInput->value(),
					(unsigned) ui->thicknessInput->value()
				});
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