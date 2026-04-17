# Multiprotocol (OpenThread+Zigbee+BLE) RCP

This multiprotocol radio co-processor (RCP) application supports running OpenThread, Zigbee, and Bluetooth stacks simultaneously on a host processor. It uses concurrent multiprotocol (CMP) / multi-PAN functionality to run the 802.15.4 networks simultaneously on the same channel, and dynamic multiprotocol (DMP) to run the Bluetooth Link Layer simultaneously.

The host stacks and the RCP communicate using the Co-Processor Communication protocol (CPC), which acts as a protocol multiplexer and serial transport layer. The host applications connect to the CPC daemon, which in turn connects to the EFR via a SPI or UART link.

Refer to _[Running Zigbee, OpenThread, and Bluetooth Concurrently on a Linux Host with a Multiprotocol Co-processor](https://docs.silabs.com/multiprotocol/latest/multiprotocol-solution-linux/)_ for more information on running the multiprotocol RCP with different host applications.
