// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_P18_TYPES_H
#define INVERTER_TOOLS_P18_TYPES_H

#include <string>

#define ENUM_STR(enum_type) std::ostream& operator<< (std::ostream& os, enum_type val)
#define ENUM_STR_DEFAULT    return os << val

namespace p18 {

enum class CommandType {
    GetProtocolID = 0,
    GetCurrentTime,
    GetTotalGenerated,
    GetYearGenerated,
    GetMonthGenerated,
    GetDayGenerated,
    GetSerialNumber,
    GetCPUVersion,
    GetRatedInformation,
    GetGeneralStatus,
    GetWorkingMode,
    GetFaultsAndWarnings,
    GetFlagsAndStatuses,
    GetRatedDefaults,
    GetAllowedChargeCurrents,
    GetAllowedACChargeCurrents,
    GetParallelRatedInformation,
    GetParallelGeneralStatus,
    GetACChargeTimeBucket,
    GetACSupplyTimeBucket,
    SetACSupply = 100,
    SetFlag,
    SetDefaults,
    SetBatteryMaxChargeCurrent,
    SetBatteryMaxACChargeCurrent,
    SetACOutputFreq,
    SetBatteryMaxChargeVoltage,
    SetACOutputVoltage,
    SetOutputSourcePriority,
    SetBatteryChargeThresholds, /* Battery re-charge and re-discharge voltage when utility is available */
    SetChargeSourcePriority,
    SetSolarPowerPriority,
    SetACInputVoltageRange,
    SetBatteryType,
    SetOutputMode,
    SetBatteryCutOffVoltage,
    SetSolarConfig,
    ClearGenerated,
    SetDateTime,
    SetACChargeTimeBucket,
    SetACSupplyTimeBucket,
};

enum class BatteryType {
    AGM     = 0,
    Flooded = 1,
    User    = 2,
};
ENUM_STR(BatteryType);

enum class InputVoltageRange {
    Appliance = 0,
    USP       = 1,
};
ENUM_STR(InputVoltageRange);

enum class OutputSourcePriority {
    SolarUtilityBattery = 0,
    SolarBatteryUtility = 1,
};
ENUM_STR(OutputSourcePriority);

enum class ChargeSourcePriority {
    SolarFirst      = 0,
    SolarAndUtility = 1,
    SolarOnly       = 2,
};
ENUM_STR(ChargeSourcePriority);

enum class MachineType {
    OffGridTie = 0,
    GridTie    = 1,
};
ENUM_STR(MachineType);

enum class Topology {
    TransformerLess = 0,
    Transformer     = 1,
};
ENUM_STR(Topology);

enum class OutputMode {
    SingleOutput   = 0,
    ParallelOutput = 1,
    Phase_1_of_3   = 2,
    Phase_2_of_3   = 3,
    Phase_3_of_3   = 4,
};
ENUM_STR(OutputMode);

enum class SolarPowerPriority {
    BatteryLoadUtility = 0,
    LoadBatteryUtility = 1,
};
ENUM_STR(SolarPowerPriority);

enum class MPPTChargerStatus {
    Abnormal    = 0,
    NotCharging = 1,
    Charging    = 2,
};
ENUM_STR(MPPTChargerStatus);

enum class BatteryPowerDirection {
    DoNothing = 0,
    Charge    = 1,
    Discharge = 2,
};
ENUM_STR(BatteryPowerDirection);

enum class DC_AC_PowerDirection {
    DoNothing = 0,
    AC_DC     = 1,
    DC_AC     = 2,
};
ENUM_STR(DC_AC_PowerDirection);

enum class LinePowerDirection {
    DoNothing = 0,
    Input     = 1,
    Output    = 2,
};
ENUM_STR(LinePowerDirection);

enum class WorkingMode {
    PowerOnMode  = 0,
    StandbyMode  = 1,
    BypassMode   = 2,
    BatteryMode  = 3,
    FaultMode    = 4,
    HybridMode   = 5,
};
ENUM_STR(WorkingMode);

enum class ParallelConnectionStatus {
    NotExistent = 0,
    Existent    = 1,
};
ENUM_STR(ParallelConnectionStatus);

enum class LoadConnectionStatus {
    Disconnected = 0,
    Connected    = 1,
};
ENUM_STR(LoadConnectionStatus);

enum class ConfigurationStatus {
    Default = 0,
    Changed = 1,
};
ENUM_STR(ConfigurationStatus);

struct Flag {
    std::string flag;
    char letter;
    std::string description;
};

}

#endif //INVERTER_TOOLS_P18_TYPES_H
