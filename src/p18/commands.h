// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_P18_COMMANDS_H
#define INVERTER_TOOLS_P18_COMMANDS_H

#include <map>
#include <string>
#include <vector>

#include "types.h"

namespace p18 {

#ifdef INVERTERCTL
struct CommandInput {
    int argc;
    char** argv;
};
#endif

#ifdef INVERTERD
struct CommandInput {
    std::vector<std::string>* argv;
};
#endif

extern const std::map<std::string, p18::CommandType> client_commands;

static void validate_date_args(const std::string* ys, const std::string* ms, const std::string* ds);
static void validate_time_args(const std::string* hs, const std::string* ms, const std::string* ss);
CommandType validate_input(std::string& command, std::vector<std::string>& arguments, void* input);

}

#endif //INVERTER_TOOLS_P18_COMMANDS_H
