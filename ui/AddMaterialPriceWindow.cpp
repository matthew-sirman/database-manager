#include "AddMaterialPriceWindow.h"
#include "../build/ui_AddMaterialPriceWindow.h"
#include "MaterialPricingWindow.h"

// adding a price
AddMaterialPriceWindow::AddMaterialPriceWindow(Client* client, MaterialPricingWindow* caller, int handle, QWidget* parent)
	: QDialog(parent), ui(new Ui::AddMaterialPriceWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	materialNameComboBox = ui->materialNameComboBox;

	materialNameComboBox->setDataSource(materialSource);

	materialNameComboBox->setManualIndexFunc([handle](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(comboBox->findData(handle)); });

	DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
	materialSource.updateSource();
    connect(materialNameComboBox, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this, client](int index){
		std::cout << materialNameComboBox->currentIndex() << " " << materialNameComboBox->currentText().toStdString() << std::endl;
        });
	QComboBox* priceTypeComboBox = ui->priceTypeComboBox;
	priceTypeComboBox->addItem("Running Metre");
	priceTypeComboBox->addItem("Square Metre");
	priceTypeComboBox->addItem("Sheet");

	QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);

	QLineEdit* widthEdit = ui->widthEdit;
	widthEdit->setText(QString::number(0, 'f', 2));
	QLineEdit* lengthEdit = ui->lengthEdit;
	lengthEdit->setDisabled(true);
	QLineEdit* priceEdit = ui->priceEdit;
	priceEdit->setText(QString::number(0, 'f', 2));
	widthEdit->setValidator(validator);
	lengthEdit->setValidator(validator);
	priceEdit->setValidator(validator);

	connect(priceTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [lengthEdit](int index) {
		if (index == 2) {
			lengthEdit->setText(QString::number(0, 'f', 2));
			lengthEdit->setDisabled(false);
		}
		else {
			lengthEdit->setText("");
			lengthEdit->setDisabled(true);
		}
		});

	this->setWindowTitle("Add Material Price");
	ui->AddPriceTitleLabel->setText("Add Material Price");

	for (QAbstractButton* button : ui->acceptButtons->buttons()) {
		if (button->text() == "OK") {
			button->setText("Add");
		}
	}

	connect(this, &QDialog::accepted, [widthEdit, lengthEdit, priceEdit, caller, client, this, priceTypeComboBox, handle]() {
		materialSource.updateSource();
		if (DrawingComponentManager<Material>::validComponentHandle(materialNameComboBox->currentIndex() && widthEdit->text().size() > 0 && priceEdit->text().size() > 0)) {

			ComponentInsert insert;

			if (priceTypeComboBox->currentText() == "Running Metre") {

				insert.setComponentData<ComponentInsert::MaterialPriceData>({
					0,
					DrawingComponentManager<Material>::getComponentByHandle(handle).componentID(),
					widthEdit->text().toFloat(),
					lengthEdit->text().toFloat(),
					priceEdit->text().toFloat(),
					MaterialPricingType::RUNNING_M,
					ComponentInsert::PriceMode::ADD
					});
			}
			else if (priceTypeComboBox->currentText() == "Square Metre") {
				insert.setComponentData<ComponentInsert::MaterialPriceData>({
					0,
					DrawingComponentManager<Material>::getComponentByHandle(handle).componentID(),
					widthEdit->text().toFloat(),
					lengthEdit->text().toFloat(),
					priceEdit->text().toFloat(),
					MaterialPricingType::SQUARE_M,
					ComponentInsert::PriceMode::ADD
					});
			}
			else if (priceTypeComboBox->currentText() == "Sheet") {
				insert.setComponentData<ComponentInsert::MaterialPriceData>({
					0,
					DrawingComponentManager<Material>::getComponentByHandle(handle).componentID(),
					widthEdit->text().toFloat(),
					lengthEdit->text().toFloat(),
					priceEdit->text().toFloat(),
					MaterialPricingType::SHEET,
					ComponentInsert::PriceMode::ADD
					});
			}
			unsigned bufferSize = insert.serialisedSize();
			void* buffer = alloca(bufferSize);
			insert.serialise(buffer);
			client->addMessageToSendQueue(buffer, bufferSize);

			//caller->setComboboxCallback([handle, caller, client](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(comboBox->findData(handle)); caller->update(client); });
			//caller->updateSource();
			//caller->update(client);
		}
		else {
			QMessageBox* box = new QMessageBox(caller);
			box->setText("Invalid inputs, check that all fields are filled in correctly.");
			box->exec();
		}
		});

}


// updating or removing a price
AddMaterialPriceWindow::AddMaterialPriceWindow(Client* client, MaterialPricingWindow* caller, int handle, ComponentInsert::PriceMode priceMode, Material::MaterialPrice pricing, QWidget* parent)
	: QDialog(parent), ui(new Ui::AddMaterialPriceWindow()) {

	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	materialNameComboBox = ui->materialNameComboBox;

	materialNameComboBox->setManualIndexFunc([handle](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(comboBox->findData(handle)); comboBox->setDisabled(true); });

	materialNameComboBox->setDataSource(materialSource);



	DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
	materialSource.updateSource();

	QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
	QComboBox* priceTypeComboBox = ui->priceTypeComboBox;
	


	QLineEdit* widthEdit = ui->widthEdit;
	widthEdit->setValidator(validator);
	widthEdit->setText(QString::number(std::get<1>(pricing), 'f', 2));

	QLineEdit* lengthEdit = ui->lengthEdit;
	if ((int)(std::get<4>(pricing)) == 2) {
		lengthEdit->setText(QString::number(std::get<2>(pricing), 'f', 2));
		lengthEdit->setDisabled(false);
	}
	else {
		lengthEdit->setText("");
		lengthEdit->setDisabled(true);
	}
	ui->lengthEdit->setValidator(validator);

	
	QLineEdit* priceEdit = ui->priceEdit;
	priceEdit->setText(QString::number(std::get<3>(pricing), 'f', 2));
	priceEdit->setValidator(validator);

	priceTypeComboBox->addItem("Running Metre");
	priceTypeComboBox->addItem("Square Metre");
	priceTypeComboBox->addItem("Sheet");
	priceTypeComboBox->setCurrentIndex((int)(std::get<4>(pricing)));

	connect(priceTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [lengthEdit](int index) {
		if (index == 2) {
			lengthEdit->setText(QString::number(0, 'f', 2));
			lengthEdit->setDisabled(false);
		}
		else {
			lengthEdit->setText("");
			lengthEdit->setDisabled(true);
		}
		});


	if (priceMode == ComponentInsert::PriceMode::UPDATE) {

		for (QAbstractButton* button : ui->acceptButtons->buttons()) {
			if (button->text() == "OK") {
				button->setText("Edit");
				priceEdit->setFocus();
			}
		}

		this->setWindowTitle("Edit Material Price");
		ui->AddPriceTitleLabel->setText("Edit Material Price");

		connect(this, &QDialog::accepted, [widthEdit, lengthEdit, priceEdit, pricing, handle, caller, client, priceTypeComboBox]() {
			if (widthEdit->text().size() > 0 && priceEdit->text().size() > 0) {
				ComponentInsert insert;

				if (priceTypeComboBox->currentText() == "Running Metre") {

					insert.setComponentData<ComponentInsert::MaterialPriceData>({
						std::get<0>(pricing),
						DrawingComponentManager<Material>::getComponentByHandle(handle).componentID(),
						widthEdit->text().toFloat(),
						lengthEdit->text().toFloat(),
						priceEdit->text().toFloat(),
						MaterialPricingType::RUNNING_M,
						ComponentInsert::PriceMode::UPDATE,
						});
				}
				else if (priceTypeComboBox->currentText() == "Square Metre") {
					insert.setComponentData<ComponentInsert::MaterialPriceData>({
						std::get<0>(pricing),
						DrawingComponentManager<Material>::getComponentByHandle(handle).componentID(),
						widthEdit->text().toFloat(),
						lengthEdit->text().toFloat(),
						priceEdit->text().toFloat(),
						MaterialPricingType::SQUARE_M,
						ComponentInsert::PriceMode::UPDATE,
						});
				}
				else if (priceTypeComboBox->currentText() == "Sheet") {
					insert.setComponentData<ComponentInsert::MaterialPriceData>({
						std::get<0>(pricing),
						DrawingComponentManager<Material>::getComponentByHandle(handle).componentID(),
						widthEdit->text().toFloat(),
						lengthEdit->text().toFloat(),
						priceEdit->text().toFloat(),
						MaterialPricingType::SHEET,
						ComponentInsert::PriceMode::UPDATE,
						});
				}
				unsigned bufferSize = insert.serialisedSize();
				void* buffer = alloca(bufferSize);
				insert.serialise(buffer);
				client->addMessageToSendQueue(buffer, bufferSize);

				//caller->setComboboxCallback([handle, caller, client](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(comboBox->findData(handle)); caller->update(client); });
				//caller->updateSource();
				//caller->update(client);
			}
			else {
				QMessageBox* box = new QMessageBox(caller);
				box->setText("Invalid inputs, check that all fields are filled in correctly.");
				box->exec();
			}
			});
	}
	else if (priceMode == ComponentInsert::PriceMode::REMOVE) {

		this->setWindowTitle("Remove Material Price");
		ui->AddPriceTitleLabel->setText("Remove Material Price");

		for (QAbstractButton* button : ui->acceptButtons->buttons()) {
			if (button->text() == "OK") {
				button->setText("Delete");
			}
		}

		widthEdit->setDisabled(true);
		lengthEdit->setDisabled(true);
		priceEdit->setDisabled(true);
		ui->priceTypeComboBox->setDisabled(true);
		connect(this, &QDialog::accepted, [client, caller, handle, pricing, widthEdit, lengthEdit, priceEdit]() {
			if (widthEdit->text().size() > 0 && priceEdit->text().size() > 0) {
				ComponentInsert insert;

				insert.setComponentData<ComponentInsert::MaterialPriceData>({
							std::get<0>(pricing),
							DrawingComponentManager<Material>::getComponentByHandle(handle).componentID(),
							widthEdit->text().toFloat(),
							lengthEdit->text().toFloat(),
							priceEdit->text().toFloat(),
							MaterialPricingType::RUNNING_M,
							ComponentInsert::PriceMode::REMOVE
					});

				unsigned bufferSize = insert.serialisedSize();
				void* buffer = alloca(bufferSize);
				insert.serialise(buffer);
				client->addMessageToSendQueue(buffer, bufferSize);

				//caller->setComboboxCallback([handle, caller, client](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(comboBox->findData(handle)); caller->update(client); });
				//caller->updateSource();
				//caller->update(client);
			}
			else {
				QMessageBox* box = new QMessageBox(caller);
				box->setText("Invalid inputs, check that all fields are filled in correctly.");
				box->exec();
			}
			});
	}
}

// Removing a price