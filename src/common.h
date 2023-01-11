// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_COMMON_H
#define INVERTER_TOOLS_COMMON_H

#include "formatter/formatter.h"

enum class DeviceType {
    USB,
    Serial,
    Pseudo
};

// long opts
enum {
    LO_HELP = 1,
    LO_VERBOSE,
    LO_RAW,
    LO_TIMEOUT,
    LO_CACHE_TIMEOUT,
    LO_DELAY,
    LO_FORMAT,
    LO_DEVICE,
    LO_DEVICE_ERROR_LIMIT,
    LO_USB_VENDOR_ID,
    LO_USB_DEVICE_ID,
    LO_USB_PATH,
    LO_SERIAL_NAME,
    LO_SERIAL_BAUD_RATE,
    LO_SERIAL_DATA_BITS,
    LO_SERIAL_STOP_BITS,
    LO_SERIAL_PARITY,
    LO_HOST,
    LO_PORT,
};

formatter::Format format_from_string(std::string& s);

#endif //INVERTER_TOOLS_COMMON_H
