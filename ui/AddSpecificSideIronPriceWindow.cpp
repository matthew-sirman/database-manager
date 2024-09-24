#include "AddSpecificSideIronPriceWindow.h"

#include "../build/ui_AddSpecificSideIronPriceWindow.h"

AddSpecificSideIronPriceWindow::AddSpecificSideIronPriceWindow(
    Client* client, SideIron* sideIron, ComponentInsert::PriceMode priceMode,
    QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AddSpecificSideIronPriceWindow) {  // update | remove
    ui->setupUi(this);
    this->setWindowModality(Qt::WindowModality::ApplicationModal);

    ui->sideIronComboBox->setDataSource(sideIronSource);

    QDoubleValidator* validator =
        new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
    QRegularExpressionValidator* intValidator =
        new QRegularExpressionValidator(QRegularExpression("[1-9][0-9]*"));
    ui->priceEdit->setValidator(validator);
    ui->screwsEdit->setValidator(intValidator);
    sideIronComboBox = ui->sideIronComboBox;

    sideIronComboBox->setCurrentIndex(
        sideIronComboBox->findData(sideIron->handle()));
    sideIronComboBox->setManualIndexFunc([sideIron](DynamicComboBox* comboBox) {
        comboBox->setCurrentIndex(comboBox->findData(sideIron->handle()));
    });

    DrawingComponentManager<SideIron>::addCallback(
        [this]() { sideIronSource.updateSource(); });
    sideIronSource.updateSource();

    if (sideIron->price.has_value())
        ui->priceEdit->setText(QString::number(*sideIron->price, 'f', 2));
    if (sideIron->screws.has_value())
        ui->screwsEdit->setText(QString::number(*sideIron->screws));

    connect(
        sideIronComboBox, qOverload<int>(&DynamicComboBox::currentIndexChanged),
        [this, client](int index) {
            SideIron s =
                DrawingComponentManager<SideIron>::getComponentByHandle(
                    sideIronComboBox->itemData(sideIronComboBox->currentIndex())
                        .toInt());
            if (s.price.has_value())
                ui->priceEdit->setText(QString::number(*s.price, 'f', 2));
            else
                ui->priceEdit->clear();
            if (s.screws.has_value())
                ui->screwsEdit->setText(QString::number(*s.screws));
            else
                ui->screwsEdit->clear();
        });

    switch (priceMode) {
        case ComponentInsert::PriceMode::REMOVE:
            this->setWindowTitle("Remove Specific Side Iron Price");
            ui->AddPriceTitleLabel->setText("Remove Specific Side Iron Price");
            break;
        case ComponentInsert::PriceMode::UPDATE:
            this->setWindowTitle("Update Specific Side Iron Price");
            if (sideIron->price.has_value() && sideIron->screws.has_value())
                ui->AddPriceTitleLabel->setText("Update Specific Side Iron Price");
            else
                ui->AddPriceTitleLabel->setText("Add Specific Side Iron Price");
            break;
    }

    connect(ui->acceptButtons, &QDialogButtonBox::accepted, this, [=]() {
        SideIron* s = &DrawingComponentManager<SideIron>::getComponentByHandle(
            sideIronComboBox->itemData(sideIronComboBox->currentIndex())
                .toInt());
        ComponentInsert insert;
        insert.setComponentData<ComponentInsert::SpecificSideIronPriceData>(
            {priceMode, s->componentID(), ui->priceEdit->text().toFloat(),
             ui->screwsEdit->text().toUInt()});

        unsigned bufferSize = insert.serialisedSize();
        void* buffer = alloca(bufferSize);
        insert.serialise(buffer);
        client->addMessageToSendQueue(buffer, bufferSize);
        s->price = ui->priceEdit->text().toFloat();
        s->screws = ui->screwsEdit->text().toUInt();
    });
}