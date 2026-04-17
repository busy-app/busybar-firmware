# Wi-SUN - SoC Border Router In-Meter Gateway

- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Send Sensor Data to a CoAP Collector](#send-sensor-data-to-a-coap-collector)
- [Troubleshooting](#troubleshooting)
- [Resources](#resources)
- [Report Bugs & Get Support](#report-bugs--get-support)

## Introduction

The Wi-SUN Border Router In-Meter Gateway sample application combines the functionality of the Border Router Empty and CoAP Meter applications.
This example showcases how to use the Constrained Application Protocol (CoAP) to implement a metering application on top of a border router, commonly known as an "In-Meter Gateway." The In-Meter Gateway sends sensor measurements to a CoAP Collector device in the same Wi-SUN network and also acts as a border router.

> We will use a CoAP Collector in the same Wi-SUN network just to simplify the demo.

The SoC term stands for "System on Chip", meaning that this is a standalone application running on the EFR32 without any external MCU required.
It provides an easy and quick way to evaluate the Silicon Labs Wi-SUN stack solution without deploying an expensive and cumbersome production-grade Wi-SUN Border Router.

## Getting Started

To get started with Wi-SUN and Simplicity Studio, see [Getting Started with Wi-SUN Application Development](https://docs.silabs.com/wisun/latest/wisun-getting-started-development).

To get started with the example, follow the steps below:

- Create and build the "Wi-SUN Border Router In-Meter Gateway" project.
- Flash the In-Meter Gateway project to a device.
- Create and build the CoAP Collector project.
- Flash the CoAP Collector project to a second device.
- Using Simplicity Studio, open the console on the Collector device.
- Wait for the CoAP Collector to join the Wi-SUN Border Router In-Meter Gateway network.

## Send Sensor Data to a CoAP Collector

The two Wi-SUN devices (In-Meter Gateway, Collector) are now part of the same Wi-SUN network. See the *Wi-SUN - SoC Collector* readme to configure the Collector.

The connection between an In-Meter Gateway and the CoAP Collector starts with a registration request from the CoAP Collector.

    {
      "token_len": 0,
      "coap_status": 0,
      "msg_code": 1,
      "msg_type": 0,
      "content_format": 0,
      "msg_id": 7,
      "payload_len": 4,
      "uri_path_len": 10,
      "token": "n/a",
      "uri_path": "sensor/all",
      "payload": "register"
    }
    [Registration request from fd2a:6e01:9bfc:990c:20d:6fff:fe20:bd45]
    [Building response message (88 bytes)]

After receiving a registration request, the In-Meter Gateway device sends groups of measurement data to the CoAP Collector periodically.
The time between the cycles can be configured via the configuration interface (SL_WISUN_METER_SCHEDULE_TIME).

The CoAP Collector can stop monitoring the In-Meter Gateway with a remove request.

    {
      "token_len": 0,
      "coap_status": 0,
      "msg_code": 1,
      "msg_type": 0,
      "content_format": 0,
      "msg_id": 7,
      "payload_len": 4,
      "uri_path_len": 10,
      "token": "n/a",
      "uri_path": "sensor/all",
      "payload": "remove"
    }
    [Remove request from fd2a:6e01:9bfc:990c:20d:6fff:fe20:bd45]
    [Collector has been removed: fd2a:6e01:9bfc:990c:20d:6fff:fe20:bd45]

The In-Meter Gateway device can respond to async requests.

    {
      "token_len": 0,
      "coap_status": 0,
      "msg_code": 1,
      "msg_type": 0,
      "content_format": 0,
      "msg_id": 7,
      "payload_len": 4,
      "uri_path_len": 10,
      "token": "n/a",
      "uri_path": "sensor/all",
      "payload": "async"
    }
    [Async request from fd2a:6e01:9bfc:990c:20d:6fff:fe20:bd45]
    [Building response message (88 bytes)]

The In-Meter Gateway application includes sensor/all (mentioned above), sensor/temperature, sensor/humidity, sensor/light and gpio/led, as additional registered resources.
Resource Handler service is not enabled by default in order to reduce power consumption. Enabling Resource Handler service disables the power optimization functionality. If Resource Handler service is enabled, information about resources is available using any third-party CoAP client application, like libcoap-client.

## Troubleshooting

Before programming the radio board mounted on the WSTK, ensure the power supply switch is in the AEM position (right side), as shown.

![Radio Board Power Supply Switch](readme_img0.png)

## Resources

- [Wi-SUN Stack API documentation](https://docs.silabs.com/wisun/latest)

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com/s/topic/0TO1M000000qHc6WAE/wisun).
