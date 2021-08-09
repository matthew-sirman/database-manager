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

    void updateSourceList();

    void setManualIndexFunc(std::function<void(DynamicComboBox*)> func);

protected:
    void showPopup() override;

    void focusInEvent(QFocusEvent *e) override;

private:
    ComboboxDataSource *source = nullptr;

    std::vector<ComboboxDataElement> elementsBeforeSource, elementsAfterSource;

    unsigned sourceState = 0;

    std::function<void(DynamicComboBox*)> setManualIndex = nullptr;
};


#endif //DATABASE_MANAGER_DYNAMICCOMBOBOX_H
