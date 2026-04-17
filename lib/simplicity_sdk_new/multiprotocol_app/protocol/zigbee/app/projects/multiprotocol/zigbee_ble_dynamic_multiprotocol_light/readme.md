# Dynamic Multiprotocol Light with Zigbee 3.0 + Bluetooth LE


Sample Dynamic Multiprotocol Light application with Zigbee 3.0 (Coordinator/Router) + Bluetooth LE. 
Uses NVM3 for persistent storage. 

The Dynamic Multiprotocol Demo Light application demonstrates the dynamic multiprotocol feature from Silicon Labs. In particular, these applications demonstrate the Zigbee and Bluetooth LE (BLE) stacks running concurrently as RTOS tasks. 
The Demo is set up to run on the WSTK6000B Mesh Development Kit. 

There are 2 demos - one makes use of the LCD, LEDs and Buttons on the mainboard and the other is a minimal application that does not use these peripherals.

**NOTE:** This demo application cannot perform OTA updates in its default configuration. In order to do OTA updates, uninstall the Zigbee LCD Display component. This will disable the LCD UI, but will allow the OTA to run successfully. 

The Light is configured to host a Zigbee network as a coordinator and allow other devices to join. A help screen is shown on the LCD to assist the user to form, permit join and control the Light. 

This application may be used in conjunction with any Zigbee 3.0 end device, such as zigbee_z3_switch, to join the network formed by the light and control the state of the light. 

The EFR Connect Bluetooth app may also be used to control the light over the BLE Link.  

On the BLE interface the Light is set up to advertise itself using Eddystone and iBeacon in an alternate manner. The Light advertises itself as available for a connection by a smartphone running the Wireless Gecko application. Once connected, the user can control the Light (represented by LEDs on the mainboard) using the Wireless Gecko smartphone application.

A change in the Light state initiated by the either the smartphone over the BLE connection or via the Zigbee-only Switch is propagated to connected devices over both the Zigbee and BLE interface.
