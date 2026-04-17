# Host NCP test

## Features

- Demonstrates the usage of user NCP commands, and
- demonstrates how to implement a simple application to test NCP
  performance using the default user commands.

## Requirements

- NCP target EFR device

## Usage

```bash
bt_host_ncp_test -u <serial_port> -c <command_id>
```

Replace <serial_port> with your actual serial port device, and supply a valid command
id as <command_id>.

Valid commands are
- 1: Periodic Async
- 3: Get Board Name
- 5: Periodic Sync

For a complete list of command line options, use the `-h` option.
