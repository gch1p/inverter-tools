// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_VOLTRONIC_DEVICE_H
#define INVERTER_TOOLS_VOLTRONIC_DEVICE_H

#include <string>
#include <memory>
#include <hidapi/hidapi.h>
#include <libserialport.h>

#include "../numeric_types.h"

namespace voltronic {

enum {
    FLAG_WRITE_CRC = 1,
    FLAG_READ_CRC = 2,
    FLAG_VERIFY_CRC = 4,
};


/**
 * Common device
 */

class Device {
protected:
    int flags_;
    u64 timeout_;
    u64 timeStarted_;
    bool verbose_;

    void send(const u8* buf, size_t bufSize);
    size_t recv(u8* buf, size_t bufSize);

    void writeLoop(const u8* data, size_t dataSize);
    size_t readLoop(u8* buf, size_t bufSize);

    u64 getElapsedTime() const;
    u64 getTimeLeft() const;

public:
    static const u64 TIMEOUT = 1000;

    Device();

    virtual size_t read(u8* buf, size_t bufSize) = 0;
    virtual size_t write(const u8* data, size_t dataSize) = 0;

    void setTimeout(u64 timeout);
    size_t run(const u8* inbuf, size_t inbufSize, u8* outbuf, size_t outbufSize);

    void setFlags(int flags);
    int getFlags() const;

    void setVerbose(bool verbose);
};


/**
 * USB device
 */

class USBDevice : public Device {
private:
    hid_device* device_;

public:
    static const u16 VENDOR_ID = 0x0665;
    static const u16 PRODUCT_ID = 0x5161;
    static const u16 HID_REPORT_SIZE = 8;
    static u16 GET_HID_REPORT_SIZE(size_t size);

    USBDevice(u16 vendorId, u16 productId);
    explicit USBDevice(const std::string& path);
    ~USBDevice();

    static inline void init();

    size_t read(u8* buf, size_t bufSize) override;
    size_t write(const u8* data, size_t dataSize) override;
};


/**
 * Serial device
 */

typedef unsigned SerialBaudRate;

enum class SerialDataBits {
    Five = 5,
    Six = 6,
    Seven = 7,
    Eight = 8,
};

enum class SerialStopBits {
    One = 1,
    OneAndHalf = 3,
    Two = 2
};

enum class SerialParity {
    Invalid = SP_PARITY_INVALID,
    None = SP_PARITY_NONE,
    Odd = SP_PARITY_ODD,
    Even = SP_PARITY_EVEN,
    Mark = SP_PARITY_MARK,
    Space = SP_PARITY_SPACE,
};

class SerialDevice : public Device {
private:
    struct sp_port* port_;
    SerialBaudRate baudRate_;
    SerialDataBits dataBits_;
    SerialStopBits stopBits_;
    SerialParity parity_;
    std::string name_;

    unsigned getTimeout();

public:
    static const char* DEVICE_NAME;
    static const SerialBaudRate BAUD_RATE = 2400;
    static const SerialDataBits DATA_BITS = SerialDataBits::Eight;
    static const SerialStopBits STOP_BITS = SerialStopBits::One;
    static const SerialParity PARITY = SerialParity::None;

    explicit SerialDevice(std::string& name,
                 SerialBaudRate baudRate,
                 SerialDataBits dataBits,
                 SerialStopBits stopBits,
                 SerialParity parity);
    ~SerialDevice();

    [[nodiscard]] inline struct sp_port* getPort() const {
        return port_;
    }

    size_t read(u8* buf, size_t bufSize) override;
    size_t write(const u8* data, size_t dataSize) override;
};

class SerialPortConfiguration {
private:
    struct sp_port_config* config_;
    SerialDevice& device_;

public:
    explicit SerialPortConfiguration(SerialDevice& device);
    ~SerialPortConfiguration();

    void setConfiguration(SerialBaudRate baudRate, SerialDataBits dataBits, SerialStopBits stopBits, SerialParity parity);
};

bool is_serial_baud_rate_valid(SerialBaudRate baudRate);
bool is_serial_data_bits_valid(SerialDataBits dataBits);
bool is_serial_stop_bits_valid(SerialStopBits stopBits);
bool is_serial_parity_valid(SerialParity parity);


/**
 * Pseudo device
 */

class PseudoDevice : public Device {
public:
    PseudoDevice() = default;
    ~PseudoDevice() = default;

    size_t read(u8* buf, size_t bufSize) override;
    size_t write(const u8* data, size_t dataSize) override;
};

}

#endif //INVERTER_TOOLS_VOLTRONIC_DEVICE_H
