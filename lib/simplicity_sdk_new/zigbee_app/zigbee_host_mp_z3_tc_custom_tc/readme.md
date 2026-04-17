# Multi-PAN Host Application with Zigbee 3.0 Trust Center and Custom Trust Center


This is a Multi-PAN application with a Zigbee 3.0 Trust Center with security and a Custom Trust Center without security. 

Refer to https://www.silabs.com/documents/public/user-guides/UG102.pdf for more information.

Multiple PANs are achieved by configuring a single radio to filter packets on configured networks (two in this case). Both networks must be on the same channel. The networks can operate using different security settings, different network parameters, such as short ID, PAN ID, and extended PAN ID, and so on. The channel and EUI64 of the node stay the same on all network. The multi-PAN node can only act as coordinator.
