// SPDX-License-Identifier: BSD-3-Clause

#include <iostream>

#include "defines.h"
#include "types.h"

namespace p18 {

const std::map<CommandType, std::string> raw_commands = {
    {CommandType::GetProtocolID,               "PI"},
    {CommandType::GetCurrentTime,              "T"},
    {CommandType::GetTotalGenerated,           "ET"},
    {CommandType::GetYearGenerated,            "EY"},
    {CommandType::GetMonthGenerated,           "EM"},
    {CommandType::GetDayGenerated,             "ED"},
    {CommandType::GetSerialNumber,             "ID"},
    {CommandType::GetCPUVersion,               "VFW"},
    {CommandType::GetRatedInformation,         "PIRI"},
    {CommandType::GetGeneralStatus,            "GS"},
    {CommandType::GetWorkingMode,              "MOD"},
    {CommandType::GetFaultsAndWarnings,        "FWS"},
    {CommandType::GetFlagsAndStatuses,         "FLAG"},
    {CommandType::GetRatedDefaults,            "DI"},
    {CommandType::GetAllowedChargeCurrents,    "MCHGCR"},
    {CommandType::GetAllowedACChargeCurrents,  "MUCHGCR"},
    {CommandType::GetParallelRatedInformation, "PRI"},
    {CommandType::GetParallelGeneralStatus,    "PGS"},
    {CommandType::GetACChargeTimeBucket,       "ACCT"},
    {CommandType::GetACSupplyTimeBucket,       "ACLT"},
    {CommandType::SetACSupply,                 "LON"},
    {CommandType::SetFlag,                     "P"},
    {CommandType::SetDefaults,                 "PF"},
    {CommandType::SetBatteryMaxChargeCurrent,  "MCHGC"},
    {CommandType::SetBatteryMaxACChargeCurrent, "MUCHGC"},
    /* The protocol documentation defines two commands, "F50" and "F60",
       but it's identical as if there were just one "F" command with an argument. */
    {CommandType::SetACOutputFreq,            "F"},
    {CommandType::SetBatteryMaxChargeVoltage, "MCHGV"},
    {CommandType::SetACOutputVoltage,         "V"},
    {CommandType::SetOutputSourcePriority,    "POP"},
    {CommandType::SetBatteryChargeThresholds, "BUCD"},
    {CommandType::SetChargeSourcePriority,    "PCP"},
    {CommandType::SetSolarPowerPriority,      "PSP"},
    {CommandType::SetACInputVoltageRange,     "PGR"},
    {CommandType::SetBatteryType,             "PBT"},
    {CommandType::SetOutputMode,              "POPM"},
    {CommandType::SetBatteryCutOffVoltage,    "PSDV"},
    {CommandType::SetSolarConfig,             "ID"},
    {CommandType::ClearGenerated,             "CLE"},
    {CommandType::SetDateTime,                "DAT"},
    {CommandType::SetACChargeTimeBucket,      "ACCT"},
    {CommandType::SetACSupplyTimeBucket,      "ACLT"},
};

const std::array<int, 5> ac_output_voltages = {202, 208, 220, 230, 240};

const std::array<float, 8> bat_ac_recharge_voltages_12v = {11, 11.3, 11.5, 11.8, 12, 12.3, 12.5, 12.8};
const std::array<float, 8> bat_ac_recharge_voltages_24v = {22, 22.5, 23, 23.5, 24, 24.5, 25, 25.5};
const std::array<float, 8> bat_ac_recharge_voltages_48v = {44, 45, 46, 47, 48, 49, 50, 51};

const std::array<float, 12> bat_ac_redischarge_voltages_12v = {0, 12, 12.3, 12.5, 12.8, 13, 13.3, 13.5, 13.8, 14, 14.3, 14.5};
const std::array<float, 12> bat_ac_redischarge_voltages_24v = {0, 24, 24.5, 25, 25.5, 26, 26.5, 27, 27.5, 28, 28.5, 29};
const std::array<float, 12> bat_ac_redischarge_voltages_48v = {0, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58};

const std::map<int, std::string> fault_codes = {
    {1, "Fan is locked"},
    {2, "Over temperature"},
    {3, "Battery voltage is too high"},
    {4, "Battery voltage is too low"},
    {5, "Output short circuited or Over temperature"},
    {6, "Output voltage is too high"},
    {7, "Over load time out"},
    {8, "Bus voltage is too high"},
    {9, "Bus soft start failed"},
    {11, "Main relay failed"},
    {51, "Over current inverter"},
    {52, "Bus soft start failed"},
    {53, "Inverter soft start failed"},
    {54, "Self-test failed"},
    {55, "Over DC voltage on output of inverter"},
    {56, "Battery connection is open"},
    {57, "Current sensor failed"},
    {58, "Output voltage is too low"},
    {60, "Inverter negative power"},
    {71, "Parallel version different"},
    {72, "Output circuit failed"},
    {80, "CAN communication failed"},
    {81, "Parallel host line lost"},
    {82, "Parallel synchronized signal lost"},
    {83, "Parallel battery voltage detect different"},
    {84, "Parallel LINE voltage or frequency detect different"},
    {85, "Parallel LINE input current unbalanced"},
    {86, "Parallel output setting different"},
};

const std::array<Flag, 9> flags = {{
    {"BUZZ", 'A', "Silence buzzer or open buzzer"},
    {"OLBP", 'B', "Overload bypass function"},
    {"LCDE", 'C', "LCD display escape to default page after 1min timeout"},
    {"OLRS", 'D', "Overload restart"},
    {"OTRS", 'E', "Overload temperature restart"},
    {"BLON", 'F', "Backlight on"},
    {"ALRM", 'G', "Alarm on primary source interrupt"},
    {"FTCR", 'H', "Fault code record"},
    {"MTYP", 'I', "Machine type (1=Grid-Tie, 0=Off-Grid-Tie)"},
}};

ENUM_STR(BatteryType) {
    switch (val) {
        case BatteryType::AGM:     return os << "AGM" ;
        case BatteryType::Flooded: return os << "Flooded";
        case BatteryType::User:    return os << "User";
    };
    ENUM_STR_DEFAULT;
}

ENUM_STR(InputVoltageRange) {
    switch (val) {
        case InputVoltageRange::Appliance: return os << "Appliance";
        case InputVoltageRange::USP:       return os << "USP";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(OutputSourcePriority) {
    switch (val) {
        case OutputSourcePriority::SolarUtilityBattery:
            return os << "Solar-Utility-Battery";
        case OutputSourcePriority::SolarBatteryUtility:
            return os << "Solar-Battery-Utility";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(ChargeSourcePriority) {
    switch (val) {
        case ChargeSourcePriority::SolarFirst:
            return os << "Solar-First";
        case ChargeSourcePriority::SolarAndUtility:
            return os << "Solar-and-Utility";
        case ChargeSourcePriority::SolarOnly:
            return os << "Solar-only";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(MachineType) {
    switch (val) {
        case MachineType::OffGridTie: return os << "Off-Grid-Tie";
        case MachineType::GridTie:    return os << "Grid-Tie";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(Topology) {
    switch (val) {
        case Topology::TransformerLess: return os << "Transformer-less";
        case Topology::Transformer:     return os << "Transformer";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(OutputMode) {
    switch (val) {
        case OutputMode::SingleOutput:
            return os << "Single output";
        case OutputMode::ParallelOutput:
            return os << "Parallel output";
        case OutputMode::Phase_1_of_3:
            return os << "Phase 1 of 3-phase output";
        case OutputMode::Phase_2_of_3:
            return os << "Phase 2 of 3-phase output";
        case OutputMode::Phase_3_of_3:
            return os << "Phase 3 of 3-phase";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(SolarPowerPriority) {
    switch (val) {
        case SolarPowerPriority::BatteryLoadUtility:
            return os << "Battery-Load-Utility";
        case SolarPowerPriority::LoadBatteryUtility:
            return os << "Load-Battery-Utility";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(MPPTChargerStatus) {
    switch (val) {
        case MPPTChargerStatus::Abnormal:    return os << "Abnormal";
        case MPPTChargerStatus::NotCharging: return os << "Not charging";
        case MPPTChargerStatus::Charging:    return os << "Charging";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(BatteryPowerDirection) {
    switch (val) {
        case BatteryPowerDirection::DoNothing: return os << "Do nothing";
        case BatteryPowerDirection::Charge:    return os << "Charge";
        case BatteryPowerDirection::Discharge: return os << "Discharge";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(DC_AC_PowerDirection) {
    switch (val) {
        case DC_AC_PowerDirection::DoNothing: return os << "Do nothing";
        case DC_AC_PowerDirection::AC_DC:     return os << "AC/DC";
        case DC_AC_PowerDirection::DC_AC:     return os << "DC/AC";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(LinePowerDirection) {
    switch (val) {
        case LinePowerDirection::DoNothing: return os << "Do nothing";
        case LinePowerDirection::Input:     return os << "Input";
        case LinePowerDirection::Output:    return os << "Output";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(WorkingMode) {
    switch (val) {
        case WorkingMode::PowerOnMode: return os << "Power on mode";
        case WorkingMode::StandbyMode: return os << "Standby mode";
        case WorkingMode::BypassMode:  return os << "Bypass mode";
        case WorkingMode::BatteryMode: return os << "Battery mode";
        case WorkingMode::FaultMode:   return os << "Fault mode";
        case WorkingMode::HybridMode:  return os << "Hybrid mode";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(ParallelConnectionStatus) {
    switch (val) {
        case ParallelConnectionStatus::NotExistent: return os << "Non-existent";
        case ParallelConnectionStatus::Existent:    return os << "Existent";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(LoadConnectionStatus) {
    switch (val) {
        case LoadConnectionStatus::Disconnected: return os << "Disconnected";
        case LoadConnectionStatus::Connected:    return os << "Connected";
    }
    ENUM_STR_DEFAULT;
}

ENUM_STR(ConfigurationStatus) {
    switch (val) {
        case ConfigurationStatus::Default: return os << "Default";
        case ConfigurationStatus::Changed: return os << "Changed";
    }
    ENUM_STR_DEFAULT;
}

}