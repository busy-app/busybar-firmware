# Wi-SUN - SoC Empty

## Table of Contents

- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Responding to Wi-SUN Events](#responding-to-wi-sun-events)
- [Implementing Application Logic](#implementing-application-logic)
- [EFR32FG23 Support](#efr32fg23-support)
- [Troubleshooting](#troubleshooting)
- [Resources](#resources)
- [Report Bugs & Get Support](#report-bugs--get-support)

## Introduction

The Wi-SUN Empty sample application is a bare-bones application. This application can be used as a template to develop a Wi-SUN application.

## Getting Started

To get started with Wi-SUN and Simplicity Studio, see [Getting Started with Wi-SUN Application Development](https://docs.silabs.com/wisun/latest/wisun-getting-started-development).

The SoC term stands for "System on Chip", meaning that this is a standalone application running on the EFR32 without any external MCU required.

As the name implies, the example is an empty template that only has the bare minimum to make a working Wi-SUN application. For this purpose, the example includes the Wi-SUN stack component and its dependencies (RTOS, cryptographic library...).This skeleton can be extended with application logic.

The development of a Wi-SUN application consists of two main steps:

- Responding to the events raised by the Wi-SUN stack
- Implementing additional application logic

## Responding to Wi-SUN Events

A Wi-SUN application is event-driven. The Wi-SUN stack generates events when a connection is successful, data has been sent, or an IP packet is received. The application has to handle these events in the *sl_wisun_on_event()* function. The prototype of this function is implemented in *app.c*. To handle more events, the switch-case statement of this function must be created and extended. For the list of Wi-SUN events, see [Wi-SUN events API Reference](https://docs.silabs.com/wisun/latest/wisun-stack-api/sl-wisun-evt).

## Implementing Application Logic

Additional application logic can be implemented in the *app_task()* function. Find the definition of this function in *app.c*. The *app_task()* function is called once when the device is booted and after the Wi-SUN stack initialization.

The first step in most Wi-SUN applications is to call *sl_wisun_join()* to connect the Wi-SUN device to a Wi-SUN border router. The remaining implementation is up to the developer. Refer to the [Wi-SUN API Reference](https://docs.silabs.com/wisun/latest/wisun-stack-api) to check the list of Wi-SUN APIs available to the application.

## EFR32FG23 Support

This sample application provides limited functionality for the EFR32FG23 platform. It is designed to operate as a Limited Function Node (LFN) only. To ensure minimal memory footprint, the application has been optimized for size and RTT buffer sizes are decreased.

## Troubleshooting

Before programming the radio board mounted on the WSTK, ensure the power supply switch is in the AEM position (right side), as shown.

![Radio Board Power Supply Switch](readme_img0.png)

## Resources

- [Wi-SUN Stack API documentation](https://docs.silabs.com/wisun/latest)

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com/s/topic/0TO1M000000qHc6WAE/wisun).
