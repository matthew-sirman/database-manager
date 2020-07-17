//
// Created by matthew on 12/07/2020.
//

#include "DynamicComboBox.h"

DynamicComboBox::DynamicComboBox(QWidget *parent) : QComboBox(parent) {

}

DynamicComboBox::~DynamicComboBox() {

}

void DynamicComboBox::setDataSource(ComboboxDataSource &dataSource) {
    delete source;
    source = &dataSource;
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

bool DynamicComboBox::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::MouseButtonRelease:
            if (isEnabled()) {
                updateSourceList();
            }
            break;
        default:
            break;
    }

    return QComboBox::event(event);
}

void DynamicComboBox::updateSourceList() {
    if (sourceState != source->state()) {
        clear();

        for (const ComboboxDataElement &element : elementsBeforeSource) {
            if (element.index.has_value()) {
                addItem(element.text.c_str(), element.index.value());
            } else {
                addItem(element.text.c_str());
            }
        }

        for (const ComboboxDataElement &element : *source) {
            if (element.index.has_value()) {
                addItem(element.text.c_str(), element.index.value());
            } else {
                addItem(element.text.c_str());
            }
        }

        for (const ComboboxDataElement &element : elementsAfterSource) {
            if (element.index.has_value()) {
                addItem(element.text.c_str(), element.index.value());
            } else {
                addItem(element.text.c_str());
            }
        }

        sourceState = source->state();
    }
}
