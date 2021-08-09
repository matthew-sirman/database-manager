#include "AddExtraPriceWindow.h"
#include "../build/ui_AddExtraPriceWindow.h"
#include "ExtraPricingWindow.h"

AddExtraPriceWindow::AddExtraPriceWindow(Client* client, ExtraPricingWindow* caller,
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
            QLineEdit* sideIronNutsAmount = new QLineEdit(QString::number(extraPrice.amount, 'f', 0));
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
            QLineEdit* sideIronScrewsAmount = new QLineEdit(QString::number(extraPrice.amount, 'f', 0));
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
            QLineEdit* tackybackGlueAmount = new QLineEdit(QString::number(extraPrice.squareMetres, 'f', 2));
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
        default:
            break;
    }

    connect(this, &QDialog::accepted, [index, extraPrice, client, caller, values]() {
        ComponentInsert insert;

        ComponentInsert::ExtraPriceData* priceData = nullptr;
        std::cout << values[1]->text().toFloat() << std::endl;
        switch (extraPrice.type) {
            case (ExtraPriceType::SIDE_IRON_NUTS) :
                priceData = new ComponentInsert::ExtraPriceData(extraPrice.componentID(), extraPrice.type, values[0]->text().toFloat(), std::nullopt, values[1]->text().toInt());
                //std::get<0>(edited).price = values[0]->text().toFloat();
                //std::get<1>(edited)[1]->setText(QString::number(values[0]->text().toFloat(), 'f', 2));
                //std::get<0>(edited).amount = values[1]->text().toInt();
                //std::get<1>(edited)[0]->setText(QString::number(values[1]->text().toInt(), 'f', 0));
                break;
            case (ExtraPriceType::SIDE_IRON_SCREWS):
                priceData = new ComponentInsert::ExtraPriceData(extraPrice.componentID(), extraPrice.type, values[0]->text().toFloat(), std::nullopt, values[1]->text().toInt());
                //std::get<0>(edited).price = values[0]->text().toFloat();
                //std::get<1>(edited)[1]->setText(QString::number(values[0]->text().toFloat(), 'f', 2));
                //std::get<0>(edited).amount = values[1]->text().toInt();
                //std::get<1>(edited)[0]->setText(QString::number(values[1]->text().toInt(), 'f', 0));
                break;
            case (ExtraPriceType::TACKYBACK_GLUE):
                priceData = new ComponentInsert::ExtraPriceData(extraPrice.componentID(), extraPrice.type, values[0]->text().toFloat(), values[1]->text().toFloat(), std::nullopt);
                //std::get<0>(edited).price = values[0]->text().toFloat();
                //std::get<1>(edited)[1]->setText(QString::number(values[0]->text().toFloat(), 'f', 2));
                //std::get<0>(edited).squareMetres = values[1]->text().toFloat();
                //std::get<1>(edited)[0]->setText(QString::number(values[1]->text().toFloat(), 'f', 2));
                break;
            case (ExtraPriceType::LABOUR):
                priceData = new ComponentInsert::ExtraPriceData(extraPrice.componentID(), extraPrice.type, values[0]->text().toFloat(), std::nullopt, std::nullopt);
                //std::get<0>(edited).price = values[0]->text().toFloat();
                //std::get<1>(edited)[1]->setText(QString::number(values[0]->text().toFloat(), 'f', 2));
                //std::get<0>(edited).squareMetres = values[1]->text().toFloat();
                //std::get<1>(edited)[0]->setText(QString::number(values[1]->text().toFloat(), 'f', 2));
                break;
        }
        insert.setComponentData<ComponentInsert::ExtraPriceData>(*priceData);


        // somehow changes from whatever you set here to 70

        unsigned bufferSize = insert.serialisedSize();
        void* buffer = alloca(bufferSize);
        insert.serialise(buffer);
        client->addMessageToSendQueue(buffer, bufferSize);

        caller->setComboboxCallback([index, caller, client](DynamicComboBox* comboBox) {comboBox->setCurrentIndex(index); caller->update(client); });
        caller->updateSource();
        caller->update(client);
            });
}