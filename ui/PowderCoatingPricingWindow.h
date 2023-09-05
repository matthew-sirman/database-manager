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

class PowderCoatingPricingWindow : public QDialog {
	Q_OBJECT
public:
	explicit PowderCoatingPricingWindow(Client* client, QWidget *parent = nullptr);

private:
	Ui::PowderCoatingPricingWindow* ui = nullptr;
};

#endif // POWDERCOATINGPRICINGWINDOW_H