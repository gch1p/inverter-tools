// SPDX-License-Identifier: BSD-3-Clause

#include <string>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <cxxabi.h>

#include "util.h"

bool is_numeric(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool is_date_valid(const int y, const int m, const int d) {
    /* primitive out of range checks */
    if (y < 2000 || y > 2099)
        return false;

    if (d < 1 || d > 31)
        return false;

    if (m < 1 || m > 12)
        return false;

    /* some more clever date validity checks */
    if ((m == 4 || m == 6 || m == 9 || m == 11) && d == 31)
        return false;

    /* and finally a february check */
    /* i always wondered, when do people born at feb 29 celebrate their bday? */
    return m != 2 || ((y % 4 != 0 && d <= 28) || (y % 4 == 0 && d <= 29));
}

std::vector<std::string> split(const std::string& s, char separator) {
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;

    while ((pos = s.find(separator, pos)) != std::string::npos) {
        std::string substring(s.substr(prev_pos, pos-prev_pos));
        output.push_back(substring);
        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos));

    return output;
}

unsigned stou(const std::string& s) {
    return static_cast<unsigned>(std::stoul(s));
}

unsigned short stouh(const std::string& s) {
    return static_cast<unsigned short>(std::stoul(s));
}

bool string_has(std::string& s, char c) {
    return s.find(c) != std::string::npos;
}

unsigned long hextoul(std::string& s) {
    // strtol will store a pointer to first invalid character here
    char* endptr = nullptr;

    unsigned long n = strtol(s.c_str(), &endptr, 16);
    if (*endptr != 0)
        throw std::invalid_argument("input string is not a hex number");

    return n;
}

// https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
std::string demangle_type_name(const char* name) {
    int status = -4; // some arbitrary value to eliminate the compiler warning

    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };

    return status == 0 ? res.get() : name;
}