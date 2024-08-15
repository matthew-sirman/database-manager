#pragma once

#include "../include/database/DatabaseQuery.h"
#include "../include/networking/Client.h"
#include "widgets/DynamicComboBox.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>

namespace Ui {
    class AddExtraPriceWindow;
}

/// <summary>
/// AddExtraPriceWindow inherits QDialog
/// A dialog to edit a specific extra price.
/// </summary>
class AddExtraPriceWindow : public QDialog {
    Q_OBJECT
public:
    /// <summary>
    /// Creates a new dialog which allows the provided extra price to be edited.
    /// </summary>
	/// <param name="client">The TCP client for updating the database.</param>
    /// <param name="price">The extra price to be edited.</param>
    /// <param name="parent">The parent widget of this.</param>
    explicit AddExtraPriceWindow(Client* client, ExtraPrice& price, QWidget* parent = nullptr);
private:
    Ui::AddExtraPriceWindow* ui = nullptr;
signals:
    void updateParent(QString, std::optional<QString>);
};