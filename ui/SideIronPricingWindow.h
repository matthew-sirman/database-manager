#ifndef SIDEIRONPRICINGWINDOW_H
#define SIDEIRONPRICINGWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "AddSideIronPriceWindow.h"
#include "widgets/DynamicComboBox.h"

#include <QDialog>

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QFormLayout>
#include <QInputEvent>
#include <QCheckBox>

namespace Ui {
    class SideIronPricingWindow;
}

/// <summary>
/// SideIronPricingWindow inherits QDialog
/// Opens a window that can be used to view and update generic side iron prices.
/// </summary>
class SideIronPricingWindow : public QDialog {
    Q_OBJECT
public:
    /// <summary>
    /// Constructs a new window that can update and view generic side iron prices.
    /// </summary>
    /// <param name="client">Network client to update the database through.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit SideIronPricingWindow(Client* client, QWidget* parent = nullptr);

    /// <summary>
    /// Updates the information displayed on the window.
    /// </summary>
    void update();
private:
    Ui::SideIronPricingWindow* ui = nullptr;

    QWidget* sideIronPricingScroll;
    Client* client;
};
#endif // SIDEIRONPRICINGWINDOW_H
