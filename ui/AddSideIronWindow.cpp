//
// Created by matthew on 27/07/2020.
//

#include "AddSideIronWindow.h"
#include "../build/ui_AddSideIronWindow.h"

AddSideIronWindow::AddSideIronWindow(Client *client, QWidget *parent)
	: QDialog(parent), ui(new Ui::AddSideIronWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	connect(ui->openDrawingHyperlinkButton, &QPushButton::clicked, [this]() {
		const std::filesystem::path hyperlinkFile = QFileDialog::getOpenFileName(this, "Select a Side Iron Drawing PDF File", QString(), "PDF (*.pdf)").toStdString();
		if (!hyperlinkFile.empty()) {
			ui->drawingHyperlinkDisplay->setText(hyperlinkFile.string().c_str());
			ui->drawingHyperlinkDisplay->setToolTip(hyperlinkFile.string().c_str());
			
			ui->drawingNumberDisplay->setText(hyperlinkFile.filename().replace_extension().string().c_str());
		}
	});

	connect(this, &QDialog::finished, [this, client](int result) {
		switch ((DialogCode)result) {
			case DialogCode::Accepted: {
				ComponentInsert insert;
				SideIronType siType = SideIronType::None;

				if (ui->typeInput->currentText() == "A") {
					siType = SideIronType::A;
				} else if (ui->typeInput->currentText() == "B") {
					siType = SideIronType::B;
				} else if (ui->typeInput->currentText() == "C") {
					siType = SideIronType::C;
				} else if (ui->typeInput->currentText() == "D") {
					siType = SideIronType::D;
				} else if (ui->typeInput->currentText() == "E") {
					siType = SideIronType::E;
				}

				insert.setComponentData<ComponentInsert::SideIronData>({
					siType,
					(unsigned) ui->lengthInput->value(),
					ui->drawingNumberDisplay->text().toStdString(),
					ui->drawingHyperlinkDisplay->text().toStdString(),
                                     ui->extraflexCheckbox->isChecked()
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