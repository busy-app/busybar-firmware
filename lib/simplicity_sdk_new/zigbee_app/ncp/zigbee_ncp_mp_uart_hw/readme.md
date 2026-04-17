# Multi-PAN NCP UART Application


This application provides support to form two personal area networks on same channel on sigle radio. Multiple PANs are achieved by configuring a single radio to filter packets on configured networks (two in this case). The both networks must be on the same channel. The networks can operate using different security settings, different network parameters such as short ID, PAN ID, extended PAN ID and so on. The parameters that stay the same on all networks are the channel and EUI64 of the node. The multi PAN node can only act as coordinator.

The zigbee_host_mp_z3_tc_custom_tc application can be used with this NCP application. This network coprocessor (NCP) application supports communication with a host application over a UART interface with hardware flow control. This NCP application can be built as configured, or optionally can be augmented with customizations for target hardware, initialization, main loop processing, event definition/handling, and messaging with the host.

Refer to the Silicon Labs Zigbee documentation for more information about NCP customization.
