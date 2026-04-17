# NCP UART Application


This network coprocessor (NCP) application supports communication with a host application over a UART interface with hardware flow control. This NCP application can be built as configured, or optionally can be augmented with customizations for target hardware, initialization, main loop processing, event definition/handling, and messaging with the host.

As configured, this NCP UART application is the basis for building the corresponding binary ncp-uart images delivered with the Silicon Labs Zigbee stack.

Due to thread safety concerns with EZSP, this application cannot include an RTOS unless CPC (Co-Processor Communication) components are also added.
  
Refer to the Silicon Labs Zigbee documentation for more information about NCP customization.
