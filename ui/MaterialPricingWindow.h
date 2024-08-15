#ifndef MATERIALPRICINGWINDOW_H
#define MATERIALPRICINGWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "AddMaterialPriceWindow.h"
#include "widgets/DynamicComboBox.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QInputEvent>

namespace Ui {
    class MaterialPricingWindow;
}

/// <summary>
/// MaterialPricingWindow inherits QDialog
/// A window to view material prices and open AddMaterialPriceWindow's to update them.
/// </summary>
class MaterialPricingWindow : public QDialog {
    Q_OBJECT
public:
    /// <summary>
    /// Constructs a new window to view material prices.
    /// </summary>
    /// <param name="client">Client to update database through.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit MaterialPricingWindow(Client* client, QWidget* parent = nullptr);

    /// <summary>
    /// Causes the displayed information to be updated.
    /// </summary>
    /// <param name="client">Client to update the database through.</param>
    void update(Client* client);

    /// <summary>
    /// Sets a callback to be called when the combobox index has been changed.
    /// </summary>
    /// <param name="callback">The callback to be triggered.</param>
    void setComboboxCallback(std::function<void(DynamicComboBox*)> callback);

    /// <summary>
    /// Updates the source of the combo box.
    /// </summary>
    void updateSource();

    /// <summary>
    /// Forces the information displayed to be updated at the next paint event.
    /// </summary>
    void setUpdateRequired();
protected:
    /// <summary>
    /// Overriden paint event to facilitate updating on demand, then propogates event.
    /// </summary>
    /// <param name="event">The event that triggered this call.</param>
    void paintEvent(QPaintEvent *event) override;
private:
    Ui::MaterialPricingWindow* ui = nullptr;

    QWidget* materialPricingScroll;

    DynamicComboBox* materialComboBox;

    ComboboxComponentDataSource<Material> materialSource;

    bool updateRequired = true;

    Client* client;
};

#endif // MATERIALPRICINGWINDOW_H
