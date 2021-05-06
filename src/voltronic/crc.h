// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_VOLTRONIC_CRC_H
#define INVERTER_TOOLS_VOLTRONIC_CRC_H

#include <cstdint>
#include <cstdlib>
#include "../numeric_types.h"

namespace voltronic {

typedef u16 CRC;

void crc_write(CRC crc, u8* buffer);
CRC crc_read(const u8* buf);
CRC crc_calculate(const u8* buf, size_t bufSize);

}

#endif //INVERTER_TOOLS_VOLTRONIC_CRC_H
