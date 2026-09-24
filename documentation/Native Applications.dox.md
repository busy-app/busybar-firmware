# Native Applications {#native-applications}

Native applications are part of the firmware image. `application.fam` manifests declare them, the directories described in `applications/README.md` group them, and the loader starts them (see @ref user-interface). This page documents the manifest format, the application life cycle and every application in the tree.

# Manifest format

Every `application.fam` is a Python file with one `App(...)` call per unit. Fields in use:

| Field | Meaning |
| --- | --- |
| `appid` | Unique id. Also the thread application id, which selects the per-application data directory. |
| `apptype` | `FlipperAppType.SERVICE`, `STARTUP`, `CLICMD`, `APP`, `SETTINGS`, `SYSTEM`, `DEBUG` or `METAPACKAGE` |
| `name` | Display name, or the command word for `CLICMD` |
| `entry_point` | C function to run |
| `targets` | Target list, for example `["f20", "f21", "f22"]` or `["f64", "f65"]`. Two manifests may declare the same `appid` with disjoint target lists. |
| `stack_size` | Thread stack in bytes |
| `order` | Start order for services and hooks, menu order for settings applications |
| `requires` | Application ids that must be present in the build |
| `provides` | Metapackage members, or extra units such as CLI commands |
| `sources` | Explicit source list (default: glob) |
| `cdefines` | Defines exported to the whole build, used for feature detection (`SRV_WIFI`, `SRV_TIME`, ...) |
| `resources` | Directory merged into the resources tree |
| `flags` | Application flags, for example `InsomniaSafe` |
| `sdk_headers` | Headers exported to the external application SDK |

Types and their meaning:

| Type | Role |
| --- | --- |
| `SERVICE` | Background thread started at boot |
| `STARTUP` | Hook function run once at boot in a temporary thread |
| `CLICMD` | Command registered in the CLI registry |
| `APP` | Application reachable from the mode switch or the APPS menu |
| `SETTINGS` | Entry of the settings menu |
| `SYSTEM` | Application launched programmatically, not listed in any menu |
| `DEBUG` | Entry of the Debug apps menu, visible when Dev mode is on |
| `METAPACKAGE` | Group selected by an application set |

The external `.fap` application type is supported by the tooling (`scripts/fbt_tools/fbt_extapps.py`) but no manifest in the tree uses it.

# Application life cycle

An application is a thread entry point that receives an argument string. The common skeleton:

1. Open the records it needs (`gui`, `storage`, `time`, ...).
2. Under `with_gui()`, take the `Main` layer root widget of each display and build the screens. Most applications use a `SceneManager`, a front window widget, and a back display column with a `NavBar` and a scene window.
3. Install a thread signal callback. `FuriSignalAboutToExit` (sent by the desktop before a switch transition) lets the application play an exit animation. `FuriSignalExit` (sent by the loader) must stop the event loop.
4. Run the event loop. Input arrives through widgets or a layer input callback. Usually the application handles only `Back` itself.
5. Free the widgets and close the records, then return.

Applications that must not be interrupted by an automatic firmware update call `updater_pause_autoupdates()` and `updater_resume_autoupdates()`. Applications that need to control remote drawing change their loader priority.

# Main applications (`applications/main/`)

| Application | Launched by | Description |
| --- | --- | --- |
| `busy` (`APP`, order 10) | BUSY and CUSTOM switch positions, remote start | The busy timer front end. See @ref busy-timer. |
| `clock` (`APP`, order 20) | APPS menu | Full screen clock. Setup toggles date, seconds and colon blink; settings in `/ext/apps_data/clock/settings.json`. The time format comes from the `time` service. Pauses automatic updates while open. |
| `power_on` (`SYSTEM`) | Desktop at boot | Waits for the initial switch position, showing "Starting..." after 500 ms. On the first boot (no `/ext/apps_data/power_on/done.txt`) it plays the power-on animations on both displays and waits for any key, powering the device off after 15 minutes without interaction. |
| `settings_menu` (`SYSTEM`) | SETTINGS switch position | Title card, then a menu built from every `SETTINGS` application descriptor, ordered by `order`. An argument names the sub-application to return to. |
| `soft_off` (`SYSTEM`) | OFF switch position | Plays the turn-off animation, then releases the low power lock so the displays and light sensor sleep. Re-takes the lock when the switch moves. |

# Settings applications (`applications/settings/`)

Each settings application, when called with a descriptor argument, fills a `SettingsAppDescriptor` (titles, icons, `menu_extra`, `display_in_menu`) and returns. When launched, it shows its screens and returns to `settings_menu` on Back.

| Order | Application | Screens |
| --- | --- | --- |
| 0 | `ble_settings` | Pairing mode (5 minute window, blinking status lights) when not paired, "Connected" when paired, Forget device dialog |
| 10 | `wifi_settings` | Not configured: QR code to the connection guide. Configured: state and SSID, View IP address, Forget network. No on-device SSID entry. |
| 20 | `matter_settings` | Pair device (QR and manual code on the back display), Forget all pairings, commissioning result screens, "Contact support" if the Matter stack is broken |
| 30 | `account_settings` | Get pairing code (4 digit PIN for cloud.busy.app), linked account info with a shortened email, Unlink |
| 40 | `sound_settings` | Volume slider 0..100 in steps of 5 with a preview sound |
| 50 | `brightness_settings` | Mode Auto or Manual, Level 0..100 in steps of 5 |
| 55 | `time_settings` | Time zone list, 12 h or 24 h format |
| 60 | `firmware_settings` | Check for update, install dialog, low battery screen (40% rule), version info, auto-update switch, channel selector in Dev mode |
| 70 | `system_settings` | Power (shut down, restart, battery info), Debug (Dev mode on or off), Factory reset |
| 80 | `about` | General (name, serial, hardware version, MAC addresses, display sizes), Firmware (version, branch, commit, API, build date, uptime), Compliance (FCC and IC ids), Open-source libraries |
| 90 | `debug_app_list` | Lists the `DEBUG` applications. Visible only in Dev mode. |

# System applications (`applications/system/`)

| Application | Type | Description |
| --- | --- | --- |
| `apps_menu` | `SYSTEM` | The APPS menu. Lists `clock` and, when the flag file `/ext/apps_data/apps_menu/js_apps_enabled` exists, every installed JavaScript application. Remembers `active_application` and relaunches it when the switch returns to APPS. |
| `js_app_launcher` | `SYSTEM` | Start, Setup and Run screens for a JavaScript application. See @ref javascript-applications. |
| `message` | `SYSTEM` | Shows its argument as a status message. Used by the desktop to display application start errors. |
| `updater` family | `SERVICE`, `STARTUP`, `CLICMD` | The updater service, the update UI hook, the update executor (recovery side), the `update` and `factory_reset` commands. See @ref updater. |
| `tar_cli` | `CLICMD` | `tar` command |
| `fetch_cli` | `CLICMD` | `fetch` command, an HTTP client with TLS client authentication |
| `crypto_backup` | `CLICMD` (U5), `STARTUP` (Si917) | Raw backup and restore of the Si917 key storage |
| `crypto` | `CLICMD` (Si917) | Key storage management |
| `crypto_test`, `nvm_test`, `wifi_cli_test` | `CLICMD` (Si917) | Test shells for the crypto accelerators, NVM3 and the radio |

# Debug applications (`applications/debug/`)

Visible under Settings, Debug apps when Dev mode is on. Built by the `debug_apps` metapackage.

| Application | Purpose |
| --- | --- |
| `anim_test` | Plays two test animations through `AnimPlayer` |
| `back_display_test`, `back_display_factory` | Drawing test and factory test patterns for the OLED (the factory variant is in the `factory_testing` set) |
| `front_display_test` | LED matrix test patterns |
| `gui_test` | Widget showcase |
| `input_test` | Shows key and switch events. Hold Back to exit. |
| `light_sensor_test` | Raw channel readings and derived values |
| `dummy` | Shows a label with its argument |
| `storage_bench` | `storage_benchmark` CLI command |
| `unit_tests` | `unit_tests` CLI command with the minunit suites (`unit_tests` application set) |
| `crash_test`, `led_indicator`, `time_test` | Present in the tree but not built |

# Dev mode

The NVM debug flag, toggled from Settings, System, Debug or with `sysctl debug 1`, gates: the Debug apps entry, the demo mode in the busy timer setup, the firmware channel selector, `update 917_probe`, the `gpio` and `otp` commands, the low battery bypass for updates while on USB, and the automatic reboot on intercom failure (suppressed in Dev mode). The `sysctl` service mirrors the flag into its settings file so it survives reboots.
