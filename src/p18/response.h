// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_P18_RESPONSE_H
#define INVERTER_TOOLS_P18_RESPONSE_H

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <nlohmann/json.hpp>

#include "types.h"
#include "src/formatter/formatter.h"

namespace p18::response_type {

using nlohmann::json;

typedef std::shared_ptr<formatter::Formattable> formattable_ptr;


/**
 * Value holder for the formatter module
 */

typedef std::variant<
    unsigned,
    unsigned short,
    unsigned long,
    bool,
    double,
    std::string,
    p18::BatteryType,
    p18::BatteryPowerDirection,
    p18::ChargeSourcePriority,
    p18::DC_AC_PowerDirection,
    p18::InputVoltageRange,
    p18::LinePowerDirection,
    p18::MachineType,
    p18::MPPTChargerStatus,
    p18::Topology,
    p18::OutputSourcePriority,
    p18::OutputMode,
    p18::ParallelConnectionStatus,
    p18::SolarPowerPriority,
    p18::WorkingMode,
    p18::LoadConnectionStatus,
    p18::ConfigurationStatus
> Variant;

class VariantHolder {
private:
    Variant v_;

public:
    // implicit conversion constructors
    VariantHolder(unsigned v) : v_(v) {}
    VariantHolder(unsigned short v) : v_(v) {}
    VariantHolder(unsigned long v) : v_(v) {}
    VariantHolder(bool v) : v_(v) {}
    VariantHolder(double v) : v_(v) {}
    VariantHolder(std::string v) : v_(v) {}
    VariantHolder(p18::BatteryType v) : v_(v) {}
    VariantHolder(p18::BatteryPowerDirection v) : v_(v) {}
    VariantHolder(p18::ChargeSourcePriority v) : v_(v) {}
    VariantHolder(p18::DC_AC_PowerDirection v) : v_(v) {}
    VariantHolder(p18::InputVoltageRange v) : v_(v) {}
    VariantHolder(p18::LinePowerDirection v) : v_(v) {}
    VariantHolder(p18::MachineType v) : v_(v) {}
    VariantHolder(p18::MPPTChargerStatus v) : v_(v) {}
    VariantHolder(p18::Topology v) : v_(v) {}
    VariantHolder(p18::OutputSourcePriority v) : v_(v) {}
    VariantHolder(p18::OutputMode v) : v_(v) {}
    VariantHolder(p18::ParallelConnectionStatus v) : v_(v) {}
    VariantHolder(p18::SolarPowerPriority v) : v_(v) {}
    VariantHolder(p18::WorkingMode v) : v_(v) {}
    VariantHolder(p18::LoadConnectionStatus v) : v_(v) {}
    VariantHolder(p18::ConfigurationStatus v) : v_(v) {}

    friend std::ostream &operator<<(std::ostream &os, VariantHolder const& ref) {
        std::visit([&os](const auto& elem) {
            os << elem;
        }, ref.v_);
        return os;
    }

    inline json toJSON() const {
        json j;
        bool isEnum =
            std::holds_alternative<p18::BatteryType>(v_) ||
            std::holds_alternative<p18::BatteryPowerDirection>(v_) ||
            std::holds_alternative<p18::ChargeSourcePriority>(v_) ||
            std::holds_alternative<p18::DC_AC_PowerDirection>(v_) ||
            std::holds_alternative<p18::InputVoltageRange>(v_) ||
            std::holds_alternative<p18::LinePowerDirection>(v_) ||
            std::holds_alternative<p18::MachineType>(v_) ||
            std::holds_alternative<p18::MPPTChargerStatus>(v_) ||
            std::holds_alternative<p18::Topology>(v_) ||
            std::holds_alternative<p18::OutputSourcePriority>(v_) ||
            std::holds_alternative<p18::OutputMode>(v_) ||
            std::holds_alternative<p18::ParallelConnectionStatus>(v_) ||
            std::holds_alternative<p18::SolarPowerPriority>(v_) ||
            std::holds_alternative<p18::WorkingMode>(v_) ||
            std::holds_alternative<p18::LoadConnectionStatus>(v_) ||
            std::holds_alternative<p18::ConfigurationStatus>(v_);

        std::visit([&j, &isEnum](const auto& elem) {
            if (isEnum)
                j = formatter::to_str(elem);
            else
                j = elem;
        }, v_);

        return j;
    }

    inline json toSimpleJSON() const {
        json j;
        std::visit([&j](const auto& elem) {
            j = elem;
        }, v_);
        return j;
    }
};


/**
 * Some helpers
 */
class FieldLength {
protected:
    size_t min_;
    size_t max_;

public:
    FieldLength(size_t n) : min_(n), max_(n) {}
    FieldLength(size_t min, size_t max) : min_(min), max_(max) {}

    [[nodiscard]] bool validate(size_t len) const {
        return len >= min_ && len <= max_;
    }

    friend std::ostream& operator<<(std::ostream& os, FieldLength fl);
};


/**
 * Base responses
 */

class BaseResponse {
protected:
    std::shared_ptr<char> raw_;
    size_t rawSize_;

public:
    BaseResponse(std::shared_ptr<char> raw, size_t rawSize);
    virtual ~BaseResponse() = default;
    virtual bool validate() = 0;
    virtual void unpack() = 0;
    virtual formattable_ptr format(formatter::Format format) = 0;
};

class GetResponse : public BaseResponse {
protected:
    const char* getData() const;
    size_t getDataSize() const;
    std::vector<std::string> getList(std::vector<FieldLength> itemLengths, int expectAtLeast = -1) const;

public:
    using BaseResponse::BaseResponse;
    bool validate() override;
//    virtual void output() = 0;
};

class SetResponse : public BaseResponse {
public:
    using BaseResponse::BaseResponse;
    void unpack() override;
    bool validate() override;
    formattable_ptr format(formatter::Format format) override;
    bool get();
};

class ErrorResponse : public BaseResponse {
private:
    std::string error_;

public:
    explicit ErrorResponse(std::string error)
        : BaseResponse(nullptr, 0), error_(std::move(error)) {}

    bool validate() override {
        return true;
    }
    void unpack() override {}
    formattable_ptr format(formatter::Format format) override;
};


/**
 * Actual typed responses
 */

class ProtocolID : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    unsigned id = 0;
};

class CurrentTime : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    unsigned year = 0;
    unsigned short month = 0;
    unsigned short day = 0;
    unsigned short hour = 0;
    unsigned short minute = 0;
    unsigned short second = 0;
};

class TotalGenerated : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    unsigned long wh = 0;
};

class YearGenerated : public TotalGenerated {
public:
    using TotalGenerated::TotalGenerated;
};

class MonthGenerated : public TotalGenerated {
public:
    using TotalGenerated::TotalGenerated;
};

class DayGenerated : public TotalGenerated {
public:
    using TotalGenerated::TotalGenerated;
};

class SerialNumber : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    std::string id;
};

class CPUVersion : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    std::string main_cpu_version;
    std::string slave1_cpu_version;
    std::string slave2_cpu_version;
};

class RatedInformation : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    unsigned ac_input_rating_voltage;         /* unit: 0.1V */
    unsigned ac_input_rating_current;         /* unit: 0.1A */
    unsigned ac_output_rating_voltage;        /* unit: 0.1A */
    unsigned ac_output_rating_freq;           /* unit: 0.1Hz */
    unsigned ac_output_rating_current;        /* unit: 0.1A */
    unsigned ac_output_rating_apparent_power; /* unit: VA */
    unsigned ac_output_rating_active_power;   /* unit: W */
    unsigned battery_rating_voltage;          /* unit: 0.1V */
    unsigned battery_recharge_voltage;        /* unit: 0.1V */
    unsigned battery_redischarge_voltage;     /* unit: 0.1V */
    unsigned battery_under_voltage;           /* unit: 0.1V */
    unsigned battery_bulk_voltage;            /* unit: 0.1V */
    unsigned battery_float_voltage;           /* unit: 0.1V */
    p18::BatteryType battery_type;
    unsigned max_ac_charge_current;           /* unit: A */
    unsigned max_charge_current;              /* unit: A */
    p18::InputVoltageRange input_voltage_range;
    p18::OutputSourcePriority output_source_priority;
    p18::ChargeSourcePriority charge_source_priority;
    unsigned parallel_max_num;
    p18::MachineType machine_type;
    p18::Topology topology;
    p18::OutputMode output_mode;
    p18::SolarPowerPriority solar_power_priority;
    std::string mppt;
};

class GeneralStatus : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    unsigned grid_voltage;              /* unit: 0.1V */
    unsigned grid_freq;                 /* unit: 0.1Hz */
    unsigned ac_output_voltage;         /* unit: 0.1V */
    unsigned ac_output_freq;            /* unit: 0.1Hz */
    unsigned ac_output_apparent_power;  /* unit: VA */
    unsigned ac_output_active_power;    /* unit: W */
    unsigned output_load_percent;       /* unit: % */
    unsigned battery_voltage;           /* unit: 0.1V */
    unsigned battery_voltage_scc;       /* unit: 0.1V */
    unsigned battery_voltage_scc2;      /* unit: 0.1V */
    unsigned battery_discharge_current; /* unit: A */
    unsigned battery_charge_current;    /* unit: A */
    unsigned battery_capacity;          /* unit: % */
    unsigned inverter_heat_sink_temp;   /* unit: C */
    unsigned mppt1_charger_temp;        /* unit: C */
    unsigned mppt2_charger_temp;        /* unit: C */
    unsigned pv1_input_power;           /* unit: W */
    unsigned pv2_input_power;           /* unit: W */
    unsigned pv1_input_voltage;         /* unit: 0.1V */
    unsigned pv2_input_voltage;         /* unit: 0.1V */
    p18::ConfigurationStatus configuration_status;
    p18::MPPTChargerStatus mppt1_charger_status;
    p18::MPPTChargerStatus mppt2_charger_status;
    p18::LoadConnectionStatus load_connected;
    p18::BatteryPowerDirection battery_power_direction;
    p18::DC_AC_PowerDirection dc_ac_power_direction;
    p18::LinePowerDirection line_power_direction;
    unsigned local_parallel_id;         /* 0 .. (parallel number - 1) */
};

class WorkingMode : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    p18::WorkingMode mode = static_cast<p18::WorkingMode>(0);
};

class FaultsAndWarnings : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    unsigned fault_code = 0;
    bool line_fail = false;
    bool output_circuit_short = false;
    bool inverter_over_temperature = false;
    bool fan_lock = false;
    bool battery_voltage_high = false;
    bool battery_low = false;
    bool battery_under = false;
    bool over_load = false;
    bool eeprom_fail = false;
    bool power_limit = false;
    bool pv1_voltage_high = false;
    bool pv2_voltage_high = false;
    bool mppt1_overload_warning = false;
    bool mppt2_overload_warning = false;
    bool battery_too_low_to_charge_for_scc1 = false;
    bool battery_too_low_to_charge_for_scc2 = false;
};

class FlagsAndStatuses : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    bool buzzer = false;
    bool overload_bypass = false;
    bool lcd_escape_to_default_page_after_1min_timeout = false;
    bool overload_restart = false;
    bool over_temp_restart = false;
    bool backlight_on = false;
    bool alarm_on_primary_source_interrupt = false;
    bool fault_code_record = false;
    char reserved = '0';
};

class RatedDefaults : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    unsigned ac_output_voltage = 0;      /* unit: 0.1V */
    unsigned ac_output_freq = 0;
    p18::InputVoltageRange ac_input_voltage_range = static_cast<InputVoltageRange>(0);
    unsigned battery_under_voltage = 0;
    unsigned charging_float_voltage = 0;
    unsigned charging_bulk_voltage = 0;
    unsigned battery_recharge_voltage = 0;
    unsigned battery_redischarge_voltage = 0;
    unsigned max_charge_current = 0;
    unsigned max_ac_charge_current = 0;
    p18::BatteryType battery_type = static_cast<BatteryType>(0);
    p18::OutputSourcePriority output_source_priority = static_cast<OutputSourcePriority>(0);
    p18::ChargeSourcePriority charge_source_priority = static_cast<ChargeSourcePriority>(0);
    p18::SolarPowerPriority solar_power_priority = static_cast<SolarPowerPriority>(0);
    p18::MachineType machine_type = static_cast<MachineType>(0);
    p18::OutputMode output_mode = static_cast<OutputMode>(0);
    bool flag_buzzer = false;
    bool flag_overload_restart = false;
    bool flag_over_temp_restart = false;
    bool flag_backlight_on = false;
    bool flag_alarm_on_primary_source_interrupt = false;
    bool flag_fault_code_record = false;
    bool flag_overload_bypass = false;
    bool flag_lcd_escape_to_default_page_after_1min_timeout = false;
};

class AllowedChargeCurrents : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    std::vector<unsigned> amps;
};

class AllowedACChargeCurrents : public AllowedChargeCurrents {
public:
    using AllowedChargeCurrents::AllowedChargeCurrents;
};

class ParallelRatedInformation : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    p18::ParallelConnectionStatus parallel_connection_status = static_cast<ParallelConnectionStatus>(0);
    unsigned serial_number_valid_length = 0;
    std::string serial_number;
    p18::ChargeSourcePriority charge_source_priority = static_cast<ChargeSourcePriority>(0);
    unsigned max_ac_charge_current = 0; // unit: A
    unsigned max_charge_current = 0;    // unit: A
    p18::OutputMode output_mode = static_cast<OutputMode>(0);
};

class ParallelGeneralStatus : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    p18::ParallelConnectionStatus parallel_connection_status;
    p18::WorkingMode work_mode;
    unsigned fault_code;
    unsigned grid_voltage;                   /* unit: 0.1V */
    unsigned grid_freq;                      /* unit: 0.1Hz */
    unsigned ac_output_voltage;              /* unit: 0.1V */
    unsigned ac_output_freq;                 /* unit: 0.1Hz */
    unsigned ac_output_apparent_power;       /* unit: VA */
    unsigned ac_output_active_power;         /* unit: W */
    unsigned total_ac_output_apparent_power; /* unit: VA */
    unsigned total_ac_output_active_power;   /* unit: W */
    unsigned output_load_percent;            /* unit: % */
    unsigned total_output_load_percent;      /* unit: % */
    unsigned battery_voltage;                /* unit: 0.1V */
    unsigned battery_discharge_current;      /* unit: A */
    unsigned battery_charge_current;         /* unit: A */
    unsigned total_battery_charge_current;   /* unit: A */
    unsigned battery_capacity;               /* unit: % */
    unsigned pv1_input_power;                /* unit: W */
    unsigned pv2_input_power;                /* unit: W */
    unsigned pv1_input_voltage;              /* unit: 0.1V */
    unsigned pv2_input_voltage;              /* unit: 0.1V */
    p18::MPPTChargerStatus mppt1_charger_status;
    p18::MPPTChargerStatus mppt2_charger_status;
    p18::LoadConnectionStatus load_connected;
    p18::BatteryPowerDirection battery_power_direction;
    p18::DC_AC_PowerDirection dc_ac_power_direction;
    p18::LinePowerDirection line_power_direction;

    bool max_temp_present = false;
    unsigned max_temp;                       /* unit: C */
};

class ACChargeTimeBucket : public GetResponse {
public:
    using GetResponse::GetResponse;
    void unpack() override;
    formattable_ptr format(formatter::Format format) override;

    unsigned short start_h = 0;
    unsigned short start_m = 0;
    unsigned short end_h = 0;
    unsigned short end_m = 0;
};

class ACSupplyTimeBucket : public ACChargeTimeBucket {
public:
    using ACChargeTimeBucket::ACChargeTimeBucket;
};

} // namespace p18

#endif //INVERTER_TOOLS_P18_RESPONSE_H
