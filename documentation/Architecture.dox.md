# Architecture {#architecture}

This page describes the structure of the firmware: the split between the two MCUs, the runtime that both share, the service model, and the link between them. Hardware details are on the @ref hardware page. The individual subsystems have their own pages under @ref firmware.

# Two MCUs, one source tree

Both microcontrollers run firmware built from this repository, on the same FreeRTOS based runtime, with the same application manifest format. The `targets=` field in each `application.fam` decides on which side an application or service is compiled. Many services have two halves: a U5 half that owns the public API and a Si917 "backend" half that talks to the hardware.

| Responsibility | STM32U5 (`f2x`) | SiWG917 (`f6x`) |
| --- | --- | --- |
| Displays, GUI, animations, fonts | Yes | |
| Storage (eMMC), settings, logs | Yes | NVM3 key-value store and the crypto key storage only |
| Power, charger, USB-PD, RTC, time sync | Yes | |
| USB (CDC-NCM), lwIP IPv4, HTTP server, MQTT, TLS clients | Yes | |
| Wi-Fi association, BLE GATT server | | Yes |
| IPv6 lwIP and the Matter node | | Yes |
| Buttons, mode switch, encoder, RGB status lights | | Yes |
| TLS private keys, device certificates, Matter credentials | | Yes, in the secure key storage |
| Busy timer, updater, JavaScript runtime, CLI | Yes | Small CLI only |

Both images must come from the same git commit. The intercom handshake exchanges the git hash and refuses to start when it differs (see below).

# Runtime

The runtime is "furi", a set of core libraries that lives in the `fbt_layers/core_libs` submodule and wraps FreeRTOS. The firmware uses these primitives everywhere:

- **Threads** (`FuriThread`) with an application id. The application id of the calling thread selects the per-application data directory (see @ref storage-and-settings).
- **Records** (`furi_record_create`, `furi_record_open`): named singletons through which services publish their instance. `furi_record_open` blocks until the record exists, which orders service start-up implicitly.
- **PubSub** (`FuriPubSub`) for broadcast events and **FuriState** for observable values with a current value (Wi-Fi info, switch position, brightness, timer state).
- **Message queues** and **event loops** (`FuriEventLoop`). Almost every service is a single thread that runs an event loop subscribed to an API queue, a few timers and some pubsubs.
- **API lock** (`lib/toolbox/api_lock.h`): the standard pattern for a synchronous call across a queue. The caller posts a message with a lock and blocks until the service thread unlocks it.
- **Logging** (`FURI_LOG_E/W/I/D/T`) with pluggable handlers, and `furi_check` / `furi_assert` / `furi_crash` for invariants.

The memory allocator returns zeroed memory, refuses zero-byte allocations and aborts on allocation failure, so allocation results are not checked in application code.

# Service model

An `application.fam` manifest declares every unit of code (see @ref native-applications for the format). The build (`lib/firmware_applications_u5`, `lib/firmware_applications_si917`) collects the manifests of the selected application set and generates the tables that the start-up code consumes:

| Manifest type | Generated table | Started how |
| --- | --- | --- |
| `SERVICE` | `FLIPPER_SERVICES[]` | One thread per service, in `order` sequence, at boot |
| `STARTUP` | `FLIPPER_ON_SYSTEM_START[]` | A short-lived hook thread, in `order` sequence, after the services |
| `CLICMD` | `FLIPPER_CLI_COMMANDS[]` | Registered in the CLI registry by the CLI start-up hook |
| `APP`, `SETTINGS`, `SYSTEM`, `DEBUG` | `FLIPPER_APPS`, `FLIPPER_SETTINGS_APPS`, ... | Launched on demand by the loader |
| `METAPACKAGE` | none | Groups other applications through `provides=` |

The `FIRMWARE_APP_SET` build variable selects a named set of metapackages, defined in `targets/<target>/fbt_conf/`:

| Set | U5 | Si917 |
| --- | --- | --- |
| `default` | basic_services, extended_services, main_apps, settings, system_apps, debug_apps | basic_services, intercom_services, system_apps |
| `unit_tests` | default plus `unit_tests` | default plus debug_apps |
| `recovery` | basic_services, recovery | basic_services, intercom_services |
| `factory_testing` | default plus `back_display_factory` | Same as default |
| `917_crash` | Same as default | default plus `crash` (a build that crashes on purpose, used by the CI brick-recovery test) |
| `hwtest` | | basic_services, system_apps, cli, cli_uart |

The complete list of services, with their order and purpose, is on the @ref services page.

# Boot to running

1. Reset, HAL early init, NVM boot mode check (see @ref hardware).
2. The scheduler starts. The Init thread runs `furi_hal_init()` and starts every service thread in `order` sequence, then every start-up hook.
3. The `desktop` service starts the `power_on` application and waits for the mode switch position, which arrives from the Si917 over the intercom. If nothing arrives within 1.5 s the position defaults to BUSY.
4. The `desktop` service launches the application that belongs to the switch position (see @ref user-interface).

On the Si917 the sequence is the same without the desktop: the services bring up the radio, the input backend, the BLE and Matter stacks, and open their intercom channels.

# Intercom: the U5 to Si917 link

Service: `applications/services/intercom/` (both MCUs, record `intercom`).

## Transport and framing

- UART with hardware RTS/CTS flow control at 11.25 Mbaud, DMA on both sides. U5 USART1, Si917 USART0.
- Fixed 1024 byte frames: 1 byte channel id, 2 byte data size (0..1019), 1019 bytes of payload, 2 byte CRC. A framing or CRC error is unrecoverable: the intercom logs the error, sets an error status and stops. Only a reset clears it.
- One transmit buffer, one transmission in flight at a time. A transmission times out after 1 s and sets `IntercomStatusErrorTimeout`.

## Start-up and version check

1. The U5 hard-resets the Si917 (reset line low for 20 ms).
2. Each side waits up to 2 s for the peer to assert CTS.
3. Each side sends its control string one character at a time and expects each character echoed back. The control string is the firmware git hash, so images from different commits fail with "Handshake failure, possible version mismatch". The build variable `INTERCOM_FORCE_VERSION=<string>` replaces the hash with a fixed string and disables the check. It is meant for iterating on the U5 firmware without reflashing the Si917 (see @ref build-system).
4. On success the status becomes `IntercomStatusOk`. A receive thread and a heartbeat thread (one meta frame every 5 s, no timeout logic) start.

Clients subscribe to `intercom_get_state()`, initialize on `IntercomStatusOk` and tear down on any error status. The U5 `supervisor` service reacts to an error status by dumping the logs to `/ext/intercom_failure_log.txt` and rebooting the device after a 30 s grace period, unless the NVM debug flag is set.

## Channels

`intercom_channel_open(channel, callback, context)` registers a receive callback and announces the channel to the peer. `intercom_tx()` blocks until the peer announces the same channel, then sends the buffer in 1019 byte chunks. Receive callbacks run in the intercom receive thread and must copy the data out.

| Channel | Users |
| --- | --- |
| `Input` | Button, switch and encoder events from the Si917 |
| `WifiControl`, `WifiData` | Wi-Fi commands and raw Ethernet frames |
| `StatusLights` | RGB LED presets and brightness |
| `Cli` | Remote shell on the Si917 (`sl_cli`) |
| `Ble` | GATT service and characteristic mirroring |
| `CryptoBackup` | Raw backup of the crypto key storage |
| `TlsCrypto` | Certificate retrieval and signing requests |
| `Matter` | Commissioning, switch state, factory reset |
| `SlInfo` | Si917 device information dump at start-up |
| `Debug` | Loopback throughput test (`intercom_test`) |

There is no generic RPC layer and no protobuf on the link. Each channel defines its own C structures in a header shared by both sides. Only the `state_publisher` service uses protobuf (nanopb, `assets/proto`) to serialize device state for WebSocket, BLE and MQTT clients.

## Si917 firmware updates

The Si917 has two images: the M4 application built from this tree, and the NWP radio firmware, a vendor binary. Both are installed by the U5 through the Si917 ROM bootloader over USART2 with the Kermit protocol. See @ref updater.

# Network stack split

There are two independent lwIP instances:

- The **U5** runs IPv4 only, with a DHCP client, a DNS client, IGMP, an mDNS responder and TCP with SACK. It has two network interfaces: `WL` (Wi-Fi) and `EX` (USB CDC-NCM). Mongoose supplies the HTTP server, the MQTT client and the TLS client on top of lwIP sockets.
- The **Si917** runs IPv6 only, without a socket API. It exists for the Matter node.

On the Si917, the Wi-Fi backend inspects every Ethernet frame from the radio. IPv6 frames go to the local stack. It forwards everything else (IPv4, ARP) verbatim over the `WifiData` intercom channel into the U5 `WL` interface. Outbound frames travel the reverse path. Consequences: the U5 has IPv4 only over Wi-Fi, the `WL` MTU is 1005 bytes so a frame fits one intercom payload, and Matter commissioning is IPv6 "on network" only. See @ref connectivity.

# Security model in one view

| Asset | Where | Protection |
| --- | --- | --- |
| TLS device certificate and private key, signing CA | Si917 crypto key storage, slots 0x10 and 0x11 | Wrapped by the Si917 device key when secure boot is on. Signing happens on the Si917. |
| Matter DAC, PAI, SPAKE2+ verifier, setup codes | Si917 crypto key storage, types 13, 14, 15 | Same |
| BLE bond (LTK, IRK) | Si917 NVM3 key `0x10001` | Single bonded peer |
| Wi-Fi passphrase, MQTT account token, HTTP access key, API token hashes | `/ext/apps_data/...` on the eMMC | Clear text (tokens are stored as SHA-256 hashes) |
| Firmware images | Update bundles | Si917 images signed for secure-boot devices. U5 image unsigned. |
| HTTP API | Port 80 | Trusted on USB and localhost. Access mode, key or tokens over Wi-Fi. See @ref http-api. |
| CLI | TCP port 23 | No authentication. USB only unless `sysctl cli_wifi_enabled 1`. |
