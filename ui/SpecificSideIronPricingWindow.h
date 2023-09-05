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

class SpecificSideIronPricingWindow : public QDialog {
	Q_OBJECT
public:
	explicit SpecificSideIronPricingWindow(Client* client, QWidget* = nullptr);

	void update();

private:
	Ui::SpecificSideIronPricingWindow* ui = nullptr;

	DynamicComboBox* sideIronComboBox;

	ComboboxComponentDataSource<SideIron> sideIronSource;
};

#endif // SPECIFICSIDEIRONPRICINGWINDOW_H