#include "AddSideIronPriceWindow.h"
#include "../build/ui_AddSideIronPriceWindow.h"
#include "SideIronPricingWindow.h"

AddSideIronPriceWindow::AddSideIronPriceWindow(Client *client, QWidget* parent)
	: QDialog(parent), ui(new Ui::AddSideIronPriceWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	this->setWindowTitle("Add new Side Iron Price");

	QObject::connect(ui->acceptButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	QObject::connect(ui->acceptButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QDoubleValidator* doubleValidator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
	QRegularExpressionValidator* numberValidator = new QRegularExpressionValidator(QRegularExpression("[1-9][0-9]*"));
	ui->lowerLengthEdit->setValidator(numberValidator);
	ui->upperLengthEdit->setValidator(numberValidator);
	ui->priceEdit->setValidator(doubleValidator);

	connect(ui->acceptButtons, &QDialogButtonBox::accepted, this, [=]() {
		ComponentInsert insert;
		insert.setComponentData<ComponentInsert::SideIronPriceData>({
			(SideIronType)ui->sideIronTypeComboBox->currentIndex(),
			ui->lowerLengthEdit->text().toUInt(),
			ui->upperLengthEdit->text().toUInt(),
			ui->priceEdit->text().toFloat(),
			(bool)ui->extraflexCheckBox->checkState(),
			ComponentInsert::PriceMode::ADD,
			std::nullopt
			});


		// somehow changes from whatever you set here to 70

		unsigned bufferSize = insert.serialisedSize();
		void* buffer = alloca(bufferSize);
		insert.serialise(buffer);
		client->addMessageToSendQueue(buffer, bufferSize);
		RequestType req = RequestType::SOURCE_SIDE_IRON_PRICES_TABLE;
		client->addMessageToSendQueue(&req, sizeof(RequestType));
	});
}

AddSideIronPriceWindow::AddSideIronPriceWindow(Client* client, ComponentInsert::PriceMode priceMode, const SideIronPrice& price, QWidget* parent)
: QDialog(parent), ui(new Ui::AddSideIronPriceWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	switch (priceMode) {
	case ComponentInsert::PriceMode::UPDATE:
		this->setWindowTitle("Update Side Iron Price");
		ui->AddPriceTitleLabel->setText("Update Side Iron Pirce");
		ui->acceptButtons->button(QDialogButtonBox::StandardButton::Ok)->setText("Update");
		break;
	case ComponentInsert::PriceMode::REMOVE:
		this->setWindowTitle("Remove Side Iron Price");
		ui->AddPriceTitleLabel->setText("Remove Side Iron Pirce");
		ui->acceptButtons->button(QDialogButtonBox::StandardButton::Ok)->setText("Remove");
		break;
	}

	QObject::connect(ui->acceptButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	QObject::connect(ui->acceptButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QDoubleValidator* doubleValidator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
	QRegularExpressionValidator* numberValidator = new QRegularExpressionValidator(QRegularExpression("[1-9][0-9]*"));
	ui->lowerLengthEdit->setValidator(numberValidator);
	ui->upperLengthEdit->setValidator(numberValidator);
	ui->priceEdit->setValidator(doubleValidator);

	ui->sideIronTypeComboBox->setCurrentIndex((unsigned)price.type);
	ui->lowerLengthEdit->setText(QString::number(price.lowerLength));
	ui->upperLengthEdit->setText(QString::number(price.upperLength));
	ui->priceEdit->setText(QString::number(price.price, 'f', 2));
	ui->extraflexCheckBox->setChecked(price.extraflex);

	connect(ui->acceptButtons, &QDialogButtonBox::accepted, this, [=]() {
		ComponentInsert insert;
		insert.setComponentData<ComponentInsert::SideIronPriceData>({
			(SideIronType)ui->sideIronTypeComboBox->currentIndex(),
			ui->lowerLengthEdit->text().toUInt(),
			ui->upperLengthEdit->text().toUInt(),
			ui->priceEdit->text().toFloat(),
			(bool)ui->extraflexCheckBox->checkState(),
			priceMode,
			price.componentID()
			});
		unsigned bufferSize = insert.serialisedSize();
		void* buffer = alloca(bufferSize);
		insert.serialise(buffer);
		client->addMessageToSendQueue(buffer, bufferSize);
		});

}