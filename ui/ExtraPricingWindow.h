#ifndef EXTRAPRICINGWINDOW_H
#define EXTRAPRICINGWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/database/ExtraPriceManager.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"
#include "../include/database/DrawingComponentManager.h"
#include "AddExtraPriceWindow.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QInputEvent>

namespace Ui {
    class ExtraPricingWindow;
}

/// <summary>
/// ExtraPricingWindow inherits QDialog
/// A dialog to help manage all extra prices.
/// </summary>
class ExtraPricingWindow : public QDialog {
    Q_OBJECT
public:
    /// <summary>
    /// Constructs a new dialog to display all extra prices.
    /// </summary>
    /// <param name="client">Network client to facilitiate any updates to the database by child windows.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit ExtraPricingWindow(Client* client, QWidget* parent = nullptr);

    /// <summary>
    /// Updates the information displayed from the DrawingComponentManager.
    /// </summary>
    /// <param name="client">Network client to facilitate any updates to the database through.</param>
    void update(Client* client);

    /// <summary>
    /// Sets a callback to be called when the combobox's index is changed.
    /// </summary>
    /// <param name="callback">The callback to be triggered in index change.</param>
    void setComboboxCallback(std::function<void(DynamicComboBox*)> callback);

    /// <summary>
    /// Causes the source used by the combobox to be updated from the \ref DrawingComponentManager.
    /// </summary>
    void updateSource();

    /// <summary>
    /// Causes the window to be updated on next paintEvent.
    /// </summary>
    void setUpdateRequired();
protected:
    /// <summary>
    /// Overriden paint event to update displayed infromation, then propogates.
    /// </summary>
    /// <param name="event">The event that caused this call.</param>
    void paintEvent(QPaintEvent* event) override;
private:
    Ui::ExtraPricingWindow* ui = nullptr;

    QWidget* extraPricingScroll;

    DynamicComboBox* extraPriceComboBox;

    ComboboxComponentDataSource<ExtraPrice> extraPriceSource;

    bool updateRequired = false;

    Client* client;
};

#endif // EXTRAPRICINGWINDOW_H
