//
// Created by matthew on 12/07/2020.
//

#include "DynamicComboBox.h"
#include <iostream>

DynamicComboBox::DynamicComboBox(QWidget *parent) : QComboBox(parent) {

}

DynamicComboBox::~DynamicComboBox() {

}

void DynamicComboBox::setDataSource(ComboboxDataSource &dataSource) {
    delete source;
    source = &dataSource;
    updateSourceList();
}

void DynamicComboBox::showPopup() {
    updateSourceList();

    QComboBox::showPopup();
}

void DynamicComboBox::addExtraSourceItem(const ComboboxDataElement& element, bool beforeSource) {
    if (beforeSource) {
        elementsBeforeSource.push_back(element);
    } else {
        elementsAfterSource.push_back(element);
    }
}

void DynamicComboBox::updateSourceList() {
    if (sourceState != source->state()) {
        clear();

        for (const ComboboxDataElement &element : elementsBeforeSource) {
            /*if (sourceFilter) {
                if (!sourceFilter(element)) {
                    continue;
                }
            }*/
            if (element.index.has_value()) {
                addItem(element.text.c_str(), element.index.value());
            } else {
                addItem(element.text.c_str());
            }
        }

        for (const ComboboxDataElement &element : *source) {
            /*if (source->filter()) {
                if (!sourceFilter(element)) {
                    continue;
                }
            }*/
            if (element.index.has_value()) {
                addItem(element.text.c_str(), element.index.value());
            } else {
                addItem(element.text.c_str());
            }
        }

        for (const ComboboxDataElement &element : elementsAfterSource) {
            /*if (sourceFilter) {
                if (!sourceFilter(element)) {
                    continue;
                }
            }*/
            if (element.index.has_value()) {
                addItem(element.text.c_str(), element.index.value());
            } else {
                addItem(element.text.c_str());
            }
        }

        sourceState = source->state();
    }

    if (currentIndex() == 0 && setManualIndex != nullptr) {
        setManualIndex(this);
        setManualIndex = nullptr;
    }
}

void DynamicComboBox::setManualIndexFunc(std::function<void(DynamicComboBox*)> func) {
    setManualIndex = func;
}

void DynamicComboBox::focusInEvent(QFocusEvent *e) {
    if (isEnabled()) {
        updateSourceList();
    }
    QComboBox::focusInEvent(e);
}
