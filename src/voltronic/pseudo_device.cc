// SPDX-License-Identifier: BSD-3-Clause

#include <stdexcept>
#include <sstream>
#include <cstring>

#include "device.h"
#include "crc.h"
#include "hexdump/hexdump.h"
#include "../logging.h"

namespace voltronic {

// PI
//static const char* response = "^D00518";

// GS
static const char* response = "^D1060000,000,2300,500,0115,0018,002,500,000,000,000,000,078,019,000,000,0000,0000,0000,0000,0,0,0,1,2,2,0,0";

// PIRI
//static const char* response = "^D0882300,217,2300,500,217,5000,5000,480,500,570,420,576,540,2,30,060,0,1,1,6,0,0,0,1,2,00";

// DI
//static const char* response = "^D0682300,500,0,408,540,564,460,540,060,30,0,0,1,0,0,0,1,0,0,1,1,0,1,1";

// set response
//static const char* response = "^1";

// TODO: maybe move size and crc stuff to readLoop()?
size_t PseudoDevice::read(u8* buf, size_t bufSize) {
    size_t pseudoResponseSize = strlen(response);

    size_t responseSize = pseudoResponseSize;
    if (flags_ & FLAG_READ_CRC)
        responseSize += 2;

    if (responseSize + 1 > bufSize) {
        std::ostringstream error;
        error << "buffer is not large enough (" << (responseSize + 1) << " > " << bufSize << ")";
        throw std::overflow_error(error.str());
    }

    memcpy(buf, response, responseSize);

    if (flags_ & FLAG_READ_CRC) {
        CRC crc = crc_calculate(buf, pseudoResponseSize);
        crc_write(crc, &buf[pseudoResponseSize]);
    }

    buf[responseSize] = '\r';

    return responseSize + 1;
}

size_t PseudoDevice::write(const u8* data, size_t dataSize) {
    if (verbose_) {
        myerr << "dataSize=" << dataSize;
        std::cerr << hexdump((void*)data, dataSize);
    }
    return dataSize;
}

}