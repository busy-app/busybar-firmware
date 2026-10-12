# Updater {#updater}

Firmware updates install a bundle that can contain the U5 firmware, the Si917 application and radio images, and the resources. The normal firmware downloads, verifies and stages the bundle, then reboots into a RAM resident updater stage that writes the images. This page covers the bundle format, both halves of the flow, automatic updates, recovery and factory reset. The @ref build-system and @ref tooling-tests-ci pages cover the build side (how bundles are produced).

# Bundle format

A bundle is a tar archive, optionally gzip compressed, produced by `scripts/update_bundle.py`. All entries have uid and gid 0 and mtime 0 so builds are reproducible.

| Entry | Manifest key | Content |
| --- | --- | --- |
| `update.json` | | The manifest |
| `updater.bin` | `updater_stage`, `updater_stage_crc32` | The RAM resident updater stage |
| `firmware.dfu` | `updater_dfu` | U5 firmware as a DfuSe file (vendor 0x0483, product 0xDF11) |
| `firmware.rps` | `updater_sil_fw` | Si917 M4 application |
| `SiWG917-*.rps` | `updater_sil_radio_fw` | Si917 NWP radio firmware |
| `resources.tar` | `updater_resources` | The `/ext/apps_assets` tree |
| `resources/` | | Backup partition content, only in `bkp` bundles |

Manifest fields: `version` (format version 1), `target` (hardware target number, for example 22), `update_name`, `firmware_info`, `security_flags` and the paths above. Security flags: `NwpSigned` 1, `M4Signed` 2, `NwpEncrypted` 4, `M4Encrypted` 8, `U5Encrypted` 16. Signed bundles carry `NwpSigned | M4Signed`. `lib/toolbox/update_lib/update_manifest.c` parses the manifest and `update_config.c` validates it (format version, hardware target match, stage file presence and CRC).

The build produces these bundles in the combined `./fbt` build: `update` (resources, M4, NWP), `bkp` (the same plus backup partition resources), and signed variants when Si917 signing is configured. The `flash_usb` presets build a bundle with a chosen subset and push it to `POST /api/update`.

# Normal firmware side

Service: `applications/system/updater/` (`updater`, order 310, record `updater`). Paths: root `/ext/update`, download `/ext/update/bundle.tar`, staging `/ext/update/staging`, directory cache `/ext/update/directory.json`, pointer file `/ext/.sys_update.txt`, session configuration `/ext/.update_session.json`.

## Update check

`update_checker.c` downloads the channel directory from `check_url` (default `https://update.busy.app/busybar-firmware/directory.json`) and walks `channels[]` for the configured channel (`release` by default; `dev` and `rc` are selectable when the debug flag is set), then `versions[]` and `files[]` for the entry whose `target` is `f<hw target>` and whose `type` is `update_tgz`, or `update_signed_tgz` on devices that report signed Si917 firmware. It records the URL, SHA-256, version, changelog and id. The check runs 10 minutes after boot, then every 5 hours, and on demand (`updater_check_for_update()`, `POST /api/update/check`, the Firmware settings screen). The result is `Available` when the found version differs from the active version.

## Installation

An installation requires an updater session, which takes a single-slot lock and checks the battery: at least 40% charge, unless the debug flag is set and USB is connected. The stages, each reported on `UpdaterUpdateState` with progress:

1. **Download** (`FetchLoader` to `bundle.tar`, abortable).
2. **SHA verification** of the file against the expected hash, when one is known.
3. **Unpack** into the staging directory.
4. **Prepare**: validate the manifest, compare the security flags with the live device (from `sl_info`), decide which components to install (the radio image is skipped when its version equals the running NWP version), write the session configuration and the pointer file.
5. **Apply**: set the NVM boot mode to `Update` and reboot the U5.

Entry points: `updater_install_from_url(url, sha)` runs steps 1 to 5 in an install thread. `POST /api/update` streams a bundle to `bundle.tar` and runs steps 3 to 5. The CLI commands `update install`, `install_tar` and `install_web` start at different stages. The `update_ui` start-up hook draws the download and prepare screens on the GUI system layer above the running application; Back aborts a download.

## Automatic updates

A timer runs every `autoupdate_attempt_delay` (5 min). The timer applies an update only when all of these hold: auto update is enabled, the local time is inside the window (02:00 to 05:00 by default), a checked update is available, no check is in flight, the debug flag is off, and no application holds an update pause. The BUSY timer scene and the clock application pause updates while they run (`updater_pause_autoupdates()`, reference counted).

Settings are in `/ext/apps_data/updater/settings.json` (see @ref storage-and-settings) and exposed on `/api/update/autoupdate`.

# Updater stage

At the next boot the U5 sees boot mode `Update`, clears it, mounts the eMMC without the RTOS, reads the pointer file and the manifest, CRC-checks `updater.bin`, copies it to SRAM and jumps to it (see @ref hardware). The stage is the same firmware linked for RAM execution with the `update_executor` service (`applications/system/updater/update_executor/`).

Stages of the worker (`update_task.c`), with weighted progress:

1. **ReadManifest**: check the hardware target, read the pointer file, load the session configuration and the manifest, check that every referenced file exists.
2. **ValidateDFUImage**, **FlashWrite**, **FlashValidate**: validate the DfuSe headers and CRC, program flash pages, then read back and compare every page.
3. **917RadioWrite**, **917RadioInstall**, **917Write**, **917Install**: reset the Si917 into its ROM bootloader, negotiate the baud rate (921600 down to 9600), select the image slot and stream the RPS file with Kermit (`sl_updater/`, 3 retries, 30 s install timeout for the radio and 15 s for the application).
4. **ResourcesFileCleanup**, **ResourcesDirCleanup**, **ResourcesFileUnpack**: delete every file and directory listed in the old `/ext/Manifest`, then unpack the new `resources.tar` into `/ext`.
5. **Completed**: show "Update completed" and "Restarting device", delete the session configuration, reset.

On failure the stage becomes `Error` with a message and a stage-percent code, the screen shows "Update failed", and the session configuration is left in place. There is no automatic rollback. If U5 flashing never started, the previous image remains. Otherwise a reboot retries the update.

# Recovery firmware and factory reset

The `recovery` application set builds a firmware whose only job is a factory reset: its `recovery` service shows "Recovering...", starts an updater session and calls `factory_reset_perform()`. CI builds and publishes it as `busybar-f<hw>-recovery-<suffix>.dfu`.

`factory_reset_perform(updater, shipping_mode)` (`lib/toolbox/update_lib/factory_reset.c`):

1. Forget the BLE bond and reset the Matter fabrics (skipped in the recovery firmware, which lacks those services).
2. Format `/ext`, which also removes the Wi-Fi credentials and all settings.
3. Reset the U5 NVM block. Optionally set the shipping mode flag.
4. Prepare and apply the factory bundle `/bkp/recovery/update.json` from the read-only backup partition, so the updater stage reinstalls the factory firmware and resources.

It is reachable from Settings, System, Factory reset (requires 40% battery), from the CLI command `factory_reset [-s]`, and from the recovery firmware.

# Si917 images outside the bundle

`update 917 <rps>` and `update 917_ta <rps>` on the CLI flash an application or radio image directly from a file on `/ext`. `scripts/update.py 917 <rps> [--nwp]` uploads the file and runs the command. `scripts/flashrps.py` flashes from a host over the Si917 UART with the SWO line grounded. See @ref hardware for the bootloader details.

# Version check between the two MCUs

The intercom handshake compares the git hash of both images. A `flash_usb` bundle that omits the Si917 application would therefore fail after the U5 update unless the hashes match. The build variable `INTERCOM_FORCE_VERSION` pins the handshake string, and the `flash_usb` targets pass the same string to `scripts/update_over_http.py`, which reads `intercom_version` from `GET /api/status/firmware` and refuses the upload on a mismatch. See @ref build-system.
