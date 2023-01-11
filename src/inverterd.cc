// SPDX-License-Identifier: BSD-3-Clause

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ios>
#include <getopt.h>

#include "numeric_types.h"
#include "common.h"
#include "voltronic/device.h"
#include "voltronic/exceptions.h"
#include "p18/exceptions.h"
#include "util.h"
#include "logging.h"
#include "server/server.h"
#include "server/signal.h"

static const char* DEFAULT_HOST = "127.0.0.1";
static int DEFAULT_PORT = 8305;

static void usage(const char* progname) {
    std::cout << "Usage: " << progname << " OPTIONS [COMMAND]\n" <<
              "\n"
              "Options:\n"
              "    -h, --help:          Show this help\n"
              "    --host <HOST>:       Server host (default: " << DEFAULT_HOST << ")\n"
              "    --port <PORT>        Server port (default: " << DEFAULT_PORT << ")\n"
              "    --device <DEVICE>:   'usb' (default), 'serial' or 'pseudo'\n"
              "    --timeout <TIMEOUT>: Device timeout in ms (default: " << voltronic::Device::TIMEOUT << ")\n"
              "    --cache-timeout <TIMEOUT>\n"
              "                         Default: " << server::Server::CACHE_TIMEOUT << "\n"
              "    --delay <DELAY>:     Delay between commands in ms (default: " << server::Server::DELAY << ")\n"
              "                         Cache validity time, in ms (default: " << server::Server::CACHE_TIMEOUT << ")\n"
              "    --device-error-limit <LIMIT>\n"
              "                         Default: " << server::Server::DEVICE_ERROR_LIMIT << "\n"
              "    --verbose:           Be verbose\n"
              "\n";

    std::ios_base::fmtflags f(std::cout.flags());
    std::cout << std::hex << std::setfill('0') <<
              "USB device options:\n"
              "    --usb-vendor-id <ID>: Vendor ID (default: " << std::setw(4) << voltronic::USBDevice::VENDOR_ID << ")\n"
              "    --usb-device-id <ID>: Device ID (default: " << std::setw(4) << voltronic::USBDevice::PRODUCT_ID << ")\n"
              "\n"
              "    Alternatively, you can specify device path (e.g., /dev/hidraw0):\n"
              "    --usb-path <PATH>: Device path\n";
    std::cout.flags(f);

    std::cout << "\n"
              "Serial device options:\n"
              "    --serial-name <NAME>: Path to serial device (default: " << voltronic::SerialDevice::DEVICE_NAME << ")\n"
              "    --serial-baud-rate 110|300|1200|2400|4800|9600|19200|38400|57600|115200\n"
              "    --serial-data-bits 5|6|7|8\n"
              "    --serial-stop-bits 1|1.5|2\n"
              "    --serial-parity none|odd|even|mark|space\n";
    exit(1);
}


int main(int argc, char *argv[]) {
    // common params
    u64 timeout = voltronic::Device::TIMEOUT;
    u64 cacheTimeout = server::Server::CACHE_TIMEOUT;
    u64 delay = server::Server::DELAY;
    u32 deviceErrorLimit = server::Server::DEVICE_ERROR_LIMIT;
    bool verbose = false;

    // server params
    std::string host(DEFAULT_HOST);
    int port = DEFAULT_PORT;

    // device params
    DeviceType deviceType = DeviceType::USB;

    unsigned short usbVendorId = voltronic::USBDevice::VENDOR_ID;
    unsigned short usbDeviceId = voltronic::USBDevice::PRODUCT_ID;
    std::string usbDevicePath {};

    std::string serialDeviceName(voltronic::SerialDevice::DEVICE_NAME);
    voltronic::SerialBaudRate serialBaudRate = voltronic::SerialDevice::BAUD_RATE;
    voltronic::SerialDataBits serialDataBits = voltronic::SerialDevice::DATA_BITS;
    voltronic::SerialStopBits serialStopBits = voltronic::SerialDevice::STOP_BITS;
    voltronic::SerialParity serialParity = voltronic::SerialDevice::PARITY;

    try {
        int opt;
        struct option long_options[] = {
            {"help",    no_argument,       nullptr,            'h'},
            {"verbose", no_argument,       nullptr,            LO_VERBOSE},
            {"timeout",            required_argument, nullptr, LO_TIMEOUT},
            {"cache-timeout",      required_argument, nullptr, LO_CACHE_TIMEOUT},
            {"delay",              required_argument, nullptr, LO_DELAY},
            {"device",             required_argument, nullptr, LO_DEVICE},
            {"device-error-limit", required_argument, nullptr, LO_DEVICE_ERROR_LIMIT},
            {"usb-vendor-id",      required_argument, nullptr, LO_USB_VENDOR_ID},
            {"usb-device-id",      required_argument, nullptr, LO_USB_DEVICE_ID},
            {"usb-path",           required_argument, nullptr, LO_USB_PATH},
            {"serial-name",        required_argument, nullptr, LO_SERIAL_NAME},
            {"serial-baud-rate",   required_argument, nullptr, LO_SERIAL_BAUD_RATE},
            {"serial-data-bits",   required_argument, nullptr, LO_SERIAL_DATA_BITS},
            {"serial-stop-bits",   required_argument, nullptr, LO_SERIAL_STOP_BITS},
            {"serial-parity",      required_argument, nullptr, LO_SERIAL_PARITY},
            {"host",               required_argument, nullptr, LO_HOST},
            {"port",               required_argument, nullptr, LO_PORT},
            {nullptr, 0, nullptr,                              0}
        };

        bool getoptError = false; // FIXME
        while ((opt = getopt_long(argc, argv, "h", long_options, nullptr)) != EOF) {
            if (opt == '?') {
                getoptError = true;
                break;
            }

            // simple options (flags), no arguments
            switch (opt) {
                case 'h':
                    usage(argv[0]);

                case LO_VERBOSE:
                    verbose = true;
                    continue;

                default:
                    break;
            }

            // options with arguments
            std::string arg;
            if (optarg)
                arg = std::string(optarg);

            switch (opt) {
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

                case LO_TIMEOUT:
                    timeout = std::stoull(arg);
                    break;

                case LO_CACHE_TIMEOUT:
                    cacheTimeout = std::stoull(arg);
                    break;

                case LO_DELAY:
                    delay = std::stoull(arg);
                    break;

                case LO_DEVICE_ERROR_LIMIT:
                    deviceErrorLimit = static_cast<u32>(std::stoul(arg));
                    break;

                case LO_USB_VENDOR_ID:
                    try {
                        if (arg.size() != 4)
                            throw std::invalid_argument("usb-vendor-id: invalid length");
                        usbVendorId = static_cast<unsigned short>(hextoul(arg));
                    } catch (std::invalid_argument& e) {
                        throw std::invalid_argument(std::string("usb-vendor-id: invalid format: ") + e.what());
                    }
                    break;

                case LO_USB_DEVICE_ID:
                    try {
                        if (arg.size() != 4)
                            throw std::invalid_argument("usb-device-id: invalid length");
                        usbDeviceId = static_cast<unsigned short>(hextoul(arg));
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

                case LO_HOST:
                    host = arg;
                    break;

                case LO_PORT:
                    port = std::stoi(arg);
                    break;

                default:
                    break;
            }
        }

        if (optind < argc)
            throw std::invalid_argument("extra parameter found");
    } catch (std::invalid_argument& e) {
        myerr << "error: " << e.what();
        return 1;
    }

    // open device
    std::shared_ptr<voltronic::Device> dev;
    try {
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

        dev->setTimeout(timeout);
    }
    catch (voltronic::DeviceError& e) {
        myerr << "device error: " << e.what();
        return 1;
    }
    catch (voltronic::TimeoutError& e) {
        myerr << "timeout error: " << e.what();
        return 1;
    }
    catch (voltronic::InvalidDataError& e) {
        myerr << "data is invalid: " << e.what();
        return 1;
    }
    catch (p18::InvalidResponseError& e) {
        myerr << "response is invalid: " << e.what();
        return 1;
    }

    // create server
    server::set_signal_handlers();

    server::Server server(dev);
    server.setVerbose(verbose);
    server.setDelay(delay);
    server.setDeviceErrorLimit(deviceErrorLimit);
    server.setCacheTimeout(cacheTimeout);

    server.start(host, port);

    if (verbose)
        mylog << "done";

    return 0;
}