// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_P18_DEFINES_H
#define INVERTER_TOOLS_P18_DEFINES_H

#include <map>
#include <string>
#include <array>

#include "types.h"

namespace p18 {

extern const std::map<CommandType, std::string> raw_commands;

extern const std::array<int, 5> ac_output_rated_voltages;

extern const std::array<float, 8> bat_ac_recharging_voltages_12v;
extern const std::array<float, 8> bat_ac_recharging_voltages_24v;
extern const std::array<float, 8> bat_ac_recharging_voltages_48v;

extern const std::array<float, 12> bat_ac_redischarging_voltages_12v;
extern const std::array<float, 12> bat_ac_redischarging_voltages_24v;
extern const std::array<float, 12> bat_ac_redischarging_voltages_48v;

extern const std::map<int, std::string> fault_codes;

extern const std::array<Flag, 9> flags;

}

#endif //INVERTER_TOOLS_P18_DEFINES_H
