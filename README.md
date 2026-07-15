<picture>
    <source media="(prefers-color-scheme: dark)" srcset="/.github/assets/dark_theme_banner.png">
    <source media="(prefers-color-scheme: light)" srcset="/.github/assets/light_theme_banner.png">
    <img
        alt="Banner with the '▶ BUSY Bar' logo on the left, the text 'Official Firmware Repository' beneath it, and a BUSY Bar device on the right."
        src="/.github/assets/light_theme_banner.png">
</picture>

# BUSY Bar Firmware

## Cloning

Ensure there is enough free disk space and clone the source code:

```shell
git clone --recursive https://github.com/busy-app/busybar-firmware.git
```

## VS Code integration

Run the following to generate the project configuration in the `.vscode` folder:

```shell
./fbt vscode_dist
```

Then open the workspace file (`.vscode/fbt.code-workspace`) in VS Code (File > Open Workspace from File...) and pick a build task from the `Ctrl+Shift+B` menu. See the [Build System documentation](documentation/Build%20System.dox.md) for additional options, such as selecting a language server.

## Building

To build the firmware, run:

```shell
./fbt
```

The build output is placed in the `dist/` folder. See the [Build System documentation](documentation/Build%20System.dox.md) for hardware target selection and other build options.

## Flashing

### Using USB

With the device connected via USB and its virtual ethernet interface initialised, the firmware can be flashed with:

```shell
./fbt flash_usb --signed
```

### Using an in-circuit debugger (Main firmware only)

The SWD interface is not accessible on an assembled device — it has to be partially disassembled and the BSB debug board attached. Connect an ST-Link or a CMSIS-DAP compatible debugger to the SWD pins on the debug board and run:

```shell
./fbt flash
```

### Resource provisioning

Resource files are required for correct firmware operation. They are included by default when flashing with `./fbt flash_usb`, so this step is only needed when the firmware was flashed separately (e.g. via a debugger).

To build and upload the resources, run

```shell
./fbt resources_upload
```

while the device is connected via USB and its virtual ethernet interface is initialised.

## Project structure

- `applications`        - Applications and services used in firmware
- `assets`              - Assets used by applications and services
- `documentation`       - Documentation generation system configs and input files
- `fbt_layers`          - Build system layers
- `lib`                 - Custom and third-party libraries, drivers and tools
- `site_scons`          - Build system configuration and modules
- `scripts`             - Supplementary scripts and various python libraries
- `targets`             - Firmware targets: platform specific code

Also, see `ReadMe.md` files inside those directories for further details.

## Documentation

The developer documentation is authored as Doxygen sources in the [documentation](documentation/) folder. Render and view it by running `./fbt doxy`.

### Documentation sources

The sources are `.dox.md` files, which are best read in the rendered Doxygen output. They can also be browsed directly:

- [Quick Start](documentation/Quick%20Start.dox.md)
- [Concepts](documentation/Concepts.dox.md)
- [Hardware](documentation/Hardware.dox.md)
- [Firmware](documentation/Firmware.dox.md)
- [Build System](documentation/Build%20System.dox.md)
- [Contributing](documentation/Contributing.dox.md)
