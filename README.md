# Busy Status Bar Firmware

## Cloning

Make sure you have enough space and clone the source code:

```shell
git clone --recursive https://github.com/flipperdevices/bsb-firmware.git
```

## Building

Control which firmware gets built by passing `TARGET_HW` to the fbt call:

```shell
# Replace XX with 20 for Main firmware (U5), 64 for Wireless firmware (SI917)
./fbt TARGET_HW=XX
```

You can store the `TARGET_HW` (and other commandline variables) variable in `fbt_options_local.py` to avoid passing it each time. The file must be in `fbt_layers/fbtng/` directory and does not exist by default. Example content:

```python
TARGET_HW = 20
```

## Flashing

### Using an in-circuit debugger (Main firmware only)

Connect an ST-Link or a CMSIS-DAP compatible debugger to its respective pins on the BSB debug board and run:

```shell
./fbt flash
```

### Using a serial bootloader (Wireless firmware only)

The following steps need to be done only once:

1. Connect a USB to UART adapter to the respective pins on the BSB debug board,
2. Add the following line to `fbt_layers/fbtng/fbt_options_local.py`: `SI917_PORT="/dev/your/serial/port"` (replace it with the actual device path).

The following steps need to be done each time the firmware needs to be flashed:

1. Run `./fbt flash`, ensure that "Waiting for target" message is showing,
2. Press and hold the `917_RST` button, then press and hold the `917_BOOT` button,
3. Release the `917_RST` button whilst still holding the `917_BOOT` button,
4. Once the process has been started, release the `917_BOOT` button as well,
5. Wait until the "Firmware has been flashed" message shows and briefly press the `917_RST` button again.

## Project structure

- `applications`        - Applications and services used in firmware
- `assets`              - Assets used by applications and services
- `documentation`       - Documentation generation system configs and input files
- `fbt_layers`          - Build system layers
- `lib`                 - Our and 3rd party libraries, drivers, tools and etc...
- `site_scons`          - Build system configuration and modules
- `scripts`             - Supplementary scripts and various python libraries
- `targets`             - Firmware targets: platform specific code

Also, see `ReadMe.md` files inside those directories for further details.
