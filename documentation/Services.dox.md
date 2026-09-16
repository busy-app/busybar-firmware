# Services {#services}

Services are background threads and start-up hooks declared in `applications/services/application.fam`. They start in `order` sequence at boot and publish their instance through a record. The @ref architecture page describes the mechanism. This page is the catalogue: what each service does and where it is documented in detail.

# STM32U5 services

The `basic_services` metapackage (storage, input, gui, front and back display, font registry, power, status bar, brightness control, device info, log storage) is present in every U5 application set, including recovery. `extended_services` adds everything else.

| Order | Service | Record | Purpose | Details |
| --- | --- | --- | --- | --- |
| 1 | `cli_intercom` | `cli_intercom` | Relays a shell to the Si917 over the intercom (`sl_cli`) | @ref cli |
| 3 | `log_storage` | `log_storage` | Two 16 KiB log rings (local U5 log and the Si917 console on USART2) and `log_dump` | @ref cli |
| 10 | `storage` | `storage` | eMMC filesystems, path aliases, file API | @ref storage-and-settings |
| 10 | `device_info` (hook) | `device_info` | Registry of device information key-value segments | @ref cli |
| 15 | `sl_info` (hook) | `sl_info` | Caches the Si917 device information received over the intercom | @ref architecture |
| 20 | `cli_u5` (hook) | `cli` | Creates the CLI registry and registers every command | @ref cli |
| 20 | `usb_srv` | `usb_network` | TinyUSB CDC-NCM, the `EX` network interface, a DHCP server | @ref connectivity |
| 30 | `intercom` | `intercom` | The link to the Si917 | @ref architecture |
| 30 | `network` (hook) | `network` | lwIP initialization, Mongoose DNS, `netstat` | @ref architecture |
| 35 | `sysctl` (hook) | | Dev mode, UI debug overlay, CLI over Wi-Fi, access log level | @ref cli |
| 40 | `power_srv` | `power` | BQ25798 charger, USB-PD, state of charge, battery state machine, power off and reboot | below |
| 40 | `cli_socket` (hook) | | The CLI on TCP port 23 | @ref cli |
| 45 | `device_name` | `device_name` | The user-visible device name, validated and persisted, published to MQTT and BLE | @ref storage-and-settings |
| 50 | `supervisor` | | User-facing fault handling: battery, storage, intercom errors | below |
| 60 | `input` | `input`, `input_events` | Logical input events from the Si917 | @ref user-interface |
| 60 | `font_registry` (hook) | `font_registry` | Font loading and cache | @ref user-interface |
| 65 | `canvas` | `CANVAS` | Remote drawing overlay for the HTTP display API | @ref user-interface |
| 70 | `back_display` | `back_display` | SSD1320 OLED driver service | @ref user-interface |
| 80 | `front_display` | `front_display` | LED matrix driver service | @ref user-interface |
| 90 | `light_sensor` | `light_sensor` | Ambient light level | @ref user-interface |
| 95 | `brightness_control` | `brightness_control` | Auto and manual brightness for displays and lights | @ref user-interface |
| 100 | `audio` | `audio` | PCM playback, volume | @ref user-interface |
| 110 | `wifi` | `wifi` | Wi-Fi state machine, saved network, the `WL` interface | @ref connectivity |
| 120 | `loader` | `loader` | Runs one application at a time | @ref user-interface |
| 130 | `status_lights` | `status_lights` | RGB LED presets, forwarded to the Si917 | @ref user-interface |
| 140 | `ble` | `ble` | BLE policy, HTTP tunnel and state streaming over GATT | @ref connectivity |
| 150 | `time` | `time` | RTC, SNTP, time zones | @ref connectivity |
| 160 | `mqtt` | `mqtt` | Cloud link, account linking, HTTP proxy and streaming modules | @ref connectivity |
| 165 | `discovery` | `discovery` | mDNS hostname and service records | @ref connectivity |
| 180 | `tls_crypto` | `tls_crypto` | Certificates and signing from the Si917 key storage | @ref connectivity |
| 185 | `state_publisher` | `state_publisher` | Aggregates device state into protobuf frames for WebSocket, BLE and MQTT | below |
| 190 | `busy_timer` | `busy_timer` | The timer model, cloud sync, Matter coupling | @ref busy-timer |
| 200 | `gui` | `gui` | LVGL, layers, widgets, input dispatch | @ref user-interface |
| 230 | `status_bar` | `status_bar` | Indicators on the back display | @ref user-interface |
| 240 | `desktop` | `desktop` | Mode switch to application mapping, transitions | @ref user-interface |
| 250 | `matter` | `matter` | Matter switch, commissioning, credentials | @ref connectivity |
| 260, 270 | `mqtt_streaming`, `mqtt_http_proxy` | | MQTT modules | @ref connectivity |
| 300 | `recovery` | | Recovery firmware only: runs a factory reset | @ref updater |
| 300 | `update_executor` | | Updater stage only: flashes the images | @ref updater |
| 310, 320 | `updater`, `update_ui` (hook) | `updater` | Update check, download, staging, automatic updates, update screens | @ref updater |
| 330 | `low_power` | `low_power` | Reference counted lock. At zero it sleeps the displays and the light sensor. | below |
| | `web_server` | | Mongoose HTTP server and the API | @ref http-api |
| | `api_tokens` | `api_tokens` | Bearer tokens for the HTTP API | @ref http-api |
| | `ca_storage` (hook) | `ca_storage` | Parses the CA bundle once at boot | @ref connectivity |
| | `js_runner` | `js_runner` | JerryScript runtime and the `js` command | @ref javascript-applications |
| | `startup_dfu_hook` | | Si917 only, listed here for completeness | @ref hardware |

`cdc_echo` and `cdc_screen` are unused USB CDC test services. `intercom_test` and `wifi_test` are demo services that are not part of any application set.

# SiWG917 services

`basic_services`: cli, input, network, wifi, nvm, matter, startup_dfu_hook, device_info. `intercom_services`: status_lights, cli_intercom, ble, tls_crypto, sl_info, supervisor.

| Order | Service | Purpose |
| --- | --- | --- |
| 1 | `cli_intercom` | Shell endpoint for `sl_cli` |
| 5 | `startup_dfu_hook` | DFU button handling at boot |
| 10 | `device_info` | Device information registry |
| 20 | `cli_si917` | CLI registry |
| 30 | `intercom`, `network` | Intercom transport, IPv6 lwIP |
| 80 | `input` | Buttons, switch, encoder |
| 90 | `tls_crypto` | Serves certificates and signatures from the key storage |
| 100 | `supervisor` | Signs random data every second and crashes after 10 failures |
| 110 | `wifi` | Radio association, frame bridging |
| 120 | `nvm` | NVM3 key-value store |
| 130 | `sl_info`, `status_lights` | Device information dump, RGB LED backend |
| 140 | `ble` | GATT server |
| 200 | `matter` | Matter node |
| | `cli_uart` | UART shell in the `hwtest` set |
| | `crash` | Crashes on start in the `917_crash` set |

# Services without a dedicated page

## power

`applications/services/power/` (record `power`). A 1 s tick reads the BQ25798 status and ADC, low-pass filters voltage and state of charge (charge only moves in the charging direction while charging), and publishes `ChargingStateUpdate`, `ChargeAmountUpdate`, `BatteryPresent`, `BatteryNotPresent` and `UsbConnectionStateUpdate`. The service applies the charge limit setting with a 10% re-enable hysteresis.

Battery state machine: `Normal`, `Low` (below 15%) and `Critical` (below 5%) with a 3% hysteresis; charging forces `Normal`. Each transition publishes a `Start` and `Stop` event pair.

`power_off()` refuses while USB is connected. `power_reboot(mode)` supports `Hardware` (charger reset), `Normal` (reset the Si917 then the U5), `NormalU5`, `Normal917`, `DfuU5` (jump to the ST ROM bootloader) and `Dfu917` (Si917 into its ROM bootloader). Shipping mode: at boot, if the NVM flag is set, the service waits for USB to be unplugged and powers off. CLI: `power`.

## supervisor

`applications/services/supervisor/supervisor.c` shows fault screens on the GUI system layer of both displays, locks input while a blocking fault is shown, and holds the low power lock while any warning is active:

| Warning | Trigger | Screen and action |
| --- | --- | --- |
| Battery not ready | Battery not present at boot | "Battery issue, contact support", input locked |
| Battery critical | `PowerEventBatteryCriticalStart` | "Connect charger", 30 s countdown, then power off |
| Rebooting | Matter `WillReboot` | Spinner |
| Storage errors | `/bkp` or `/ext` failed to mount | "Storage error", OK formats or repartitions, resets NVM, reboots |
| Intercom error | Any intercom error status | "System error, restart device". Dumps logs to `/ext/intercom_failure_log.txt`, reboots after 30 s of uptime unless Dev mode is on. |

## state_publisher

`applications/services/state_publisher/` (record `state_publisher`, 16 KiB stack plus a fetch thread). It collects device state from the brightness, time, Wi-Fi, updater, power, audio, device name, Matter, input, busy timer and BLE services, and front display frames from `screen_streamer.c` (RGB888, L8 or L4, optionally run length encoded). The service serializes updates as protobuf `BSB_State_StateUpdate` messages (nanopb, schema in the `assets/proto` submodule) and fans them out to up to 16 registered transports (`WebSocket`, `BLE`, `MQTT` classes), each with its own frame interval and rate limit. A heartbeat goes out every 991 ms. `send_complete_snapshot` sends a full `BSB_State_State`.

## low_power

A reference counted lock (`low_power_lock()`, `low_power_unlock()`) that starts at 1. When the count reaches 0 the service puts both displays and the light sensor to sleep; any lock wakes them. It does not touch MCU clocks. `soft_off` releases the initial lock, the supervisor and the canvas take locks while active.

## log_storage

Keeps a 16 KiB ring of the U5 log (through a log handler) and a 16 KiB ring of the Si917 console received on USART2 at 230400 baud. `log_storage_dump(path)` writes the device information and both rings to `/ext/log.txt` or the given path (only complete lines). `log_storage_suspend_remote()` releases the UART so the updater can use it. There is no on-device crash dump; the test bench detects crashes through the serial console.

## device_info, sl_info, device_name

`device_info` holds an ordered list of key-value segments that the CLI, the About screen and the log dump print. The U5 keys carry the prefix `u5_` (commit hash, branch, version, build date, target, API version). The Si917 keys carry the prefix `sl_` (the same set, plus Wi-Fi and BLE MACs, NWP firmware version, secure boot, signature, encryption and anti-rollback flags, enclave validity) and arrive through `sl_info` over the intercom at boot; until then `sl_intercom_status` reports `error`.

`device_name` validates the name (20 characters, printable ASCII, not only spaces), persists it, publishes it as a `FuriState`, sends `{"name": ...}` on the MQTT `state` topic, and feeds the BLE Device Name characteristic, the DHCP hostname and the mDNS hostname.
