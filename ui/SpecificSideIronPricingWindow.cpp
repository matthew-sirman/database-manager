#include "SpecificSideIronPricingWindow.h"
#include "../build/ui_SpecificSideIronPricingWindow.h"

SpecificSideIronPricingWindow::SpecificSideIronPricingWindow(Client* client, QWidget* parent)
	: QDialog(parent), ui(new Ui::SpecificSideIronPricingWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

	this->setWindowTitle("Specific Side Iron Prices");

	QPushButton* updateButton = ui->acceptButtons->addButton(QString("Update"), QDialogButtonBox::ActionRole);
	QPushButton* removeButton = ui->acceptButtons->addButton(QString("Remove"), QDialogButtonBox::ActionRole);
	ui->sideIronComboBox->setDataSource(sideIronSource);

	QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
	sideIronComboBox = ui->sideIronComboBox;

	DrawingComponentManager<SideIron>::addCallback([this]() { sideIronSource.updateSource(); });
	sideIronSource.updateSource();

	connect(updateButton, &QPushButton::clicked, [client, this]() {
		SideIron* s = &DrawingComponentManager<SideIron>::getComponentByHandle(sideIronComboBox->itemData(sideIronComboBox->currentIndex()).toInt());
		AddSpecificSideIronPriceWindow* window = new AddSpecificSideIronPriceWindow(client, s, ComponentInsert::PriceMode::UPDATE);
		connect(window, &AddSpecificSideIronPriceWindow::updateParent, this, &SpecificSideIronPricingWindow::update);
		window->show();
		});
	connect(removeButton, &QPushButton::clicked, [client, this]() {
		SideIron* s = &DrawingComponentManager<SideIron>::getComponentByHandle(sideIronComboBox->itemData(sideIronComboBox->currentIndex()).toInt());
		AddSpecificSideIronPriceWindow* window = new AddSpecificSideIronPriceWindow(client, s, ComponentInsert::PriceMode::REMOVE);
		connect(window, &AddSpecificSideIronPriceWindow::updateParent, this, &SpecificSideIronPricingWindow::update);
		window->show();
		});
	connect(sideIronComboBox, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this, client](int index) {
		update();
		});
	update();
}

void SpecificSideIronPricingWindow::update() {
	SideIron& sideIron = DrawingComponentManager<SideIron>::getComponentByHandle(sideIronComboBox->itemData(sideIronComboBox->currentIndex()).toInt());

	ui->priceEdit->setDisabled(true);
	ui->screwsEdit->setDisabled(true);
	if (sideIron.price.has_value()) {
		ui->priceEdit->setText(QString::number(sideIron.price.value(), 'f', 2));
	}
	else {
		ui->priceEdit->clear();
	}
	if (sideIron.screws.has_value()) {
		ui->screwsEdit->setText(QString::number(sideIron.screws.value()));
	}
	else {
		ui->screwsEdit->clear();
	}
}

void SpecificSideIronPricingWindow::setUpdateRequired() {
	updateRequired = true;
}

void SpecificSideIronPricingWindow::paintEvent(QPaintEvent* event) {
	if (updateRequired) {
		update();
		updateRequired = false;
	}
	QDialog::paintEvent(event);
}