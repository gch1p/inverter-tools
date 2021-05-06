// SPDX-License-Identifier: BSD-3-Clause

#ifndef INFINISOLAR_TOOLS_P18_EXCEPTIONS_H
#define INFINISOLAR_TOOLS_P18_EXCEPTIONS_H

#include <stdexcept>

namespace p18 {

class InvalidResponseError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ParseError : public InvalidResponseError {
public:
    using InvalidResponseError::InvalidResponseError;
};

}

#endif //INFINISOLAR_TOOLS_P18_EXCEPTIONS_H
