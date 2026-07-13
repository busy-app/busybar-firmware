# Quick Start {#quick-start}

This page covers the common flows for building, flashing and working with the firmware in an IDE.
For an in-depth reference of the build system and its targets, see @ref build-system.

# Cloning

Ensure there is enough free disk space and clone the source code:

```shell
git clone --recursive https://github.com/flipperdevices/bsb-firmware.git
```

# VS Code integration

Run the following to generate the project configuration in the `.vscode` folder:

```shell
./fbt vscode_dist
```

Then open the workspace file (`.vscode/fbt.code-workspace`) in VS Code (File > Open Workspace from File...) and pick a build task from the `Ctrl+Shift+B` menu. See @ref build-system for additional options, such as selecting a language server.

# Building

Control which firmware gets built by passing `TARGET_HW` to the fbt call:

```shell
# Replace XX with 22 for Main firmware (U5), 65 for Wireless firmware (SI917)
./fbt TARGET_HW=XX
```

The `TARGET_HW` variable (and other commandline variables) can be stored in `fbt_options_local.py` to avoid passing it each time. The file must be in `fbt_layers/fbtng/` directory and does not exist by default. Example content:

```python
TARGET_HW = 22
```

# Flashing

## Using USB

With the device connected via USB and its virtual ethernet interface initialised, the firmware can be flashed with:

```shell
./fbt flash_usb
```

For the available presets and component flags, see @ref flashing-usb.

## Using an in-circuit debugger (Main firmware only)

Connect an ST-Link or a CMSIS-DAP compatible debugger to its respective pins on the BSB debug board and run:

```shell
./fbt flash
```

## Using a serial bootloader (Wireless firmware only)

The following steps need to be done only once:

1. Connect a USB to UART adapter to the respective pins on the BSB debug board,
2. Add the following line to `fbt_layers/fbtng/fbt_options_local.py`: `SI917_PORT="/dev/your/serial/port"` (replace it with the actual device path).

The following steps need to be done each time the firmware needs to be flashed:

1. Run `./fbt flash`, ensure that "Waiting for target" message is showing,
2. Press and hold the `917_RST` button, then press and hold the `917_BOOT` button,
3. Release the `917_RST` button whilst still holding the `917_BOOT` button,
4. Once the process has been started, release the `917_BOOT` button as well,
5. Wait until the "Firmware has been flashed" message shows and briefly press the `917_RST` button again.

## Resource provisioning

Resource files are required for correct firmware operation.

To build and upload the resources, run

```shell
./fbt resources_upload
```

while the device is connected via USB and its virtual ethernet interface is initialised.
