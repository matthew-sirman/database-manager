//
// Created by matthew on 12/07/2020.
//

#include "DynamicComboBox.h"

ElementIndex::ElementIndex(unsigned int index) {
    this->index = index;
    this->differentiator = 0;
}

ElementIndex::ElementIndex(unsigned int index, unsigned int differentiator) {
    this->index = index;
    this->differentiator = differentiator;
}

ElementIndex::operator unsigned() const {
    return index;
}

DynamicComboBox::DynamicComboBox(QWidget *parent) : QComboBox(parent) {

}

DynamicComboBox::~DynamicComboBox() {

}

void DynamicComboBox::setDataSource(ComboboxDataSource &dataSource) {
    delete source;
    source = &dataSource;
}

void DynamicComboBox::setFilter(const std::function<bool(const ComboboxDataElement &)> &filter) {
    sourceFilter = filter;
}

void DynamicComboBox::removeFilter() {
    sourceFilter = nullptr;
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
            if (sourceFilter) {
                if (!sourceFilter(element)) {
                    continue;
                }
            }
            if (element.index.has_value()) {
                addItem(element.text.c_str(), element.index.value());
            } else {
                addItem(element.text.c_str());
            }
        }

        for (const std::vector<ComboboxDataElement> &elementGroup : *source) {
            for (const ComboboxDataElement &element : elementGroup) {
                if (sourceFilter) {
                    if (!sourceFilter(element)) {
                        continue;
                    }
                }
                if (element.index.has_value()) {
                    QVariant var;
                    if (element.differentiator.has_value()) {
                        var.setValue(ElementIndex(element.index.value(), element.differentiator.value()));
                    } else {
                        var.setValue(ElementIndex(element.index.value()));
                    }
                    addItem(element.text.c_str(), var);
                } else {
                    addItem(element.text.c_str());
                }
            }
        }

        for (const ComboboxDataElement &element : elementsAfterSource) {
            if (sourceFilter) {
                if (!sourceFilter(element)) {
                    continue;
                }
            }
            if (element.index.has_value()) {
                addItem(element.text.c_str(), element.index.value());
            } else {
                addItem(element.text.c_str());
            }
        }

        sourceState = source->state();
    }
}

void DynamicComboBox::focusInEvent(QFocusEvent *e) {
    if (isEnabled()) {
        updateSourceList();
    }
    QComboBox::focusInEvent(e);
}
