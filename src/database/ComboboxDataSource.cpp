//
// Created by matthew on 12/07/2020.
//

#include "../../include/database/ComboboxDataSource.h"

ComboboxDataElement::ComboboxDataElement(const std::string &text) {
    this->text = text;
    this->index = std::nullopt;
    this->differentiator = std::nullopt;
}

ComboboxDataElement::ComboboxDataElement(const std::string &text, unsigned int index) {
    this->text = text;
    this->index = index;
    this->differentiator = std::nullopt;
}

ComboboxDataElement::ComboboxDataElement(const std::string &text, unsigned int index, unsigned int differentiator) {
    this->text = text;
    this->index = index;
    this->differentiator = differentiator;
}

bool ComboboxDataElement::operator==(const ComboboxDataElement &other) const {
    return (this->text == other.text) && (this->index == other.index) && (this->differentiator == other.differentiator);
}

bool ComboboxDataElement::operator!=(const ComboboxDataElement &other) const {
    return (this->text != other.text) || (this->index != other.index) || (this->differentiator != other.differentiator);
}
