// SPDX-License-Identifier: BSD-3-Clause

#include "functions.h"

namespace p18 {

bool is_valid_parallel_id(unsigned id)
{
    return id >= 0 && id <= 6;
}

}