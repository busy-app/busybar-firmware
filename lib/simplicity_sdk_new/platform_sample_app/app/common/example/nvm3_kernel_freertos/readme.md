
# NVM3 Kernel FreeRTOS Example

This example application demonstrates the use of Third Generation Non-Volatile Memory (NVM3) data storage with a command line interface (CLI) for interactive operations.

## Features

- Store, read, and delete up to 200 data objects in NVM3 using CLI commands
- Track the number of write and delete operations using dedicated counter objects
- Display valid and deleted objects, including key, type, size, and data preview
- Show and reset operation counters on each display
- Display current NVM3 memory information (available memory, low memory, cache status)
- Repack NVM3 with custom headroom and repack count for testing
- Repack timings and available memory are displayed after the repack operation
- Erase all NVM3 data and reinitialize counters

## CLI Commands

- `write <key> <data>`: Store string data at the specified key (0x00 to 0xC7)
- `read <key>`: Read and print data stored at the specified key
- `delete <key>`: Delete the data object at the specified key
- `display`: Show deleted objects, valid objects (with key, type, size, data), and operation counters since last display
- `meminfo`: Display available memory, low memory status, and cache status
- `repack <headroom> <count>`: Fill NVM3 until memory is low and perform repack with custom headroom and repack count
- `reset`: Erase all NVM3 data and reinitialize counters

## Counters

The application maintains two counter objects:
- Write counter: Tracks the number of write operations since the last `display` command
- Delete counter: Tracks the number of delete operations since the last `display` command
Counters are reset to zero after each `display`.

## Requirements

- Silicon Labs board with MPU module

## Resources

- [AN1135: Using Third Generation Non-Volatile Memory (NVM3) Data Storage](https://www.silabs.com/documents/public/application-notes/an1135-using-third-generation-nonvolatile-memory.pdf)