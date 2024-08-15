
#include "LabourTimesWindow.h"
#include "../build/ui_LabourTimesWindow.h"

LabourTimesWindow::LabourTimesWindow(Client* client, QWidget* parent)
    : QDialog(parent), ui(new Ui::LabourTimesWindow()), client(client) {
    ui->setupUi(this);
    this->setWindowModality(Qt::WindowModality::ApplicationModal);

    this->setWindowTitle("Labour Times");

    ui->labourTimesComboBox->setDataSource(labourTimesSource);

    QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
    labourTimesScroll = ui->labourTimesScroll;
    labourTimesComboBox = ui->labourTimesComboBox;

    DrawingComponentManager<LabourTime>::addCallback([this]() { labourTimesSource.updateSource(); this->setUpdateRequired(); });
    labourTimesSource.updateSource();

    connect(labourTimesComboBox, qOverload<int>(&DynamicComboBox::currentIndexChanged), [this, client](int index) {
        update(client);
        });
    update(client);
}

void LabourTimesWindow::update(Client* client) {
    if (!labourTimesSource.empty()) {
        if (labourTimesComboBox->currentIndex() >= 0) {
            LabourTime& labourTime = DrawingComponentManager<LabourTime>::getComponentByHandle(labourTimesComboBox->currentData().toInt());



            if (labourTimesScroll->layout() != nullptr) {
                for (int _ = 0; _ < 5; _++) {
                    for (int i = 0; i < labourTimesScroll->layout()->count(); ++i) {
                        QWidget* widget = labourTimesScroll->layout()->takeAt(i)->widget();
                        if (widget != NULL) {
                            delete widget;
                        }
                    }
                }
                delete labourTimesScroll->layout();
            }
            QFormLayout* layout = new QFormLayout();
            QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);

            QLabel* jobDesc = new QLabel(labourTime.labourTime().c_str());
            QLineEdit *timeTextBox = new QLineEdit(QString::number(labourTime.time, 'f', 0));
            timeTextBox->setReadOnly(true);
            timeTextBox->setValidator(validator);
            layout->addRow("Description", jobDesc);
            layout->addRow("Time", timeTextBox);

            QPushButton* edit = new QPushButton("Edit");
            layout->addRow(edit);


            connect(edit, &QPushButton::clicked, [client, this, labourTime, timeTextBox]() mutable {
                AddLabourTimesWindow* window = new AddLabourTimesWindow(client, labourTime);
                connect(window, &AddLabourTimesWindow::updateParent, this, [this, timeTextBox, client](QString s) {
                    DrawingComponentManager<LabourTime>::getComponentByHandle(labourTimesComboBox->currentData().toInt()).time = s.toFloat();
                    labourTimesSource.updateSource();
                    update(client);
                    });
                window->show();
                });

            labourTimesScroll->setLayout(layout);
        }
    }
    else {
        reject();
        QMessageBox* popup = new QMessageBox(this);
        popup->setText("No Labour Times loaded, try again in a few seconds.");
        popup->show();
    }
}

void LabourTimesWindow::setComboboxCallback(std::function<void(DynamicComboBox*)> func) {
    labourTimesComboBox->setManualIndexFunc(func);
}

void LabourTimesWindow::setUpdateRequired() {
    updateRequired = true;
}

void LabourTimesWindow::paintEvent(QPaintEvent* event) {
    if (updateRequired) {
        update(client);
        updateRequired = false;
    }
    QDialog::paintEvent(event);
}