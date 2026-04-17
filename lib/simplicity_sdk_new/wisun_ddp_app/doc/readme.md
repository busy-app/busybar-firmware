# Wi-SUN - SoC DDP RAM Application

The Wi-SUN DDP RAM application provides support for run-time provisioning of device-specific Wi-SUN data, such as private keys and certificates. It implements the Dynamic Data Provisioning (DDP) framework, accepting DDP commands from a host application.

## Example DDP Host Application

An example DDP host application _provision.py_ written in Python can be found in [GitHub](https://github.com/SiliconLabs/wisun_applications). It creates a Public Key Infrastructure with a local Certificate Authority (CA), and demonstrates how DDP can be utilized to generate and provision a signed Wi-SUN device certificate from a keypair created on the device.

## Getting Started

To get started with Dynamic Data Provisioning, see [docs.silabs.com](https://docs.silabs.com/wisun/latest/wisun-start/).