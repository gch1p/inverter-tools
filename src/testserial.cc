// SPDX-License-Identifier: BSD-3-Clause
//
// This is a test program, used to test libserialport.
// You don't need it.

#include <libserialport.h>
#include <cstdio>
#include <cstdlib>
#include <hexdump.h>
#include <iostream>
#include <unistd.h>

#define BUFSIZE 256
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void die_on_fail(const char* s, int result) {
    if (result != SP_OK) {
        fprintf(stderr, "%s failed\n", s);
        exit(1);
    }
}


static const unsigned char PI[] = {
    0x5e, 0x50, 0x30, 0x30, 0x35, 0x50, 0x49,
    0x71, 0x8b, 0x0d
};
static const unsigned char GS[] = {
    0x5e, 0x50, 0x30, 0x30, 0x35, 0x47, 0x53,
    0x58, 0x14, 0x0d
};

int main(int argc, char** argv) {
    struct sp_port* port;
    struct sp_port_config *config;
    char buf[BUFSIZE] = {0};

    die_on_fail("sp_get_port_by_name", sp_get_port_by_name("/dev/ttyUSB0", &port));
    die_on_fail("sp_open", sp_open(port, SP_MODE_READ_WRITE));

    printf("configuring...\n");

    die_on_fail("sp_new_config", sp_new_config(&config));
    die_on_fail("sp_get_config", sp_get_config(port, config));

    die_on_fail("sp_set_config_baudrate", sp_set_config_baudrate(config, 2400));
    die_on_fail("sp_set_config_stopbits", sp_set_config_stopbits(config, 1));
    die_on_fail("sp_set_config_bits", sp_set_config_bits(config, 8));
    die_on_fail("sp_set_config_parity", sp_set_config_parity(config, SP_PARITY_NONE));
    die_on_fail("sp_set_config_flowcontrol", sp_set_config_flowcontrol(config, SP_FLOWCONTROL_NONE));
    die_on_fail("sp_set_config", sp_set_config(port, config));

    printf("configured.\n");
    sp_flush(port, SP_BUF_BOTH);

    printf("writing %lu bytes...\n", ARRAY_SIZE(PI));
    int written = sp_blocking_write(port, PI, ARRAY_SIZE(PI), 0);
    printf("%d bytes written\n", written);

    usleep(200000);

    printf("reading...\n");
    int read = sp_blocking_read_next(port, buf, ARRAY_SIZE(buf), 0);
    printf("got %d bytes:\n", read);
    std::cout << hexdump(buf, read) << std::endl;

    printf("cleaning up...\n");

    sp_free_config(config);
    sp_free_port(port);

    return 0;
}