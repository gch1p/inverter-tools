# inverter-tools

**inverter-tools** is a collection of tools for controlling Voltronic hybrid solar
inverters. Only P18 protocol is supported at the moment, supporting more hardware 
is planned.

- `inverterctl` is a full-featured command line utility with all P18 commands
  supported.
- `inverterd` is a daemon that starts a TCP server and implements simple text-based
  [telnet-compatible](inverterd-telnet.gif) chat [protocol](PROTOCOL.md). It supports all commands and options
  supported by inverterctl, and it's supposed to be used instead of inverterctl in
  multi-user or multi-threaded scenarios, where there may be more than one simultaneous
  request to device, to avoid errors or device lockups.

## Requirements

- Linux (tested on x86_64 and armhf), macOS (tested on aarch64)
- C++17 compiler
- CMake
- HIDAPI
- libserialport

## Using docker

`docker build -t inverterd .`
`docker run --rm --name inverterd --device /dev/ttyUSB0 inverterd --help`

## Tested devices

- [InfiniSolar V 5KW](https://ekoproekt-energo.ru/infinisolar-v-5k-48)
- [Crown Micro Elego 6KW](https://coollineenergy.com/product/crown-micro-elego-6kw-solar-hybrid-inverter/)

## Supported interfaces

* USB (HIDAPI)
* RS232 (libserialport)

## Usage

Please use the `--help` option for now. The help message has full description
for all possible options and commands.

## Related projects

- [inverterd-client](https://github.com/gch1p/inverterd-client) - a Python library
  for querying inverterd server.
- [inverter-bot](https://github.com/gch1p/inverter-bot) - Telegram bot that uses inverterd
  for querying data.
- [inverter-http-proxy](https://github.com/gch1p/inverter-http-proxy) - HTTP
  "frontend" for inverterd

## License

BSD-3-Clause