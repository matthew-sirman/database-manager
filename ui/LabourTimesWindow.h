#ifndef LABOURTIMESWINDOW_H
#define LABOURTIMESWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/database/ExtraPriceManager.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"
#include "AddLabourTimesWindow.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QInputEvent>

namespace Ui {
    class LabourTimesWindow;
}
/// <summary>
/// LabourTimesWindow inherits QDialog
/// Opens a new window to view all labour times, and allow \ref AddLabourTimesWindow to be called.
/// </summary>
class LabourTimesWindow : public QDialog {
    Q_OBJECT
public:

    /// <summary>
    /// Constructs a new window for viewing labour times.
    /// </summary>
    /// <param name="client">Network client to update the database through.</param>
    /// <param name="parent">The parent of this widget.</param>
    LabourTimesWindow(Client* client, QWidget* parent = nullptr);

    /// <summary>
    /// Updates the window with the most recent data from DrawingComponentManager.
    /// </summary>
    /// <param name="client"></param>
    void update(Client* client);

    /// <summary>
    /// Adds a callback to be called when the index is changed on the combo box.
    /// </summary>
    /// <param name=""></param>
    void setComboboxCallback(std::function<void(DynamicComboBox*)>);

    /// <summary>
    /// Sets the window to update its details on next paint event.
    /// </summary>
    void setUpdateRequired();
protected:
    /// <summary>
    /// Overriden paint event to update the information as required, then propogate.
    /// </summary>
    /// <param name="event">The event responsible for this call.</param>
    void paintEvent(QPaintEvent* event) override;
private:
    Ui::LabourTimesWindow* ui = nullptr;

    QWidget* labourTimesScroll;

    DynamicComboBox* labourTimesComboBox;

    ComboboxComponentDataSource<LabourTime> labourTimesSource;
    
    bool updateRequired = false;

    Client* client;
};

#endif // LABOURTIMESWINDOW_H