# Build System {#build-system}

The BSB firmware is built with **FBT** (Flipper Build Tool), a wrapper around the [SCons](https://scons.org/) build system. It is the entry point for all firmware-related commands and is invoked as `./fbt` (`fbt.cmd` on Windows) from the project root. For a condensed getting-started guide, see @ref quick-start.

FBT itself lives in the `fbt_layers/fbtng/` submodule layer and is shared with the upstream project; the BSB-specific target configuration resides under `targets/`.

# Environment

To use `fbt`, only `git` is required on the system.

By default, `fbt` downloads and unpacks a pre-built toolchain and sets up its own environment without contaminating the global `PATH`. The toolchain is unpacked into the `toolchain/` directory in the repository root.

> To use the toolchain's tools outside of `fbt`, an *fbt shell* can be opened with a configured environment:
> - On Windows, run `scripts/toolchain/fbtenv.cmd`.
> - On Linux & macOS, run `source scripts/toolchain/fbtenv.sh` in a new shell.

If the pre-built toolchain is not available for the system, or a custom toolchain is preferred, set `FBT_NOENV=1` and `fbt` will skip environment configuration and expect all tools to be on the `PATH`.

The following environment variables control basic `fbt` behavior:

- `FBT_NO_SYNC=1` — skip the `git submodule update --init` that `fbt` performs on start.
- `FBT_VERBOSE=1` — enable extra debug output from `fbt` and the toolchain scripts.
- `FBT_TOOLCHAIN_PATH` — directory to unpack the toolchain into (default: `toolchain/` in the repo root).

# Invoking FBT

Specifying `TARGET_HW` builds only the requested firmware target:

```shell
./fbt TARGET_HW=22
```

To build the update package, run `./fbt` without `TARGET_HW` (or `./fbt dist`); both the U5 and Si917 firmwares are built and bundled into the `dist/` folder.

The hardware variants can be overridden with `U5_TARGET_HW` and `SIL_TARGET_HW` variables (e.g. `./fbt U5_TARGET_HW=22 SIL_TARGET_HW=65`).

To clean the build products for the specified targets (similar to `make clean`), add the `-c` option.

# Build directories

SCons is invoked inside the `fbtng` layer, so build artifacts are placed under `fbt_layers/fbtng/build/` (for example, `fbt_layers/fbtng/build/f22-firmware-D/`).

The `dist` target publishes the resulting binaries to the `dist/` directory at the project root (for example, `dist/f22-D/`).

For IDE integration, `fbt` also generates a `compile_commands.json` compilation database and links the most recently built firmware variant, updated whenever a firmware build target is run.

# VS Code integration

`fbt` can generate a ready-to-use VS Code configuration. Run:

```shell
./fbt vscode_dist
```

This deploys build tasks, launch and settings files into the `.vscode/` folder. Open the generated workspace file (`.vscode/fbt.code-workspace`) in VS Code — via *File > Open Workspace from File...* — which is a multi-root workspace covering the project root and the build-system layers. Use the `Ctrl+Shift+B` menu for the basic build tasks. Recommended extensions are listed in `.vscode/extensions.json`, and VS Code will prompt to install them on first start.

To use a language server other than the default, pass `LANG_SERVER=`. Supported values are `clangd` (the default) and `cpptools`:

```shell
./fbt vscode_dist LANG_SERVER=cpptools
```

On-device debugging requires a supported SWD probe. `fbt` auto-detects the attached probe; the probe type can be pinned with `DEBUG_INTERFACE=` (`auto`, `stlink`, `cmsis-dap`) and, when several probes are connected, a specific one can be selected with `DEBUG_INTERFACE_SERIAL=`. Blackmagic probes (USB and Wi-Fi) are also supported.

# FBT targets

`fbt` tracks internal dependencies, so only the highest-level target needs to be requested.

## High-level

- *(no target)* — build the firmware. This is the default.
- `dist` — build and publish the firmware to the `dist/` folder.
- `flash` — flash the attached device. On the U5 (Main) target this is done over SWD with a supported debugger (auto-detected; override it with `DEBUG_INTERFACE=` and `DEBUG_INTERFACE_SERIAL=`); on the Si917 (Wireless) target it uses the serial bootloader (configured with `SI917_PORT`).
- `flash_usb` — build, upload and install the firmware to the device over USB. See [Flashing over USB](#flashing-usb) below.
- `resources` — build the U5 resources and their manifest.
- `resources_upload` — build and upload the resources to the U5 device over USB (requires the device's virtual ethernet interface to be initialised).
- `debug` — build and flash the firmware, then attach GDB with the firmware's `.elf` loaded. Requires `TARGET_HW` to select the MCU to attach to.
- `debug_other` — attach GDB without loading a `.elf`, allowing external `.elf` files to be added manually with `add-symbol-file`. Requires `TARGET_HW` to select the MCU to attach to.
- `lint`, `format` — run `clang-format` on the C/C++ sources to check and reformat them according to `.clang-format` (see @ref code-style for the full style guide). Extra arguments can be passed with `ARGS="..."`.
- `doxygen` — generate the documentation. The `doxy` target additionally opens the generated documentation in a web browser.
- `vscode_dist`, `subl_dist` — generate project configuration for VS Code and Sublime Text respectively.

A number of specialized targets are also available, including `sdk_tree`, `update_bundle`, `update_cacert`, `openapi_dist`, `openapi_spec`, `crypto_provision`, `crypto_wipe`, `mqtt_provision`, `mqtt_wipe`, `api_table` and `launch_app`. Run `./fbt -h` for the full list.

# Flashing over USB {#flashing-usb}

The `flash_usb` family of targets builds an update bundle and uploads it over the device's USB virtual ethernet interface (an HTTP POST to `http://10.0.4.20/api/update`), which then installs it. The device must be connected over USB with its virtual ethernet interface initialised.

## Presets

Each preset is a separate target. Only one preset may be built at a time.

| Target                    | U5 resources | Si917 M4 firmware | Si917 NWP radio firmware | Signed |
| :------------------------ | :----------: | :---------------: | :----------------------: | :----: |
| `flash_usb`               |     yes      |        no         |           no             |  yes   |
| `flash_usb_full`          |     yes      |        yes        |           yes            |  yes   |
| `flash_usb_min`           |     no       |        no         |           no             |  yes   |
| `flash_usb_main`          |     no       |        yes        |           no             |  yes   |
| `flash_usb_unsigned`      |     yes      |        no         |           no             |   no   |
| `flash_usb_full_unsigned` |     yes      |        yes        |           yes            |   no   |
| `flash_usb_min_unsigned`  |     no       |        no         |           no             |   no   |
| `flash_usb_main_unsigned` |     no       |        yes        |           no             |   no   |

Production (secured) devices can only be flashed with the signed presets, and the Si917 firmware must be signed to be flashed on them.

## Component flags

The presets define defaults that can be overridden with component flags. The flags are only valid on the `flash_usb_*` targets:

- `--resources` / `--no-resources` — include or exclude the U5 resources.
- `--sil-m4` / `--no-sil-m4` — include or exclude the Si917 M4 firmware.
- `--sil-nwp` / `--no-sil-nwp` — include or exclude the Si917 NWP radio firmware.
- `--signed` / `--unsigned` — mark the bundle as suitable for secured (production/secure-boot) devices. If the Si917 firmware is included in the bundle, sign it.

For example, to flash everything except the resources:

```shell
./fbt flash_usb_full --no-resources
```

# Command-line parameters {#command-line-parameters}

- `--options optionfile.py` (default `fbt_options.py`) — load a file with multiple configuration values.
- `--extra-define=A --extra-define=B=C` — extra global defines passed to the C/C++ compiler; can be specified multiple times.
- `--proxy-env=VAR1,VAR2` — additional environment variables to forward to child SCons processes.

Run `./fbt -h` for the complete list.

# Configuration

Default configuration values are set in `fbt_options.py` (located in `fbt_layers/fbtng/`). Values passed on the command line take precedence over the file.

A `fbt_options_local.py` file (placed in `fbt_layers/fbtng/`, not created by default and git-ignored) is evaluated on top of the defaults, enabling persistent local overrides without modifying the tracked configuration. Available options can be listed with `./fbt -h`.

Notable options:

- `TARGET_HW` — hardware target. `22` for the Main firmware (U5), `65` for the Wireless firmware (SI917). See @ref hardware for details on the hardware targets.
- `U5_TARGET_HW` (default `22`) — U5 hardware variant for the combined dist build (used when `TARGET_HW` is not set).
- `SIL_TARGET_HW` (default `65`) — Si917 hardware variant for the combined dist build.
- `DEBUG` (default `1`) — enable a debug build.
- `COMPACT` (default `0`) — optimize for size.
- `DIST_SUFFIX` — suffix for binaries produced by `dist` targets (default `local`).
- `FIRMWARE_APP_SET` — name of the application set to build, selected from the `FIRMWARE_APPS` map (for example, `unit_tests` or `recovery`).
- `DEBUG_INTERFACE` — SWD probe type (`auto`, `stlink`, `cmsis-dap`).
- `LANG_SERVER` — language server for `vscode_dist` (`clangd`, `cpptools`).
- `SVD_FILE` — path to an SVD file for peripheral register view in the debugger.
- `INTERCOM_FORCE_VERSION` — force the intercom (Si917) handshake version, overriding the default git-hash-based check. Only effective when flashing a bundle without the Si917 M4 firmware (for example, `flash_usb`).

## Firmware application set

The applications compiled into the firmware are defined by the `FIRMWARE_APPS` option — a map of `configuration name → application list`. The active set is selected by name through `FIRMWARE_APP_SET`. For example, to build a firmware image that includes the unit tests, run:

```shell
./fbt FIRMWARE_APP_SET=unit_tests
```

Common presets, available on both targets, are `default`, `unit_tests` and `recovery`. The full set is defined per target in `targets/<target>/fbt_conf/`.
