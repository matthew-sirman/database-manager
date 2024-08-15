#include "AddLabourTimesWindow.h"
#include "../build/ui_AddLabourTimesWindow.h"
#include "LabourTimesWindow.h"

AddLabourTimesWindow::AddLabourTimesWindow(Client* client,
										 LabourTime& times, QWidget* parent)
	: QDialog(parent), ui(new Ui::AddLabourTimesWindow()) {
	ui->setupUi(this);
	this->setWindowModality(Qt::WindowModality::ApplicationModal);

    delete ui->addLabourTimesWidget->layout();
    QFormLayout* layout = new QFormLayout(ui->addLabourTimesWidget);

    for (QAbstractButton* button : ui->acceptButtons->buttons()) {
        if (button->text() == "OK") {
            button->setText("Update");
        }
    }

    // TODO : make this work
    LabourTime& labourTime = times;
    layout->addRow(new QLabel(labourTime.labourTime().c_str()));
    QLineEdit* time = new QLineEdit();
    QDoubleValidator* validator = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2);
    time->setText(QString::number(labourTime.time, 'f', 0));
    time->setValidator(validator);
    layout->addRow("Time: ", time);
    

    connect(this, &QDialog::accepted, [this, labourTime, time, client]() {
        emit updateParent(time->text());

        ComponentInsert insert;

        ComponentInsert::LabourTimeData* timeData = nullptr;
        timeData = new ComponentInsert::LabourTimeData(labourTime.componentID(), labourTime.job, std::stof(time->text().toStdString()));
        insert.setComponentData<ComponentInsert::LabourTimeData>(*timeData);


        // somehow changes from whatever you set here to 70

        unsigned bufferSize = insert.serialisedSize();
        void* buffer = alloca(bufferSize);
        insert.serialise(buffer);
        client->addMessageToSendQueue(buffer, bufferSize);
            });
}