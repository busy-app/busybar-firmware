# Services {#mainpage}

## Introduction

The RAIL SDK components offer a practical framework for building wireless
communication applications. These components help developers create applications
that can adapt to various communication needs and manage power consumption
effectively. Whether you're working on a simple project or a complex
multi-protocol system, the RAIL SDK provides the necessary tools and features to
support your development.

@defgroup rail_sdk_extension Extension

## Extension

Extensions for the RAIL SDK are components that enhance existing sample
applications or custom sample applications with new features without modifying
the existing code. One such extension is the \ref rail_sdk_simple_os extension.

@defgroup rail_sdk_simple_os Simple OS

The \ref rail_sdk_simple_os component in the RAIL SDK provides a basic structure
for RAIL SDK users to update a baremetal application to an OS-based application.
It supports FreeRTOS and Micrium OS, offering a seamless transition for
developers who need to integrate OS functionalities into their applications.

## Key Features

- **Task Initialization**: The \ref app_task_init() function initializes the
proprietary application task without using any dynamic memory allocation. This
ensures that the task is created with a predefined stack and task buffer, making
it suitable for memory-constrained environments.
- **Task Notification**: The \ref app_task_notify function is used to notify the
kernel to allow the proprietary task to run. This function ensures that the task
is only notified if it has been successfully created.
- **Static Task Allocation**: The component uses static task allocation to avoid
dynamic memory allocation, which is crucial for systems with limited memory
resources. The task stack and buffer are statically allocated, ensuring
predictable memory usage.
- **Max Blocking Time**: The component defines a maximum blocking time for the
proprietary application task, ensuring that the task does not block
indefinitely.

@defgroup rail_sdk_utility Utility

## Utility

The Utility components in the RAIL SDK provide a toolbox of essential tools and
functions that enhance the development and management of wireless communication
applications. These utilities leverage various RAIL library features to offer
additional capabilities, such as channel and PHY (Physical Layer) selection,
power management integration, and packet handling. Below is a detailed
description of the key components and their functionalities:

@defgroup rail_sdk_channel_selector Channel Selector

The RAIL Channel Selector component allows the application to dynamically select
the communication channel at both compile-time and runtime. This flexibility is
crucial for applications that need to operate on different channels based on
environmental conditions or regulatory requirements. The component provides
functions to set and get the selected channel, ensuring that the application can
easily switch channels as needed.

**Key functions include:**

- \ref set_selected_channel(): Sets the communication channel.
- \ref get_selected_channel(): Retrieves the current communication channel.
- \ref restart_rx_channel(): Restarts the RX operation on the selected channel.

@defgroup rail_sdk_phy_selector PHY Selector

The RAIL PHY Selector component allows the application to select the PHY
configuration for communication. This component is useful for applications
that need to switch between different PHY settings to optimize performance or
comply with different communication standards. The component provides functions
to set and get the selected PHY, ensuring that the application can easily switch
PHY configurations as needed.

**Key functions include:**

- \ref set_selected_phy(): Sets the PHY configuration for communication.
- \ref get_selected_phy(): Retrieves the current PHY configuration.

@defgroup rail_sdk_packet_assistant Packet Assistant

The RAIL Packet Assistant component provides functions to prepare and unpack
packets for various protocols such as Wi-SUN FSK, Wi-SUN OFDM, SUN OQPSK, and
Sidewalk. This component ensures that the application can generate the
appropriate packet headers for any selected PHY, allowing RAIL to send out the
proper packet with the user payload. The component provides functions to prepare
and unpack packets, ensuring that the application can handle different packet
formats as needed.

**Key functions include:**

- \ref prepare_packet(): Prepares the packet for sending and loads it into the
RAIL TX FIFO.
- \ref unpack_packet(): Unpacks the received packet, points to the payload, and
returns the length.

@defgroup sl_rail_sdk_sleep Power Manager Integration

The Power Manager Integration component integrates RAIL with the Power Manager to enable
efficient power management. This integration ensures that the RAIL and Power
Manager work together in a synchronized mode, allowing the application to
manage power consumption effectively. The component provides functions to
initialize the power management integration, ensuring that the application can
enter and exit low-power states as needed.

**Key functions include:**

- \ref sl_rail_sdk_sleep_init(): Prepares the RAIL and Power Manager to work
together in a synchronized mode.

@defgroup rail_sdk_wmbus Wireless M-Bus

## Wireless M-Bus Support

The Wireless M-Bus components in the RAIL SDK provide a comprehensive suite of
tools and functionalities for developing and managing wireless communication
applications that adhere to the Wireless M-Bus standard. These components are
designed to facilitate the implementation of Wireless M-Bus communication in
various types of devices, such as meters and sensors, by offering support
for packet handling, encryption, channel and PHY selection, and power
management. Below is a detailed description of the key components and their
functionalities.

@defgroup rail_sdk_wmbus_support Wireless M-Bus Support

The Wireless M-Bus Support component adds minimalist support for coding and
decoding Wireless M-Bus packets. This component is essential for applications
that need to communicate using the Wireless M-Bus protocol, providing the
necessary tools to handle packet formatting and interpretation.

**Key Features:**

- **Packet Coding and Decoding**: Functions to encode and decode Wireless M-Bus
packets, ensuring correct data formatting for transmission and reception.
- **Function Codes**: Defines a set of function codes used in the first block's
C-field (see \ref sl_rail_sdk_wmbus_function_code_t), such as WMBUS_FUNCTION_SND_NKE, WMBUS_FUNCTION_SND_UD, and WMBUS_FUNCTION_SND_NR.
- **Device Types**: Supports various device types used in address fields,
including oil meters, electricity meters, gas meters, and more.
- **Manufacturer ID**: Functions to retrieve the manufacturer ID (M-field)
for M-Bus addressing.
- **Encryption and Decryption**: Includes functions for encrypting and
decrypting Wireless M-Bus frames using the Crypto5 algorithm.

@defgroup rail_sdk_wmbus_sensor_core Wireless M-Bus Sensor Core

The Wireless M-Bus Sensor Core component is designed for use with
Wireless M-Bus meters, providing the necessary tools to integrate various
sensors into the Wireless M-Bus communication framework. This component
simplifies the process of adding sensor data to Wireless M-Bus packets and
ensures that the data is correctly formatted and transmitted.

**Key Features:**

- **Sensor Initialization**: Functions to initialize Wireless M-Bus sensors,
ensuring they are correctly configured for communication.
- **Data Handling**: Functions to retrieve and format sensor data for inclusion
in Wireless M-Bus packets.
- **Packet Assembly**: Tools to assemble packets with sensor data, ensuring that
the data is correctly formatted and transmitted.

@defgroup rail_sdk_wmbus_sensor_pulse_counter Wireless M-Bus Button Pulse Counter

The Wireless M-Bus Pulse Counter component is part of the RAIL SDK's
Wireless M-Bus suite, designed to count and send button presses between
transmission frames. This component is specifically tailored for
Wireless M-Bus meters, providing a solution for integrating pulse counter
data into the Wireless M-Bus communication framework.

**Key Features:**

- **Pulse Counting**: Making button as a virtual pulse counter.

@defgroup rail_sdk_wmbus_sensor_thermometer Wireless M-Bus Thermometer Sensor

The Wireless M-Bus Thermometer Sensor component is designed to monitor and
measure temperature using a thermometer sensor. This component provides the
necessary tools to integrate temperature data into Wireless M-Bus packets,
ensuring that the data is correctly formatted and transmitted.

**Key Features:**

- **Temperature Measurement**: Functions to measure temperature using a thermometer sensor.
- **Data Handling**: Functions to retrieve and format temperature data for
inclusion in Wireless M-Bus packets.
- **Packet Assembly**: Tools to assemble packets with temperature data, ensuring
that the data is correctly formatted and transmitted.

@defgroup rail_sdk_wmbus_sensor_virtual_water_meter Wireless M-Bus Virtual Water Meter Sensor

The Wireless M-Bus Virtual Water Meter Sensor component is a software-based
solution designed to monitor and measure water usage in a given environment.
This component provides the necessary tools to integrate water meter data into
Wireless M-Bus packets, ensuring that the data is correctly formatted and
transmitted.

**Key Features:**

- **Water Usage Measurement**: Functions to measure water usage using a virtual
water meter sensor.
- **Data Handling**: Functions to retrieve and format water meter data for
inclusion in Wireless M-Bus packets.
- **Packet Assembly**: Tools to assemble packets with water meter data,
ensuring that the data is correctly formatted and transmitted.
