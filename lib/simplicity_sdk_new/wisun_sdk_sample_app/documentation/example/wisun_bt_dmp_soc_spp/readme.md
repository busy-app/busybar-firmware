# Wi-SUN Bluetooth DMP - SoC SPP

## Table of Contents

- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
  - [Materials](#materials)
- [Project Setup](#project-setup)
- [Development](#development)
  - [Designing the GATT Database](#designing-the-gatt-database)
  - [Responding to Wi-SUN Events](#responding-to-wi-sun-events)
  - [Responding to Bluetooth Events](#responding-to-bluetooth-events)
  - [Implementing Application Logic](#implementing-application-logic)
- [Push-button actions](#push-button-actions)
- [LED status indications](#led-status-indications)
- [Troubleshooting](#troubleshooting)
- [Resources](#resources)
  - [Wi-SUN documents](#wi-sun-documents)
  - [Bluetooth Low Energy documents](#bluetooth-low-energy-documents)
- [Report Bugs & Get Support](#report-bugs--get-support)

## Introduction

The Wi-SUN Bluetooth DMP - SoC SPP example project demonstrates how Serial Port Profile (SPP) can be used over Bluetooth Low Energy (BLE) to enable wireless serial communication between devices, effectively replacing traditional wired connections.

> Note: This example can be tested using the **Serial Bluetooth Terminal** smartphone application, available on [Android](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en).
This project **does not** expect a specific Gecko Bootloader to be present on your device.

## Prerequisites

The provided method is only applicable for **EFR32xG28** devices which support Bluetooth Low Energy and Wi-SUN protocols.

## Getting Started

The principle is the same for all Bluetooth DMP (Dynamic Multiprotocol) applications: the device is able to communicate via Bluetooth and Wi-SUN at the same time, making it possible to control the DMP device with different type of devices.

### Materials

- To get started with Wi-SUN and Simplicity Studio, see [Getting Started with Wi-SUN Application Development](https://docs.silabs.com/wisun/latest/wisun-getting-started-development).

- To get started with Silicon Labs Bluetooth and Simplicity Studio, see [Getting Started with Silicon Labs Bluetooth LE Development](https://docs.silabs.com/bluetooth/latest/bluetooth-getting-started-overview).

- To learn more about programming an SoC application using the Silicon Labs Bluetooth stack, see [Silicon Labs Bluetooth C Application Developer's Guide for SDK](https://docs.silabs.com/bluetooth/latest/bluetooth-c-soc-dev-guide-sdk-v9x).

## Project Setup

![Project Setup](readme_img1.png)

As the name implies, this example utilizes the Serial Port Profile (SPP) to facilitate serial communication, allowing for the execution of Wi-SUN CLI commands and enabling RTT trace logging over BLE for debug purposes. For this purpose, the example includes a custom GATT profile with SPP data characteristic.

On the Wi-SUN side the stack is initialized with only the necessary components, and it automatically joins a Wi-SUN network upon startup.

On the BLE side the stack is initialized, basic functions are implemented and user interaction is provided through buttons to enable/disable advertisement with additional status LED indications. Once a BLE connection is successfully established, SPP mode activates, allowing the user to send Wi-SUN CLI commands to the device over BLE.

## Development

### Project Structure

This example implements custom I/O stream read and write callback functions that utilize the Serial Port Profile (SPP) BLE service, running in parallel with Wi-SUN protocol on the device. The project is based on a Real Time Operating System (Micrium OS or FreeRTOS according to your choice). Callbacks for Bluetooth and Wi-SUN event handling are added, but not implemented (except a very basic functionalities). The developer can add any functionality.

The development of a Wi-SUN - BLE DMP application consists of four main steps:

- [Designing the GATT Database](#designing-the-gatt-database)
- [Responding to the events raised by the Wi-SUN stack](#responding-to-wi-sun-events)
- [Responding to the events raised by the Bluetooth stack](#responding-to-bluetooth-events)
- [Implementing additional application logic](#implementing-application-logic)

These steps are covered in the following sections.

For more information about using Real Time Operating Systems with Bluetooth, see [Integrating v3.x Silicon Labs Bluetooth Applications with Real-Time Operating Systems](https://docs.silabs.com/bluetooth/latest/integrating-v3x-bluetooth-applications-with-rtos).

### Designing the GATT Database

Because Bluetooth Low Energy does not have a standard SPP service, it is implemented as a custom service in this example. The custom service is as minimal as possible. Only one characteristic is used for both incoming and outgoing data. The service is defined in the **gatt_configuration.btconf** file.

- BLE advertiser name: `WiSUN SP`
- SPP Service UUID: `4880c12c-fdcb-4077-8920-a450d7f9b907`
- SPP Characteristic UUID: `fec26ec4-6d71-4442-9f81-55bc21d658d6`

For incoming data (data sent by the SPP client/smartphone), unacknowledged write transfers (write_no_response) are used, which provides better performance than normal acknowledged writes because several write operations can fit into one connection interval.

For outgoing data (data sent by SPP server/EFR device), BLE notifications are used. Notifications are unacknowledged, which again allows several notifications to fit into one connection interval.

GATT definitions (services/characteristics) can be extended using the GATT Configurator, which can be found under Advanced Configurators in the Software Components tab of the Project Configurator. To open the Project Configurator, open the .slcp file of the project.

![Opening GATT Configurator](readme_img2.png)

To learn how to use the GATT Configurator, see [GATT Configurator User’s Guide for Bluetooth LE and Bluetooth Mesh](https://docs.silabs.com/bluetooth/latest/gatt-configurator-users-guide-ble-btmesh).

### Responding to Wi-SUN Events

A Wi-SUN application is event-driven. The Wi-SUN stack generates events when a connection is successful, data has been sent or an IP packet is received. The application has to handle these events in the `sl_wisun_on_event()` function. The prototype of this function is implemented in *app.c*. To handle more events, the switch-case statement of this function must be created and extended. For the list of Wi-SUN events, see [Wi-SUN events API Reference](https://docs.silabs.com/wisun/latest/wisun-stack-api/sl-wisun-evt).

### Responding to Bluetooth Events

A Bluetooth application is event driven. The Bluetooth stack generates events e.g., when a remote device connects or disconnects or when it writes a characteristic in the local GATT database. The application has to handle these events in the `sl_bt_on_event()` function. The prototype of this function is implemented in *app.c*. To handle more events, the switch-case statement of this function is to be extended. For the list of Bluetooth events, see the online [Bluetooth API Reference](https://docs.silabs.com/bluetooth/latest/bluetooth-stack-api).

### Implementing Application Logic

Additional application logic can be implemented in the *app_task()* function. Find the definition of this function in *app.c*. The *app_task()* function is called once when the device is booted and after the Wi-SUN stack initialization.

The first step in most Wi-SUN applications is to call *sl_wisun_join()* to connect the Wi-SUN device to a Wi-SUN border router. The remaining implementation is up to the developer. Refer to the [Wi-SUN API Reference](https://docs.silabs.com/wisun/latest/wisun-stack-api) to check the list of Wi-SUN APIs available to the application.

## Push-button actions

| Description | Main Board Button |
|---|---|
| Toggle BLE advertisement | PB0/BTN0 **or** PB1/BTN1 |

## LED status indications

| Description | Main Board LED |
|---|---|
| LED blinks to indicate that BLE advertisement is enabled | LED0 |

## RTT trace log commands

| Command | Description | Example |
|---|---|---|
| start_rtt_report | Start RTT trace log reports to the client periodically | > wisun start_rtt_report |
| stop_rtt_report | Stop RTT trace log reports to the client | > wisun stop_rtt_report |

## Troubleshooting

Before programming the radio board mounted on the WSTK, ensure the power supply switch is in the AEM position (right side), as shown.

![Radio Board Power Supply Switch](readme_img0.png)

## Resources

### Wi-SUN documents

- [Wi-SUN Stack API documentation](https://docs.silabs.com/wisun/latest)

### Bluetooth Low Energy documents

- [Bluetooth LE Documentation](https://docs.silabs.com/bluetooth/latest)
- [Bluetooth LE Fundamentals](https://docs.silabs.com/bluetooth/latest/bluetooth-le-fundamentals)
- [Getting Started with Silicon Labs Bluetooth LE Development](https://docs.silabs.com/bluetooth/latest/bluetooth-getting-started-overview)
- [Silicon Labs Bluetooth C Application Developer's Guide for SDK v10.x](https://docs.silabs.com/bluetooth/latest/bluetooth-c-soc-dev-guide-sdk-v9x)
- [Integrating v3.x Silicon Labs Bluetooth Applications with Real-Time Operating Systems](https://docs.silabs.com/bluetooth/latest/integrating-v3x-bluetooth-applications-with-rtos)
- [Dynamic Multiprotocol Development with Bluetooth and Proprietary Protocols on RAIL in GSDK v3.x and Higher](https://docs.silabs.com/multiprotocol/latest/multiprotocol-dynamic-ble-proprietary-on-rail)
- [Dynamic Multiprotocol User's Guide](https://docs.silabs.com/multiprotocol/latest/multiprotocol-dynamic-ug)
- [Bluetooth Training](https://www.silabs.com/support/training/bluetooth)

## Report Bugs & Get Support

You are always encouraged and welcome to report any issues you found to us via [Silicon Labs Community](https://www.silabs.com/community).
