# OTA DFU

## Features

This application performs Over The Air (OTA) Device Firmware Upgrade (DFU).

## Requirements

- Target SoC application with a bootloader that supports OTA DFU, for example the internal storage bootloader.
- NCP target
- GBL file

## Usage

- Prepare your SoC application by adding the `Application OTA DFU` component, or
- Flash build and flash the Bluetooth - SoC app OTA DFU or SoC in place OTA DFU workspace examples
- Prepare the GBL file. When the component above added to the project, the GBL file
    is created automatically by building your SoC application
- Connect your NCP to the host, and proceed the the example invocation

## Example invocation

```bash
bt_host_ota_dfu -g <gbl_file> -a <remote_address>
```

Where <gbl_file> is your generated GBL, and the <remote_address> is the BT address of
the device, where your bootloader is situated.

For the complete list of available command line argument, use the `-h` command line
option.

## Resources

[Using the Gecko Bootloader](https://docs.silabs.com/bluetooth/latest/using-gecko-bootloader-with-bluetooth-apps/03-bluetooth-ota-upgrade)

