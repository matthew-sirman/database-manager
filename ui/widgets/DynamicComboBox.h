//
// Created by matthew on 12/07/2020.
//

#ifndef DATABASE_MANAGER_DYNAMICCOMBOBOX_H
#define DATABASE_MANAGER_DYNAMICCOMBOBOX_H

#include <QComboBox>
#include <QEvent>
#include <QLineEdit>

#include "../../include/database/ComboboxDataSource.h"

class DynamicComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit DynamicComboBox(QWidget *parent = nullptr);

    ~DynamicComboBox() override;

    void addExtraSourceItem(const ComboboxDataElement& element, bool beforeSource=true);

    void setDataSource(ComboboxDataSource &dataSource);

    void setFilter(const std::function<bool(const ComboboxDataElement &)> &filter);

    void removeFilter();

    void updateSourceList();

protected:
    void showPopup() override;

    void focusInEvent(QFocusEvent *e) override;

private:
    ComboboxDataSource *source = nullptr;

    std::vector<ComboboxDataElement> elementsBeforeSource, elementsAfterSource;

    std::function<bool(const ComboboxDataElement &)> sourceFilter = nullptr;

    unsigned sourceState = 0;
};


#endif //DATABASE_MANAGER_DYNAMICCOMBOBOX_H
