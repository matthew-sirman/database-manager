#include "AddExtraPriceWindow.h"
#include "../build/ui_AddExtraPriceWindow.h"
#include "ExtraPricingWindow.h"

AddExtraPriceWindow::AddExtraPriceWindow(Client* client,
										 ExtraPrice& price, int index, QWidget* parent)
	: QDialog(parent), ui(new Ui::AddExtraPriceWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

    delete ui->addExtraPriceWidget->layout();
    QFormLayout* layout = new QFormLayout(ui->addExtraPriceWidget);

    for (QAbstractButton* button : ui->acceptButtons->buttons()) {
        if (button->text() == "OK") {
            button->setText("Update");
        }
    }

    ExtraPrice& extraPrice = price;
    std::vector<QLineEdit*> values;

    switch (extraPrice.type) {
        case (ExtraPriceType::SIDE_IRON_NUTS):
        {
            layout->addRow(new QLabel("Side Iron Nuts"));
            QLineEdit* sideIronNutsAmount = new QLineEdit(QString::number(extraPrice.amount.value(), 'f', 0));
            QLineEdit* sideIronNutsPrice = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
            layout->addRow("Amount: ", sideIronNutsAmount);
            layout->addRow("Price: ", sideIronNutsPrice);
            values.push_back(sideIronNutsPrice);
            values.push_back(sideIronNutsAmount);
            break;
        }
        case (ExtraPriceType::SIDE_IRON_SCREWS):
        {
            layout->addRow(new QLabel("Side Iron Screws"));
            QLineEdit* sideIronScrewsAmount = new QLineEdit(QString::number(extraPrice.amount.value(), 'f', 0));
            QLineEdit* sideIronScrewsPrice = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
            layout->addRow("Amount: ", sideIronScrewsAmount);
            layout->addRow("Price: ", sideIronScrewsPrice);
            values.push_back(sideIronScrewsPrice);
            values.push_back(sideIronScrewsAmount);
            break;
        }
        case (ExtraPriceType::TACKYBACK_GLUE):
        {
            layout->addRow(new QLabel("Tackyback Glue"));
            QLineEdit* tackybackGlueAmount = new QLineEdit(QString::number(extraPrice.squareMetres.value(), 'f', 2));
            QLineEdit* tackybackGluePrice = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
            layout->addRow("Square Metre Coverage: ", tackybackGlueAmount);
            layout->addRow("Price: ", tackybackGluePrice);
            values.push_back(tackybackGluePrice);
            values.push_back(tackybackGlueAmount);
            break;
        }
        case (ExtraPriceType::LABOUR):
        {
            layout->addRow(new QLabel("Labour"));
            QLineEdit* labourPrice = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
            layout->addRow("Per hour: ", labourPrice);
            values.push_back(labourPrice);
            break;
        }
        case (ExtraPriceType::PRIMER):
        {
            layout->addRow(new QLabel("Primer"));
            QLineEdit* primerAmount = new QLineEdit(QString::number(extraPrice.squareMetres.value(), 'f', 2));
            QLineEdit* primerPrice = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
            layout->addRow("Square Metre Coverage: ", primerAmount);
            layout->addRow("Price: ", primerPrice);
            values.push_back(primerPrice);
            values.push_back(primerAmount);
            break;
        }
        default:
            break;
    }

    connect(this, &QDialog::accepted, [this, extraPrice, values, client, index]() {
        ComponentInsert insert;

        ComponentInsert::ExtraPriceData* priceData = nullptr;
        switch (extraPrice.type) {
            //price then amount
            case ExtraPriceType::SIDE_IRON_NUTS: case ExtraPriceType::SIDE_IRON_SCREWS:
                priceData = new ComponentInsert::ExtraPriceData(extraPrice.componentID(), extraPrice.type, values[0]->text().toFloat(), std::nullopt, values[1]->text().toInt());
                break;
            //price then surface area
            case ExtraPriceType::TACKYBACK_GLUE: case ExtraPriceType::PRIMER:
                priceData = new ComponentInsert::ExtraPriceData(extraPrice.componentID(), extraPrice.type, values[0]->text().toFloat(), values[1]->text().toFloat(), std::nullopt);
                break;
            //price
            case (ExtraPriceType::LABOUR):
                priceData = new ComponentInsert::ExtraPriceData(extraPrice.componentID(), extraPrice.type, values[0]->text().toFloat(), std::nullopt, std::nullopt);
                break;
            default:
                return;
        }
        insert.setComponentData<ComponentInsert::ExtraPriceData>(*priceData);


        // somehow changes from whatever you set here to 70

        unsigned bufferSize = insert.serialisedSize();
        void* buffer = alloca(bufferSize);
        insert.serialise(buffer);
        client->addMessageToSendQueue(buffer, bufferSize);
        emit updateParent(values[0]->text(),extraPrice.type != ExtraPriceType::LABOUR ? std::make_optional(values[1]->text()) : std::nullopt);
            });
}