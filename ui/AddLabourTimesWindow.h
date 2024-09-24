#pragma once

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <PricingPackage.h>

namespace Ui {
    class AddLabourTimesWindow;
}

/// <summary>
/// AddLabourTimesWindow inherits QDialog
/// Opens a new dialog that allows edits to be made to any labour times.
/// </summary>
class AddLabourTimesWindow : public QDialog {
    Q_OBJECT
public:
    /// <summary>
    /// Creates a new window to edit labour times.
    /// </summary>
    /// <param name="client">The network client for updating the database through.</param>
    /// <param name="times">The labour time to be editted.</param>
    /// <param name="parent">The parent of this widget.</param>
    explicit AddLabourTimesWindow(Client* client, LabourTime& times, QWidget* parent = nullptr); // Update
private:
    Ui::AddLabourTimesWindow* ui = nullptr;
};