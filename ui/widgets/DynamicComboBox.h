//
// Created by matthew on 12/07/2020.
//

#ifndef DATABASE_MANAGER_DYNAMICCOMBOBOX_H
#define DATABASE_MANAGER_DYNAMICCOMBOBOX_H

#include <QComboBox>
#include <QEvent>
#include <QLineEdit>

#include "../../include/database/ComboboxDataSource.h"

/// <summary>
/// DynamicComboBox inherits QComboBox
/// A dynamic combo box that can populate itself with all components registered with the DrawingComponentManager
/// </summary>
class DynamicComboBox : public QComboBox {
    Q_OBJECT
public:
    /// <summary>
    /// Constructs a new DynamicComboBox
    /// </summary>
    /// <param name="parent">The parent of this widget.</param>
    explicit DynamicComboBox(QWidget *parent = nullptr);

    /// <summary>
    /// Default destructor
    /// </summary>
    ~DynamicComboBox() override;

    /// <summary>
    /// Adds an additional element to the combo box
    /// </summary>
    /// <param name="element">The element to add.</param>
    /// <param name="beforeSource">Whether or no the combo box has sourced its elements.</param>
    void addExtraSourceItem(const ComboboxDataElement& element, bool beforeSource=true);

    /// <summary>
    /// Sets the data source to fill components from.
    /// </summary>
    /// <param name="dataSource">Source to populate from.</param>
    void setDataSource(ComboboxDataSource &dataSource);

    /// <summary>
    /// Forces an updates from the source.
    /// </summary>
    void updateSourceList();

    /// <summary>
    /// Set a function to be called on the initialisation of a combobox, to set the first index manually.
    /// </summary>
    /// <param name="func">The new indexing function.</param>
    void setManualIndexFunc(std::function<void(DynamicComboBox*)> func);

protected:
    /// <summary>
    /// forces an update of the source, then propogates.
    /// </summary>
    void showPopup() override;

    /// <summary>
    /// forces an update of the source, then propogates.
    /// </summary>
    /// <param name="e">The causing event.</param>
    void focusInEvent(QFocusEvent *e) override;

private:
    ComboboxDataSource *source = nullptr;

    std::vector<ComboboxDataElement> elementsBeforeSource, elementsAfterSource;

    unsigned sourceState = 0;

    // Lambda that sets the index of the combobox at the begining
    std::function<void(DynamicComboBox*)> setManualIndex = nullptr;
};


#endif //DATABASE_MANAGER_DYNAMICCOMBOBOX_H
