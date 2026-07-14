# Quick Start {#quick-start}

This page covers the common flows for building, flashing and working with the firmware in an IDE.
For an in-depth reference of the build system and its targets, see @ref build-system.

# Cloning

Ensure there is enough free disk space and clone the source code:

```shell
git clone --recursive https://github.com/busy-app/busybar-firmware.git
```

# VS Code integration

Run the following to generate the project configuration in the `.vscode` folder:

```shell
./fbt vscode_dist
```

Then open the workspace file (`.vscode/fbt.code-workspace`) in VS Code (File > Open Workspace from File...) and pick a build task from the `Ctrl+Shift+B` menu. See @ref build-system for additional options, such as selecting a language server.

# Building

To build the firmware, run:

```shell
./fbt
```

The build output is placed in the `dist/` folder. See @ref build-system for hardware target selection and other build options.

# Flashing

## Using USB

With the device connected via USB and its virtual ethernet interface initialised, the firmware can be flashed with:

```shell
./fbt flash_usb
```

For the available presets and component flags, see @ref flashing-usb.

## Using an in-circuit debugger (Main firmware only)

The SWD interface is not accessible on an assembled device — it has to be partially disassembled and the BSB debug board attached. Connect an ST-Link or a CMSIS-DAP compatible debugger to the SWD pins on the debug board and run:

```shell
./fbt flash
```

## Resource provisioning

Resource files are required for correct firmware operation. They are included by default when flashing with `./fbt flash_usb`, so this step is only needed when the firmware was flashed separately (e.g. via a debugger).

To build and upload the resources, run

```shell
./fbt resources_upload
```

while the device is connected via USB and its virtual ethernet interface is initialised.
