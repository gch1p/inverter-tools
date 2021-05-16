// SPDX-License-Identifier: BSD-3-Clause

#include "common.h"
#include <stdexcept>

formatter::Format format_from_string(std::string& s) {
    if (s == "json")
        return formatter::Format::JSON;
    else if (s == "simple-json")
        return formatter::Format::SimpleJSON;
    else if (s == "table")
        return formatter::Format::Table;
    else if (s == "simple-table")
        return formatter::Format::SimpleTable;
    else
        throw std::invalid_argument("invalid format");
}