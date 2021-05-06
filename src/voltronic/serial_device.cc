// SPDX-License-Identifier: BSD-3-Clause

#include <stdexcept>
#include <algorithm>
#include <libserialport.h>

#include "device.h"
#include "exceptions.h"
#include "../logging.h"

namespace voltronic {

const char* SerialDevice::DEVICE_NAME = "/dev/ttyUSB0";

SerialDevice::SerialDevice(std::string& name,
                           SerialBaudRate baudRate,
                           SerialDataBits dataBits,
                           SerialStopBits stopBits,
                           SerialParity parity)
    : port_(nullptr)
    , name_(name)
    , baudRate_(baudRate)
    , dataBits_(dataBits)
    , stopBits_(stopBits)
    , parity_(parity)
{
    if (sp_get_port_by_name(name_.c_str(), &port_) != SP_OK)
        throw DeviceError("failed to get port by name");

    if (sp_open(port_, SP_MODE_READ_WRITE) != SP_OK)
        throw DeviceError("failed to open device");

    SerialPortConfiguration config(*this);
    config.setConfiguration(baudRate_, dataBits_, stopBits_, parity_);

    sp_flush(port_, SP_BUF_BOTH);
}

SerialDevice::~SerialDevice() {
    if (port_ != nullptr) {
        if (sp_close(port_) == SP_OK)
            sp_free_port(port_);
    }
}

unsigned int SerialDevice::getTimeout() {
    return !timeout_
           // to wait indefinitely if no timeout set
           ? 0
           // if getTimeLeft() suddently returns 0, pass 1,
           // otherwise libserialport will treat it like 'wait indefinitely'
           : std::max(static_cast<unsigned>(getTimeLeft()), static_cast<unsigned>(1));
}

size_t SerialDevice::read(u8* buf, size_t bufSize) {
    if (verbose_)
        myerr << "reading...";
    return sp_blocking_read_next(port_, buf, bufSize, getTimeout());
}

size_t SerialDevice::write(const u8* data, size_t dataSize) {
    return sp_blocking_write(port_, data, dataSize, getTimeout());
}


/**
 * Serial port configuration
 */

SerialPortConfiguration::SerialPortConfiguration(SerialDevice& device)
    : config_(nullptr), device_(device)
{
    if (sp_new_config(&config_) != SP_OK)
        throw DeviceError("failed to allocate port configuration");

    if (sp_get_config(device.getPort(), config_) != SP_OK)
        throw DeviceError("failed to get current port configuration");
}

SerialPortConfiguration::~SerialPortConfiguration() {
    if (config_ != nullptr)
        sp_free_config(config_);
}

void SerialPortConfiguration::setConfiguration(SerialBaudRate baudRate,
                                               SerialDataBits dataBits,
                                               SerialStopBits stopBits,
                                               SerialParity parity) {
    if (sp_set_config_baudrate(config_, static_cast<int>(baudRate)) != SP_OK)
        throw DeviceError("failed to set baud rate");

    if (sp_set_config_bits(config_, static_cast<int>(dataBits)) != SP_OK)
        throw DeviceError("failed to set data bits");

    if (sp_set_config_stopbits(config_, static_cast<int>(stopBits)) != SP_OK)
        throw DeviceError("failed to set stop bits");

    if (sp_set_config_parity(config_, static_cast<enum sp_parity>(parity)) != SP_OK)
        throw DeviceError("failed to set parity");

    if (sp_set_config(device_.getPort(), config_) != SP_OK)
        throw DeviceError("failed to set port configuration");
}

bool is_serial_baud_rate_valid(SerialBaudRate baudRate) {
    switch (baudRate) {
        case 110:
        case 300:
        case 1200:
        case 2400:
        case 4800:
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
            return true;

        default: break;
    }
    return false;
}

bool is_serial_data_bits_valid(SerialDataBits dataBits) {
    switch (dataBits) {
        case SerialDataBits::Five:
        case SerialDataBits::Six:
        case SerialDataBits::Seven:
        case SerialDataBits::Eight:
            return true;
        default: break;
    }
    return false;
}

bool is_serial_stop_bits_valid(SerialStopBits stopBits) {
    switch (stopBits) {
        case SerialStopBits::One:
        case SerialStopBits::OneAndHalf:
        case SerialStopBits::Two:
            return true;
        default: break;
    }
    return false;
}

bool is_serial_parity_valid(SerialParity parity) {
    switch (parity) {
        case SerialParity::None:
        case SerialParity::Odd:
        case SerialParity::Even:
        case SerialParity::Mark:
        case SerialParity::Space:
            return true;
        default: break;
    }
    return false;
}

}