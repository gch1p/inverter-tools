// SPDX-License-Identifier: BSD-3-Clause

#include <utility>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <typeinfo>

#include "response.h"
#include "exceptions.h"
#include "../logging.h"

#define RETURN_TABLE(...)                                        \
    return std::shared_ptr<formatter::Table<VariantHolder>>(     \
        new formatter::Table<VariantHolder>(format, __VA_ARGS__) \
    );

#define RETURN_STATUS(...)                         \
    return std::shared_ptr<formatter::Status>(     \
        new formatter::Status(format, __VA_ARGS__) \
    );


namespace p18::response_type {

typedef formatter::TableItem<VariantHolder> LINE;

using formatter::Unit;


/**
 * Helpers
 */
std::ostream& operator<<(std::ostream& os, FieldLength fl) {
    if (fl.min_ == fl.max_)
        os << fl.min_;
    else
        os << "[" << fl.min_ << ", " << fl.max_ << "]";
    return os;
}


/**
 * Base responses
 */

BaseResponse::BaseResponse(std::shared_ptr<char> raw, size_t rawSize)
    : raw_(std::move(raw)), rawSize_(rawSize) {}

bool GetResponse::validate() {
    if (rawSize_ < 5)
        return false;

    const char* raw = raw_.get();
    if (raw[0] != '^' || raw[1] != 'D')
        return false;

    char lenbuf[4];
    memcpy(lenbuf, &raw[2], 3);
    lenbuf[3] = '\0';

    auto len = static_cast<size_t>(std::stoul(lenbuf));
    return rawSize_ >= len-5 /* exclude ^Dxxx*/;
}

const char* GetResponse::getData() const {
    return raw_.get() + 5;
}

size_t GetResponse::getDataSize() const {
    return rawSize_ - 5;
}

std::vector<std::string> GetResponse::getList(std::vector<FieldLength> itemLengths, int expectAtLeast) const {
    std::string buf(getData(), getDataSize());
    auto list = ::split(buf, ',');

    if (expectAtLeast == -1)
        expectAtLeast = (int)itemLengths.size();

    if (!itemLengths.empty()) {
        // check list length
        if (list.size() < expectAtLeast) {
            std::ostringstream error;
            error << "while parsing " << demangle_type_name(typeid(*this).name());
            error << ": list is expected to be " << expectAtLeast << " items long, ";
            error << "got only " << list.size() << " items";
            throw ParseError(error.str());
        }

        // check each item's length
        for (int i = 0; i < list.size(); i++) {
            if (i >= itemLengths.size()) {
                myerr << "while parsing " << demangle_type_name(typeid(*this).name())
                      << ": item " << i << " is not expected";
                break;
            }

            if (!itemLengths[i].validate(list[i].size())) {
                std::ostringstream error;
                error << "while parsing " << demangle_type_name(typeid(*this).name());
                error << ": item " << i << " is expected to be " << itemLengths[i] << " characters long, ";
                error << "got " << list[i].size() << " characters";
                throw ParseError(error.str());
            }
        }
    }

    return list;
}

bool SetResponse::validate() {
    if (rawSize_ < 2)
        return false;

    const char* raw = raw_.get();
    return raw[0] == '^' && (raw[1] == '0' || raw[1] == '1');
}

bool SetResponse::get() {
    return raw_.get()[1] == '1';
}

void SetResponse::unpack() {}

formattable_ptr SetResponse::format(formatter::Format format) {
    RETURN_STATUS(get(), "");
}

formattable_ptr ErrorResponse::format(formatter::Format format) {
    return std::shared_ptr<formatter::Status>(
        new formatter::Status(format, false, error_)
    );
}


/**
 * Actual typed responses
 */

void ProtocolID::unpack() {
    auto data = getData();

    char s[4];
    strncpy(s, data, 2);
    s[2] = '\0';

    id = stou(s);
}

formattable_ptr ProtocolID::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("id", "Protocol ID", id),
    });
}


void CurrentTime::unpack() {
    auto data = getData();

    std::string buf;
    buf = std::string(data, 4);

    year = stou(buf);

    for (int i = 0; i < 5; i++) {
        buf = std::string(data + 4 + (i * 2), 2);
        auto n = stou(buf);

        switch (i) {
            case 0:
                month = n;
                break;

            case 1:
                day = n;
                break;

            case 2:
                hour = n;
                break;

            case 3:
                minute = n;
                break;

            case 4:
                second = n;
                break;

            default:
                std::ostringstream error;
                error << "unexpected value while parsing CurrentTime (i = " << i << ")";
                throw ParseError(error.str());
        }
    }
}

formattable_ptr CurrentTime::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("year", "Year", year),
        LINE("month", "Month", month),
        LINE("day", "Day", day),
        LINE("hour", "Hour", hour),
        LINE("minute", "Minute", minute),
        LINE("second", "Second", second),
    });
}


void TotalGenerated::unpack() {
    auto data = getData();

    std::string buf(data, 8);
    wh = stou(buf);
}

formattable_ptr TotalGenerated::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("wh", "Wh", wh)
    });
}


void SerialNumber::unpack() {
    auto data = getData();

    std::string buf(data, 2);
    size_t len = std::stoul(buf);

    id = std::string(data+2, len);
}

formattable_ptr SerialNumber::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("sn", "Serial number", id)
    });
}


void CPUVersion::unpack() {
    auto list = getList({5, 5, 5});

    main_cpu_version = list[0];
    slave1_cpu_version = list[1];
    slave2_cpu_version = list[2];
}

formattable_ptr CPUVersion::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("main_v", "Main CPU version", main_cpu_version),
        LINE("slave1_v", "Slave 1 CPU version", slave1_cpu_version),
        LINE("slave2_v", "Slave 2 CPU version", slave2_cpu_version)
    });
}


void RatedInformation::unpack() {
    auto list = getList({
        4, // AAAA
        3, // BBB
        4, // CCCC
        3, // DDD
        3, // EEE
        4, // FFFF
        4, // GGGG
        3, // HHH
        3, // III
        3, // JJJ
        3, // KKK
        3, // LLL
        3, // MMM
        1, // N
        2, // OO
        3, // PPP
        1, // O
        1, // R
        1, // S
        1, // T
        1, // U
        1, // V
        1, // W
        1, // Z
        1, // a
    });

    ac_input_rating_voltage = stou(list[0]);
    ac_input_rating_current = stou(list[1]);
    ac_output_rating_voltage = stou(list[2]);
    ac_output_rating_freq = stou(list[3]);
    ac_output_rating_current = stou(list[4]);
    ac_output_rating_apparent_power = stou(list[5]);
    ac_output_rating_active_power = stou(list[6]);
    battery_rating_voltage = stou(list[7]);
    battery_recharge_voltage = stou(list[8]);
    battery_redischarge_voltage = stou(list[9]);
    battery_under_voltage = stou(list[10]);
    battery_bulk_voltage = stou(list[11]);
    battery_float_voltage = stou(list[12]);
    battery_type = static_cast<BatteryType>(stou(list[13]));
    max_ac_charge_current = stou(list[14]);
    max_charge_current = stou(list[15]);
    input_voltage_range = static_cast<InputVoltageRange>(stou(list[16]));
    output_source_priority = static_cast<OutputSourcePriority>(stou(list[17]));
    charge_source_priority = static_cast<ChargeSourcePriority>(stou(list[18]));
    parallel_max_num = stou(list[19]);
    machine_type = static_cast<MachineType>(stou(list[20]));
    topology = static_cast<Topology>(stou(list[21]));
    output_mode = static_cast<OutputMode>(stou(list[22]));
    solar_power_priority = static_cast<SolarPowerPriority>(stou(list[23]));
    mppt = list[24];
}

formattable_ptr RatedInformation::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("ac_input_rating_voltage", "AC input rating voltage", ac_input_rating_voltage / 10.0, Unit::V),
        LINE("ac_input_rating_current", "AC input rating current", ac_input_rating_current / 10.0, Unit::A),
        LINE("ac_output_rating_voltage", "AC output rating voltage", ac_output_rating_voltage / 10.0, Unit::V),
        LINE("ac_output_rating_freq", "AC output rating frequency", ac_output_rating_freq / 10.0, Unit::Hz),
        LINE("ac_output_rating_current", "AC output rating current", ac_output_rating_current / 10.0, Unit::A),
        LINE("ac_output_rating_apparent_power", "AC output rating apparent power", ac_output_rating_apparent_power, Unit::VA),
        LINE("ac_output_rating_active_power", "AC output rating active power", ac_output_rating_active_power, Unit::Wh),
        LINE("battery_rating_voltage", "Battery rating voltage", battery_rating_voltage / 10.0, Unit::V),
        LINE("battery_recharge_voltage", "Battery re-charge voltage", battery_recharge_voltage / 10.0, Unit::V),
        LINE("battery_redischarge_voltage", "Battery re-discharge voltage", battery_redischarge_voltage / 10.0, Unit::V),
        LINE("battery_under_voltage", "Battery under voltage", battery_under_voltage / 10.0, Unit::V),
        LINE("battery_bulk_voltage", "Battery bulk voltage", battery_bulk_voltage / 10.0, Unit::V),
        LINE("battery_float_voltage", "Battery float voltage", battery_float_voltage / 10.0, Unit::V),
        LINE("battery_type", "Battery type", battery_type),
        LINE("max_charge_current", "Max charge current", max_charge_current, Unit::A),
        LINE("max_ac_charge_current", "Max AC charge current", max_ac_charge_current, Unit::A),
        LINE("input_voltage_range", "Input voltage range", input_voltage_range),
        LINE("output_source_priority", "Output source priority", output_source_priority),
        LINE("charge_source_priority", "Charge source priority", charge_source_priority),
        LINE("parallel_max_num", "Parallel max num", parallel_max_num),
        LINE("machine_type", "Machine type", machine_type),
        LINE("topology", "Topology", topology),
        LINE("output_mode", "Output mode", output_mode),
        LINE("solar_power_priority", "Solar power priority", solar_power_priority),
        LINE("mppt", "MPPT string", mppt)
    });
}


void GeneralStatus::unpack() {
    auto list = getList({
        4, // AAAA
        3, // BBB
        4, // CCCC
        3, // DDD
        4, // EEEE
        4, // FFFF
        3, // GGG
        3, // HHH
        3, // III
        3, // JJJ
        3, // KKK
        3, // LLL
        3, // MMM
        3, // NNN
        3, // OOO
        3, // PPP
        4, // QQQQ
        4, // RRRR
        4, // SSSS
        4, // TTTT
        1, // U
        1, // V
        1, // W
        1, // X
        1, // Y
        1, // Z
        1, // a
        1, // b
    });

    grid_voltage = stou(list[0]);
    grid_freq = stou(list[1]);
    ac_output_voltage = stou(list[2]);
    ac_output_freq = stou(list[3]);
    ac_output_apparent_power = stou(list[4]);
    ac_output_active_power = stou(list[5]);
    output_load_percent = stou(list[6]);
    battery_voltage = stou(list[7]);
    battery_voltage_scc = stou(list[8]);
    battery_voltage_scc2 = stou(list[9]);
    battery_discharge_current = stou(list[10]);
    battery_charge_current = stou(list[11]);
    battery_capacity = stou(list[12]);
    inverter_heat_sink_temp = stou(list[13]);
    mppt1_charger_temp = stou(list[14]);
    mppt2_charger_temp = stou(list[15]);
    pv1_input_power = stou(list[16]);
    pv2_input_power = stou(list[17]);
    pv1_input_voltage = stou(list[18]);
    pv2_input_voltage = stou(list[19]);
    configuration_status = static_cast<ConfigurationStatus>(stou(list[20]));
    mppt1_charger_status = static_cast<MPPTChargerStatus>(stou(list[21]));
    mppt2_charger_status = static_cast<MPPTChargerStatus>(stou(list[22]));
    load_connected = static_cast<LoadConnectionStatus>(stou(list[23]));
    battery_power_direction = static_cast<BatteryPowerDirection>(stou(list[24]));
    dc_ac_power_direction = static_cast<DC_AC_PowerDirection>(stou(list[25]));
    line_power_direction = static_cast<LinePowerDirection>(stou(list[26]));
    local_parallel_id = stou(list[27]);
}

formattable_ptr GeneralStatus::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("grid_voltage", "Grid voltage", grid_voltage / 10.0, Unit::V),
        LINE("grid_freq", "Grid frequency", grid_freq / 10.0, Unit::Hz),
        LINE("ac_output_voltage", "AC output voltage", ac_output_voltage / 10.0, Unit::V),
        LINE("ac_output_freq", "AC output frequency", ac_output_freq / 10.0, Unit::Hz),
        LINE("ac_output_apparent_power", "AC output apparent power", ac_output_apparent_power, Unit::VA),
        LINE("ac_output_active_power", "AC output active power", ac_output_active_power, Unit::Wh),
        LINE("output_load_percent", "Output load percent", output_load_percent, Unit::Percentage),
        LINE("battery_voltage", "Battery voltage", battery_voltage / 10.0, Unit::V),
        LINE("battery_voltage_scc", "Battery voltage from SCC", battery_voltage_scc / 10.0, Unit::V),
        LINE("battery_voltage_scc2", "Battery voltage from SCC2", battery_voltage_scc2 / 10.0, Unit::V),
        LINE("battery_discharge_current", "Battery discharge current", battery_discharge_current, Unit::A),
        LINE("battery_charge_current", "Battery charge current", battery_charge_current, Unit::A),
        LINE("battery_capacity", "Battery capacity", battery_capacity, Unit::Percentage),
        LINE("inverter_heat_sink_temp", "Inverter heat sink temperature", inverter_heat_sink_temp, Unit::Celsius),
        LINE("mppt1_charger_temp", "MPPT1 charger temperature", mppt1_charger_temp, Unit::Celsius),
        LINE("mppt2_charger_temp", "MPPT2 charger temperature", mppt2_charger_temp, Unit::Celsius),
        LINE("pv1_input_power", "PV1 input power", pv1_input_power, Unit::Wh),
        LINE("pv2_input_power", "PV2 input power", pv2_input_power, Unit::Wh),
        LINE("pv1_input_voltage", "PV1 input voltage", pv1_input_voltage / 10.0, Unit::V),
        LINE("pv2_input_voltage", "PV2 input voltage", pv2_input_voltage / 10.0, Unit::V),
        LINE("configuration_status", "Configuration state", configuration_status),
        LINE("mppt1_charger_status", "MPPT1 charger status", mppt1_charger_status),
        LINE("mppt2_charger_status", "MPPT2 charger status", mppt2_charger_status),
        LINE("load_connected", "Load connection", load_connected),
        LINE("battery_power_direction", "Battery power direction", battery_power_direction),
        LINE("dc_ac_power_direction", "DC/AC power direction", dc_ac_power_direction),
        LINE("line_power_direction", "Line power direction", line_power_direction),
        LINE("local_parallel_id", "Local parallel ID", local_parallel_id),
    });
}


void WorkingMode::unpack() {
    auto data = getData();
    mode = static_cast<p18::WorkingMode>(stou(std::string(data, 2)));
}

formattable_ptr WorkingMode::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("mode", "Working mode", mode)
    })
}


void FaultsAndWarnings::unpack() {
    auto list = getList({2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});

    fault_code = stou(list[0]);
    line_fail = stou(list[1]) > 0;
    output_circuit_short = stou(list[2]) > 0;
    inverter_over_temperature = stou(list[3]) > 0;
    fan_lock = stou(list[4]) > 0;
    battery_voltage_high = stou(list[5]) > 0;
    battery_low = stou(list[6]) > 0;
    battery_under = stou(list[7]) > 0;
    over_load = stou(list[8]) > 0;
    eeprom_fail = stou(list[9]) > 0;
    power_limit = stou(list[10]) > 0;
    pv1_voltage_high = stou(list[11]) > 0;
    pv2_voltage_high = stou(list[12]) > 0;
    mppt1_overload_warning = stou(list[13]) > 0;
    mppt2_overload_warning = stou(list[14]) > 0;
    battery_too_low_to_charge_for_scc1 = stou(list[15]) > 0;
    battery_too_low_to_charge_for_scc2 = stou(list[16]) > 0;
}

formattable_ptr FaultsAndWarnings::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("fault_code", "Fault code", fault_code),
        LINE("line_fail", "Line fail", line_fail),
        LINE("output_circuit_short", "Output circuit short", output_circuit_short),
        LINE("inverter_over_temperature", "Inverter over temperature", inverter_over_temperature),
        LINE("fan_lock", "Fan lock", fan_lock),
        LINE("battery_voltage_high", "Battery voltage high", battery_voltage_high),
        LINE("battery_low", "Battery low", battery_low),
        LINE("battery_under", "Battery under", battery_under),
        LINE("over_load", "Over load", over_load),
        LINE("eeprom_fail", "EEPROM fail", eeprom_fail),
        LINE("power_limit", "Power limit", power_limit),
        LINE("pv1_voltage_high", "PV1 voltage high", pv1_voltage_high),
        LINE("pv2_voltage_high", "PV2 voltage high", pv2_voltage_high),
        LINE("mppt1_overload_warning", "MPPT1 overload warning", mppt1_overload_warning),
        LINE("mppt2_overload_warning", "MPPT2 overload warning", mppt2_overload_warning),
        LINE("battery_too_low_to_charge_for_scc1", "Battery too low to charge for SCC1", battery_too_low_to_charge_for_scc1),
        LINE("battery_too_low_to_charge_for_scc2", "Battery too low to charge for SCC2", battery_too_low_to_charge_for_scc2),
    })
}


void FlagsAndStatuses::unpack() {
    auto list = getList({1, 1, 1, 1, 1, 1, 1, 1, 1});

    buzzer = stou(list[0]) > 0;
    overload_bypass = stou(list[1]) > 0;
    lcd_escape_to_default_page_after_1min_timeout = stou(list[2]) > 0;
    overload_restart = stou(list[3]) > 0;
    over_temp_restart = stou(list[4]) > 0;
    backlight_on = stou(list[5]) > 0;
    alarm_on_primary_source_interrupt = stou(list[6]) > 0;
    fault_code_record = stou(list[7]) > 0;
    reserved = *list[8].c_str();
}

formattable_ptr FlagsAndStatuses::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("buzzer",
             "Buzzer",
             buzzer),

        LINE("overload_bypass",
             "Overload bypass function",
             overload_bypass),

        LINE("escape_to_default_screen_after_1min_timeout",
             "Escape to default screen after 1min timeout",
             lcd_escape_to_default_page_after_1min_timeout),

        LINE("overload_restart",
             "Overload restart",
             overload_restart),

        LINE("over_temp_restart",
             "Over temperature restart",
             over_temp_restart),

        LINE("backlight_on",
             "Backlight on",
             backlight_on),

        LINE("alarm_on_on_primary_source_interrupt",
             "Alarm on on primary source interrupt",
             alarm_on_primary_source_interrupt),

        LINE("fault_code_record",
             "Fault code record",
             fault_code_record)
    })
}


void RatedDefaults::unpack() {
    auto list = getList({
        4, // AAAA
        3, // BBB
        1, // C
        3, // DDD
        3, // EEE
        3, // FFF
        3, // GGG
        3, // HHH
        3, // III
        2, // JJ
        1, // K
        1, // L
        1, // M
        1, // N
        1, // O
        1, // P
        1, // S
        1, // T
        1, // U
        1, // V
        1, // W
        1, // X
        1, // Y
        1, // Z
    });

    ac_output_voltage = stou(list[0]);
    ac_output_freq = stou(list[1]);
    ac_input_voltage_range = static_cast<InputVoltageRange>(stou(list[2]));
    battery_under_voltage = stou(list[3]);
    charging_float_voltage = stou(list[4]);
    charging_bulk_voltage = stou(list[5]);
    battery_recharge_voltage = stou(list[6]);
    battery_redischarge_voltage = stou(list[7]);
    max_charge_current = stou(list[8]);
    max_ac_charge_current = stou(list[9]);
    battery_type = static_cast<BatteryType>(stou(list[10]));
    output_source_priority = static_cast<OutputSourcePriority>(stou(list[11]));
    charge_source_priority = static_cast<ChargeSourcePriority>(stou(list[12]));
    solar_power_priority = static_cast<SolarPowerPriority>(stou(list[13]));
    machine_type = static_cast<MachineType>(stou(list[14]));
    output_mode = static_cast<OutputMode>(stou(list[15]));
    flag_buzzer = stou(list[16]) > 0;
    flag_overload_restart = stou(list[17]) > 0;
    flag_over_temp_restart = stou(list[18]) > 0;
    flag_backlight_on = stou(list[19]) > 0;
    flag_alarm_on_primary_source_interrupt = stou(list[20]) > 0;
    flag_fault_code_record = stou(list[21]) > 0;
    flag_overload_bypass = stou(list[22]) > 0;
    flag_lcd_escape_to_default_page_after_1min_timeout = stou(list[23]) > 0;
}

formattable_ptr RatedDefaults::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("ac_output_voltage", "AC output voltage", ac_output_voltage / 10.0, Unit::V),
        LINE("ac_output_freq", "AC output frequency", ac_output_freq / 10.0, Unit::Hz),
        LINE("ac_input_voltage_range", "AC input voltage range", ac_input_voltage_range),
        LINE("battery_under_voltage", "Battery under voltage", battery_under_voltage / 10.0, Unit::V),
        LINE("battery_bulk_voltage", "Charging bulk voltage", charging_bulk_voltage / 10.0, Unit::V),
        LINE("battery_float_voltage",  "Charging float voltage", charging_float_voltage / 10.0, Unit::V),
        LINE("battery_recharge_voltage", "Battery re-charge voltage", battery_recharge_voltage / 10.0, Unit::V),
        LINE("battery_redischarge_voltage", "Battery re-discharge voltage", battery_redischarge_voltage / 10.0, Unit::V),
        LINE("max_charge_current", "Max charge current", max_charge_current, Unit::A),
        LINE("max_ac_charge_current", "Max AC charge current", max_ac_charge_current, Unit::A),
        LINE("battery_type", "Battery type", battery_type),
        LINE("output_source_priority", "Output source priority", output_source_priority),
        LINE("charge_source_priority", "Charge source priority", charge_source_priority),
        LINE("solar_power_priority", "Solar power priority", solar_power_priority),
        LINE("machine_type", "Machine type", machine_type),
        LINE("output_mode", "Output mode", output_mode),
        LINE("buzzer_flag", "Buzzer flag", flag_buzzer),
        LINE("overload_bypass_flag", "Overload bypass function flag", flag_overload_bypass),
        LINE("escape_to_default_screen_after_1min_timeout_flag", "Escape to default screen after 1min timeout flag", flag_lcd_escape_to_default_page_after_1min_timeout),
        LINE("overload_restart_flag", "Overload restart flag", flag_overload_restart),
        LINE("over_temp_restart_flag", "Over temperature restart flag", flag_over_temp_restart),
        LINE("backlight_on_flag", "Backlight on flag", flag_backlight_on),
        LINE("alarm_on_on_primary_source_interrupt_flag", "Alarm on on primary source interrupt flag", flag_alarm_on_primary_source_interrupt),
        LINE("fault_code_record_flag", "Fault code record flag", flag_fault_code_record),
    })
}

void AllowedChargeCurrents::unpack() {
    auto list = getList({});
    for (const std::string& i: list) {
        amps.emplace_back(stou(i));
    }
}

formattable_ptr AllowedChargeCurrents::format(formatter::Format format) {
    std::vector<formatter::ListItem<VariantHolder>> v;
    for (const auto& n: amps)
        v.emplace_back(n);

    return std::shared_ptr<formatter::List<VariantHolder>>(
        new formatter::List<VariantHolder>(format, v)
    );
}


void ParallelRatedInformation::unpack() {
    auto list = getList({
        1, // A
        2, // BB
        20, // CCCCCCCCCCCCCCCCCCCC
        1, // D
        3, // EEE

        // FF
        // note: protocol documentation says that the following field is 2 bytes long,
        // but actual tests of the 6kw unit shows it can be 3 bytes long
        FieldLength(2, 3),

        1 // G
    });

    parallel_connection_status = static_cast<ParallelConnectionStatus>(stou(list[0]));
    serial_number_valid_length = stou(list[1]);
    serial_number = std::string(list[2], 0, serial_number_valid_length);
    charge_source_priority = static_cast<ChargeSourcePriority>(stou(list[3]));
    max_charge_current = stou(list[4]);
    max_ac_charge_current = stou(list[5]);
    output_mode = static_cast<OutputMode>(stou(list[6]));
}

formattable_ptr ParallelRatedInformation::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("parallel_connection_status", "Parallel connection status", parallel_connection_status),
        LINE("serial_number", "Serial number", serial_number),
        LINE("charge_source_priority", "Charge source priority", charge_source_priority),
        LINE("max_charge_current", "Max charge current", max_charge_current, Unit::A),
        LINE("max_ac_charge_current", "Max AC charge current", max_ac_charge_current, Unit::A),
        LINE("output_mode", "Output mode", output_mode),
    })
}


void ParallelGeneralStatus::unpack() {
    auto list = getList({
        1, // A
        1, // B
        2, // CC
        4, // DDDD
        3, // EEE
        4, // FFFF
        3, // GGG
        4, // HHHH
        4, // IIII
        5, // JJJJJ
        5, // KKKKK
        3, // LLL
        3, // MMM
        3, // NNN
        3, // OOO
        3, // PPP
        3, // QQQ
        3, // MMM. It's not my mistake, it's per the doc.
        4, // RRRR
        4, // SSSS
        4, // TTTT
        4, // UUUU
        1, // V
           // FIXME: marked red in the docs
        1, // W
           // FIXME: marked red in the docs
        1, // X
        1, // Y
        1, // Z
        1, // a
        3, // bbb. Note: this one is marked in red in the doc. Apparently it means
           // that it may be missing on some models, see
           // https://github.com/gch1p/inverter-tools/issues/1#issuecomment-981158688
    }, 28);

    parallel_connection_status = static_cast<ParallelConnectionStatus>(stou(list[0]));
    work_mode = static_cast<p18::WorkingMode>(stou(list[1]));
    fault_code = stou(list[2]);
    grid_voltage = stou(list[3]);
    grid_freq = stou(list[4]);
    ac_output_voltage = stou(list[5]);
    ac_output_freq = stou(list[6]);
    ac_output_apparent_power = stou(list[7]);
    ac_output_active_power = stou(list[8]);
    total_ac_output_apparent_power = stou(list[9]);
    total_ac_output_active_power = stou(list[10]);
    output_load_percent = stou(list[11]);
    total_output_load_percent = stou(list[12]);
    battery_voltage = stou(list[13]);
    battery_discharge_current = stou(list[14]);
    battery_charge_current = stou(list[15]);
    total_battery_charge_current = stou(list[16]);
    battery_capacity = stou(list[17]);
    pv1_input_power = stou(list[18]);
    pv2_input_power = stou(list[19]);
    pv1_input_voltage = stou(list[20]);
    pv2_input_voltage = stou(list[21]);
    mppt1_charger_status = static_cast<MPPTChargerStatus>(stou(list[22]));
    mppt2_charger_status = static_cast<MPPTChargerStatus>(stou(list[23]));
    load_connected = static_cast<LoadConnectionStatus>(stou(list[24]));
    battery_power_direction = static_cast<BatteryPowerDirection>(stou(list[25]));
    dc_ac_power_direction = static_cast<DC_AC_PowerDirection>(stou(list[26]));
    line_power_direction = static_cast<LinePowerDirection>(stou(list[27]));
    if (list.size() >= 29) {
        max_temp_present = true;
        max_temp = stou(list[28]);
    }
}

formattable_ptr ParallelGeneralStatus::format(formatter::Format format) {
    auto table = new formatter::Table<VariantHolder>(format, {
        LINE("parallel_connection_status", "Parallel connection status", parallel_connection_status),
        LINE("mode", "Working mode", work_mode),
        LINE("fault_code", "Fault code", fault_code),
        LINE("grid_voltage", "Grid voltage", grid_voltage / 10.0, Unit::V),
        LINE("grid_freq", "Grid frequency", grid_freq / 10.0, Unit::Hz),
        LINE("ac_output_voltage", "AC output voltage", ac_output_voltage / 10.0, Unit::V),
        LINE("ac_output_freq", "AC output frequency", ac_output_freq / 10.0, Unit::Hz),
        LINE("ac_output_apparent_power", "AC output apparent power", ac_output_apparent_power, Unit::VA),
        LINE("ac_output_active_power", "AC output active power", ac_output_active_power, Unit::Wh),
        LINE("total_ac_output_apparent_power", "Total AC output apparent power", total_ac_output_apparent_power, Unit::VA),
        LINE("total_ac_output_active_power", "Total AC output active power", total_ac_output_active_power, Unit::Wh),
        LINE("output_load_percent", "Output load percent", output_load_percent, Unit::Percentage),
        LINE("total_output_load_percent", "Total output load percent", total_output_load_percent, Unit::Percentage),
        LINE("battery_voltage", "Battery voltage", battery_voltage / 10.0, Unit::V),
        LINE("battery_discharge_current", "Battery discharge current", battery_discharge_current, Unit::A),
        LINE("battery_charge_current", "Battery charge current", battery_charge_current, Unit::A),
        LINE("total_battery_charge_current", "Total battery charge current", total_battery_charge_current, Unit::A),
        LINE("battery_capacity", "Battery capacity", battery_capacity, Unit::Percentage),
        LINE("pv1_input_power", "PV1 input power", pv1_input_power, Unit::Wh),
        LINE("pv2_input_power", "PV2 input power", pv2_input_power, Unit::Wh),
        LINE("pv1_input_voltage", "PV1 input voltage", pv1_input_voltage / 10.0, Unit::V),
        LINE("pv2_input_voltage", "PV2 input voltage", pv2_input_voltage / 10.0, Unit::V),
        LINE("mppt1_charger_status", "MPPT1 charger status", mppt1_charger_status),
        LINE("mppt2_charger_status", "MPPT2 charger status", mppt2_charger_status),
        LINE("load_connected", "Load connection", load_connected),
        LINE("battery_power_direction", "Battery power direction", battery_power_direction),
        LINE("dc_ac_power_direction", "DC/AC power direction", dc_ac_power_direction),
        LINE("line_power_direction", "Line power direction", line_power_direction),
    });

    if (max_temp_present) {
        table->push(
            LINE("max_temp", "Max. temperature", max_temp)
        );
    }

    return std::shared_ptr<formatter::Table<VariantHolder>>(table);
}


void ACChargeTimeBucket::unpack() {
    auto list = getList({4 /* AAAA */, 4 /* BBBB */});

    start_h = stouh(list[0].substr(0, 2));
    start_m = stouh(list[0].substr(2, 2));

    end_h = stouh(list[1].substr(0, 2));
    end_m = stouh(list[1].substr(2, 2));
}

static inline std::string get_time(unsigned short h, unsigned short m) {
    std::ostringstream buf;
    buf << std::setfill('0');
    buf << std::setw(2) << h << ":" << std::setw(2) << m;
    return buf.str();
}

formattable_ptr ACChargeTimeBucket::format(formatter::Format format) {
    RETURN_TABLE({
        LINE("start_time", "Start time", get_time(start_h, start_m)),
        LINE("end_time", "End time", get_time(end_h, end_m)),
    })
}

}