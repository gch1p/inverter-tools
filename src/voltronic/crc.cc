// SPDX-License-Identifier: BSD-3-Clause

#include "crc.h"

namespace voltronic {

static const u16 table[16] = {
        0x0000, 0x1021, 0x2042, 0x3063,
        0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B,
        0xC18C, 0xD1AD, 0xE1CE, 0xF1EF
};

static inline bool is_reserved(u8 b) {
    return b == 0x28 || b == 0x0D || b == 0x0A;
}

CRC crc_read(const u8* buf) {
    CRC crc = 0;

    crc |= (u16) buf[0] << 8;
    crc |= (u16) buf[1];

    return crc;
}

void crc_write(CRC crc, u8* buffer) {
    if (buffer != nullptr) {
        buffer[0] = (crc >> 8) & 0xFF;
        buffer[1] = crc & 0xFF;
    }
}

CRC crc_calculate(const u8* buf, size_t bufSize) {
    CRC crc = 0;

    if (bufSize > 0) {
        u8 byte;
        do {
            byte = *buf;

            crc = table[(crc >> 12) ^ (byte >> 4)] ^ (crc << 4);
            crc = table[(crc >> 12) ^ (byte & 0x0F)] ^ (crc << 4);

            buf += 1;
        } while (--bufSize);

        byte = crc;
        if (is_reserved(byte))
            crc += 1;

        byte = crc >> 8;
        if (is_reserved(byte))
            crc += 1 << 8;
    }

    return crc;
}

}