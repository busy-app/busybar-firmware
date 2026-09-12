# Connectivity {#connectivity}

This page covers Wi-Fi, BLE, the cloud link over MQTT, Matter, the USB network, service discovery, and the TLS and key storage that these rely on. The @ref architecture page describes the split of work between the two MCUs and the intercom link. The HTTP API has its own page, @ref http-api.

# Wi-Fi

Service: `applications/services/wifi/` (record `wifi` on both MCUs, order 110).

## Roles

- The **Si917** does association: `sl_net` client mode, 2.4 GHz, WPA, WPA2 and WPA3 personal (PSK). The backend reports Enterprise modes as unsupported. The NWP reconnects on its own after a beacon loss. The backend reports `Connected`, `Reconnecting` and `Disconnected` to the U5. While associated it polls link statistics (BSSID, RSSI, channel) every 15 s.
- The **U5** owns the IPv4 stack, DHCP, the saved network and the public API. Raw Ethernet frames cross the intercom `WifiData` channel and enter the `WL` lwIP interface, whose MTU is 1005 bytes. IPv6 frames stay on the Si917 for Matter.

There is no access point mode and no captive portal in the production firmware. Provisioning happens from a PC over USB or from the phone application over BLE, both through the HTTP API. `applications/services/wifi_test/` contains stand-alone Si917 demos that are not part of the normal build.

## API and state machine

`wifi.h`: `wifi_get_state()` returns a `FuriState` of `WifiInfo{ssid, bssid, rssi, channel, security_mode, ip_config, state}`. `wifi_scan()` (only when disconnected, up to 28 results), `wifi_connect(credentials, ip_config)`, `wifi_disconnect()`, `wifi_forget()` (only when disconnected). The service runs one call at a time.

States: `Unknown` (before the intercom is up), `Disconnected`, `Connecting`, `Connected`, `Reconnecting`, `Disconnecting`. Status codes: `Ok`, `Error`, `Timeout`, `AlreadyConnected`, `AlreadyDisconnected`, `ScanNotPossible`, `AccessPointNotFound`, `AuthenticationFailed`, `ConfigurationFailed` (DHCP failure after a 30 s wait).

Connect flow: the credentials go to the Si917. On success the `WL` interface comes up with a static configuration or a DHCP client. Once the interface has an address, the state becomes `Connected` and the service saves the settings. The DHCP hostname is `BUSY Bar` or `BUSY Bar <device name>`. On `PowerEventShutdown` the service disconnects synchronously.

## Saved network

The service stores one network in `/ext/apps_data/wifi/settings.json` (see @ref storage-and-settings), with the passphrase in clear text. `wifi_forget()` resets the file. On boot, a saved SSID triggers an automatic connect.

Consumers of the Wi-Fi state: MQTT (connects only when `Connected`), discovery, the DNS forwarder, the web server, the time service (SNTP on connect) and the Wi-Fi settings application.

# Bluetooth Low Energy

Service: `applications/services/ble/` (record `ble` on both MCUs, order 140).

## Architecture

The GATT server runs on the Si917 (WiSeConnect `rsi_ble_*` APIs). The U5 owns the characteristic content, the on/off policy and the two bridges described below. Both sides compile the same service table. The intercom `Ble` channel mirrors characteristic data with a small command protocol (`Init`, `Deinit`, `Enable`, `Disable`, `GetStatus`, `SetStatus`, `ForgetPairing`, plus per-service `Init`, `Run`, `Update`).

Status (`BleServiceStatus`): `Reset`, `Initialization`, `Ready`, `Advertising` (enabled, not paired, visible to all), `Connectable` (enabled, paired, only the bonded peer can connect), `Connected`, `Error`. The service publishes the state on `ble_get_pubsub()`.

## GATT services

| Service | UUID | Characteristics |
| --- | --- | --- |
| Generic Access | 0x1800 | Device Name (from the `device_name` service, live), Appearance |
| Generic Attribute | 0x1801 | Standard |
| Device Information | 0x180A | Serial Number (hardware UID), Hardware Revision (target), Software Revision (commit) |
| Battery | 0x180F | Battery Level from the power service |
| Nordic UART (NUS) | 6E400001-B5A3-F393-E0A9-E50E24DCCA9E | RX (write), TX (read, indicate), Session counter (read, write) |
| HM-10 UART | 0000FFE0-0000-1000-8000-00805F9B34FB | RX (read, write), TX (read, notify) |

The MTU is 240, so the service splits payloads into 237 byte chunks. Advertising uses the extended API with a legacy PDU, service UUID `0x308A`, manufacturer id `0x0E29` and the local name in the scan response. Once paired, the device advertises with a resolvable private address, an accept list holding the bonded peer, and no name or manufacturer data.

## Pairing and bonding

Pairing is LE Secure Connections "Just Works" (no input, no output). The service supports **one bonded peer** only. It refuses a pairing request from a second device until the first bond is forgotten. It stores the keys (LTK, IRK) in Si917 NVM3 key `0x10001`. `ble_forget()` removes the bond, regenerates the IRK and returns to open advertising. After a connection the stack negotiates 2M PHY and data length extension.

## What the mobile application does over BLE

1. **HTTP over NUS** (`http/ble_http_repeater.c`). The U5 forwards bytes written to NUS RX into a TCP connection to `http://127.0.0.1:80`. It indicates the response back on NUS TX. The Session characteristic counts connections; writing 0 resets it. The phone therefore uses the same REST API as over USB or Wi-Fi. Because the source is localhost, it needs no API token. This is the Wi-Fi provisioning path.
2. **State streaming over HM-10** (`streaming/ble_streaming.c`). While connected, the U5 registers a BLE transport with the `state_publisher` service and notifies protobuf state frames, chunked with a `{num, count, size}` header, rate limited between 1 s and 5 s per frame.

## Settings and API

`/ext/apps_data/ble/settings.json` holds `enabled` (default false), updated on every successful enable or disable so the state persists across reboots. HTTP: `/api/ble/enable`, `/api/ble/disable`, `/api/ble/status`, `DELETE /api/ble/pairing`. The Bluetooth settings application starts a 5 minute pairing window. The service registers no CLI command.

# MQTT and the BUSY account

Service: `applications/services/mqtt/` (record `mqtt`, order 160, U5). Built on the Mongoose MQTT 5 client.

## Connection

| Property | Value |
| --- | --- |
| Broker | `mqtts://mqtt.busy.app:8883` when the configured URL is `default` |
| Protocol | MQTT 5, clean session, keep-alive ping every 10 min |
| Client id | `busybar-<16 hex>`, random, regenerated on unlink |
| Username | `BusyBar device <serial hex>` |
| Password | The account token (empty when not linked) |
| TLS | Server verified against the CA bundle. Mutual TLS with the device certificate from the Si917 key storage (`client_cert_type` `default`), custom PEM files under `/ext/apps_assets/mqtt/` (`custom`), or none. |
| Last will | `{"status":"offline"}` on the presence topic |
| Reconnect | Exponential backoff 2 s to 60 s, only while Wi-Fi is connected |

Status: `Error`, `NotConnected`, `ConnectedNotLinked`, `ConnectedLinked`. A `Not authorized` reply to a subscription while linked wipes the saved account (token revoked).

## Topics

Layout: `<root>/<id>/<dir>/v1/<topic>`, where root and id are `devices/<serial>` (device scope) or `sessions/<session_id>` (session scope, available only when linked), and dir is `up` for publish and `down` for subscribe.

| Scope | Direction | Topic | Content |
| --- | --- | --- | --- |
| device | up | `presence` | `{"firmware_version","api_version","status":"online"}` |
| device | up | `link/request` | Asks for a linking PIN |
| device | down | `link/otp` | `{"code":"1234","expires_at":N}` |
| device | down | `link/token` | `{"session_id","token","email","user_id"}` |
| session | up | `unlink` | Sent when the user unlinks on the device |
| session | down | `gone` | The cloud unlinked the device |
| session | down | `http-request` | Raw HTTP request text with a response topic and correlation data |
| session | down | `stream-request` | `{"message_limits":{"max_count":N,"interval_s":S}}` |
| session | up | `busy/snapshot`, `busy/profiles/busy`, `busy/profiles/custom` | Busy timer state and profiles, see @ref busy-timer |
| session | up | `state` | `{"name":"..."}` when the device name changes |

`mqtt_publish()` and `mqtt_subscribe()` work only in session scope. Supported MQTT 5 properties: expiry interval, response topic, correlation data.

## Modules

- **HTTP proxy** (`modules/mqtt_http_proxy.c`): the module parses each `http-request` message, replays it to `http://127.0.0.1` with a 5 s timeout, and publishes the full response to the response topic. Requests must target `/api/`, must not be WebSocket upgrades, and a block list refuses `POST update`, `DELETE account`, `POST account/link`, `PUT account/backend`, `POST wifi/connect`, `POST wifi/disconnect` and `GET wifi/networks`.
- **State streaming** (`modules/mqtt_streaming.c`): a `stream-request` starts or reconfigures a `state_publisher` transport with the given rate limit and a 60 s default expiry. The module publishes frames at QoS 0 to the response topic.

## Account linking

1. The device is connected to Wi-Fi and to the broker (`ConnectedNotLinked`).
2. The user opens Settings, Account, or a client calls `POST /api/account/link`. The device publishes `link/request`.
3. The cloud answers on `link/otp` with a 4 digit PIN and its expiry. The device shows the PIN. The user enters it at `cloud.busy.app`.
4. The cloud publishes `link/token`. The device stores the session, user, email and token in `/ext/apps_data/mqtt/state.json`, reconnects with the token as password, and reaches `ConnectedLinked`.
5. Unlinking from the device publishes `unlink` and wipes the state. Unlinking from the cloud arrives as `gone`.

Events: `StatusChanged`, `LinkPinReceived`, `LinkDone`, `Unlinked`. The @ref http-api page lists the endpoints under `/api/account`.

# Matter

Service: `applications/services/matter/` (Si917 half `matter_f64.cpp`, order 200; U5 half `matter.c`, order 250, record `matter`). SDK: `lib/matter_ext` (Silicon Labs Matter extension), ZAP generated clusters in `lib/matter_zap`, platform glue in `lib/matter_glue/platform/bsb/`.

## What is exposed

- One endpoint (id 1) with the **On/Off** cluster (including `StartUpOnOff`) and the **Identify** cluster (the status lights blink amber). Vendor id `0x158A` (Flipper FZCO), product id `0xBB01`, product name `BUSY Bar`.
- Commissioning is **on network over IPv6 only**. BLE commissioning is compiled out. The Si917 lwIP instance and the CHIP DNS-SD (`_matterc._udp`, `_matter._tcp`) serve it.
- The U5 sees the switch state as a `FuriState` (`matter_get_switch_state()`). The U5 can set it, set the start-up mode (`off`, `on`, `toggle`, `last`), open a commissioning window, factory reset, and read the fabric count.

## Commissioning flow

1. The settings application (Settings, Smart home, Pair device), the `matter comm` CLI command, or `POST /api/smart_home/pairing` calls `matter_enable_commissioning()`. The UI first checks that Wi-Fi is connected.
2. The Si917 opens a basic commissioning window for 15 minutes and returns the QR payload and the manual pairing code. The back display shows the QR code, the status lights blink white, the front display says "Look at back screen".
3. The commissioner discovers the node over mDNS, runs PASE with the SPAKE2+ verifier from the key storage, device attestation with the DAC, PAI and Certification Declaration, then CASE.
4. Status events (`Started`, `Complete`, `Failed`) update the UI. Fabrics persist in Si917 NVM3 in the reserved key range starting at `0x87000`.

A factory reset (`DELETE /api/smart_home/pairing`, `matter reset`, or the device factory reset) wipes the fabrics and reboots the device.

## Credentials

| Item | Location |
| --- | --- |
| DAC private key (wrapped when secure boot is on), DAC, PAI | Si917 crypto storage, type `MatterAttestation` (13), ids 0, 1, 2 |
| SPAKE2+ salt, verifier, iteration count, discriminator, passcode | Type `MatterSetup` (14), ids 0..4. Read-only at run time. |
| Vendor and product ids and names, part number, URL, serial, manufacturing date | Type `MatterDeviceInfo` (15) |
| Certification Declaration | `/ext/apps_assets/matter/cd-production.der`, `cd-dev.der`, `cd-certification.der`, selected by `/ext/apps_data/matter/cd_selection.txt` (takes effect after reboot) |

`scripts/matter_provision.py` writes these during manufacturing (see Provisioning below). The hardware version comes from OTP; unprovisioned lab samples are reported as version `4.F22.B7.C2` by a workaround in `matter.c`.

CLI: `matter` opens a sub-shell with `switch`, `startup`, `reset`, `comm`, `fabrics` and `cert`.

# USB network

Service: `applications/services/usb_network/` (`usb_srv`, order 20, record `usb_network`, U5).

- Class: **CDC-NCM** only. VID `0x37C1`, PID `0x6213`, product string `BUSY Bar USB Ethernet`. MS OS 2.0 descriptors make Windows bind its inbox NCM driver.
- lwIP interface `EX` with a MAC derived from the OTP USB MAC (last byte XOR 1 on the device side).
- Static device address `10.0.4.20/24` by default, from `/ext/apps_data/usb_srv/settings.json`. A minimal DHCP server (`lib/network/dhserver`, 3 leases, no router or DNS) assigns the host an address on the same subnet.
- Reachable over USB: the HTTP API and web UI on port 80, the CLI on TCP port 23, mDNS. The HTTP API trusts requests that arrive on this interface (see @ref http-api).

State: `UsbNetworkInfo{state Unknown, Down, Up}` via `usb_network_get_state()`.

# Discovery (mDNS)

Service: `applications/services/discovery/` (order 165, U5). The lwIP mDNS responder announces the hostname `<device name>.local` with all non-alphanumeric characters removed and lower cased, so the default name becomes `busybar.local`. The `EX` interface is added when USB comes up, `WL` when Wi-Fi connects, and a rename re-announces. The only registered service is the web server: `_http._tcp` on port 80 with TXT records `path=/` and `name=<device name>`.

# TLS, keys and certificates

## Si917 crypto storage

`targets/f64/furi_hal/furi_hal_crypto_storage.h`: a 20 KiB region in NWP managed common flash, split into a 16 KiB `Main` partition and a 4 KiB `User` partition. Each slot has a header with magic, size, type, flags, id and CRC32. Key types: AES 128/192/256, HMAC SHA1/256/384/512, ECDSA private and public keys for P-224 and P-256, `CsrDerEcdsa256`, `CrtDerEcdsa256`, `MatterAttestation`, `MatterSetup`, `MatterDeviceInfo`. The `Wrap` flag stores a private key wrapped with the Si917 device key. A wrapped key can sign, but nothing can read it out. Wrapping requires secure boot. NVM3 key `0x10002` persists a read-only access mode.

The Si917 CLI command `crypto` (reachable from the U5 through `sl_cli`) manages the slots: `list`, `read`, `write`, `gen` (on-device key generation), `gen_csr` (P-256 key pair plus a DER CSR generated on the device), `wipe`, `protect`. `crypto_backup` on the U5 can `create`, `restore`, `verify` and `remove` a raw copy of the region at `/bkp/crypto_backup.bin`, through the `CryptoBackup` intercom channel, with an enclave self-test after restore.

## tls_crypto service

The Si917 serves the U5 calls `tls_crypto_get_certificate(key_id)` and `tls_crypto_sign(key_id, message)` over the `TlsCrypto` channel. Key id 0 is the signing CA certificate (slot `0x10`), key id 1 is the device certificate and private key (slot `0x11`). Signing is ECDSA-SHA256 in hardware. Limits: 800 byte messages and certificates, 80 byte signatures, one request at a time. The Si917 `supervisor` service signs random data once per second as a health check and crashes after more than 10 consecutive failures.

## TLS client

`targets/f21/mongoose/mongoose_tls.c` implements TLS for every outbound connection (MQTT, the `fetch` HTTP client used by the updater and the `fetch` CLI command, JavaScript `fetch`):

- Mbed TLS 3.x with PSA, TLS 1.2 and 1.3, ECDHE with P-256, P-384, P-521 and X25519, AES-GCM and AES-CCM, SNI from the URL, hostname verification. The RNG is the hardware RNG.
- Server verification against the CA chain from `ca_storage` unless the caller sets `is_server_cert_ignored`.
- Client certificates (`TlsClientCertType`): `None`, `Device` (certificates fetched from the Si917, and a patched `mbedtls_pk_info_t` hook forwards the CertificateVerify signature to the Si917), `Custom` (PEM files). The patches live in `lib/mbedtls_patch/`.

`ca_storage` (start-up hook) parses `/ext/apps_assets/shared/ca/cacert.pem` once at boot. `./fbt update_cacert` refreshes the bundle.

## Provisioning

The host scripts drive the Si917 `crypto` CLI through the U5 CLI on `10.0.4.20:23`:

| Script | Purpose |
| --- | --- |
| `scripts/matter_provision.py` | Wipes the `Main` partition, writes the DAC key (wrapped unless `--insecure-crypto`), DAC, PAI, SPAKE2+ values and device information. Test certificates come from `scripts/test_certs/matter/`. |
| `scripts/mqtt_provision.py` | On-device CSR (`gen_csr`), signed by the local test CA in `scripts/test_certs/mqtt/`, written to slots `0x10` and `0x11`. Refuses to overwrite occupied slots. |
| `scripts/vault_provision.py` | Same slots, but the CSR is signed by a HashiCorp Vault intermediate PKI. |
| `scripts/credentials.py`, `scripts/crypto_storage.py` | Lower level primitives |

`./fbt crypto_provision` runs the Matter and MQTT scripts in order (Matter first, because it wipes the partition) and adds `--insecure-crypto` when `SIL917_INSECURE=True`, which is the default for lab devices without secure boot.

# Time synchronization

The `time` service (record `time`, order 150) keeps the RTC in sync with SNTP through Mongoose. Sync runs 300 s after boot, again when Wi-Fi connects, every 3 h after a success and every 10 min after a failure. The server, intervals, time zone (a `utz` zone name) and 12 h or 24 h format are settings (see @ref storage-and-settings). The API offers `time_get_timestamp()`, `time_get_local_time()` and `time_set_settings()`. CLI: `date [iso8601]`, `timezone [name]`. There is no RTC alarm API.

# Known limitations

- The eMMC holds Wi-Fi passphrases, MQTT account tokens and the HTTP access key in clear text.
- The CLI socket on port 23 has no authentication. Only the network interface origin guards it.
- The intercom does not recover from framing or timeout errors without a reset.
- USB-origin HTTP requests bypass authentication by design, and the BLE tunnel inherits the localhost exemption. BLE pairing is the effective access control for the mobile application.
- Nothing can rotate the Matter setup codes at run time.
