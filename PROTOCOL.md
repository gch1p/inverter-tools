# inverterd protocol

inverterd implements simple text-based, telnet-compatible protocol.

## Requests

Each request is represented by a single line that ends with `\r\n`, in the
following format:
```
COMMAND [...ARGUMENTS]
```

Available commands:

- `v` `VERSION`<br>
  Sets the protocol version, affects subsequents requests. Default version is `1`.
  
- `format` `FORMAT`<br>
  Sets the data format for device responses.
  
- `exec` `COMMAND` `[...ARGUMENTS]`<br>
  Runs a command.

Sending `EOT` (`0x04`) closes connection.

## Responses

Each response is represented by one or more lines, each ending with `\r\n`, plus
extra `\r\n` in the end.

First line is always a status, which may be either `ok` or `err`.