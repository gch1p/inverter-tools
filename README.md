# inverter-tools

**inverter-tools** is a collection of tools for controlling Voltronic hybrid solar
inverters. Only P18 protocol is supported at the moment, supporting more hardware 
is planned.

- `inverterctl` is a full-featured command line utility with all P18 commands
  supported.
  
- `inverterd` is a daemon that starts TCP server that accepts user requests. It 
  replaces inverterctl for multi-user scenarios, where there may be more than one
  simultaneous request to device, to avoid errors or lockups. 

## Requirements

- Linux (tested on x86_64 and armhf), macOS (tested on aarch64)
- C++17 compiler
- CMake
- HIDAPI
- libserialport

## Supported devices

As the time of writing, only InfiniSolar V 5KW was tested.

## Supported interfaces

* USB (HIDAPI)
* RS232 (libserialport)

## Usage

Please use the `--help` option for now. The help message has full description
for all possible options and commands.

## License

BSD-3-Clause