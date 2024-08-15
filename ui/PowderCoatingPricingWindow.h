#ifndef POWDERCOATINGPRICINGWINDOW_H
#define POWDERCOATINGPRICINGWINDOW_H

#include "widgets/DynamicComboBox.h"
#include "../include/database/drawingComponents.h"
#include <QDialog>
#include <QPushButton>
#include <QMessageBox>
#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"

namespace Ui {
	class PowderCoatingPricingWindow;
}

/// <summary>
/// PowderCoatingPricingWindow inherits QDialog
/// A window to view and edit the powder coating prices.
/// </summary>
class PowderCoatingPricingWindow : public QDialog {
	Q_OBJECT
public:
	/// <summary>
	/// Opens a new window to view and edit powder coating prices.
	/// </summary>
	/// <param name="client">Network client to update database through.</param>
	/// <param name="parent">The parent of this widget.</param>
	explicit PowderCoatingPricingWindow(Client* client, QWidget *parent = nullptr);

private:
	Ui::PowderCoatingPricingWindow* ui = nullptr;
};

#endif // POWDERCOATINGPRICINGWINDOW_H