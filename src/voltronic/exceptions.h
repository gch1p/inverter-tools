// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_VOLTRONIC_EXCEPTIONS_H
#define INVERTER_TOOLS_VOLTRONIC_EXCEPTIONS_H

#include <stdexcept>

namespace voltronic {

class DeviceError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class TimeoutError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class InvalidDataError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

}

#endif //INVERTER_TOOLS_VOLTRONIC_EXCEPTIONS_H
