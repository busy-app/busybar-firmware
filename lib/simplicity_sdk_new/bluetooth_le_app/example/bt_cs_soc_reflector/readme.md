# SoC - CS Reflector

The Bluetooth SoC-CS Reflector is a project that can be used to test the Channel Sounding (CS) feature. In the provided CS setup, the reflector measures the phase (in PBR mode) or flying time (RTT) of the received signal and sends this information back to the initiator using the Ranging Service (RAS). The reflector has the RAS Server included which can operate in two different modes: real-time and on-demand. The setting of this mode is available in the initiator config. The initiator will estimate the distance, the estimated values are not available on the reflector side.

> Note: this example does not include Device Firmware Update (DFU) functionality by default. For details see the [Device Firmware Update](#device-firmware-update) section.

## Getting Started
After startup it will advertise the name "CS RFLCT", and will provide the Ranging Service (RAS).
Measurement data is collected on the initiator side.
The Reflector can connect to multiple initiators (maximum 4).
Reflector specific default values can be found and modified in the component configuration file (config/cs_reflector_config.h).
CS mode and object tracking mode is configured on the initiator side.

## Usage
- Generate the project
- Configure values if needed
- Build and flash the sample application

## Resource optimization
- Flash usage can be reduced by
  - turning off some of the "Supported features" in "CS Ranging Service Server" component. Note that "Real-Time Ranging Data" feature is used by default on the Initiator,
  - turning off "Logging" in "CS Reflector" component or completely in "Application"- "Utility" -"Log" component,
- RAM usage can be reduced by
  - decreasing the "Max number of connections reserved for user" in Bluetooth "Connection" component configuration to the required amount.
  - decreasing "Procedure maximum length" or "Procedure per connection" "CS Ranging Service Server" component configuration. Note that reducing "Procedure maximum length" also affects the maximum number of procedures that can be stored in the buffer in case of RAS On-Demand mode.

### Calculating the size of "Procedure maximum length"
The optimal value of "Procedure maximum length" is dependent on several configuration values, and can be calculated by the following equation:

proc_max_len = 4 + (subevents * 8) + (mode0_steps * mode0_size) + channels * ( ( 1 + ( antenna_paths + 1 ) * 4) + 1 )

where
- subevents value is constant 1 since one subevent per procedure is supported,
- mode0_size is
  - 4 for Reflector and
  - 6 for Initiator,
- mode0_steps value is the configuration "Mode 0 steps",
- channels value means the number of channels from the channel mask that can be derived from the "Channel map preset" settings:
  - "High"   - 72 (default),
  - "Medium" - 37,
  - "Low"    - 20,
  - "Custom" - Number of 1s in channel mask,
- antenna_paths value is controlled by the "Antenna configuration", and limited by number of antennas presented on each board (capabilities). Maximum can be calculated using the product of used Initiator and Reflector antennae. The default maximum value for antenna_paths is 4.

These settings were selected by assuming that the controller creates only one subevent per procedure, and the measuring mode is PBR. In RTT mode there are far less data is created.

If you use submode, you should add the following to the sum:

(1 + mode1_size) * channels / main_mode_steps

where
- mode1_size is 6
- main_mode_steps is the value of min_main_mode_steps ranging from to 2. This can be changed in cs_initiator_client.h.

The default is calculated by using the constants and settings above using the worst case scenario, which gives 1866 bytes.
RAM consumption can be reduced by changing the affected settings and reducing "Procedure maximum length" accordingly.

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

## Resources

[Bluetooth Documentation](https://docs.silabs.com/bluetooth/latest/)

[UG103.14: Bluetooth LE Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-14-fundamentals-ble.pdf)

[QSG169: Bluetooth SDK v3.x Quick Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf)

[UG434: Silicon Labs Bluetooth ® C Application Developer's Guide for SDK v3.x](https://www.silabs.com/documents/public/user-guides/ug434-bluetooth-c-soc-dev-guide-sdk-v3x.pdf)

[Bluetooth Training](https://www.silabs.com/support/training/bluetooth)

## Report Bugs & Get Support

You are always encouraged and welcome to report any issues you found to us via [Silicon Labs Community](https://www.silabs.com/community).