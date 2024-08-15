//
// Created by matthew on 12/07/2020.
//

#ifndef DATABASE_MANAGER_COMBOBOXDATASOURCE_H
#define DATABASE_MANAGER_COMBOBOXDATASOURCE_H

#include <string>
#include <vector>
#include <optional>

#include "DataSource.h"

/// <summary>\ingroup database
/// ComboboxDataElement
/// Stores a single data element for a ComboBox data source.
/// An element contains a string to be displayed and an associated index
/// to retrieve the data element.
/// </summary>
struct ComboboxDataElement {
    // String for storing the element's text
    std::string text;
    // Optional index to associate with the element. If no index is specified, the
    // combobox should just display the text.
    std::optional<unsigned> index = std::nullopt;

    /// <summary>
    /// Constructor for ComboboxDataSource
    /// </summary>
    /// <param name="text">The text field to show in the ComboBox</param>
    ComboboxDataElement(const std::string &text);

    /// <summary>
    /// Constructor for ComboboxDataSource
    /// </summary>
    /// <param name="text">The text field to show in the ComboBox</param>
    /// <param name="index">The associated element index for this item</param>
    ComboboxDataElement(const std::string &text, unsigned index);

    // Equality operator. Equal if and only if the texts and indices are both equal.
    bool operator==(const ComboboxDataElement &other) const;

    // Inequality operator. Inequal if and only if the texts are different 
    // or the indices are different.
    bool operator!=(const ComboboxDataElement &other) const;
};

/// <summary>\ingroup database
/// ComboboxDataSource
/// This is the type for the base datasource which uses a vector of unsigned
/// values as the base iterator, and adapts them to the type of ComboboxDataElement
/// for use in a DynamicComboBox.
/// </summary>
typedef DataSource<ComboboxDataElement, std::vector<unsigned>::const_iterator> ComboboxDataSource;


#endif //DATABASE_MANAGER_COMBOBOXDATASOURCE_H
