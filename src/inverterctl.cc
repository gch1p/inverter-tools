// SPDX-License-Identifier: BSD-3-Clause

#include <cstdlib>
#include <string>
#include <iostream>
#include <ios>
#include <iomanip>
#include <array>
#include <vector>
#include <stdexcept>
#include <getopt.h>

#include "logging.h"
#include "util.h"
#include "common.h"
#include "p18/client.h"
#include "p18/types.h"
#include "p18/defines.h"
#include "p18/exceptions.h"
#include "p18/commands.h"
#include "formatter/formatter.h"
#include "voltronic/device.h"
#include "voltronic/exceptions.h"
#include "hexdump/hexdump.h"

const size_t MAX_RAW_COMMAND_LENGTH = 128;

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& P) {
    for (auto const& item: P) {
        std::cout << item;
        if (&item != &P.back())
            std::cout << "|";
    }
    return os;
}

static void short_usage(const char* progname) {
    std::cout << "Usage: " << progname << " OPTIONS [COMMAND]\n" <<
        "\n"
        "Options:\n"
        "    -h:                  Show this help\n"
        "    --help:              Show full help (with all commands)\n"
        "    --raw <DATA>:        Execute arbitrary command and print response\n"
        "    --device <DEVICE>:   'usb' (default), 'serial' or 'pseudo'\n"
        "    --timeout <TIMEOUT>: Timeout in ms (default: " << voltronic::Device::TIMEOUT << ")\n"
        "    --verbose:           Be verbose\n"
        "    --format <FORMAT>:   'table' (default), 'simple-table', 'json' or\n"
        "                         'simple-json'\n"
        "\n"
        "To see list of supported commands, use --help.\n";
    exit(1);
}

static void usage(const char* progname) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::cout << "Usage: " << progname << " OPTIONS [COMMAND]\n" <<
           "\n"
           "Options:\n"
           "    -h:                  Show short help\n"
           "    --help:              Show this help\n"
           "    --raw <DATA>:        Execute arbitrary command and print response\n"
           "                         (example: ^P005PI)\n"
           "    --device <DEVICE>:   Device type to use. See below for list of supported\n"
           "                         devices\n"
           "    --timeout <TIMEOUT>: Device read/write timeout, in milliseconds\n"
           "                         (default: " << voltronic::Device::TIMEOUT << ")\n"
           "    --verbose:           Print debug information (including hex dumps of\n"
           "                         device traffic)\n"
           "    --format <FORMAT>:   Output format for command responses\n"
           "\n"
           "Device types:\n"
           "    usb     USB device\n"
           "    serial  Serial device\n"
           "    pseudo  Pseudo device (only useful for development/debugging purposes)\n"
           "\n";
    std::cout << std::hex << std::setfill('0') <<
           "USB device options:\n"
           "    --usb-vendor-id <ID>: Vendor ID (default: " << std::setw(4) << voltronic::USBDevice::VENDOR_ID << ")\n"
           "    --usb-device-id <ID>: Device ID (default: " << std::setw(4) << voltronic::USBDevice::PRODUCT_ID << ")\n"
           "\n"
           "    Alternatively, you can specify device path (e.g., /dev/hidraw0):\n"
           "    --usb-path <PATH>: Device path\n"
           "\n";
    std::cout.flags(f);
    std::cout <<
           "Serial device options:\n"
           "    --serial-name <NAME>: Path to serial device (default: " << voltronic::SerialDevice::DEVICE_NAME << ")\n"
           "    --serial-baud-rate 110|300|1200|2400|4800|9600|19200|38400|57600|115200\n"
           "    --serial-data-bits 5|6|7|8\n"
           "    --serial-stop-bits 1|1.5|2\n"
           "    --serial-parity none|odd|even|mark|space\n"
           "\n"
           "Commands:\n"
           "    get-protocol-id\n"
           "    get-date-time\n"
           "    get-total-generated\n"
           "    get-year-generated <yyyy>\n"
           "    get-month-generated <yyyy> <mm>\n"
           "    get-day-generated <yyyy> <mm> <dd>\n"
           "    get-serial-number\n"
           "    get-cpu-version\n"
           "    get-rated\n"
           "    get-status\n"
           "    get-p-rated <id>\n"
           "        id: Parallel machine ID\n"
           "\n"
           "    get-p-status <id>\n"
           "        id: Parallel machine ID\n"
           "\n"
           "    get-mode\n"
           "    get-errors\n"
           "    get-flags\n"
           "    get-rated-defaults\n"
           "    get-allowed-charge-currents\n"
           "    get-allowed-ac-charge-currents\n"
           "    get-ac-charge-time\n"
           "    get-ac-supply-time\n"
           "    set-ac-supply 0|1\n"
           "    set-flag <flag> 0|1\n"
           "    set-rated-defaults\n"
           "    set-max-charge-current <id> <amps>\n"
           "        id: Parallel machine ID\n"
           "        amps: Use get-allowed-charge-currents\n"
           "              to see a list of allowed values.\n"
           "\n"
           "    set-max-ac-charge-current <id> <amps>\n"
           "        id: Parallel machine ID\n"
           "        amps: Use get-allowed-ac-charge-currents\n"
           "              to see a list of allowed values.\n"
           "\n"
           "    set-max-charge-voltage <cv> <fv>\n"
           "        cv: Constant voltage (48.0 ~ 58.4)\n"
           "        fv: Float voltage (48.0 ~ 58.4)\n"
           "\n"
           "    set-ac-output-freq 50|60\n"
           "    set-ac-output-voltage <v>\n"
           "        v: " << p18::ac_output_voltages << "\n"
           "\n"
           "    set-output-source-priority SUB|SBU\n"
           "        'SUB' means " << p18::OutputSourcePriority::SolarUtilityBattery << "\n"
           "        'SBU' means " << p18::OutputSourcePriority::SolarBatteryUtility << "\n"
           "\n"
           "    set-charge-thresholds <cv> <dv>\n"
           "        Set battery re-charge and re-discharge voltages when\n"
           "        grid is connected.\n"
           "\n"
           "        cv: re-charge voltage\n"
           "            For 12 V unit: " << p18::bat_ac_recharge_voltages_12v << "\n"
           "            For 24 V unit: " << p18::bat_ac_recharge_voltages_24v << "\n"
           "            For 48 V unit: " << p18::bat_ac_recharge_voltages_48v << "\n"
           "\n"
           "        dv: re-discharge voltage\n"
           "            For 12 V unit: " << p18::bat_ac_redischarge_voltages_12v << "\n"
           "            For 24 V unit: " << p18::bat_ac_redischarge_voltages_24v << "\n"
           "            For 48 V unit: " << p18::bat_ac_redischarge_voltages_48v << "\n"
           "\n"
           "    set-charge-source-priority <id> <priority>\n"
           "        id: Parallel machine ID\n"
           "        priority: SF|SU|S\n"
           "            'SF' means " << p18::ChargeSourcePriority::SolarFirst << "\n"
           "            'SU' means " << p18::ChargeSourcePriority::SolarAndUtility << "\n"
           "            'S' means " << p18::ChargeSourcePriority::SolarOnly << "\n"
           "\n"
           "    set-solar-power-priority BLU|LBU\n"
           "        'BLU' means " << p18::SolarPowerPriority::BatteryLoadUtility << "\n"
           "        'LBU' means " << p18::SolarPowerPriority::LoadBatteryUtility << "\n"
           "\n"
           "    set-ac-input-voltage-range APPLIANCE|UPS\n"
           "    set-battery-type AGM|FLOODED|USER\n"
           "    set-output-mode <id> <mode>\n"
           "        id: Machine ID\n"
           "        mode: S|P|1|2|3\n"
           "            S: " << p18::OutputMode::SingleOutput << "\n"
           "            P: " << p18::OutputMode::ParallelOutput << "\n"
           "            1: " << p18::OutputMode::Phase_1_of_3 << "\n"
           "            2: " << p18::OutputMode::Phase_2_of_3 << "\n"
           "            3: " << p18::OutputMode::Phase_3_of_3 << "\n"
           "\n"
           "    set-battery-cutoff-voltage <v>\n"
           "        v: Cut-off voltage (40.0~48.0)\n"
           "\n"
           "    set-solar-configuration <id>\n"
           "        id: Serial number\n"
           "\n"
           "    clear-generated-data\n"
           "        Clear all recorded stats about generated energy.\n"
           "\n"
           "    set-date-time <YYYY> <MM> <DD> <hh> <mm> <ss>\n"
           "        YYYY: Year\n"
           "        MM:   Month\n"
           "        DD:   Day\n"
           "        hh:   Hours\n"
           "        mm:   Minutes\n"
           "        ss:   Seconds\n"
           "\n"
           "    set-ac-charge-time <start> <end>\n"
           "        start: Starting time, hh:mm format\n"
           "        end:   Ending time, hh:mm format\n"
           "\n"
           "    set-ac-supply-time <start> <end>\n"
           "        start: Starting time, hh:mm format\n"
           "        end:   Ending time, hh:mm format\n"
           "\n"
           "Note: use 0 as parallel machine ID for single machine.\n"
           "\n"
           "Flags:\n";
    for (const p18::Flag& flag: p18::flags)
        std::cout << "    " << flag.flag << ": " << flag.description << "\n";
    std::cout <<
           "\n"
           "Formats:\n"
           "    table         Human-readable table\n"
           "    simple-table  Conveniently-parsable table\n"
           "    json          JSON object or array\n"
           "    simple-json   no units, enumerations represented as numbers\n";

    exit(1);
}

static void output_formatted_error(formatter::Format format, std::exception& e, std::string s = "") {
    std::ostringstream buf;
    if (!s.empty())
        buf << s << ": ";
    buf << e.what();

    auto err = p18::response_type::ErrorResponse(buf.str());
    auto output = err.format(format);

    if (format == formatter::Format::JSON) {
        std::cout << *output;
    } else {
        std::cerr << *output << std::endl;
    }
}


enum class Action {
    ShortHelp,
    FullHelp,
    Raw,
    Command,
};

int main(int argc, char *argv[]) {
    if (argv[1] == nullptr)
        short_usage(argv[0]);

    // common params
    Action action = Action::Command;
    u64 timeout = voltronic::Device::TIMEOUT;
    bool verbose = false;
    p18::CommandType commandType;
    std::vector<std::string> arguments;

    // format params
    bool formatChanged = false;
    formatter::Format format = formatter::Format::Table;

    // raw command param
    std::string raw;

    // device params
    DeviceType deviceType = DeviceType::USB;

    u16 usbVendorId = voltronic::USBDevice::VENDOR_ID;
    u16 usbDeviceId = voltronic::USBDevice::PRODUCT_ID;
    std::string usbDevicePath {};

    std::string serialDeviceName(voltronic::SerialDevice::DEVICE_NAME);
    voltronic::SerialBaudRate serialBaudRate = voltronic::SerialDevice::BAUD_RATE;
    voltronic::SerialDataBits serialDataBits = voltronic::SerialDevice::DATA_BITS;
    voltronic::SerialStopBits serialStopBits = voltronic::SerialDevice::STOP_BITS;
    voltronic::SerialParity serialParity = voltronic::SerialDevice::PARITY;

    try {
        int opt;
        struct option long_options[] = {
            {"help",    no_argument,       nullptr, LO_HELP},
            {"verbose", no_argument,       nullptr, LO_VERBOSE},
            {"raw",                 required_argument, nullptr, LO_RAW},
            {"timeout",             required_argument, nullptr, LO_TIMEOUT},
            {"format",              required_argument, nullptr, LO_FORMAT},
            {"device",              required_argument, nullptr, LO_DEVICE},
            {"usb-vendor-id",       required_argument, nullptr, LO_USB_VENDOR_ID},
            {"usb-device-id",       required_argument, nullptr, LO_USB_DEVICE_ID},
            {"usb-path",            required_argument, nullptr, LO_USB_PATH},
            {"serial-name",         required_argument, nullptr, LO_SERIAL_NAME},
            {"serial-baud-rate",    required_argument, nullptr, LO_SERIAL_BAUD_RATE},
            {"serial-data-bits",    required_argument, nullptr, LO_SERIAL_DATA_BITS},
            {"serial-stop-bits",    required_argument, nullptr, LO_SERIAL_STOP_BITS},
            {"serial-parity",       required_argument, nullptr, LO_SERIAL_PARITY},
            {nullptr, 0, nullptr, 0}
        };

        bool getoptError = false; // FIXME
        while ((opt = getopt_long(argc, argv, "h", long_options, nullptr)) != EOF) {
            if (opt == '?') {
                getoptError = true;
                break;
            }

            // simple options (flags), no arguments
            switch (opt) {
                case 'h': action = Action::ShortHelp; continue;
                case LO_HELP: action = Action::FullHelp; continue;
                case LO_VERBOSE: verbose = true; continue;
                default: break;
            }

            // options with arguments
            std::string arg;
            if (optarg)
                arg = std::string(optarg);

            switch (opt) {
                case LO_FORMAT:
                    format = format_from_string(arg);
                    formatChanged = true;
                    break;

                case LO_DEVICE:
                    if (arg == "usb")
                        deviceType = DeviceType::USB;
                    else if (arg == "serial")
                        deviceType = DeviceType::Serial;
                    else if (arg == "pseudo")
                        deviceType = DeviceType::Pseudo;
                    else
                        throw std::invalid_argument("invalid device");

                    break;

                case LO_RAW:
                    raw = arg;
                    if (raw.size() > MAX_RAW_COMMAND_LENGTH)
                        throw std::invalid_argument("command is too long");
                    action = Action::Raw;
                    break;

                case LO_TIMEOUT:
                    timeout = std::stoull(arg);
                    break;

                case LO_USB_VENDOR_ID:
                    try {
                        if (arg.size() != 4)
                            throw std::invalid_argument("usb-vendor-id: invalid length");
                        usbVendorId = static_cast<u16>(hextoul(arg));
                    } catch (std::invalid_argument& e) {
                        throw std::invalid_argument(std::string("usb-vendor-id: invalid format: ") + e.what());
                    }
                    break;

                case LO_USB_DEVICE_ID:
                    try {
                        if (arg.size() != 4)
                            throw std::invalid_argument("usb-device-id: invalid length");
                        usbDeviceId = static_cast<u16>(hextoul(arg));
                    } catch (std::invalid_argument& e) {
                        throw std::invalid_argument(std::string("usb-device-id: invalid format: ") + e.what());
                    }
                    break;

                case LO_USB_PATH:
                    usbDevicePath = arg;
                    break;

                case LO_SERIAL_NAME:
                    serialDeviceName = arg;
                    break;

                case LO_SERIAL_BAUD_RATE:
                    serialBaudRate = static_cast<voltronic::SerialBaudRate>(std::stoul(arg));
                    if (!voltronic::is_serial_baud_rate_valid(serialBaudRate))
                        throw std::invalid_argument("invalid serial baud rate");
                    break;

                case LO_SERIAL_DATA_BITS:
                    serialDataBits = static_cast<voltronic::SerialDataBits>(std::stoul(arg));
                    if (voltronic::is_serial_data_bits_valid(serialDataBits))
                        throw std::invalid_argument("invalid serial data bits");
                    break;

                case LO_SERIAL_STOP_BITS:
                    if (arg == "1")
                        serialStopBits = voltronic::SerialStopBits::One;
                    else if (arg == "1.5")
                        serialStopBits = voltronic::SerialStopBits::OneAndHalf;
                    else if (arg == "2")
                        serialStopBits = voltronic::SerialStopBits::Two;
                    else
                        throw std::invalid_argument("invalid serial stop bits");
                    break;

                case LO_SERIAL_PARITY:
                    if (arg == "none")
                        serialParity = voltronic::SerialParity::None;
                    else if (arg == "odd")
                        serialParity = voltronic::SerialParity::Odd;
                    else if (arg == "even")
                        serialParity = voltronic::SerialParity::Even;
                    else if (arg == "mark")
                        serialParity = voltronic::SerialParity::Mark;
                    else if (arg == "space")
                        serialParity = voltronic::SerialParity::Space;
                    else
                        throw std::invalid_argument("invalid serial parity");
                    break;

                default:
                    break;
            }
        }

        switch (action) {
            case Action::ShortHelp:
                short_usage(argv[0]);
                break;

            case Action::FullHelp:
                usage(argv[0]);
                break;

            case Action::Command: {
                if (argc <= optind)
                    throw std::invalid_argument("missing command");

                std::string command = argv[optind++];

                p18::CommandInput input{argc, argv};
                commandType = p18::validate_input(command, arguments, (void*)&input);
                break;
            }

            case Action::Raw:
                if (formatChanged)
                    throw std::invalid_argument("--format is not allowed with --raw");
                break;
        }

        if (optind < argc)
            throw std::invalid_argument("extra parameter found");
    } catch (std::invalid_argument& e) {
        output_formatted_error(format, e);
        return 1;
    }

    bool success = false;
    try {
        std::shared_ptr<voltronic::Device> dev;
        switch (deviceType) {
            case DeviceType::USB:
                if (usbDevicePath.empty()) {
                    dev = std::shared_ptr<voltronic::Device>(new voltronic::USBDevice(usbVendorId,
                                                                                      usbDeviceId));
                } else {
                    dev = std::shared_ptr<voltronic::Device>(new voltronic::USBDevice(usbDevicePath));
                }
                break;

            case DeviceType::Pseudo:
                dev = std::shared_ptr<voltronic::Device>(new voltronic::PseudoDevice);
                break;

            case DeviceType::Serial:
                dev = std::shared_ptr<voltronic::Device>(new voltronic::SerialDevice(serialDeviceName,
                                                                                     serialBaudRate,
                                                                                     serialDataBits,
                                                                                     serialStopBits,
                                                                                     serialParity));
                break;
        }

        dev->setVerbose(verbose);
        dev->setTimeout(timeout);

        p18::Client client;
        client.setDevice(dev);

        if (action == Action::Raw) {
            auto result = client.runOnDevice(raw);
            if (verbose)
                std::cerr << hexdump(result.first.get(), result.second);
            std::cout << std::string(result.first.get(), result.second) << std::endl;
        } else {
            auto response = client.execute(commandType, arguments);
            std::cout << *(response->format(format).get()) << std::endl;
        }

        success = true;
    }
    catch (voltronic::DeviceError& e) {
        output_formatted_error(format, e, "device error");
    }
    catch (voltronic::TimeoutError& e) {
        output_formatted_error(format, e, "timeout");
    }
    catch (voltronic::InvalidDataError& e) {
        output_formatted_error(format, e, "data is invalid");
    }
    catch (p18::InvalidResponseError& e) {
        output_formatted_error(format, e, "response is invalid");
    }

    return success ? 1 : 0;
}