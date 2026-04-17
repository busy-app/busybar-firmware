# XNCP LED Sample Applications


This sample scenario presents an NCP application for communicating with a UNIX HOST using custom EZSP commands. This application is meant to be used with the HOST sample application zigbee_host_xncp_led.

Both of these sample applications use a very simple protocol for controlling an LED. The LED exists on the NCP, and the HOST can control it by sending custom EZSP commands defined in the header led-protocol.h. For example, the HOST can command the NCP to set its LED by issuing a custom EZSP command consisting of the single byte 0x00. The HOST can also clear the LED, strobe the LED, and get or set the frequency at which the LED strobes.

The main interface to this application exists on the HOST side, where a user can issue CLI commands to alter the state of the LED on the NCP.

These sample applications can easily be altered to add extra functionality, while using the LED commands for testing.
