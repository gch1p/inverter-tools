// SPDX-License-Identifier: BSD-3-Clause

#include <stdexcept>
#include <cstring>
#include <iostream>

#include "../logging.h"
#include "device.h"
#include "exceptions.h"
#include "hexdump/hexdump.h"

namespace voltronic {

USBDevice::USBDevice(u16 vendorId, u16 productId) {
    init();
    device_ = hid_open(vendorId, productId, nullptr);
    if (!device_)
        throw DeviceError("failed to create hidapi device");
}

USBDevice::USBDevice(const std::string& path) {
    init();
    device_ = hid_open_path(path.c_str());
    if (!device_)
        throw DeviceError("failed to create hidapi device");
}

void USBDevice::init() {
    if (hid_init() != 0)
        throw DeviceError("hidapi initialization failure");
}

USBDevice::~USBDevice() {
    if (device_)
        hid_close(device_);

    hid_exit();
}

size_t USBDevice::read(u8* buf, size_t bufSize) {
    int timeout = !timeout_ ? -1 : static_cast<int32_t>(getTimeLeft());
    const int bytesRead = hid_read_timeout(device_, buf, GET_HID_REPORT_SIZE(bufSize), timeout);
    if (bytesRead == -1)
        throw DeviceError("hidapi_read_timeout() failed");
    return bytesRead;
}

size_t USBDevice::write(const u8* data, size_t dataSize) {
    const size_t writeSize = GET_HID_REPORT_SIZE(dataSize);

    if (verbose_) {
        myerr << "dataSize=" << dataSize << ", writeSize=" << writeSize;
        std::cerr << hexdump((void*)data, dataSize);
    }

    u8 writeBuffer[HID_REPORT_SIZE+1]{0};
    memcpy(&writeBuffer[1], data, writeSize);

    const int bytesWritten = hid_write(device_, writeBuffer, HID_REPORT_SIZE + 1);
    if (bytesWritten == -1)
        throw DeviceError("hidapi_write() failed");

    return GET_HID_REPORT_SIZE(bytesWritten);
}

u16 USBDevice::GET_HID_REPORT_SIZE(size_t size) {
    return size > HID_REPORT_SIZE ? HID_REPORT_SIZE : size;
}

}