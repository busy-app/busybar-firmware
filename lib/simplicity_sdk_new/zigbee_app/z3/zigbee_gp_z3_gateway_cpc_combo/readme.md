# Zigbee 3.0 Gateway CPC Combo - Green Power Application

## Overview

The Zigbee 3.0 Gateway CPC Combo application is a host-side application designed to work in conjunction with the `zigbee_ncp_ot_rcp_uart_gp_multi_rail` Network Co-Processor (NCP) application. This combination creates a comprehensive Green Power Combo solution where the Green Power functionality is handled by the NCP while the host application provides a command-line interface (CLI) for interaction and control.

## Green Power Combo Architecture

This application demonstrates a **Green Power Combo** implementation, which combines both Green Power Proxy and Green Power Sink functionalities:

### Green Power Proxy
- Relays Green Power Device (GPD) communications to the broader Zigbee network
- Handles bidirectional communication between battery-powered Green Power devices and the Zigbee network
- Manages Green Power commissioning and operational data forwarding

### Green Power Sink  
- Acts as the final destination for Green Power commands
- Processes and responds to Green Power Device communications
- Maintains Green Power Device translation tables and security keys

## Technical Implementation

The application utilizes a **host-NCP architecture** where:
- **NCP Side**: The `zigbee_ncp_ot_rcp_uart_gp_multi_rail` handles all low-level Green Power protocol operations, including:
  - Green Power frame processing
  - Security key management
  - Device commissioning procedures
  - Translation table maintenance
- **Host Side**: This application provides high-level control and user interface through CLI commands

## CLI Interface

The gateway provides a comprehensive CLI command interface for Green Power operations, including commissioning control commands to initiate Green Power device pairing

## Key Features

- **Green Power Combo support**: Functions as both proxy and sink
- **Host-NCP communication**: Efficient separation of protocol handling and user interface
- **Network integration**: Seamless integration of Green Power devices into Zigbee 3.0 networks
- **CLI-based control**: User-friendly command interface for Green Power operations
- **Power consumption optimization**: Green Power operations handled directly on NCP reduce CPC communication overhead via UART interface, minimizing power consumption

This application serves as an excellent foundation for developing Green Power-enabled gateway solutions and demonstrates best practices for Green Power Combo implementations in Zigbee 3.0 environments.
