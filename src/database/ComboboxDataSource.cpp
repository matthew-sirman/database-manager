//
// Created by matthew on 12/07/2020.
//

#include "../../include/database/ComboboxDataSource.h"

// Constructor taking just the text argument
ComboboxDataElement::ComboboxDataElement(const std::string &text) {
    this->text = text;
    // Set the index to a nullopt by default so it has "no" value
    this->index = std::nullopt;
}

// Constructor taking both the text and index arguments
ComboboxDataElement::ComboboxDataElement(const std::string &text, unsigned index) {
    this->text = text;
    this->index = index;
}

// Equality operator
bool ComboboxDataElement::operator==(const ComboboxDataElement &other) const {
    return (this->text == other.text) && (this->index == other.index);
}

// Inequality operator
bool ComboboxDataElement::operator!=(const ComboboxDataElement &other) const {
    return (this->text != other.text) || (this->index != other.index);
}
