# HTTP API {#http-api}

The @bsb exposes a plain HTTP API on TCP port 80. The same API serves the built-in web UI, the desktop and mobile apps, host-side scripts and JavaScript applications running on the device. This page describes the server, the access control model and every endpoint as implemented in `applications/services/web_server/`.

The public, user-facing version of this reference lives at [docs.busy.app](https://docs.busy.app/bar/dev/http-api). The device also serves its own OpenAPI document at `/openapi.yaml` and a Swagger UI at `/docs/`.

# Server

| Property | Value |
| --- | --- |
| Implementation | Mongoose HTTP server on lwIP sockets, one poll thread (`web_server.c`) |
| Listener | `http://0.0.0.0`, port 80, all interfaces (USB network and Wi-Fi) |
| TLS | None. Mbed TLS is linked only for outbound client connections. |
| API version | `27.7.0` (`http_api/http_api.h`, mirrored in `openapi/openapi.yaml`) |
| USB address | `10.0.4.20` by default (see @ref connectivity) |
| mDNS | `_http._tcp` on port 80, instance `busybar-<usb mac>`, TXT `path=/`, `name=<device name>` |
| Static root | `/ext/apps_assets/web_server/www` (the built web UI, gzip-only assets) |

Routing is prefix based. The `/api` handler table (`http_api/api_root.c`) dispatches to one file per area: `version`, `transport`, `assets`, `storage`, `display`, `audio`, `input`, `status`, `status/ws`, `wifi`, `update`, `screen`, `ble`, `time`, `name`, `log_dump`, `account`, `busy`, `smart_home`. The root handler serves `access` and `access/tokens` directly. Any path under `/api` that no handler accepts returns 400. The static root serves any other path, and a missing file renders `404.html`.

The server processes requests in two phases. Access control and API version checks run when the headers arrive, before the body is read. A rejected request receives 403 and the connection closes. Upload endpoints take over the raw socket at this point and stream the body straight to storage.

## Limits

| Limit | Value |
| --- | --- |
| Request body for JSON endpoints | 256 KiB (Mongoose receive buffer) |
| Update bundle upload | 100 MiB, 413 above |
| Storage and asset upload | No explicit cap. Limited by free space. |
| Upload idle timeout | 3 s for storage and assets, 5 s for update. Returns 408. |
| Overload shedding | Any monitored lwIP pool at 85% or more returns 508 and closes the connection |
| Status WebSocket clients | 4. A fifth client receives a protobuf error frame and is closed. |
| Status WebSocket rate | 11 messages per second per client |
| Canvas elements per draw | 100 |
| Wi-Fi scan results | 20 |

There is no per-client request rate limiter.

## Response conventions

- Success replies `{"result":"OK"}` unless the endpoint documents a body.
- Errors reply `{"error":"<message>"}`. The update install endpoint adds `error_code`.
- Every response carries `Access-Control-Allow-Origin: *` and `Access-Control-Allow-Headers: *`. An `OPTIONS` request that does not match a handler method receives a CORS preflight reply with the allowed methods. Preflights bypass access control.
- Status codes in use: 200, 400, 403, 404, 405, 406, 408, 409, 410, 413, 503, 508.

## Access log

The access log format is nginx style. The CLI command sets the verbosity: `sysctl websrv_accesslog_level N`

| Level | Content |
| --- | --- |
| 0 (default) | Requests with status 400 or higher |
| 1 | All requests, plus the `x-request-id` or `x-trace-id` header when present |
| 2 | Adds the user agent |
| 3 | Adds a local timestamp |

# Access control

The access check (`api_root.c`, `http_api_access_status()`) runs for every `/api` request in this order:

1. `OPTIONS` requests are always allowed.
2. `GET /api/version`, `GET /api/access` and `GET /api/transport` are always allowed.
3. Requests from `127.0.0.1` are always allowed. This is the path used by JavaScript applications, the BLE HTTP tunnel and the MQTT HTTP proxy.
4. If the connection did not arrive on the USB network interface, the persisted access mode applies: `disabled` rejects, `enabled` accepts everyone, `key` continues to the credential check.
5. The server reads a credential from the `X-API-Token` header, or from the `x-api-token` query parameter for WebSocket upgrades. It accepts the credential if it equals the configured access key, or if it is a valid API token.
6. The server accepts a request without a credential on USB and rejects it elsewhere.

The server stores the access mode and key in `/ext/apps_data/web_server/access.json`. The default mode is `disabled`. In practice: USB and BLE clients need no credentials, and over Wi-Fi the API is closed until the user enables open access, sets a numeric key, or mints a token.

## Access key

The key is 4 to 10 ASCII digits. It is set with `POST /api/access?mode=key&key=NNNN`. A caller that authenticates with the key has admin rights over tokens.

## API tokens

The `api_tokens` service manages tokens (`applications/services/api_tokens/`):

- A token is 32 characters, derived from 24 random bytes.
- The service stores only the SHA-256 hash, with `short_id`, `display_id`, `name`, `created_at` and `last_used_at`, in `/ext/apps_data/tokens/access.json`.
- The mint response returns the full token once.
- A caller that authenticated with a token cannot mint tokens, cannot revoke all tokens, and can revoke only itself.

## API version negotiation

A client can send `X-API-Sem-Ver` (or the `x-api-sem-ver` query parameter on WebSocket upgrades). The major number must equal the firmware API major. A mismatch returns 405 `{"error":"Incompatible API version"}`. Requests without the header are not checked.

# Endpoint catalogue

The `Auth` column uses these values: `open` for the whitelisted endpoints, `std` for the rules above, and `admin` where token callers are refused.

## System

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| GET | `/api/version` | open | | `{"api_semver":"27.7.0"}` |
| GET | `/api/transport` | open | | `{"type":"usb"}` or `{"type":"wifi"}` |
| GET | `/api/status` | std | | `{device, firmware, system, power}` |
| GET | `/api/status/device` | std | | `serial_number`, `usb_mac`, `wifi_mac`, `ble_mac`, `otp_valid`, `otp_model`, `otp_timestamp`, `firmware_security` (`secure`, `insecure`, `other`, `unknown`) |
| GET | `/api/status/firmware` | std | | `version`, `target`, `branch`, `build_date`, `commit_hash`, `intercom_version`, `nwp_version`, `matter_version` |
| GET | `/api/status/system` | std | | `api_semver`, `uptime`, `boot_time`, `auto_update_enabled` |
| GET | `/api/status/power` | std | | `state` (`discharging`, `charging`, `charged`), `battery_charge`, `battery_voltage`, `battery_current`, `usb_voltage` |
| POST | `/api/log_dump` | std | query `filename` (optional, `[A-Za-z0-9_-]`) | `{"result":"OK","path":"/ext/log.txt"}` |
| GET | `/api/name` | std | | `{"name":"..."}` |
| POST | `/api/name` | std | `{"name":"..."}`, no other keys | 400 on empty, too long, illegal character, or only spaces |
| POST | `/api/input` | std | query `key` in `up`, `down`, `ok`, `back`, `start`, `busy`, `custom`, `off`, `apps`, `settings` | Toggles the key |

## Access and settings

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| GET | `/api/access` | open | | `{"mode":"disabled","key_valid":false}` |
| POST | `/api/access` | std | query `mode` (`disabled`, `enabled`, `key`), `key` required for `key` mode | |
| GET | `/api/access/tokens` | std | | `{"tokens":[{short_id, display_id, name, created_at, last_used_at}]}` |
| POST | `/api/access/tokens` | admin | `{"name":"..."}` | Token entry including `token` |
| DELETE | `/api/access/tokens` | admin | | Revokes all tokens |
| DELETE | `/api/access/tokens/{short_id}` | std | | 404 if unknown. A token caller may revoke only itself. |
| GET | `/api/display/brightness` | std | | `{"value":"auto"}` or `{"value":"50"}` |
| POST | `/api/display/brightness` | std | query `value` = `auto` or 0..100 | |
| GET | `/api/audio/volume` | std | | `{"volume":0..100}` |
| POST | `/api/audio/volume` | std | query `volume` 0..100, `silent` 0 or 1 | Plays the volume-change sound unless silent |

## Time

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| GET | `/api/time` | std | | `{"timestamp":"<local time>"}` |
| POST | `/api/time/timestamp` | std | query `timestamp` (ISO 8601, treated as UTC) | Sets the RTC |
| GET | `/api/time/timezone` | std | | `{"name","offset","abbr"}` |
| POST | `/api/time/timezone` | std | query `timezone` (a zone name known to `utz`) | |
| GET | `/api/time/tzlist` | std | | `{"list":[{name, offset, abbr}]}` |

## Busy timer

See @ref busy-timer for the meaning of the fields.

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| GET | `/api/busy/snapshot` | std | | Snapshot JSON |
| PUT | `/api/busy/snapshot` | std | Snapshot JSON | 400 on parse failure. The busy timer service ignores stale or future timestamps. |
| GET | `/api/busy/profiles/{slot}` | std | slot `busy` or `custom` | Profile JSON |
| PUT | `/api/busy/profiles/{slot}` | std | Profile JSON | |

Snapshot shape: `{"snapshot_timestamp_ms":N,"snapshot":{"type":"NOT_STARTED" or "INFINITE" or "SIMPLE" or "INTERVAL","card_id":"...","is_paused":bool, ...per-type progress fields...}}`. Profile shape: `{"profile_timestamp_ms":N,"id":"...","title":"...","sort_order":N,"timer_settings":{"type":...,"total_time_ms":N,"interval_work_ms":N,"interval_rest_ms":N,"interval_work_cycles_count":N,"is_autostart_enabled":bool,"busy_bar_settings":{"theme":"...","show_work_phase_only":bool,"trigger_smart_home":bool}}}`. The full schemas are in `openapi/busy.yaml`.

## Screen, drawing and streaming

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| GET | `/api/screen` | std | query `display` = `0` (front) or `1` (back) | `image/bmp` body containing base64 of the raw frame: front 72x16 BGR888 (3456 bytes), back 160x80 4 bpp (6400 bytes) |
| GET | `/api/status/ws` | std | WebSocket upgrade, token via `?x-api-token=` | See the Status WebSocket section below |
| POST | `/api/display/draw` | std | Draw request JSON | 200 after the canvas commits. 409 `Not drawn due to low priority`. 400 names the invalid field. |
| DELETE | `/api/display/draw` | std | query `application_name`, or JSON `{"application_name","element_ids":[...]}`. Empty body clears everything. | |

Draw request body:

- `application_name` (required, `[A-Za-z0-9._-]{1,32}`).
- `priority` (1..100, default 50). The canvas rejects the draw if the running application has a higher priority. Built-in applications run at 10. An active BUSY session runs at the blocking priority and refuses all draws.
- `led_notification_color` (`#RRGGBBAA`, optional). Blinks the status lights on success.
- `elements` (1..100 items). Common fields: `id` (required), `type` (required), `timeout` in seconds or `display_until` epoch string, `x`, `y`, `align` (`top_left`, `top_mid`, `top_right`, `mid_left`, `center`, `mid_right`, `bottom_left`, `bottom_mid`, `bottom_right`), `display` (`front` default, `back`), `z_index`.

Element types:

| Type | Fields |
| --- | --- |
| `text` | `text`, `font` (`tiny`, `small`, `normal`, `condensed`, `bold`, `large`, `extra_large`, `global`, `superscript`), `color`, `width`, `scroll_rate`, `scroll_start_delay`, `scroll_repeat_delay` |
| `image` | `path` (under `/ext/user_assets/<app>/`) or `stock_path` (shared images), `opacity` 0..100 |
| `animation` | `path` or `stock_path`, `section`, `loop`, `await_previous_end`, `opacity` |
| `rectangle` | `width`, `height`, `fill` (`none`, `solid`, `gradient_h`, `gradient_v`), `fill_colors[2]`, `border_width`, `radius`, `border_color` |
| `countdown` | `timestamp`, `direction` (`time_left`, `time_since`), `show_hours` (`when_non_zero`, `always`), `color` |
| `xpmbitmap` | `data` (XPM text, 32 colors max, 4 characters per pixel max, must fit the display), `opacity` |

The `canvas` service implements the draw API, see @ref user-interface.

## Storage

Every `path` must start with `/ext`, must not contain `..`, and is limited to 63 characters.

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| POST | `/api/storage/write` | std | query `path`, `append` 0 or 1. Raw body with `Content-Length`. | Streamed to the file |
| GET | `/api/storage/read` | std | query `path` | `application/octet-stream`, `Content-Disposition: attachment` |
| DELETE | `/api/storage/remove` | std | query `path` | Recursive |
| POST | `/api/storage/mkdir` | std | query `path` | |
| POST | `/api/storage/rename` | std | query `path`, `new_path` | |
| GET | `/api/storage/list` | std | query `path` | `{"list":[{"type":"dir","name"},{"type":"file","name","size"}]}` |
| GET | `/api/storage/status` | std | | `{"used_bytes","free_bytes","total_bytes"}` for `/ext` |

## Application assets and audio

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| POST | `/api/assets/upload` | std | query `application_name` (`[A-Za-z0-9._-]{1,31}`), `file` (relative path, `[A-Za-z0-9._/-]{1,64}`, no `..`). Raw body. | Written to `/ext/user_assets/<app>/<file>`. Parent directories are created. |
| DELETE | `/api/assets/upload` | std | query `application_name` | Removes the whole application directory |
| POST | `/api/audio/play` | std | `{"application_name","path"}` or `{"application_name","stock_path"}` | 404 `Failed to play audio` |
| DELETE | `/api/audio/play` | std | | Held until playback ends. 410 if nothing plays, 503 after 30 s. |

## Update

@ref updater describes the update flow.

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| POST | `/api/update` | std | Raw update bundle (tar or tgz), `Content-Length` required, 100 MiB max | `{"result":"OK","message":"Update accepted. System will reboot."}` then reboot. 400 with a reason, 409 if busy, 413 if too large. |
| POST | `/api/update/check` | std | | Starts an asynchronous check. 409 if busy. |
| GET | `/api/update/status` | std | | `{"install":{is_allowed, event, action, status, detail, download{speed_bytes_per_sec, received_bytes, total_bytes}},"check":{available_version, event, status}}` |
| GET | `/api/update/changelog` | std | query `version` | `{"changelog":"..."}` |
| POST | `/api/update/install` | std | query `version` (must equal the available version) | 400 with `error_code` in `version_missing`, `not_available`, `version_mismatch`. 503 when not allowed, for example `battery_low`. |
| POST | `/api/update/abort_download` | std | | |
| GET | `/api/update/autoupdate` | std | | `{"is_enabled":bool,"interval_start":"HH:MM","interval_end":"HH:MM"}` |
| POST | `/api/update/autoupdate` | std | Any subset of the three fields | |

Install `status` values: `ok`, `battery_low`, `busy`, `download_failure`, `download_abort`, `sha_mismatch`, `unpack_staging_dir_failure`, `unpack_archive_open_failure`, `unpack_archive_unpack_failure`, `install_manifest_not_found`, `install_manifest_invalid`, `install_session_config_failure`, `install_pointer_setup_failure`, `unknown_failure`. `action` values: `download`, `sha_verification`, `unpack`, `prepare`, `apply`, `none`. `event` values: `session_start`, `session_stop`, `action_begin`, `action_done`, `detail_change`, `action_progress`, `none`.

There is no endpoint for a plain reboot or for entering DFU. The OpenAPI document marks `POST /api/update` and `GET /api/status/ws` as `x-local-only`, but the firmware applies only the standard access rules to them.

## Wi-Fi

All Wi-Fi endpoints return 503 when the `wifi` service is absent.

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| GET | `/api/wifi/status` | std | | `{"state":...}` plus `ssid`, `security`, `bssid`, `channel`, `rssi`, `ip_config{ip_method, ip_type, address}` when connected. States: `unknown`, `disconnected`, `connected`, `connecting`, `disconnecting`, `reconnecting`. |
| GET | `/api/wifi/networks` | std | | Blocking scan. `{"count":N,"networks":[{ssid, security, rssi}]}`. 400 when connected. |
| POST | `/api/wifi/connect` | std | `{"ssid","password","security","ip_config":{"ip_method":"dhcp"}}` or static `{"ip_method":"static","address","mask","gateway"}`. `security` in `Open`, `WPA`, `WPA2`, `WEP`, `WPA/WPA2`, `WPA3`, `WPA2/WPA3`. | 400 with reason: `Already connected`, `Access point not found`, `Authentication failed`, `DHCP error`, `Command timed out` |
| POST | `/api/wifi/disconnect` | std | | Disconnects and forgets the credentials. 400 `Already disconnected`. |

## Bluetooth

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| POST | `/api/ble/enable` | std | | |
| POST | `/api/ble/disable` | std | | |
| GET | `/api/ble/status` | std | | `{"status":...,"address"}`. Status in `reset`, `initialization`, `disabled`, `enabled`, `connectable`, `connected`. |
| DELETE | `/api/ble/pairing` | std | | Forgets the bonded peer |

## Smart home (Matter)

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| GET | `/api/smart_home/pairing` | std | | `{"fabric_count":N,"latest_pairing_status":{"value":...,"timestamp"}}`. Values: `never_started`, `started`, `completed_successfully`, `failed`. |
| POST | `/api/smart_home/pairing` | std | | Opens a 15 minute commissioning window. `{"available_until","qr_code","manual_code"}` |
| DELETE | `/api/smart_home/pairing` | std | | Matter factory reset. The device reboots. |
| GET | `/api/smart_home/switch` | std | | `{"state":bool}` |
| POST | `/api/smart_home/switch` | std | `{"state":bool}` and/or `{"startup":"off" or "on" or "toggle" or "last"}` | 400 if neither key is present |

## Cloud account

| Method | Path | Auth | Request | Response |
| --- | --- | --- | --- | --- |
| GET | `/api/account/info` | std | | `{"linked":bool,"id","email","user_id"}` |
| GET | `/api/account/status` | std | | `{"status":"error" or "disconnected" or "connected"}` |
| POST | `/api/account/link` | std | | Held until the cloud returns a PIN. `{"code":"1234","expires_at":N}`. 400 `Already linked` or `Not connected`. 503 after 3 s. |
| DELETE | `/api/account` | std | | Unlinks the account |
| GET | `/api/account/backend` | std | | `{"server_url","client_cert_type","ignore_server_cert"}` |
| PUT | `/api/account/backend` | std | Same JSON | 400 `Malformed request` or `Invalid value` |

# Status WebSocket

`GET /api/status/ws` upgrades to a WebSocket that streams device state as protobuf `StateFrame` messages produced by the `state_publisher` service. The same payload is delivered over BLE and MQTT.

- The client sends text JSON. `{"enable":true}` starts the stream, `{"enable":false}` stops it, `{"send":"all"}` requests a complete snapshot.
- The server sends binary frames. Updates are rate limited to 11 per second with a 100 ms frame interval for screen frames.
- Each client has a queue of 8 messages. The publisher pauses at 6 and resumes at 2. Overflow drops the update.
- The server pings every 10 s and closes the socket when no pong arrives.
- The server allows four clients. It upgrades a fifth client, sends it a `FATAL` / `RESOURCE_LIMIT` error frame, and closes it.

A Python decoder and transport clients live under `tests/clients/state_publisher/`.

# Static content

The server serves files under `/ext/apps_assets/web_server/www` for every non-API path. Directory requests resolve to `index.html`. The build stores compressible files only as `.gz`. When a client sends no `Accept-Encoding`, the server injects `gzip` so the compressed variant is served. When a client explicitly excludes gzip and only the `.gz` variant exists, the server replies 406. Static content accepts `GET` only.

The served tree contains the Nuxt 3 web UI (`index.html`, `login/`, `_nuxt/`), `openapi.yaml`, the Swagger UI at `docs/`, and `api/index.html` which redirects to `../docs/`.

# OpenAPI pipeline

1. The specification consists of one YAML segment per tag in `applications/services/web_server/openapi/` plus a template `openapi.yaml` with `# {PATHS}`, `# {SCHEMAS}` and `# {TAGS}` markers.
2. `scripts/openapi_merge.py merge` combines the segments (duplicate keys are an error) and `validate` checks the result.
3. The build (`scripts/fbt_env_modules/fwenv/bsb_assets.scons`) installs the merged file into the web root and into `dist/` (targets `openapi_spec` and `openapi_dist`).
4. `scripts/swagger.py` wraps the specification with the cached Swagger UI distribution from `assets/swagger/`.
5. CI (`openapi-diff.yml`) runs `oasdiff` against the base branch, enforces a semantic version bump on changes, and checks that `API_VERSION` in `http_api.h` equals the version in `openapi.yaml`.

# Host-side clients

| Tool | Endpoints |
| --- | --- |
| `scripts/update_over_http.py` | `GET /api/status/firmware` (optional intercom version gate), `POST /api/update` |
| `scripts/testops.py update-fw` | `POST /api/update` with retries and optional OpenOCD reset |
| `tests/clients/api/*.py` | Typed clients for every area, used by the integration tests |

`scripts/streaming.py` targets a `ws://<ip>/api/screen/ws` route that no longer exists. Use `GET /api/screen` instead.
