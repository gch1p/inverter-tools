// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_SIGNAL_H
#define INVERTER_TOOLS_SIGNAL_H

#include <csignal>

namespace server {

extern volatile sig_atomic_t shutdownCaught;

void set_signal_handlers();

}

#endif //INVERTER_TOOLS_SIGNAL_H
