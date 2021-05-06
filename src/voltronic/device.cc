// SPDX-License-Identifier: BSD-3-Clause

#include <memory>
#include <iostream>
#include <limits>
#include <cstring>
#include <sstream>

#include "crc.h"
#include "device.h"
#include "time.h"
#include "exceptions.h"
#include "hexdump/hexdump.h"
#include "../logging.h"

namespace voltronic {

Device::Device() :
    flags_(FLAG_WRITE_CRC | FLAG_READ_CRC | FLAG_VERIFY_CRC),
    timeout_(TIMEOUT) {}

void Device::setFlags(int flags) {
    flags_ = flags;
}

int Device::getFlags() const {
    return flags_;
}

void Device::setVerbose(bool verbose) {
    verbose_ = verbose;
}

void Device::setTimeout(u64 timeout) {
    timeout_ = timeout;
    timeStarted_ = timestamp();
}

u64 Device::getElapsedTime() const {
    return timestamp() - timeStarted_;
}

u64 Device::getTimeLeft() const {
    if (!timeout_)
        return std::numeric_limits<uint64_t>::max();

    return std::max((u64)0, timeout_ - getElapsedTime());
}

size_t Device::run(const u8* inbuf, size_t inbufSize, u8* outbuf, size_t outbufSize) {
    send(inbuf, inbufSize);

    if (!getTimeLeft())
        throw TimeoutError("sending already took " + std::to_string(getElapsedTime()) + " ms");

    return recv(outbuf, outbufSize);
}

void Device::send(const u8* buf, size_t bufSize) {
    size_t dataLen;
    std::shared_ptr<u8> data;

    if ((flags_ & FLAG_WRITE_CRC) == FLAG_WRITE_CRC) {
        const CRC crc = crc_calculate(buf, bufSize);
        dataLen = bufSize + sizeof(u16) + 1;
        data = std::unique_ptr<u8>(new u8[dataLen]);
        crc_write(crc, &data.get()[bufSize]);
    } else {
        dataLen = bufSize + 1;
        data = std::unique_ptr<u8>(new u8[dataLen]);
    }

    u8* dataPtr = data.get();
    memcpy((void*)dataPtr, buf, bufSize);

    dataPtr[dataLen - 1] = '\r';

    if (verbose_) {
        myerr << "writing " << dataLen << (dataLen > 1 ? " bytes" : " byte");
        std::cerr << hexdump(dataPtr, dataLen);
    }

    writeLoop(dataPtr, dataLen);
}

void Device::writeLoop(const u8* data, size_t dataSize) {
    int bytesLeft = static_cast<int>(dataSize);

    while (true) {
        size_t bytesWritten = write(data, bytesLeft);
        if (verbose_)
            myerr << "bytesWritten=" << bytesWritten;

        bytesLeft -= static_cast<int>(bytesWritten);
        if (bytesLeft <= 0)
            break;

        if (!getTimeLeft())
            throw TimeoutError("data writing already took " + std::to_string(getElapsedTime()) + " ms");

        data = &data[bytesWritten];
    }
}

size_t Device::recv(u8* buf, size_t bufSize) {
    size_t bytesRead = readLoop(buf, bufSize);

    if (verbose_) {
        myerr << "got " << bytesRead << (bytesRead > 1 ? " bytes" : " byte");
        std::cerr << hexdump(buf, bytesRead);
    }

    bool crcNeeded = (flags_ & FLAG_READ_CRC) == FLAG_READ_CRC;
    size_t minSize = crcNeeded ? sizeof(u16) + 1 : 1;

    if (bytesRead < minSize)
        throw InvalidDataError("response is too small");

    const size_t dataSize = bytesRead - minSize;

    if (crcNeeded) {
        const CRC crcActual = crc_read(&buf[dataSize]);
        const CRC crcExpected = crc_calculate(buf, dataSize);

//        buf[dataSize] = 0;

        if ((flags_ & FLAG_VERIFY_CRC) == FLAG_VERIFY_CRC && crcActual == crcExpected)
            return dataSize;

        std::ostringstream error;
        error << std::hex;
        error << "crc is invalid: expected 0x" << crcExpected << ", got 0x" << crcActual;
        throw InvalidDataError(error.str());
    }

//    buf[dataSize] = 0;
    return dataSize;
}

size_t Device::readLoop(u8 *buf, size_t bufSize) {
    size_t size = 0;

    while(true) {
        size_t bytesRead = read(buf, bufSize);
        if (verbose_)
            myerr << "bytesRead=" << bytesRead;

        while (bytesRead) {
            bytesRead--;
            size++;

            if (*buf == '\r')
                return size;

            buf++;
            bufSize--;
        }

        if (!getTimeLeft())
            throw TimeoutError("data reading already took " + std::to_string(getElapsedTime()) + " ms");

        if (bufSize <= 0)
            throw std::overflow_error("input buffer is not large enough");
    }
}

}