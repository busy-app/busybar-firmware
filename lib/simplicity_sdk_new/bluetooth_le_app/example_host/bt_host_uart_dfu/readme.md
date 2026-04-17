# Host UART DFU

## Features

This example performs Device Firmware Upgrade (DFU) through UART.

## Requirements

- BGAPI UART DFU Bootloader on your target device
- NCP application
- GBL file

## Usage

- Generate and build the host application
- Flash the Bluetooth - NCP DFU workspace example on your target device, or separately
    - the BGAPI UART DFU Bootloader, and
    - the Bluetooth - NCP example

## Example invocation

bt_host_uart_dfu -u <serial_port>  <gbl_file_path>

Replace <serial_port> with your actual serial device, and <gbl_file_path> with the
path to your GBL file. For the complete list of command line option, please use the `-h` option.

## Output

Upon successful invocation, the example prints this output:

```
./bt_host_uart_dfu -u /dev/tty.usbmodem0004402253201 -b 115200 ./full.gbl
[I] NCP host initialised.
[I] Reset NCP target in bootloader mode...
[I] DFU booted: v0x03000000
[I] Pressing Crtl+C aborts the update process.
[I] WARNING! If the update process is aborted, the device will stay in bootloader mode.

259648/259648 (100%)
[I] DFU finished successfully. Resetting the device.
```

## Resources

https://docs.silabs.com/bluetooth/latest/using-gecko-bootloader-with-bluetooth-apps/02-bgapi-uart-device-firmware-upgrade-dfu