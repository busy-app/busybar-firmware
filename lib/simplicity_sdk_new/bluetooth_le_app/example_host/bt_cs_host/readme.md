# CS host application
---

This is the host application for the Channel Sounding (CS) NCP target application.

## Features

* Control of the multi role CS NCP
  * Create initiator and/or reflector instances
  * Configure wired or wireless antenna setup
  * Configure CS mode
  * Configure object tracking mode
  * Configure antenna selection for antenna switching
  * Configure connection PHY
  * Use reflector address filtering
* Display only measurement results
* Use BGAPI Trace to get logging data

In two way CS, both the initiator and the reflector instances are collecting IQ samples. The initiator starts and orchestrate the whole procedure. The reflector component responds to the initiator, and sends its measurement data to the initiator. When all data collected, the initiator estimates the distance.

The host application is extended with application specific messages that we refer to as ACP (Application Co-Processor) messages.
The following ACP commands are sent from the host to the target device:
* Create initiator instance
* Delete initiator instance
* Create reflector instance
* Delete reflector instance
* Configure antenna
* Enable BGAPI Trace

The following ACP events are sent from the target device to the host:
* CS results
* CS intermediate results
* CS extended results
* Error events

## Limitations, known issues

- Max 4 initiator/reflector instances are supported (-I4 -R4)

## Multiconnection

- Default setup is optimized for 1-1 connection, multiconnection setup requires modification of the timing parameters to operate as expected. Timing can be adjusted by the procedure_interval and connection_interval parameters.
- Use the following calculation for 1-N connection: procedure_time_1_N[ms] = connection_interval[ms] * procedure_interval * N
- Note that setting CS_INITIATOR_DEFAULT_MIN/MAX_CONNECTION_INTERVAL and CS_INITIATOR_DEFAULT_MIN/MAX_PROCEDURE_INTERVAL will only take effect if CS_INITIATOR_DEFAULT_PROCEDURE_SCHEDULING is set to CS_PROCEDURE_SCHEDULING_CUSTOM. Otherwise these parameters are managed by the application.
- If getting frequent measurement results is not priority, it's safe to use procedure_interval = 120 and connection_interval = 24 even with the maximum number of connections (4).

## Getting started

### Prerequisites

To compile the sources, you need the following tools:

* GCC and libc, or other suitable C compiler for the host
* make utility

### Building the application
On the target: Build and flash the "bt_cs_ncp" application.
On the host: Build the application by issuing the following command:

`make`

in the project's root directory. This will build your executable, and place it in the exe folder. You can clean the build products by issuing:

`make clean`

### Running the application

To run the application with the default options, use the following command line. Replace the serial device with your tty/COM device.

`./exe/bt_cs_host -u /dev/tty.usbmodem0004402717881`

For complete list of command line argument, use the `-h` option to list them.

This will create one initiator and one reflector instance on the NCP.

The initiator instances are scanning for usable reflectors, and initiate a connection to it. When connected, the Initiator component starts the CS procedure.
The Real Time Library (RTL) estimates the distance, and sends the result to the host using ACP, which displays them in the command line terminal.
Reflector instances are advertising with device name "CS RFLCT".

## Resources
[Bluetooth Channel Sounding development guide](https://docs.silabs.com/rtl-lib/latest/rtl-lib-channel-sounding-dev-guide/04-sample-applications)
