# Multiprotocol (OpenThread + BLE) NCP

This multiprotocol Network Co-Processor (NCP) application supports running OpenThread (Full Thread Device) and Bluetooth Low Energy (BLE) stacks simultaneously on a host processor. It leverages Silicon Labs multiprotocol capabilities to enable concurrent operation of Thread and BLE, allowing seamless connectivity for both mesh and point-to-point wireless applications.

The host stacks and the NCP communicate using the Co-Processor Communication protocol (CPC), which acts as a protocol multiplexer and serial transport layer. Host applications connect to the CPC daemon, which in turn communicates with the EFR device via a UART or EUSART link.

Key features include:

- OpenThread FTD stack for robust mesh networking
- BLE controller supporting central, peripheral, advertising, and scanning roles
- Multiprotocol RAIL for concurrent wireless operation
- FreeRTOS-based task management and optimized memory configuration
- Flexible UART/EUSART configuration for CPC transport

Refer to [Multiprotocol Solution for Thread and Bluetooth on Silicon Labs Devices](https://www.silabs.com/wireless/thread/solutions/multiprotocol) for more information on running Thread and BLE concurrently with a multiprotocol Network Co-Processor.
