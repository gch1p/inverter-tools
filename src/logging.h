// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_LOGGING_H
#define INVERTER_TOOLS_LOGGING_H

#include <iostream>
#include <string>
#include <string_view>

class custom_log
{
private:
    std::ostream& os_;

public:
    custom_log(std::ostream& os, const std::string& func) : os_(os) {
        os_ << func << ": ";
    }

    template <class T>
    custom_log &operator<<(const T &v) {
        os_ << v;
        return *this;
    }

    ~custom_log() {
        os_ << std::endl;
    }
};

inline std::string method_name(const std::string& function, const std::string& pretty) {
    size_t locFunName = pretty.find(function);
    size_t begin = pretty.rfind(" ", locFunName) + 1;
    size_t end = pretty.find("(", locFunName + function.length());
    return pretty.substr(begin, end - begin) + "()";
 }

#define __METHOD_NAME__ method_name(__FUNCTION__, __PRETTY_FUNCTION__)

#define mylog custom_log(std::cout, __METHOD_NAME__)
#define myerr custom_log(std::cerr, __METHOD_NAME__)

#endif //INVERTER_TOOLS_LOGGING_H
