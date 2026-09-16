# Storage and Settings {#storage-and-settings}

# Storage layout

All file storage is on the eMMC, managed by the `storage` service (`applications/services/storage/`, record `storage`, U5 only). There is no internal flash filesystem and no `/int` mount point. Two exFAT partitions exist:

| Mount point | Partition | Size | Access | Content |
| --- | --- | --- | --- | --- |
| `/bkp` | 1 | 256 MiB | Read-only after boot | Factory recovery bundle (`/bkp/recovery/update.json`), battery calibration, crypto storage backup |
| `/ext` | 2 | Rest of the device | Read-write | Everything else |

At boot the service mounts `/bkp`, checks that it is not larger than 512 MiB (an older partition table is rejected), sets it read-only, then mounts `/ext`. The `supervisor` service shows mount failures as a "Storage error" prompt. OK formats or repartitions the device, resets the NVM block and reboots. The `sysctl storage_bkp_unlock 1` CLI command makes `/bkp` writable when the NVM debug flag is set.

Other persistent state lives outside the filesystem: the 2 KiB backup SRAM block on the U5 (`furi_hal_nvm`: debug flag, boot mode, shipping mode flag, last switch position, fault data) and the NVM3 store on the Si917 (`nvm` service: BLE bonding data, crypto storage access mode, Matter fabrics).

## Path prefixes

`applications/services/storage/storage.h` defines the prefixes that every application uses:

| Macro | Expands to | Notes |
| --- | --- | --- |
| `EXT_PATH(p)` | `/ext/p` | |
| `BACKUP_PATH(p)` | `/bkp/p` | |
| `APP_DATA_PATH(p)` | `/data/p` | Rewritten by the storage service to `/ext/apps_data/<appid>/p` |
| `APP_ASSETS_PATH(p)` | `/assets/p` | Rewritten to `/ext/apps_assets/<appid>/p` |
| `SHARED_ASSETS_PATH(p)` | `/ext/apps_assets/shared/p` | Also `SHARED_ANIM_PATH`, `SHARED_IMG_PATH`, `SHARED_SOUND_PATH` |

The `<appid>` is the application id of the **calling thread**. The same `APP_DATA_PATH("settings.json")` therefore resolves to a different file when called from a service thread, from a CLI command thread or from the web server thread. Services that must be reachable from other threads use absolute paths (the `sysctl` service is one example). The service creates the data directory automatically on write.

## Directory map

| Path | Owner |
| --- | --- |
| `/ext/apps_data/<appid>/` | Per application data and settings files |
| `/ext/apps_assets/<appid>/{animations,images,sounds,themes}` | Bundled assets, installed from the resources bundle |
| `/ext/apps_assets/shared/{animations,images,sounds,fonts,ca}` | Shared assets, fonts and the CA bundle |
| `/ext/apps_assets/web_server/www/` | The built web UI, OpenAPI document and Swagger UI |
| `/ext/user_assets/<id>/` | User uploaded JavaScript applications and their resources |
| `/ext/update/` | Update download and staging area |
| `/ext/.sys_update.txt`, `/ext/.update_session.json` | Update pointer file and session configuration |
| `/ext/Manifest` | Resource manifest, used to remove old resources on update |
| `/ext/log.txt`, `/ext/intercom_failure_log.txt` | Log dumps |
| `/bkp/recovery/` | Factory bundle used by factory reset and the recovery firmware |
| `/bkp/crypto_backup.bin` | Optional raw backup of the Si917 crypto storage |

## Storage service API

The service runs one thread with an 8 deep message queue. The public API in `storage.h` is fully synchronous: file operations (`storage_file_open/read/write/seek/...`), directory operations, common operations (`stat`, `remove`, `rename`, `copy`, `mkdir`, `fs_info`, `migrate`), SD level operations (`format`, `mount`, `unmount`, `info`), and helpers such as `storage_simply_mkdir`, `storage_read_entire_file`, `storage_write_entire_file`. `storage_posix_api.c` routes newlib `fopen` and friends to the same service. LVGL reads images and fonts through the `C:` filesystem driver in `lib/lvgl_addons/fs/`, and Mongoose serves static files through the adapter in `targets/f21/mongoose/mongoose_glue.c`.

Events on the storage pubsub: `CardMount`, `CardUnmount`, `CardMountError`, `FileClose`, `DirClose`. On `PowerEventShutdown` the service syncs every open file and refuses further requests until revived.

The `storage` CLI command (see @ref cli) exposes list, read, write, copy, remove, rename, mkdir, stat, info, tree, md5, timestamp, tar extract and format.

# Settings

Every persistent setting is a JSON file managed by `lib/setting_provider`. A service describes its settings as a tree of typed descriptors, and the library loads, validates, migrates and saves the file.

## Descriptor model

`setting_provider_alloc(path, version, migrations, count)` returns a provider. `setting_provider_load(provider, root, struct)` fills a C structure from the file. `save`, `reset`, `validate` and `free` complete the API.

Descriptor types: `Bool`, `Int`, `Float`, `String` (with `max_size`), `Enum` (string map, for example `"24h"`), `Custom` (string serialized through callbacks, for example a time zone), `Union` (tagged by an enum), `Struct`, `Raw` (direct cJSON callbacks). Each descriptor has a default value (or a callback that produces it) and an optional validation callback. Descriptors map JSON keys to structure fields with `offsetof`.

## File format and semantics

Each file is one JSON object with a top-level `version` number and the keys of the root structure. On load:

- A file that does not parse, or a missing structure, is reset to defaults and rewritten.
- Missing keys are filled with defaults and the file is rewritten. Invalid values fall back to defaults.
- The library migrates a stored version lower than the current version step by step through the registered migration callbacks. A missing or failing migration, or a version newer than the firmware supports, resets the file to defaults.
- Save validates and rewrites the whole file. There is no temporary-file rename in the library. `lib/storage_utils/temp_file.h` is available for callers that need it.

All services in the tree are currently at version 1 or 2 with no registered migrations.

## Settings files

| File | Service | Keys and defaults |
| --- | --- | --- |
| `/ext/apps_data/power_srv/settings.json` | power | `charge_limit` 100 (30..100) |
| `/ext/apps_data/time/settings.json` | time | `is_enabled` true, `server_address` `udp://time.busy.app:123`, `timezone` `New York`, `boot_delay` 300 s, `background_sync_interval` 10800 s, `retry_sync_interval` 600 s, `time_format` `24h` |
| `/ext/apps_data/sysctl/settings.json` | sysctl | `cli_wifi_enabled` false, `websrv_accesslog_level` 0, `debug_enabled` (mirrors the NVM flag), `ui_debug_mode` 0 |
| `/ext/apps_data/device_name/settings.json` | device_name | `name` `BUSY Bar` (20 characters max, ASCII only) |
| `/ext/apps_data/brightness_control/config.json` | brightness_control | `mode` `auto`, `brightness` 50 |
| `/ext/apps_data/audio/audio.json` | audio | `volume` 1.0 |
| `/ext/apps_data/busy_timer/settings_busy.json`, `settings_custom.json`, `state.json` | busy_timer | Profiles and the last snapshot, see @ref busy-timer |
| `/ext/apps_data/updater/settings.json` | updater | `check_url` `default`, `check_channel_id` `release`, `check_startup_interval` 10 min, `check_interval` 5 h, `autoupdate_enabled` true, `autoupdate_interval_start` 02:00, `autoupdate_interval_end` 05:00, `autoupdate_attempt_delay` 5 min |
| `/ext/apps_data/wifi/settings.json` | wifi | `credentials{ssid, passphrase, security}`, `ip_config{management, type, ipv4{address, mask, gateway}}` |
| `/ext/apps_data/ble/settings.json` | ble | `enabled` false |
| `/ext/apps_data/mqtt/settings.json`, `state.json` | mqtt | `config{server_url, client_cert_type, ignore_server_cert}`; `client_id`, `session_id`, `user_id`, `email`, `token` |
| `/ext/apps_data/usb_srv/settings.json` | usb_network | `ip_config{address 10.0.4.20, netmask 255.255.255.0, gateway 0.0.0.0}` |
| `/ext/apps_data/web_server/access.json` | web_server | `access_mode` 0, `access_key` |
| `/ext/apps_data/tokens/access.json` | api_tokens | `tokens[]` with hashes and metadata |
| `/ext/apps_data/clock/settings.json` | clock | `show_date` true, `show_seconds` false, `blink_colons` true |
| `/ext/apps_data/apps_menu/settings.json` | apps_menu | `active_application` |
| `/ext/apps_data/jsrunner/<id>.settings.json`, `<id>.localstorage.json` | JavaScript applications | Per application values, see @ref javascript-applications |
| `/ext/apps_data/matter/cd_selection.txt` | matter | `production`, `dev` or `certification` (plain text) |

## Helper libraries

- `lib/settings_helpers/`: `app_desc.h` defines the descriptor a settings application returns to the settings menu (titles, icons, `menu_extra`, `display_in_menu`). `gui_params.h` holds the settings assets path.
- `lib/storage_utils/`: `dir_walk.h` (recursive directory iterator) and `temp_file.h`.
- `lib/toolbox/tar/`: `TarArchive` with uncompressed, heatshrink and gzip read modes, used by the updater, the `tar` CLI command and `storage extract`.
- `lib/toolbox/compress.c`: streaming decoder over heatshrink or zlib.
- `lib/flipper_format/`: the Flipper key-value text format. No service uses it. It remains for its unit tests.
