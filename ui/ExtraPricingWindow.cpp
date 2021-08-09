#include "ExtraPricingWindow.h"
#include "../build/ui_ExtraPricingWindow.h"

ExtraPricingWindow::ExtraPricingWindow(Client* client, QWidget* parent)
    : QDialog(parent), ui(new Ui::ExtraPricingWindow()) {
    ui->setupUi(this);
    this->setWindowModality(Qt::WindowModality::ApplicationModal);

    this->setWindowTitle("Extra Prices");

    ui->extraPriceComboBox->setDataSource(extraPriceSource);

    QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
    extraPricingScroll = ui->extraPricingScroll;
    extraPriceComboBox = ui->extraPriceComboBox;

    DrawingComponentManager<ExtraPrice>::addCallback([this]() { extraPriceSource.updateSource(); });
    extraPriceSource.updateSource();

    connect(extraPriceComboBox, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this, client](int index) {
        update(client);
        });
    update(client);
}

void ExtraPricingWindow::update(Client* client) {
    if (!extraPriceSource.empty()) {
        if (extraPriceComboBox->currentIndex() >= 0) {
            ExtraPrice& extraPrice = DrawingComponentManager<ExtraPrice>::getComponentByHandle(extraPriceComboBox->currentData().toInt());



            if (extraPricingScroll->layout() != nullptr) {
                for (int _ = 0; _ < 5; _++) {
                    for (int i = 0; i < extraPricingScroll->layout()->count(); ++i) {
                        QWidget* widget = extraPricingScroll->layout()->takeAt(i)->widget();
                        if (widget != NULL) {
                            delete widget;
                        }
                    }
                }
                delete extraPricingScroll->layout();
            }
            QFormLayout* layout = new QFormLayout();
            QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);

            switch (extraPrice.type) {
                case (ExtraPriceType::SIDE_IRON_NUTS):
                {
                    QLineEdit* amountTextbox = new QLineEdit(QString::number(extraPrice.amount, 'f', 0));
                    amountTextbox->setReadOnly(true);
                    amountTextbox->setValidator(validator);
                    layout->addRow("Amount: ", amountTextbox);
                    QLineEdit* priceTextbox = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
                    priceTextbox->setReadOnly(true);
                    priceTextbox->setValidator(validator);
                    layout->addRow("Price: ", priceTextbox);
                    break;
                }
                case ExtraPriceType::SIDE_IRON_SCREWS:
                {
                    QLineEdit* amountTextbox = new QLineEdit(QString::number(extraPrice.amount, 'f', 0));
                    amountTextbox->setReadOnly(true);
                    amountTextbox->setValidator(validator);
                    layout->addRow("Amount: ", amountTextbox);
                    QLineEdit* priceTextbox = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
                    priceTextbox->setReadOnly(true);
                    priceTextbox->setValidator(validator);
                    layout->addRow("Price: ", priceTextbox);
                    break;
                }
                case ExtraPriceType::TACKYBACK_GLUE:
                {
                    QLineEdit* squareMetresTextbox = new QLineEdit(QString::number(extraPrice.squareMetres, 'f', 2));
                    squareMetresTextbox->setReadOnly(true);
                    squareMetresTextbox->setValidator(validator);
                    layout->addRow("Square Metres: ", squareMetresTextbox);
                    QLineEdit* priceTextbox = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
                    priceTextbox->setReadOnly(true);
                    priceTextbox->setValidator(validator);
                    layout->addRow("Price: ", priceTextbox);
                    break;
                }
                case ExtraPriceType::LABOUR:
                {
                    QLineEdit* priceTextbox = new QLineEdit(QString::number(extraPrice.price, 'f', 2));
                    priceTextbox->setReadOnly(true);
                    priceTextbox->setValidator(validator);
                    layout->addRow("Per hour: ", priceTextbox);
                    break;
                }
            }


            QPushButton* edit = new QPushButton("Edit");
            layout->addRow(edit);

            connect(edit, &QPushButton::clicked, [client, this, extraPrice]() mutable {
                // (new AddExtraPriceWindow(client, this, extraPrice, extraPriceComboBox->currentIndex()))->show();
                (new AddExtraPriceWindow(client, this, extraPrice, extraPriceComboBox->currentIndex()))->show();
                });

            extraPricingScroll->setLayout(layout);
        }
    }
    else {
        reject();
        QMessageBox* popup = new QMessageBox(this);
        popup->setText("No Extras loaded, try again in a few seconds.");
        popup->show();
    }
}

void ExtraPricingWindow::setComboboxCallback(std::function<void(DynamicComboBox*)> func) {
    extraPriceComboBox->setManualIndexFunc(func);
}

void ExtraPricingWindow::updateSource() {
    extraPriceSource.updateSource();
}
