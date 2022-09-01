// SPDX-License-Identifier: BSD-3-Clause

#include <stdexcept>
#include <sstream>
#include <vector>
#include <string>

#ifdef INVERTERCTL
#include <getopt.h>
#endif

#include "commands.h"
#include "defines.h"
#include "functions.h"
#include "../util.h"
#include "../logging.h"

namespace p18 {

const std::map<std::string, p18::CommandType> client_commands = {
    {"get-protocol-id",                  p18::CommandType::GetProtocolID},
    {"get-date-time",                    p18::CommandType::GetCurrentTime},
    {"get-total-generated",              p18::CommandType::GetTotalGenerated},
    {"get-year-generated",               p18::CommandType::GetYearGenerated},
    {"get-month-generated",              p18::CommandType::GetMonthGenerated},
    {"get-day-generated",                p18::CommandType::GetDayGenerated},
    {"get-serial-number",                p18::CommandType::GetSerialNumber},
    {"get-cpu-version",                  p18::CommandType::GetCPUVersion},
    {"get-rated",                        p18::CommandType::GetRatedInformation},
    {"get-status",                       p18::CommandType::GetGeneralStatus},
    {"get-mode",                         p18::CommandType::GetWorkingMode},
    {"get-errors",                       p18::CommandType::GetFaultsAndWarnings},
    {"get-flags",                        p18::CommandType::GetFlagsAndStatuses},
    {"get-rated-defaults",               p18::CommandType::GetRatedDefaults},
    {"get-allowed-charge-currents",      p18::CommandType::GetAllowedChargeCurrents},
    {"get-allowed-ac-charge-currents",   p18::CommandType::GetAllowedACChargeCurrents},
    {"get-p-rated",                      p18::CommandType::GetParallelRatedInformation},
    {"get-p-status",                     p18::CommandType::GetParallelGeneralStatus},
    {"get-ac-charge-time",               p18::CommandType::GetACChargeTimeBucket},
    {"get-ac-supply-time",               p18::CommandType::GetACSupplyTimeBucket},
    {"set-ac-supply",                    p18::CommandType::SetACSupply},
    {"set-flag",                         p18::CommandType::SetFlag},
    {"set-rated-defaults",               p18::CommandType::SetDefaults},
    {"set-max-charge-current",           p18::CommandType::SetBatteryMaxChargeCurrent},
    {"set-max-ac-charge-current",        p18::CommandType::SetBatteryMaxACChargeCurrent},
    {"set-ac-output-freq",               p18::CommandType::SetACOutputFreq},
    {"set-max-charge-voltage",           p18::CommandType::SetBatteryMaxChargeVoltage},
    {"set-ac-output-voltage",            p18::CommandType::SetACOutputVoltage},
    {"set-output-source-priority",       p18::CommandType::SetOutputSourcePriority},
    {"set-charge-thresholds",            p18::CommandType::SetBatteryChargeThresholds}, /* Battery re-charge and re-discharge voltage when utility is available */
    {"set-charge-source-priority",       p18::CommandType::SetChargeSourcePriority},
    {"set-solar-power-priority",         p18::CommandType::SetSolarPowerPriority},
    {"set-ac-input-voltage-range",       p18::CommandType::SetACInputVoltageRange},
    {"set-battery-type",                 p18::CommandType::SetBatteryType},
    {"set-output-mode",                  p18::CommandType::SetOutputMode},
    {"set-battery-cutoff-voltage",       p18::CommandType::SetBatteryCutOffVoltage},
    {"set-solar-configuration",          p18::CommandType::SetSolarConfig},
    {"clear-generated-data",             p18::CommandType::ClearGenerated},
    {"set-date-time",                    p18::CommandType::SetDateTime},
    {"set-ac-charge-time",               p18::CommandType::SetACChargeTimeBucket},
    {"set-ac-supply-time",               p18::CommandType::SetACSupplyTimeBucket},
};

static void validate_date_args(const std::string* ys, const std::string* ms, const std::string* ds) {
    static const std::string err_year = "invalid year";
    static const std::string err_month = "invalid month";
    static const std::string err_day = "invalid day";

    int y, m = 0, d = 0;

    // validate year
    if (!is_numeric(*ys) || ys->size() != 4)
        throw std::invalid_argument(err_year);

    y = std::stoi(*ys);
    if (y < 2000 || y > 2099)
        throw std::invalid_argument(err_year);

    // validate month
    if (ms != nullptr) {
        if (!is_numeric(*ms) || ms->size() > 2)
            throw std::invalid_argument(err_month);

        m = std::stoi(*ms);
        if (m < 1 || m > 12)
            throw std::invalid_argument(err_month);
    }

    // validate day
    if (ds != nullptr) {
        if (!is_numeric(*ds) || ds->size() > 2)
            throw std::invalid_argument(err_day);

        d = std::stoi(*ds);
        if (d < 1 || d > 31)
            throw std::invalid_argument(err_day);
    }

    if (y != 0 && m != 0 && d != 0) {
        if (!is_date_valid(y, m, d))
            throw std::invalid_argument("invalid date");
    }
}

static void validate_time_args(const std::string* hs, const std::string* ms, const std::string* ss) {
    static const std::string err_hour = "invalid hour";
    static const std::string err_minute = "invalid minute";
    static const std::string err_second = "invalid second";

    unsigned h, m, s;

    if (!is_numeric(*hs) || hs->size() > 2)
        throw std::invalid_argument(err_hour);

    h = static_cast<unsigned>(std::stoul(*hs));
    if (h > 23)
        throw std::invalid_argument(err_hour);

    if (!is_numeric(*ms) || ms->size() > 2)
        throw std::invalid_argument(err_minute);

    m = static_cast<unsigned>(std::stoul(*ms));
    if (m > 59)
        throw std::invalid_argument(err_minute);

    if (!is_numeric(*ss) || ss->size() > 2)
        throw std::invalid_argument(err_second);

    s = static_cast<unsigned>(std::stoul(*ss));
    if (s > 59)
        throw std::invalid_argument(err_second);
}


#define GET_ARGS(__len__) get_args((CommandInput*)input, arguments, (__len__))

#ifdef INVERTERCTL
static void get_args(CommandInput* input,
                     std::vector<std::string>& arguments,
                     size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (optind < input->argc && *input->argv[optind] != '-')
            arguments.emplace_back(input->argv[optind++]);
        else {
            std::ostringstream error;
            error << "this command requires " << count << " argument";
            if (count > 1)
                error << "s";
            throw std::invalid_argument(error.str());
        }
    }
}
#endif

#ifdef INVERTERD
static void get_args(CommandInput* input,
                     std::vector<std::string>& arguments,
                     size_t count) {
    if (input->argv->size() < count) {
        std::ostringstream error;
        error << "this command requires " << count << " argument";
        if (count > 1)
            error << "s";
        throw std::invalid_argument(error.str());
    }

    for (size_t i = 0; i < count; i++)
        arguments.emplace_back((*input->argv)[i]);
}
#endif

p18::CommandType validate_input(std::string& command,
                                std::vector<std::string>& arguments,
                                void* input) {
    auto it = p18::client_commands.find(command);
    if (it == p18::client_commands.end())
        throw std::invalid_argument("invalid command");

    auto commandType = it->second;
    switch (commandType) {
    case p18::CommandType::GetYearGenerated:
        GET_ARGS(1);
        validate_date_args(&arguments[0], nullptr, nullptr);
        break;

    case p18::CommandType::GetMonthGenerated:
        GET_ARGS(2);
        validate_date_args(&arguments[0], &arguments[1], nullptr);
        break;

    case p18::CommandType::GetDayGenerated:
        GET_ARGS(3);
        validate_date_args(&arguments[0], &arguments[1], &arguments[2]);
        break;

    case p18::CommandType::GetParallelRatedInformation:
    case p18::CommandType::GetParallelGeneralStatus:
        GET_ARGS(1);
        if (!is_numeric(arguments[0]) || arguments[0].size() > 1)
            throw std::invalid_argument("invalid argument");
        break;

    case p18::CommandType::SetACSupply: {
        GET_ARGS(1);
        std::string &arg = arguments[0];
        if (arg != "0" && arg != "1")
            throw std::invalid_argument("invalid argument, only 0 or 1 allowed");
        break;
    }

    case p18::CommandType::SetFlag: {
        GET_ARGS(2);

        bool match_found = false;
        for (auto const& item: p18::flags) {
            if (arguments[0] == item.flag) {
                arguments[0] = item.letter;
                match_found = true;
                break;
            }
        }

        if (!match_found)
            throw std::invalid_argument("invalid flag");

        if (arguments[1] != "0" && arguments[1] != "1")
            throw std::invalid_argument("invalid flag state, only 0 or 1 allowed");

        break;
    }

    case p18::CommandType::SetBatteryMaxChargeCurrent:
    case p18::CommandType::SetBatteryMaxACChargeCurrent: {
        GET_ARGS(2);

        auto id = static_cast<unsigned>(std::stoul(arguments[0]));
        auto amps = static_cast<unsigned>(std::stoul(arguments[1]));

        if (!p18::is_valid_parallel_id(id))
            throw std::invalid_argument("invalid id");

        // 3 characters max
        if (amps > 999)
            throw std::invalid_argument("invalid amps");

        break;
    }

    case p18::CommandType::SetACOutputFreq: {
        GET_ARGS(1);
        std::string &freq = arguments[0];
        if (freq != "50" && freq != "60")
            throw std::invalid_argument("invalid frequency, only 50 or 60 allowed");
        break;
    }

    case p18::CommandType::SetBatteryMaxChargeVoltage: {
        GET_ARGS(2);

        float cv = std::stof(arguments[0]);
        float fv = std::stof(arguments[1]);

        if (cv < 48.0 || cv > 58.4)
            throw std::invalid_argument("invalid CV");

        if (fv < 48.0 || fv > 58.4)
            throw std::invalid_argument("invalid FV");

        break;
    }

    case p18::CommandType::SetACOutputVoltage: {
        GET_ARGS(1);

        auto v = static_cast<unsigned>(std::stoul(arguments[0]));

        bool matchFound = false;
        for (const auto &item: p18::ac_output_voltages) {
            if (v == item) {
                matchFound = true;
                break;
            }
        }

        if (!matchFound)
            throw std::invalid_argument("invalid voltage");

        break;
    }

    case p18::CommandType::SetOutputSourcePriority: {
        GET_ARGS(1);

        std::array<std::string, 2> priorities({"SUB", "SBU"});

        long index = index_of(priorities, arguments[0]);
        if (index == -1)
            throw std::invalid_argument("invalid argument");

        arguments[0] = std::to_string(index);
        break;
    }

    case p18::CommandType::SetBatteryChargeThresholds: {
        GET_ARGS(2);

        float cv = std::stof(arguments[0]);
        float dv = std::stof(arguments[1]);

        if (index_of(p18::bat_ac_recharge_voltages_12v, cv) == -1 &&
            index_of(p18::bat_ac_recharge_voltages_24v, cv) == -1 &&
            index_of(p18::bat_ac_recharge_voltages_48v, cv) == -1)
            throw std::invalid_argument("invalid CV");

        if (index_of(p18::bat_ac_redischarge_voltages_12v, dv) == -1 &&
            index_of(p18::bat_ac_redischarge_voltages_24v, dv) == -1 &&
            index_of(p18::bat_ac_redischarge_voltages_48v, dv) == -1)
            throw std::invalid_argument("invalid DV");

        break;
    }

    case p18::CommandType::SetChargeSourcePriority: {
        GET_ARGS(2);

        auto id = static_cast<unsigned>(std::stoul(arguments[0]));
        if (!p18::is_valid_parallel_id(id))
            throw std::invalid_argument("invalid id");

        std::array<std::string, 3> priorities({"SF", "SU", "S"});
        long index = index_of(priorities, arguments[1]);
        if (index == -1)
            throw std::invalid_argument("invalid argument");

        arguments[1] = std::to_string(index);
        break;
    }

    case p18::CommandType::SetSolarPowerPriority: {
        GET_ARGS(1);

        std::array<std::string, 2> allowed({"BLU", "LBU"});
        long index = index_of(allowed, arguments[0]);
        if (index == -1)
            throw std::invalid_argument("invalid priority");

        arguments[0] = std::to_string(index);
        break;
    }

    case p18::CommandType::SetACInputVoltageRange: {
        GET_ARGS(1);
        std::array<std::string, 2> allowed({"APPLIANCE", "UPS"});
        long index = index_of(allowed, arguments[0]);
        if (index == -1)
            throw std::invalid_argument("invalid argument");
        arguments[0] = std::to_string(index);
        break;
    }

    case p18::CommandType::SetBatteryType: {
        GET_ARGS(1);

        std::array<std::string, 3> allowed({"AGM", "FLOODED", "USER"});
        long index = index_of(allowed, arguments[0]);
        if (index == -1)
            throw std::invalid_argument("invalid type");
        arguments[0] = std::to_string(index);

        break;
    }

    case p18::CommandType::SetOutputMode: {
        GET_ARGS(2);

        auto id = static_cast<unsigned>(std::stoul(arguments[0]));
        if (!p18::is_valid_parallel_id(id))
            throw std::invalid_argument("invalid id");

        std::array<std::string, 5> allowed({"S", "P", "1", "2", "3"});
        long index = index_of(allowed, arguments[1]);
        if (index == -1)
            throw std::invalid_argument("invalid model");
        arguments[1] = std::to_string(index);

        break;
    }

    case p18::CommandType::SetBatteryCutOffVoltage: {
        GET_ARGS(1);

        float v = std::stof(arguments[0]);
        if (v < 40.0 || v > 48.0)
            throw std::invalid_argument("invalid voltage");

        break;
    }

    case p18::CommandType::SetSolarConfig: {
        GET_ARGS(1);

        if (!is_numeric(arguments[0]) || arguments[0].size() > 20)
            throw std::invalid_argument("invalid argument");

        break;
    }

    case p18::CommandType::SetDateTime: {
        GET_ARGS(6);

        validate_date_args(&arguments[0], &arguments[1], &arguments[2]);
        validate_time_args(&arguments[3], &arguments[4], &arguments[5]);

        break;
    }

    case p18::CommandType::SetACChargeTimeBucket:
    case p18::CommandType::SetACSupplyTimeBucket: {
        GET_ARGS(2);

        std::vector<std::string> start = split(arguments[0], ':');
        if (start.size() != 2)
            throw std::invalid_argument("invalid start time");

        std::vector<std::string> end = split(arguments[1], ':');
        if (end.size() != 2)
            throw std::invalid_argument("invalid end time");

        auto startHour = static_cast<unsigned short>(std::stoul(start[0]));
        auto startMinute = static_cast<unsigned short>(std::stoul(start[1]));
        if (startHour > 23 || startMinute > 59)
            throw std::invalid_argument("invalid start time");

        auto endHour = static_cast<unsigned short>(std::stoul(end[0]));
        auto endMinute = static_cast<unsigned short>(std::stoul(end[1]));
        if (endHour > 23 || endMinute > 59)
            throw std::invalid_argument("invalid end time");

        arguments.clear();

        arguments.emplace_back(std::to_string(startHour));
        arguments.emplace_back(std::to_string(startMinute));

        arguments.emplace_back(std::to_string(endHour));
        arguments.emplace_back(std::to_string(endMinute));

        break;
    }

    default:
        break;
    }

    return commandType;
}

}