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

    LabourTime& labourTime = times;
    layout->addRow(new QLabel(labourTime.labourTime().c_str()));
    //QLineEdit* time = new QLineEdit();
    //time->setText(QString::number(labourTime.time, 'f', 0));
    //time->setValidator(validator);
    QSpinBox* time = new QSpinBox();
    time->setMinimum(0);
    time->setMaximum(std::numeric_limits<int>::max());

    layout->addRow("Time: ", time);
    

    connect(this, &QDialog::accepted, [this, labourTime, time, client]() {

        ComponentInsert insert;

        insert.setComponentData<ComponentInsert::LabourTimeData>({labourTime.componentID(), labourTime.job, (unsigned)time->value()});

        unsigned bufferSize = insert.serialisedSize();
        void* buffer = alloca(bufferSize);
        insert.serialise(buffer);
        client->addMessageToSendQueue(buffer, bufferSize);
            });
}