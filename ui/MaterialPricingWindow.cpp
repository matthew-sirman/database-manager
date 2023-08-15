#include "MaterialPricingWindow.h"
#include "../build/ui_MaterialPricingWindow.h"

// ui->topMaterialInput->findData(drawing.material(Drawing::TOP)->handle()

MaterialPricingWindow::MaterialPricingWindow(Client* client, QWidget* parent)
    : QDialog(parent), ui(new Ui::MaterialPricingWindow()) {
    ui->setupUi(this);
    this->setWindowModality(Qt::WindowModality::ApplicationModal);

    this->setWindowTitle("Material Prices");

    QPushButton* button = ui->acceptButtons->addButton(QString("Add"), QDialogButtonBox::ActionRole);
    ui->materialComboBox->setDataSource(materialSource);

    QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
    materialPricingScroll = ui->materialPricingScroll;
    materialComboBox = ui->materialComboBox;

    DrawingComponentManager<Material>::addCallback([this]() { materialSource.updateSource(); });
    materialSource.updateSource();

    connect(button, &QPushButton::clicked, [client, this]() {
        (new AddMaterialPriceWindow(client, this, materialComboBox->itemData(materialComboBox->currentIndex()).toInt()))->show();
        });
    connect(materialComboBox, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this, client](int index){
        update(client);
        });
    update(client);
}

void MaterialPricingWindow::update(Client* client) {
    if (!materialSource.empty()) {
        Material& material = DrawingComponentManager<Material>::getComponentByHandle(materialComboBox->itemData(materialComboBox->currentIndex()).toInt());
        for (int _ = 0; _ < 5; _++) {
            for (int i = 0; i < materialPricingScroll->layout()->count(); ++i) {
                QWidget* widget = materialPricingScroll->layout()->takeAt(i)->widget();
                if (widget != NULL) {
                    delete widget;
                }
            }
        }
        delete materialPricingScroll->layout();
        QFormLayout* layout = new QFormLayout();
        QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);

        if (!material.materialPrices.empty()) {

            QFrame* lastLine = nullptr;
            for (std::vector<std::tuple<float, float, float, MaterialPricingType>>::iterator i = material.materialPrices.begin(); i != material.materialPrices.end(); i++) {
                std::tuple<float, float, float, MaterialPricingType> element = *i;
                QLineEdit* widthTextbox = new QLineEdit(QString::number(std::get<0>(element)));
                widthTextbox->setReadOnly(true);
                widthTextbox->setValidator(validator);
                layout->addRow("Width: ", widthTextbox);
                if ((int)std::get<3>(element) == 2) {
                    QLineEdit* lengthTextbox = new QLineEdit(QString::number(std::get<1>(element)));
                    lengthTextbox->setReadOnly(true);
                    lengthTextbox->setValidator(validator);
                    layout->addRow("Length: ", lengthTextbox);
                }
                else {
                    QLineEdit* lengthTextbox = new QLineEdit();
                    lengthTextbox->setReadOnly(true);
                    lengthTextbox->setDisabled(true);
                    lengthTextbox->setValidator(validator);
                    layout->addRow("Length: ", lengthTextbox);
                }
                QLineEdit* priceTextbox = new QLineEdit(QString::number(std::get<2>(element)));
                priceTextbox->setReadOnly(true);
                priceTextbox->setValidator(validator);
                layout->addRow("Price: ", priceTextbox);
                QComboBox* priceTypeBox = new QComboBox();
                priceTypeBox->addItem("Running Metre");
                priceTypeBox->addItem("Square Metre");
                priceTypeBox->addItem("Sheet");
                priceTypeBox->setCurrentIndex((int)(std::get<3>(element)));
                priceTypeBox->setDisabled(true);
                layout->addRow("per :", priceTypeBox);
                QPushButton* remove = new QPushButton("Remove");
                QPushButton* edit = new QPushButton("Edit");
                layout->addRow(remove, edit);
                QFrame* line;
                line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                layout->addRow(line);
                lastLine = line;

                connect(edit, &QPushButton::clicked, [client, this, element]() {
                    (new AddMaterialPriceWindow(client, this, materialComboBox->itemData(materialComboBox->currentIndex()).toInt(), ComponentInsert::PriceMode::UPDATE, element))->show();
                    });
                connect(remove, &QPushButton::clicked, [client, this, element]() {
                    (new AddMaterialPriceWindow(client, this, materialComboBox->itemData(materialComboBox->currentIndex()).toInt(), ComponentInsert::PriceMode::REMOVE, element))->show();
                    });
            }
            if (lastLine != nullptr) {
                layout->removeWidget(lastLine);
                delete lastLine;
            }
        }
        materialPricingScroll->setLayout(layout);
    }
    else {
        reject();
        QMessageBox* popup = new QMessageBox(this);
        popup->setText("No Materials loaded, try again in a few seconds.");
        popup->show();
    }
}

void MaterialPricingWindow::setComboboxCallback(std::function<void(DynamicComboBox*)> func) {
    materialComboBox->setManualIndexFunc(func);
}

void MaterialPricingWindow::updateSource() {
    materialSource.updateSource();
}
