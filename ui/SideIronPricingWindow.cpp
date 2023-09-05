#include "SideIronPricingWindow.h"
#include "../build/ui_SideIronPricingWindow.h"


SideIronPricingWindow::SideIronPricingWindow(Client* client, QWidget* parent)
    : QDialog(parent), ui(new Ui::SideIronPricingWindow()), client(client) {
    ui->setupUi(this);
    this->setWindowModality(Qt::WindowModality::ApplicationModal);

    this->setWindowTitle("Side Iron Pricing");

    QObject::connect(ui->acceptButtons, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(ui->acceptButtons, SIGNAL(rejected()), this, SLOT(reject()));

    QPushButton* button = ui->acceptButtons->addButton(QString("Add"), QDialogButtonBox::ActionRole);
    connect(button, &QPushButton::pressed, this, [=]() {
        (new AddSideIronPriceWindow(client, this))->show();
        });

    QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
    sideIronPricingScroll = ui->sideIronPricingScroll;

    update();
}

void SideIronPricingWindow::update() {
    QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);

    for (unsigned i : DrawingComponentManager<SideIronPrice>::dataIndexSet()) {
        std::cout << "asadasdas" << std::endl;
        SideIronPrice price = DrawingComponentManager<SideIronPrice>::getComponentByHandle(i);
        QWidget* priceContainer = new QWidget();
        ui->sideIronPricingScroll->layout()->addWidget(priceContainer);
        QFormLayout* containerLayout = new QFormLayout();
        priceContainer->setLayout(containerLayout);

        QComboBox* typeComboBox = new QComboBox();
        typeComboBox->addItem("None");
        typeComboBox->addItem("A");
        typeComboBox->addItem("B");
        typeComboBox->addItem("C");
        typeComboBox->addItem("D");
        typeComboBox->addItem("E");
        typeComboBox->setDisabled(true);
        typeComboBox->setCurrentIndex((unsigned)price.type);
        containerLayout->addRow("Type: ", typeComboBox);

        QLineEdit* lowerLengthLineEdit = new QLineEdit();
        lowerLengthLineEdit->setDisabled(true);
        lowerLengthLineEdit->setText(QString::number(price.lowerLength));
        containerLayout->addRow("Lower Length: ", lowerLengthLineEdit);

        QLineEdit* upperLengthLineEdit = new QLineEdit();
        upperLengthLineEdit->setDisabled(true);
        upperLengthLineEdit->setText(QString::number(price.upperLength));
        containerLayout->addRow("Upper Length: ", upperLengthLineEdit);

        QCheckBox* extraflexCheckBox = new QCheckBox();
        extraflexCheckBox->setDisabled(true);
        extraflexCheckBox->setChecked(price.extraflex);
        containerLayout->addRow("Extraflex: ", extraflexCheckBox);

        QLineEdit* priceLineEdit = new QLineEdit();
        priceLineEdit->setDisabled(true);
        priceLineEdit->setValidator(validator);
        priceLineEdit->setText(QString::number(price.price, 'f', 2));
        containerLayout->addRow("Price: ", priceLineEdit);

        QHBoxLayout* buttons = new QHBoxLayout();
        QPushButton* editButton = new QPushButton("Edit");
        QPushButton* removeButton = new QPushButton("Remove");
        buttons->addWidget(editButton);
        buttons->addWidget(removeButton);
        containerLayout->addRow(buttons);

        connect(editButton, &QPushButton::pressed, this, [=]() {
            (new AddSideIronPriceWindow(client, ComponentInsert::PriceMode::UPDATE, price, this))->show();
            });
        connect(removeButton, &QPushButton::pressed, this, [=]() {
            (new AddSideIronPriceWindow(client, ComponentInsert::PriceMode::REMOVE, price, this))->show();
            });
    }
}