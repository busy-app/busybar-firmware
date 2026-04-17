# Zigbee + Green Power NCP + OpenThread RCP (CPC-UART) Application

A network co-processor (NCP) application that supports concurrent multiprotocol (CMP) operation, running Zigbee + GP on NCP alongside OpenThread RCP with Co-Processor Communication Protocol (CPC) support.

## Overview

This application enables simultaneous operation of Zigbee + Green Power and OpenThread protocols on a single device, utilizing the Co-Processor Communication Protocol (CPC) as a protocol multiplexer and serial transport layer. Host applications connect to the CPC daemon, which interfaces with the EFR via a UART link.

## Key Features

- **Concurrent Multiprotocol (CMP) Operation**: Runs Zigbee NCP and OpenThread RCP simultaneously
- **CPC Communication**: Uses Co-Processor Communication Protocol for host communication
- **Multi-RAIL Support**: Utilizes multiple RAIL handles for enhanced protocol handling
- **Green Power Device Frame (GPDF) Support**: Handles bidirectional GPDF transmission
- **Custom EZSP Commands**: Provides application-specific queue management interface

## Architecture

### Multi-RAIL Integration

The application integrates the multiple RAIL demo component (`multirail-demo`), which enables:
- Use of a second RAIL handle to schedule pre-configured outgoing Green Power device frames (GPDF)
- Response to incoming bidirectional GPDFs with the rx-after-tx bit set
- Separation of Zigbee stack operations from application-specific RAIL operations

### Green Power TX Queue

Implements a simple GP TX queue system:
- Host initializes and submits outgoing GPDF packets against GPD addresses
- Automatic transmission scheduling when receiving GPDFs with rxAfterTx bit set
- Uses additional RAIL handle with configurable RX offset time (`GP_RX_OFFSET_USEC` = 20500 microseconds)

## Configuration

### Application Settings

- **Multi-RAIL Library**: Enabled instead of single RAIL
  - One handle used by Zigbee stack
  - Second handle used by application
- **multirail-demo Plugin**: Enabled (initializes additional RAIL handle)
- **GP Library**: Sink and proxy table set to non-zero values

### Queue Configuration

The RX offset time can be configured by defining:
```c
#define GP_RX_OFFSET_USEC [microseconds]  // Default: 20500
```

## Custom EZSP Commands

The application implements the following custom EZSP commands for queue management:

### SL_ZIGBEE_CUSTOM_EZSP_COMMAND_INIT_APP_GP_TX_QUEUE
Initializes and clears the application-specific GP outgoing tx queue.

### SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_APP_GP_TX_QUEUE
Sets (adds or overwrites) a GPDF frame in the queue for a given GPD.

### SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_APP_GP_TX_QUEUE
Gets (reads back) the content from the queue for a GPD. This is a test API for sending the raw command out using the additional RAIL handle.

### SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SEND_APP_GP_RAW
Sends a raw GP packet on a specific channel and time.

## Operation Flow

1. **Initialization**: Host initializes the GP TX queue using custom EZSP commands
2. **Queue Population**: Host submits outgoing GPDF packets against GPD addresses
3. **Incoming GPDF Detection**: Application monitors for incoming GPDFs via `emberPacketHandoffIncoming`
4. **Conditional Transmission**: When a GPDF with rxAfterTx bit is received:
   - Queue is read for matching GPD
   - Transmission is scheduled using additional RAIL handle
   - Transmission occurs after configured RX offset time

## Customization

The Zigbee NCP can be extended with:
- Custom initialization routines
- Main loop processing enhancements
- Event handling modifications
- Host messaging features

## Documentation References

Refer to the Silicon Labs Zigbee documentation for more information about NCP customization.
