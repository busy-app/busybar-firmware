# NCP - CS

This is a Network Co-Processor (NCP) based target application. It serves as a target application for Channel Sounding (CS) applications. The corresponding host sample application can be found in the following folder: app/bluetooth/example_host/. This application is suitable for the bt_cs_host host application. The NCP - CS integrates the initiator and reflector components, as well as a full-featured NCP. The application can be used as an NCP with the CS component.

The distance estimation is performed at the NCP side, and only the measurement results are sent to the host using special CS-specific BGAPI commands. In this mode, the host and the NCP can also act as a reflector.

If the configuration is changed to create more than 1 initiator instances, SL_BT_CONFIG_BUFFER_SIZE shall be increased.

> Note: This example does not include Device Firmware Update (DFU) functionality by default. For details see the [Device Firmware Update](#device-firmware-update) section.

## Getting Started with NCP

The NCP Host and Target communicate via a serial interface (UART), which can be tunneled either via USB or via Ethernet if you use a development kit. The communication between the NCP Host and Target is defined in the Silicon Labs proprietary protocol called BGAPI. BGLib is the C reference implementation of the BGAPI protocol, which is to be used on the NCP Host side.

[AN1259: Using the v3.x Silicon Labs Bluetooth Stack in Network Co-Processor Mode](https://www.silabs.com/documents/public/application-notes/an1259-bt-ncp-mode-sdk-v3x.pdf) provides a detailed description how NCP works and how to configure it for your custom hardware.

## Interface

The NCP is extended with application specific messages that we refer to as ACP (Application Co-Processor) messages.
The following ACP commands are sent from the host to the target device:
* Create initiator instance
* Delete initiator instance
* Create reflector instance
* Delete reflector instance
* Configure antenna

The following ACP events are sent from the target device to the host:
* CS results
* CS intermediate results, used in stationary object tracking mode
* CS extended results, used by the tooling
* Error events

All interface related data types are defined in cs_acp.h.

## Usage

Build and flash the application. Use the "bt_cs_host" host sample application to connect to it. If the host was started with any initiator instance, it will scan for a reflectors advertising with the "CS RFLCT" device name. If started with reflector instances, it will start advertising. When an initiator instance finds a reflector, it will create a connection between them and will start the distance measurement process. The initiator estimates the distance, and displays them in the command line terminal.

## Multiconnection
- Default setup is optimized for 1-1 connection, multiconnection setup requires modification of the timing parameters to operate as expected. Timing can be adjusted by the procedure_interval and connection_interval parameters.
- Use the following calculation for 1-N connection: procedure_time_1_N[ms] = connection_interval[ms] * procedure_interval * N
- Note that setting CS_INITIATOR_DEFAULT_MIN/MAX_CONNECTION_INTERVAL and CS_INITIATOR_DEFAULT_MIN/MAX_PROCEDURE_INTERVAL will only take effect if CS_INITIATOR_DEFAULT_PROCEDURE_SCHEDULING is set to CS_PROCEDURE_SCHEDULING_CUSTOM. Otherwise these parameters are managed by the application.
- If getting frequent measurement results is not priority, it's safe to use procedure_interval = 120 and connection_interval = 24 even with the maximum number of connections (4).
- If more than 1 initiator instances are created increase SL_BT_CONFIG_BUFFER_SIZE. With maximum number of instances (4) it's safe to use 22000.

## Resource optimization
- Flash usage can be reduced by
  - removing "Bluetooth controller anchor selection" component if no multiple reflector connection is required,
  - turning off some of the "Supported features" in "CS Ranging Service Server" component. Note that "Real-Time Ranging Data" feature is used by default on the Initiator,
  - turning off "Logging" in "CS Reflector" and "CS Initiator" component or completely in "Application"- "Utility" -"Log" component,
- RAM usage can be reduced by
  - decreasing the "Max number of connections reserved for user" in Bluetooth "Connection" component configuration to the required amount.
  - decreasing the "Maximum initiator connections" in "CS Initiator" component configuration to the required amount,
  - decreasing "Maximum ranging data size" in "CS Initiator" component configuration. Note that "Maximum ranging data size" should be enough to store Ranging Data in format defined in RAS specification.
  - decreasing "Procedure maximum length" or "Procedure per connection" "CS Ranging Service Server" component configuration. Note that reducing "Procedure maximum length" also affects the maximum number of procedures that can be stored in the buffer in case of RAS On-Demand mode,
  - reducing "Buffer memory size for Bluetooth stack" in "Bluetooth Core" component configuration if the "Maximum initiator connections" is changed to create less than 4 initiator instances.

### Calculating the size of "Procedure maximum length" and "Maximum ranging data size"
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

This example project does not include Device Firmware Update (DFU) functionality by default.
To add DFU to an existing project:
- Add the `Bootloader Interface` component to your project using Simplicity Studio’s Software Component browser.
- Add a post-build step to generate the GBL (Gecko Bootloader) file using Simplicity Studio’s Post Build Editor.
- Rebuild the project.
- Flash the `Bootloader - NCP BGAPI UART DFU` bootloader to the device.

See the `Bluetooth - NCP DFU` example solution for reference.

For more information on bootloaders, see [UG103.6: Bootloader Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-06-fundamentals-bootloading.pdf) and [UG489: Silicon Labs Gecko Bootloader User's Guide for GSDK 4.0 and Higher](https://www.silabs.com/documents/public/user-guides/ug489-gecko-bootloader-user-guide-gsdk-4.pdf).

## Troubleshooting

### Programming the Radio Board

In case of a WSTK & Radio Board setup, before programming the radio board mounted on the mainboard, make sure the power supply switch is in the AEM position (right side) as shown below.

![](./image/readme_img0.png)

## Resources

[Bluetooth Documentation](https://docs.silabs.com/bluetooth/latest/)

[UG103.14: Bluetooth LE Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-14-fundamentals-ble.pdf)

[QSG169: Bluetooth SDK v3.x Quick-Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf)

[AN1259: Using the v3.x Silicon Labs Bluetooth Stack in Network Co-Processor Mode](https://www.silabs.com/documents/public/application-notes/an1259-bt-ncp-mode-sdk-v3x.pdf)

[Bluetooth Training](https://www.silabs.com/support/training/bluetooth)

[Ranging Service](https://www.bluetooth.com/specifications/specs/ranging-service-1-0/)

## Report Bugs & Get Support

You are always encouraged and welcome to report any issues you found to us via [Silicon Labs Community](https://www.silabs.com/community).