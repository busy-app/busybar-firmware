# BarMetal

**An independent, community firmware fork for the BUSY Bar®**, maintained by
[@nastea1](https://github.com/nastea1) alongside the
[BarPilot](https://github.com/nastea1/barpilot) control app.

---

> ### Not affiliated
>
> BarMetal is an independent community project. It is **not affiliated with, endorsed by,
> sponsored by, or supported by Flipper Devices Inc.**
>
> "BUSY Bar", "BUSY", and "Flipper" are trademarks of their respective owners. They are
> used here **only to identify the hardware this firmware is compatible with** — a
> descriptive (nominative) use, not a claim of origin, affiliation, or endorsement.
> BarMetal does not use Flipper Devices' logos or branding as its own identity.
>
> This repository is a **fork of the official
> [BUSY Bar firmware](https://github.com/busy-app/busybar-firmware)**. Upstream code
> remains under its original licenses and copyright — most first-party code under
> **GPLv2-or-later**, `furi` under MIT, assets under CC-BY 4.0 and OFL 1.1; see
> [`LICENSE.md`](LICENSE.md) and [`REUSE.toml`](REUSE.toml). Modifications by this fork
> are released under the **same license as the file they modify**, as the GPL requires.
> Upstream commit history and authorship are preserved in full.
>
> Flashing third-party firmware may void your warranty and is done **at your own risk**.
> There is no warranty of any kind; see the license text.
>
> Requests from rights holders: open an issue and we will respond promptly.

---

## What this fork is for

BarPilot drives the bar over its local HTTP API. Building it exposed device-level
behaviour that no client could work around:

- **The Wi-Fi subsystem could wedge permanently** — stuck reporting `connecting`, every
  radio command returning `Command timed out`, unrecoverable by forgetting the network or
  power-cycling, and eventually crashing the device outright.
- **`POST /api/display/brightness` looked inert** — the low end of the range did nothing
  visible, and `0` could not turn the display off.
- **There was no software reboot**, so a wedged device needed physical recovery — in one
  case, escaping the STM32 bootloader with `dfu-util`.

[`WIFI-WEDGE-ANALYSIS.md`](WIFI-WEDGE-ANALYSIS.md) documents the root causes with
citations into this source tree. BarMetal fixes them.

## What is fixed

| Area | Upstream behaviour | BarMetal |
|---|---|---|
| **Wi-Fi hang** | `wifi_api_blocking_request()` waits on `FuriWaitForever`; one unanswered request from the Wi-Fi co-processor blocks the caller forever *and* leaks the API semaphore, so every later call returns `Command timed out` until reboot | Every backend request is bounded by a 30 s watchdog. On expiry the caller is released, the semaphore is freed and the state machine returns to `disconnected` — a recoverable error instead of a brick |
| **Unresponsive radio** | No recovery path; the service waits indefinitely | After 3 consecutive timeouts the link is torn down and re-initialised through the service's existing deinit/init paths |
| **Cannot forget the network** | `Forget` is gated on `WifiStateDisconnected`, so a device stuck in `connecting` cannot be un-stuck by the one action the UI offers | `Forget` is allowed from any state |
| **Brightness low end** | Front curve starts `{25, 25, 28, …}` — `0` sets 25 %, and every value from 0 to ~14 renders identically | Step 0 means **off** (front and back), and the low end is spread across the curve |
| **Auto-brightness** | Light-sensor level 0 maps to step 0 | Clamped to ≥ 1 step, so a dark room can never blank the display (off stays a deliberate manual choice) |

## What is new

- **`POST /api/system/reboot?target=all|mcu|wifi`** — the software reboot the stock
  firmware never had. `target=wifi` restarts only the radio co-processor, recovering a
  wedged Wi-Fi stack remotely without disturbing anything running on the main MCU.

## Compatibility

BarMetal tracks upstream and keeps the HTTP API compatible: everything the official
firmware serves still works, and existing clients (including BarPilot) run unmodified.
The only API addition is `/api/system/reboot`.

## Status

Early, and honest about it: changes are written against upstream sources and reviewed
line by line. Anything not yet flashed to hardware should be treated as untested on a
device — the commit history says which is which.

---

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
./fbt flash_usb
```

The `INTERCOM_FORCE_VERSION` variable may be used to override the intercom (Si917) version check when it differs from the build — see the [Build System documentation](documentation/Build%20System.dox.md) for details.

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
