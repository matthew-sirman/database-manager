#ifndef SPECIFICSIDEIRONPRICINGWINDOW_H
#define SPECIFICSIDEIRONPRICINGWINDOW_H

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"
#include "AddSpecificSideIronPriceWindow.h"

#include <qdialog.h>
#include <QDialogButtonBox>
#include <QPushButton>

namespace Ui {
	class SpecificSideIronPricingWindow;
}

/// <summary>
/// SpecificSideIronPricingWindow inherits QDialog
/// Opens a new window to view prices for specific side iron, and can open a AddSpecificSideIronPriceWindow to edit the price.
/// </summary>
class SpecificSideIronPricingWindow : public QDialog {
	Q_OBJECT
public:
	/// <summary>
	/// Constructs a new dialog for viewing the specific side iron prices.
	/// </summary>
	/// <param name="client">The network client to update the database through.</param>
	/// <param name="parent">The parent of this widget.</param>
	explicit SpecificSideIronPricingWindow(Client* client, QWidget* parent = nullptr);

	/// <summary>
	/// Causes the displayed information to be updated.
	/// </summary>
	void update();

	/// <summary>
	/// Causes an \ref update() on the next paint event.
	/// </summary>
	void setUpdateRequired();
protected:
	/// <summary>
	/// Overriden paint event to call \ref update() whenever \ref setUpdateRequired() is called, then propogate.
	/// </summary>
	/// <param name="event">The event that triggered this function.</param>
	void paintEvent(QPaintEvent* event) override;
private:
	Ui::SpecificSideIronPricingWindow* ui = nullptr;

	DynamicComboBox* sideIronComboBox;

	ComboboxComponentDataSource<SideIron> sideIronSource;

	bool updateRequired = false;
};

#endif // SPECIFICSIDEIRONPRICINGWINDOW_H