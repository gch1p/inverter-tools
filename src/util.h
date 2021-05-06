// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_UTIL_H
#define INVERTER_TOOLS_UTIL_H

#include <string>
#include <vector>
#include <algorithm>

bool is_numeric(const std::string& s);
bool is_date_valid(int y, int m, int d);

template <typename T, typename P>
long index_of(T& haystack, P& needle)
{
    auto _it = std::find(haystack.begin(), haystack.end(), needle);
    if (_it == haystack.end())
        return -1;
    return std::distance(haystack.begin(), _it);
}

std::vector<std::string> split(const std::string& s, char separator);
unsigned stou(const std::string& s);
unsigned short stouh(const std::string& s);

bool string_has(std::string& s, char c);
unsigned long hextoul(std::string& s);

std::string demangle_type_name(const char* name);

#endif //INVERTER_TOOLS_UTIL_H
