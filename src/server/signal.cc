// SPDX-License-Identifier: BSD-3-Clause

#include "signal.h"

namespace server {

volatile sig_atomic_t shutdownCaught = 0;

static void sighandler(int) {
    shutdownCaught = 1;
}

void set_signal_handlers() {
    struct sigaction sa = {0};
    sa.sa_handler = sighandler;
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
}

}