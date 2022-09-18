#include "SideIronPricingWindow.h"
#include "../build/ui_SideIronPricingWindow.h"


SideIronPricingWindow::SideIronPricingWindow(Client* client, QWidget* parent)
    : QDialog(parent), ui(new Ui::SideIronPricingWindow()) {
    ui->setupUi(this);
    this->setWindowModality(Qt::WindowModality::ApplicationModal);

    this->setWindowTitle("Side Iron Pricing");

    QObject::connect(ui->acceptButtons, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(ui->acceptButtons, SIGNAL(rejected()), this, SLOT(reject()));

    QPushButton* button = ui->acceptButtons->addButton(QString("Add"), QDialogButtonBox::ActionRole);

    QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
    sideIronPricingScroll = ui->sideIronPricingScroll;

    sideIronComboBox = ui->sideIronComboBox;
    sideIronComboBox->setDataSource(sideIronPriceSource);
    DrawingComponentManager<SideIronPrice>::addCallback([this]() { sideIronPriceSource.updateSource(); });
    sideIronPriceSource.updateSource();

    connect(button, &QPushButton::clicked, [client, this]() {
        (new AddSideIronPriceWindow(client, this, sideIronComboBox->currentIndex()))->show();
        });
    connect(sideIronComboBox, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this, client](int index) {
        update(client, index);
        });
    update(client, 0);
}

void SideIronPricingWindow::update(Client* client, int index) {
    if (!sideIronPriceSource.empty()) {
        for (int _ = 0; _ < 5; _++) {
            for (int i = 0; i < sideIronPricingScroll->layout()->count(); ++i) {
                QWidget* widget = sideIronPricingScroll->layout()->takeAt(i)->widget();
                if (widget != NULL) {
                    delete widget;
                }
            }
        }

        delete sideIronPricingScroll->layout();
        QFormLayout* layout = new QFormLayout();
        QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);

        SideIronPrice& sideIronPrice = DrawingComponentManager<SideIronPrice>::getComponentByHandle(sideIronComboBox->itemData(index).toInt());
        QFrame* lastLine = nullptr;
        for (std::tuple<unsigned, float, float, unsigned, bool> prices : sideIronPrice.prices) {
            QLineEdit* lengthTextbox = new QLineEdit(QString::number(std::get<1>(prices), 'f', 2));
            lengthTextbox->setReadOnly(true);
            lengthTextbox->setValidator(validator);
            layout->addRow("Length: ", lengthTextbox);
            QLineEdit* priceTextbox = new QLineEdit(QString::number(std::get<2>(prices), 'f', 2));
            priceTextbox->setReadOnly(true);
            priceTextbox->setValidator(validator);
            layout->addRow("Price: ", priceTextbox);
            QLineEdit* screwsTextbox = new QLineEdit(QString::number(std::get<3>(prices), 'f', 0));
            screwsTextbox->setReadOnly(true);
            screwsTextbox->setValidator(validator);
            layout->addRow("Screws: ", screwsTextbox);

            QComboBox* extraflexBox = new QComboBox();
            extraflexBox->addItem("Not Extraflex");
            extraflexBox->addItem("Extraflex");
            extraflexBox->setCurrentIndex((int)(std::get<4>(prices)));
            extraflexBox->setDisabled(true);
            layout->addRow("Extraflex: ", extraflexBox);

            QPushButton* remove = new QPushButton("Remove");
            QPushButton* edit = new QPushButton("Edit");
            layout->addRow(remove, edit);
            QFrame* line;
            line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            layout->addRow(line);
            lastLine = line;

            connect(edit, &QPushButton::clicked, [client, this, sideIronPrice, prices]() {
                (new AddSideIronPriceWindow(client, this, sideIronComboBox->currentIndex(), ComponentInsert::PriceMode::UPDATE, sideIronPrice, prices))->show();
                });
            connect(remove, &QPushButton::clicked, [client, this, sideIronPrice, prices]() {
                (new AddSideIronPriceWindow(client, this, sideIronComboBox->currentIndex(), ComponentInsert::PriceMode::REMOVE, sideIronPrice, prices))->show();
                });
        }
        if (lastLine != nullptr) {
            layout->removeWidget(lastLine);
        }
        sideIronPricingScroll->setLayout(layout);
    }
    else {
        reject();
        QMessageBox* popup = new QMessageBox(this);
        popup->setText("No Side Irons loaded, try again in a few seconds.");
        popup->show();
    }
}

void SideIronPricingWindow::setComboboxCallback(std::function<void(DynamicComboBox*)> func) {
    sideIronComboBox->setManualIndexFunc(func);
}
