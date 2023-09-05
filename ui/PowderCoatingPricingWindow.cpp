#include "PowderCoatingPricingWindow.h"
#include "../build/ui_PowderCoatingPricingWindow.h"

PowderCoatingPricingWindow::PowderCoatingPricingWindow(Client* client, QWidget* parent)
	: QDialog(parent), ui(new Ui::PowderCoatingPricingWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	this->setWindowTitle("Powder Coating Prices");
	QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
	ui->hookCostEdit->setValidator(validator);
	ui->strapCostEdit->setValidator(validator);

	connect(ui->sideIronTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index) {
		
			
			if (DrawingComponentManager<PowderCoatingPrice>::validComponentID(index + 1)) {
				PowderCoatingPrice p = DrawingComponentManager<PowderCoatingPrice>::findComponentByID(index + 1);
				ui->hookCostEdit->setText(QString::number(p.hookPrice, 'f', 2));
				ui->strapCostEdit->setText(QString::number(p.strapPrice, 'f', 2));
			}
			else {
				ui->hookCostEdit->clear();
				ui->strapCostEdit->clear();
			}
		});
	ui->sideIronTypeComboBox->setCurrentIndex(0);
	if (DrawingComponentManager<PowderCoatingPrice>::validComponentID(1)) {
		PowderCoatingPrice p = DrawingComponentManager<PowderCoatingPrice>::findComponentByID(1);
		ui->hookCostEdit->setText(QString::number(p.hookPrice, 'f', 2));
		ui->strapCostEdit->setText(QString::number(p.strapPrice, 'f', 2));
	}
	else {
		ui->hookCostEdit->clear();
		ui->strapCostEdit->clear();
	}

	QPushButton* applyButton = new QPushButton("Apply");
	ui->buttonBox->addButton(applyButton, QDialogButtonBox::ApplyRole);
	connect(applyButton, &QPushButton::clicked, this, [=]() {
		if (!ui->hookCostEdit->text().isEmpty() && !ui->strapCostEdit->text().isEmpty()) {
			unsigned id = ui->sideIronTypeComboBox->currentIndex() + 1;
			if (DrawingComponentManager<PowderCoatingPrice>::validComponentID(id)) {
				PowderCoatingPrice p = DrawingComponentManager<PowderCoatingPrice>::findComponentByID(id);
				p.hookPrice = ui->hookCostEdit->text().toFloat();
				p.strapPrice = ui->strapCostEdit->text().toFloat();
				ComponentInsert insert;
				insert.setComponentData<ComponentInsert::PowderCoatingPriceData>({
					p.componentID(),
					p.hookPrice,
					p.strapPrice
					});
				unsigned bufferSize = insert.serialisedSize();
				void* buffer = alloca(bufferSize);
				insert.serialise(buffer);
				client->addMessageToSendQueue(buffer, bufferSize);
			}
		}
		else {
			QMessageBox::warning(this, "Missing Values", "There are values missing from the form, please fill these in then try again.");
		}
		});
}