#include "AddSideIronPriceWindow.h"
#include "../build/ui_AddSideIronPriceWindow.h"
#include "SideIronPricingWindow.h"

// adding a price
AddSideIronPriceWindow::AddSideIronPriceWindow(Client* client, SideIronPricingWindow* caller, int index, QWidget* parent)
	: QDialog(parent), ui(new Ui::AddSideIronPriceWindow()) {

	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	sideIronNameComboBox = ui->sideIronNameComboBox;

	sideIronNameComboBox->setManualIndexFunc([index](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(index); });

	sideIronNameComboBox->setDataSource(sideIronPriceSource);
	DrawingComponentManager<SideIronPrice>::addCallback([this]() { sideIronPriceSource.updateSource(); });
	sideIronPriceSource.updateSource();

	QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);

	QLineEdit* lengthEdit = ui->lengthEdit;
	lengthEdit->setText(QString::number(0, 'f', 2));
	QLineEdit* priceEdit = ui->priceEdit;
	priceEdit->setText(QString::number(0, 'f', 2));
	QLineEdit* screwsEdit = ui->priceEdit;
	screwsEdit->setText(QString::number(0, 'f', 0));
	lengthEdit->setValidator(validator);
	priceEdit->setValidator(validator);
	screwsEdit->setValidator(validator);
	QComboBox* extraflex = ui->extraflexCombobox;

	this->setWindowTitle("Add Side Iron Price");
	ui->AddPriceTitleLabel->setText("Add Side Iron Price");

	for (QAbstractButton* button : ui->acceptButtons->buttons()) {
		if (button->text() == "OK") {
			button->setText("Add");
		}
	}

	connect(this, &QDialog::accepted, [lengthEdit, priceEdit, extraflex, caller, client, this, index, screwsEdit]() {
		if (DrawingComponentManager<SideIron>::validComponentHandle(sideIronNameComboBox->currentIndex() && lengthEdit->text().size() > 0 && priceEdit->text().size() > 0)) {

			ComponentInsert insert;

			insert.setComponentData<ComponentInsert::SideIronPriceData>({
				(SideIronType)(sideIronNameComboBox->currentIndex() + 1),
				(bool)(extraflex->currentIndex()),
				lengthEdit->text().toFloat(),
				priceEdit->text().toFloat(),
				screwsEdit->text().toUInt(),
				ComponentInsert::PriceMode::ADD
				
				});

			unsigned bufferSize = insert.serialisedSize();
			void* buffer = alloca(bufferSize);
			insert.serialise(buffer);
			client->addMessageToSendQueue(buffer, bufferSize);

			caller->setComboboxCallback([index](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(index); });
			caller->update(client, index);
		}
		else {
			QMessageBox* box = new QMessageBox(caller);
			box->setText("Invalid inputs, check that all fields are filled in correctly.");
			box->exec();
		}
		});

}

// updating or removing a price
AddSideIronPriceWindow::AddSideIronPriceWindow(Client* client, SideIronPricingWindow* caller, int index, ComponentInsert::PriceMode priceMode, SideIronPrice sideIronPrice, std::tuple<unsigned, float, float, unsigned, bool> pricing, QWidget* parent)
	: QDialog(parent), ui(new Ui::AddSideIronPriceWindow()) {

	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	sideIronNameComboBox = ui->sideIronNameComboBox;

	sideIronNameComboBox->setManualIndexFunc([index](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(index); comboBox->setDisabled(true); });

	sideIronNameComboBox->setDataSource(sideIronPriceSource);
	DrawingComponentManager<SideIronPrice>::addCallback([this]() { sideIronPriceSource.updateSource(); });
	sideIronPriceSource.updateSource();

	QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);

	QLineEdit* lengthEdit = ui->lengthEdit;
	lengthEdit->setText(QString::number(std::get<1>(pricing), 'f', 2));
	QLineEdit* priceEdit = ui->priceEdit;
	priceEdit->setText(QString::number(std::get<2>(pricing), 'f', 2));
	QLineEdit* screwsEdit = ui->screwsEdit;
	screwsEdit->setText(QString::number(std::get<3>(pricing), 'f', 0));
	lengthEdit->setValidator(validator);
	priceEdit->setValidator(validator);
	screwsEdit->setValidator(validator);
	QComboBox* extraflex = ui->extraflexCombobox;
	extraflex->setCurrentIndex((int)std::get<4>(pricing));



	switch (priceMode) {
		case (ComponentInsert::PriceMode::UPDATE):
			this->setWindowTitle("Update Side Iron Price");
			ui->AddPriceTitleLabel->setText("Update Side Iron Price");

			for (QAbstractButton* button : ui->acceptButtons->buttons()) {
				if (button->text() == "OK") {
					button->setText("Update");
				}
			}

			connect(this, &QDialog::accepted, [lengthEdit, priceEdit, screwsEdit, extraflex, caller, client, this, index, pricing]() {
				if (DrawingComponentManager<SideIron>::validComponentHandle(sideIronNameComboBox->currentIndex() && lengthEdit->text().size() > 0 && priceEdit->text().size() > 0)) {

					ComponentInsert insert;

					insert.setComponentData<ComponentInsert::SideIronPriceData>({
						(SideIronType)(sideIronNameComboBox->currentIndex() + 1),
						(bool)extraflex->currentIndex(),
						lengthEdit->text().toFloat(),
						priceEdit->text().toFloat(),
						screwsEdit->text().toUInt(),
						ComponentInsert::PriceMode::UPDATE,
						std::get<0>(pricing)
						});

					unsigned bufferSize = insert.serialisedSize();
					void* buffer = alloca(bufferSize);
					insert.serialise(buffer);
					client->addMessageToSendQueue(buffer, bufferSize);

					caller->setComboboxCallback([index](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(index); });
					caller->update(client, index);
				}
				else {
					QMessageBox* box = new QMessageBox(caller);
					box->setText("Invalid inputs, check that all fields are filled in correctly.");
					box->exec();
				}
				});

			break;

		case (ComponentInsert::PriceMode::REMOVE):
			this->setWindowTitle("Remove Side Iron Price");
			ui->AddPriceTitleLabel->setText("Remove Side Iron Price");

			for (QAbstractButton* button : ui->acceptButtons->buttons()) {
				if (button->text() == "OK") {
					button->setText("Remove");
				}
			}

			lengthEdit->setDisabled(true);
			priceEdit->setDisabled(true);
			screwsEdit->setDisabled(true);

			connect(this, &QDialog::accepted, [lengthEdit, priceEdit, screwsEdit, extraflex, caller, client, this, index, pricing]() {
				if (DrawingComponentManager<SideIron>::validComponentHandle(sideIronNameComboBox->currentIndex() && lengthEdit->text().size() > 0 && priceEdit->text().size() > 0)) {

					ComponentInsert insert;

					insert.setComponentData<ComponentInsert::SideIronPriceData>({
						(SideIronType)(sideIronNameComboBox->currentIndex() + 1),
						(bool)extraflex->currentIndex(),
						lengthEdit->text().toFloat(),
						priceEdit->text().toFloat(),
						screwsEdit->text().toUInt(),
						ComponentInsert::PriceMode::REMOVE,
						std::get<0>(pricing)
						});

					unsigned bufferSize = insert.serialisedSize();
					void* buffer = alloca(bufferSize);
					insert.serialise(buffer);
					client->addMessageToSendQueue(buffer, bufferSize);

					caller->setComboboxCallback([index](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(index); });
					caller->update(client, index);
				}
				else {
					QMessageBox* box = new QMessageBox(caller);
					box->setText("Invalid inputs, check that all fields are filled in correctly.");
					box->exec();
				}
				});

			break;
		default:
			break;
	}
}