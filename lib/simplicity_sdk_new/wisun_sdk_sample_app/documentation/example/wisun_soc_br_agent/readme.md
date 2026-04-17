# Wi-SUN - SoC Border Router Agent

- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Features](#features)
- [Architecture Overview](#architecture-overview)
  - [Data Flow](#data-flow)
- [Message Protocol](#message-protocol)
  - [Message Frame Format](#message-frame-format)
  - [Request / Response Codes](#request--response-codes)
  - [Configuration Payload Structure](#configuration-payload-structure)
- [Troubleshooting](#troubleshooting)
- [Resources](#resources)
- [Report Bugs & Get Support](#report-bugs--get-support)

## Introduction

The Wi-SUN SoC Border Router Agent sample application extends the Wi-SUN SoC Border Router by adding an Agent Service capable of:

- Exposing the current network topology and the active Border Router configuration parameters to a remote host agent via Wi-Fi.
- Stopping and restarting the Border Router operation remotely.

A limited CLI (Command-Line Interface) is exposed to facilitate the Wi-SUN / WiFi configuration.

The SoC term stands for "System on Chip", meaning that this is a standalone application running on the EFR32 without any external MCU required.
It provides an easy and quick way to evaluate the Silicon Labs Wi-SUN stack solution without deploying an expensive and cumbersome production-grade Wi-SUN Border Router.

## Getting Started

To get started with Wi-SUN and Simplicity Studio, see [Getting Started with Wi-SUN Application Development](https://docs.silabs.com/wisun/latest/wisun-getting-started-development).

This sample application is the counterpart to the external Linux "Wi-SUN SoC Border Router Agent" service (remote host agent) whose responsibilities include:

- Maintaining a D-Bus interface for GUI, see [Wi-SUN Border Router GUI](https://docs.silabs.com/wisun/latest/wisun-border-router-gui/).
- Translating device topology & configuration into D-Bus properties.
- Relaying control operations (Restart BR / Stop BR / Set config) originating from UI or scripts to the SoC via the TCP protocol described above.

For prerequisites and complete setup see, [Wi-SUN Border Router Bridge Agent](https://github.com/SiliconLabs/wisun-br-gui/tree/main/wisun-br-bridge-agent).

Wi-Fi Backhaul Connectivity is based on the SiWx91x™ chipset and the WiSeConnect™ SDK v3.x.
Follow the [Getting Started with WiSeConnect™ SDK v3.x and EFR32™ Host in NCP Mode](https://docs.silabs.com/wiseconnect/3.5.1/wiseconnect-getting-started/getting-started-with-ncp-mode-with-efr32) to configure the SiWN917 as a Network Co-Processor (NCP). This document is limited to additional commands and settings, a more detailed documentation can be found here [SoC Border Router with Wi-Fi Backhaul](https://docs.silabs.com/wisun/latest/wisun-network-configuration/06-wisun-soc-border-router-backhaul).

## Features

- TCP server listening on a configurable port for inbound Agent requests.
- Client-initiated TCP connections to a remote Agent host to push:
  - Updated network topology when routing changes occur.
  - Configuration parameters.
- DHCPv6 server integration.
- Wi-Fi adapter is directly used to communicate with the [Wi-SUN Border Router Bridge Agent](https://github.com/SiliconLabs/wisun-br-gui/tree/main/wisun-br-bridge-agent).
- Wi-Fi support for backhaul connectivity.
- Thread-safe remote address runtime reconfiguration.

## Architecture Overview

The sample application is composed of two main logical parts:

| Component | Responsibility |
|-----------|----------------|
| Core application task | Initializes Wi-SUN Border Router stack, Wi-Fi for backhaul connectivity, DHCPv6 server. |
| Agent Service | Sending Border Router specific metrics to the remote host via Wi-Fi backhaul, implement data model for sending them to the remote Agent running on Linux, serialize data and send over lwIP/TCP, gathers network topology / config, applies settings. |

### Data Flow

The application follows an event-driven architecture with the following data flow patterns:

- Wi-SUN stack events (e.g., routing table changes) trigger:
  - Network topology retrieval
  - TCP client connection to remote host
  - Automatic transmission of updated routing information via Wi-Fi

- Remote host connections are handled through:
  - Accepting connections and reading frames
  - Interpreting message headers and payloads
  - Executing requested actions, runtime configuration updates or returning data

- Border Router startup automatically sends current configuration parameters and Wi-SUN FAN global address to the remote host agent as part of the initialization sequence.

### Command Line Interface (CLI) example
Set the remote Linux host address that executes the **Wi-SUN Border Router Bridge Agent** service.

```bash
> wisun set_br_bridge_agent_addr 2001:db8::dda5:4582:bc9:2287
[Remote address is set to: 2001:db8::dda5:4582:bc9:2287]

> wisun get_br_bridge_agent_addr
[2001:DB8::DDA5:4582:BC9:2287]
```

## Troubleshooting

Before programming the radio board mounted on the WSTK, ensure the power supply switch is in the AEM position (right side), as shown.

![Radio Board Power Supply Switch](readme_img0.png)

## Resources

- [Wi-SUN Getting Started Guide](https://docs.silabs.com/wisun/latest/wisun-getting-started-development)
- [Wi-SUN Stack API documentation](https://docs.silabs.com/wisun/latest)
- [Wi-SUN Border Router GUI](https://docs.silabs.com/wisun/latest/wisun-border-router-gui/)
- [Getting Started with WiSeConnect™ SDK v3.x and EFR32™ Host in NCP Mode](https://docs.silabs.com/wiseconnect/3.5.1/wiseconnect-getting-started/getting-started-with-ncp-mode-with-efr32)
- [Wi-SUN Border Router Bridge Agent](https://github.com/SiliconLabs/wisun-br-gui/tree/main/wisun-br-bridge-agent).
- [SoC Border Router with Wi-Fi Backhaul](https://docs.silabs.com/wisun/latest/wisun-network-configuration/06-wisun-soc-border-router-backhaul)

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com/s/topic/0TO1M000000qHc6WAE/wisun).
