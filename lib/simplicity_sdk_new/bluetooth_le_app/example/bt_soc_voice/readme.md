# SoC - Voice

This is a Voice over Bluetooth Low Energy example. It demonstrates how to send voice data over GATT. The example can be used with any board that supports bluetooth and has an on-board microphone (listed below).

Boards which have an on-board microphone and capable of transmitting voice data:
  * xG24 Dev Kit
  * xG26 Dev Kit
  * xG27 Dev Kit
  * Thunderboard EFR32BG22

> Note: This example does not include Device Firmware Update (DFU) functionality by default. For details see the [Device Firmware Update](#device-firmware-update) section.

## Getting started

To get started with Silicon Labs Bluetooth and Simplicity Studio, see [QSG169: Bluetooth SDK v3.x Quick Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf).

To run the example you need two boards, one for running the SoC Voice example, and another which is able to run an NCP application. [AN1259: Using the v3.x Silicon Labs Bluetooth Stack in Network CoProcessor Mode](https://www.silabs.com/documents/public/application-notes/an1259-bt-ncp-mode-sdk-v3x.pdf) provides a detailed description of how NCP works and how to configure it.

One part of the example, the SoC Voice, runs on the Thunderboard Sense. It provides the Voice-Over-Bluetooth Low Energy GATT service. It uses the microphone of the board to record and transmit voice.

The other part of the project runs on a PC. It connects to a board which is running the **NCP empty** example project. The program on the PC uses the NCP target to scan for the Thunderboard, connect to it, and save the received audio data to the file system of the PC.

## Project Setup

The whole project setup:
![](image/readme_img1.png)

### SoC part
Build and flash the provided code example. The device advertises itself after startup with the name "VoBLE Ex". If you have multiple devices, you need to determine the Bluetooth address of the device. This can be done either with the Simplicity Connect app, or through Simplicity Commander.

The EFR32 on the samples the analog microphone using the ADC with the sampling rate and resolution configured by the GATT client. The sampled data is then run through a digital filter (if the filter usage is enabled) and coded using ADPCM codec before being sent via the Bluetooth link to the GATT client using notifications.

![](image/readme_img2.png)


## SoC project structure
The example project contains the GATT database with the necessary Voice-over-Bluetooth Low Energy Service. GATT definitions can be extended using the GATT Configurator, which can be found under Advanced Configurators in the Software Components tab of the Project Configurator. To open the Project Configurator open the .slcp file of the project.

![](image/readme_img5.png)

To learn how to use the GATT Configurator, see [UG438: GATT Configurator User’s Guide for Bluetooth SDK v3.x](https://www.silabs.com/documents/public/user-guides/ug438-gatt-configurator-users-guide-sdk-v3x.pdf).

The Bluetooth event handling is implemented in the *app.c* file, in the function sl_bt_on_event. This handles the advertising, accepts the configuration parameters, and sends out the data.
The button handling logic is also implemented in this file.
The handling of the microphone, the encoding and filtering can be found in their respective files:
* adpcm.c
* filter.c
* voice.c

## Device Firmware Update

This example project does not include Device Firmware Update (DFU) functionality by default, but it can be added easily.
SoC applications can use one of Silicon Labs' Over-the-Air (OTA) DFU implementations. The table below summarizes the options:

|                           | In-place OTA DFU                 | Application OTA DFU                 |
|---------------------------|----------------------------------|-------------------------------------|
| **Component to add**      | In-place OTA DFU                 | Application OTA DFU                 |
| **Compatible bootloader** | Bluetooth Apploader OTA DFU      | Bootloader - SoC Internal Storage (Series 2) <br> Bootloader - SoC Storage (Series 3) |
| **Reference solution**    | Bluetooth - SoC In-Place OTA DFU | Bluetooth - SoC Application OTA DFU |
| **Supported devices**     | Supports Series 2 devices only and requires a smaller flash size | Supports Series 2 and Series 3 devices with enough flash to store firmware images in 2 instances |

To add DFU to an existing project:
- Add the appropriate DFU component to your project using Simplicity Studio’s Software Component browser.
- Add a post-build step to generate the GBL (Gecko Bootloader) file using Simplicity Studio’s Post Build Editor.
- Rebuild the project.
- Flash a compatible bootloader to the device.

For more information on bootloaders, see [UG103.6: Bootloader Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-06-fundamentals-bootloading.pdf) and [UG489: Silicon Labs Gecko Bootloader User's Guide for GSDK 4.0 and Higher](https://www.silabs.com/documents/public/user-guides/ug489-gecko-bootloader-user-guide-gsdk-4.pdf).

## Troubleshooting

### Programming the Radio Board

Before programming the radio board mounted on the mainboard, make sure the power supply switch is in the AEM position (right side) as shown below.

![Radio board power supply switch](image/readme_img0.png)


## Resources

[Bluetooth Documentation](https://docs.silabs.com/bluetooth/latest/)

[UG103.14: Bluetooth LE Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-14-fundamentals-ble.pdf)

[QSG169: Bluetooth SDK v3.x Quick Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf)

[UG434: Silicon Labs Bluetooth ® C Application Developer's Guide for SDK v3.x](https://www.silabs.com/documents/public/user-guides/ug434-bluetooth-c-soc-dev-guide-sdk-v3x.pdf)

[Bluetooth Training](https://www.silabs.com/support/training/bluetooth)

## Report Bugs & Get Support

You are always encouraged and welcome to report any issues you found to us via [Silicon Labs Community](https://www.silabs.com/community).