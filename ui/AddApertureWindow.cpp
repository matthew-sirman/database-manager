//
// Created by matthew on 26/07/2020.
//

#include "AddApertureWindow.h"
#include "../build/ui_AddApertureWindow.h"

AddApertureWindow::AddApertureWindow(Client *client, QWidget *parent)
	: QDialog(parent), ui(new Ui::AddApertureWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	DrawingComponentManager<ApertureShape>::addCallback([this]() { apertureShapeSource.updateSource(); });
	DrawingComponentManager<Aperture>::addCallback([this]() { apertureSource.updateSource(); });
	apertureShapeSource.updateSource();
	apertureSource.updateSource();

	ui->apertureShapeInput->setDataSource(apertureShapeSource);
	ui->nibbleApertureInput->setDataSource(apertureSource);

	ui->nibbleApertureLabel->setActive(false);

	ui->nibbleApertureLabel->addTarget(ui->nibbleApertureInput);


	connect(this, &QDialog::finished, [this, client](int result) {
		switch ((DialogCode)result) {
			case DialogCode::Accepted: {
				ComponentInsert insert;
				if (ui->nibbleApertureLabel->active()) {
					insert.setComponentData<ComponentInsert::ApertureData>({
						(float)ui->widthInput->value(),
						(float)ui->lengthInput->value(),
						(unsigned)ui->baseWidthInput->value(),
						(unsigned)ui->baseLengthInput->value(),
						(unsigned)ui->quantityInput->value(),
						DrawingComponentManager<ApertureShape>::getComponentByHandle(ui->apertureShapeInput->currentData().toInt()).componentID(),
						ui->nibbleApertureLabel->active() ? std::make_optional(DrawingComponentManager<Aperture>::getComponentByHandle(ui->nibbleApertureInput->currentData().toInt()).componentID()) : std::nullopt
					});
				}
				else {
					insert.setComponentData<ComponentInsert::ApertureData>({
	(float)ui->widthInput->value(),
	(float)ui->lengthInput->value(),
	(unsigned)ui->baseWidthInput->value(),
	(unsigned)ui->baseLengthInput->value(),
	(unsigned)ui->quantityInput->value(),
	DrawingComponentManager<ApertureShape>::getComponentByHandle(ui->apertureShapeInput->currentData().toInt()).componentID(),
	(bool)ui->nibbleApertureLabel->active()
});
				}
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