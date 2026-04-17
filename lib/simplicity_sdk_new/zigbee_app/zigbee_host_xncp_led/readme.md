# XNCP LED Sample Applications


This sample scenario presents a HOST application for communicating with an NCP application using custom EZSP commands. This application is meant to be used with the NCP sample application zigbee_ncp_xncp_led.

Both of these sample applications use a very simple protocol for controlling an LED. The LED exists on the NCP, and the HOST can control it by sending custom EZSP commands defined in the header led-protocol.h. For example, the HOST can command the NCP to set its LED by issuing a custom EZSP command consisting of the single byte 0x00. The HOST can also clear the LED, strobe the LED, and get or set the frequency at which the LED strobes.

CLI commands can be used on the HOST to send these custom EZSP frames to the NCP. After the HOST application has been started up and connected to the NCP sample application via a UART, it can issue the following CLI command to set the LED on the NCP:
- custom set-led 0

It can command the LED into a strobe state and set its frequency with the following pair of commands:
- custom set-led 2
- custom set-frequency 256

These sample applications can easily be altered to add extra functionality, while using the LED commands for testing.
