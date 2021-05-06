/**
 * Copyright (c) 2014, Zac Bergquist
 * Copyright (c) 2021, Evgeny Zinoviev
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *    following disclaimer in the documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef THIRD_PARTY_HEXDUMP_H_
#define THIRD_PARTY_HEXDUMP_H_

#include <cctype>
#include <iomanip>
#include <ostream>
#include <ios>

template <unsigned rows, bool ascii>
class custom_hexdump {
public:
    custom_hexdump(void* data, unsigned length) :
            data(static_cast<unsigned char*>(data)), length(length) { }

    const unsigned char* data;
    const unsigned length;
};

template <unsigned rows, bool ascii>
std::ostream& operator<<(std::ostream& out, const custom_hexdump<rows, ascii>& dump)
{
    // save state
    std::ios_base::fmtflags f(out.flags());

    out.fill('0');
    for (int i = 0; i < dump.length; i += rows) {
        out << "0x" << std::setw(4) << std::hex << i << ": ";
        for (int j = 0; j < rows; ++j) {
            if (i + j < dump.length) {
                out << std::hex << std::setw(2) << static_cast<int>(dump.data[i + j]) << " ";
            } else {
                out << "   ";
            }
        }

        out << " ";
        if (ascii) {
            for (int j = 0; j < rows; ++j) {
                if (i + j < dump.length) {
                    if (std::isprint(dump.data[i + j])) {
                        out << static_cast<char>(dump.data[i + j]);
                    } else {
                        out << ".";
                    }
                }
            }
        }
        out << std::endl;
    }

    // restore state
    out.flags(f);

    return out;
}

typedef custom_hexdump<16, true> hexdump;

#endif // THIRD_PARTY_HEXDUMP_H_