//
// Created by matthew on 12/07/2020.
//

#ifndef DATABASE_MANAGER_COMBOBOXDATASOURCE_H
#define DATABASE_MANAGER_COMBOBOXDATASOURCE_H

#include <string>
#include <vector>
#include <optional>

#include "DataSource.h"

struct ComboboxDataElement {
    std::string text;
    std::optional<unsigned> index = std::nullopt;
    std::optional<unsigned> differentiator = std::nullopt;

    ComboboxDataElement(const std::string &text);

    ComboboxDataElement(const std::string &text, unsigned index);

    ComboboxDataElement(const std::string &text, unsigned index, unsigned differentiator);

    bool operator==(const ComboboxDataElement &other) const;

    bool operator!=(const ComboboxDataElement &other) const;
};

typedef DataSource<std::vector<ComboboxDataElement>, std::vector<unsigned>::const_iterator> ComboboxDataSource;


#endif //DATABASE_MANAGER_COMBOBOXDATASOURCE_H
