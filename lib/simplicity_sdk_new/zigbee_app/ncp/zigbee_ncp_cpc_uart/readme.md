# NCP UART Application


This network coprocessor (NCP) application supports communication with a host application using the Co-Processor Communication Protocol (CPC), which acts as a protocol multiplexer and serial transport layer. The host applications connect to the CPC daemon, which in turn connects to the EFR via a UART link. The Zigbee NCP part of this application can be built as configured, or can optionally be augmented with customized extensions for initialization, main loop processing, event definition/handling, and messaging with the host.
author: Silicon Laboratories, Inc.
  
Refer to the Silicon Labs Zigbee documentation for more information about NCP customization.
