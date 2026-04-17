# Dynamic Hardware Configuration

The *Dynamic Hardware Configuration* feature allows users to program certain configuration values into the device through either **NCP** or **RCP/HCI**.

The following calibration values can be set:

 - RSSI Offset
 - PA Curve
 - PA Mode
 - CTUNE calibration

These examples act as a regular NCP/RCP with the *Dynamic Hardware Configuration* feature added on top.

Use the provided host application (`host/dhc.py`) to communicate with the device under calibration.

![Setup](image/readme_img1.png)

> Note: This example does not include Device Firmware Update (DFU) functionality by default. For details see the [Device Firmware Update](#device-firmware-update) section.

## Quick start guide

 - For the host app you'll need `python3` and the `pybgapi` python3 package installed
 - Flash the example on your device
 - Launch the host app. E.g.: `python3 dhc.py -p <COM# on Windows, /dev/tty# on POSIX> --ncp -c configuration_sample.json`
 - This will apply the configuration values of `configuration_sample.json` to the selected device and verify that they have been applied correctly

## RCP Usage
 - The EFR device needs to be attached using the `btattach` command
 - Check the available hci devices using the `hcitool dev` command
 - Use the HCI device name. E.g.: `python3 dhc.py -p hci0 --rcp`
 - Note that RCP only works on Linux

## Providing custom XAPI path to the host application

The host application - when launched from it's default location in the SDK will use the XAPI file supplied with the SDK. If you wish to run this script without the SDK - you can specify the location of the XAPI file by using the `-x` or `--xapi` command line option:

`python3 dhc.py -p /dev/tty# --ncp -r --xapi /Users/gfreeman/sisdk/protocol/bluetooth/api/sl_bt.xapi -h`

## Reading configuration data from the device

The host application is also capable of reading the device's existing configuration and saving it into a JSON file.
Use the `-r` flag:

`python3 dhc.py -p /dev/tty# --ncp -r`

## Verifying configuration data

If you wish to verify if a device has the same configuration as a certain JSON file use the `-v` flag:

`python3 dhc.py -p /dev/tty# --ncp -c configuration_sample.json -v`

## Writing configuration data to the device

The configuration data is contained in a JSON file which is given as in input for the host app. The host app will parse the JSON file and apply any valid and present configuration value. It is possible and permitted to omit configuration categories from the file - only the values present in the file will be changed. For an example JSON file, please look at:

`host/configuration_sample.json`

It recognizes the following top level values:
- `version`
- `rssi_offset`
- `pa_curves`
- `pa_mode`
- `ctune`

### 'version'

This attribute is the version of the configuration layout - the host app uses this to verify if the device has the same configuration layout version. If the version mismatches the calibration won't be performed.

### 'rssi_offset'

Sets the RAIL RSSI offset in dB.
- Please refer to \ref sl_rail_get_rssi_offset and \ref sl_rail_set_rssi_offset

### 'pa_curves'

Sets the RAIL PA Curves to the provided curve structure.
- Please refer to \ref sl_rail_nvm_pa_config_t

### 'pa_mode'

Sets the RAIL PA Mode configuration value.
- Please refer to \ref sl_rail_tx_power_mode_t

### 'ctune'

Sets the HFXO CTUNE calibration value.
- Please refer to \ref slx_clock_manager_hfxo_get_ctune and \ref slx_clock_manager_hfxo_set_ctune

## Configuration payload layout

All Dynamic hardware Configuration payloads have the following format:

`{header} {command} {setting} {value}`

`{header}` - always `0xDC` (for Dynamic Configuration)

`{command}` - `0x00`: read, `0x01`: write

`{setting}` - setting to read or write (see dhc_setting_t)

`{values}` - values to write (not needed in case of reading)

Example payload: `0xDC 0x00 0x00`

This payload reads the version of the configuration settings. The version shows the revision of the configuration settings.
It is used to check if the configuration settings are compatible with the current firmware. The version is increased every time the configuration layout is changed.

Example payload: `0xDC 0x01 0x05 0x42 0x00 0x00 0x00`

This payload writes the value 0x42 to the CTUNE setting.

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

Before programming the radio board mounted on the mainboard, make sure the power supply switch is in the AEM position (right side) as shown below.

![Radio board power supply switch](image/readme_img0.png)


## Resources

[Bluetooth Documentation](https://docs.silabs.com/bluetooth/latest/)

[UG103.14: Bluetooth LE Fundamentals](https://www.silabs.com/documents/public/user-guides/ug103-14-fundamentals-ble.pdf)

[QSG169: Bluetooth SDK v3.x Quick-Start Guide](https://www.silabs.com/documents/public/quick-start-guides/qsg169-bluetooth-sdk-v3x-quick-start-guide.pdf)

[AN1259: Using the v3.x Silicon Labs Bluetooth Stack in Network Co-Processor Mode](https://www.silabs.com/documents/public/application-notes/an1259-bt-ncp-mode-sdk-v3x.pdf)

[Bluetooth Training](https://www.silabs.com/support/training/bluetooth)

## Report Bugs & Get Support

You are always encouraged and welcome to report any issues you found to us via [Silicon Labs Community](https://www.silabs.com/community).
