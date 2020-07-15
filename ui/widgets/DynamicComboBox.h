//
// Created by matthew on 12/07/2020.
//

#ifndef DATABASE_MANAGER_DYNAMICCOMBOBOX_H
#define DATABASE_MANAGER_DYNAMICCOMBOBOX_H

#include <QComboBox>
#include <QEvent>

#include "../../include/database/ComboboxDataSource.h"

class DynamicComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit DynamicComboBox(QWidget *parent = nullptr);

    ~DynamicComboBox() override;

    void addExtraSourceItem(const ComboboxDataElement& element, bool beforeSource=true);

    void setDataSource(ComboboxDataSource &dataSource);

protected:
    void showPopup() override;

    bool event(QEvent *event) override;

private:
    ComboboxDataSource *source = nullptr;

    std::vector<ComboboxDataElement> elementsBeforeSource, elementsAfterSource;

    unsigned sourceState = 0;

    void updateSourceList();
};


#endif //DATABASE_MANAGER_DYNAMICCOMBOBOX_H
