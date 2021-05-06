// SPDX-License-Identifier: BSD-3-Clause

#include <ctime>
#include <cstdint>
#include <sys/time.h>
#include "time.h"

namespace voltronic {

u64 timestamp() {
    u64 ms = 0;

#if defined(CLOCK_MONOTONIC)
    static bool monotonic_clock_error = false;
    if (!monotonic_clock_error) {
        struct timespec ts = {0};
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            ms = static_cast<u64>(ts.tv_sec);
            ms *= 1000;
            ms += static_cast<u64>(ts.tv_nsec / 1000000);
            return ms;
        } else {
            monotonic_clock_error = true;
        }
    }
#endif

    struct timeval tv = {0};
    if (gettimeofday(&tv, nullptr) == 0) {
        ms = static_cast<u64>(tv.tv_sec);
        ms *= 1000;
        ms += static_cast<u64>(tv.tv_usec / 1000);
    }

    return ms;
}

}