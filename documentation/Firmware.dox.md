# Firmware {#firmware}

The firmware runs on two microcontrollers (see @ref hardware): the STM32U5 main MCU and the SiWG917 wireless co-processor. Both are built from this tree on the same FreeRTOS based runtime. The user facing functionality is split into applications and services (see @ref concepts). The pages below describe each subsystem.

- @subpage architecture -- The two MCUs, the runtime, the service model, the intercom link and the network stack split
- @subpage storage-and-settings -- Partitions, path aliases, the settings library and every settings file
- @subpage user-interface -- Displays, GUI framework, input, desktop and loader, animations, status indicators, brightness, audio, assets
- @subpage busy-timer -- The timer model, cloud synchronization, smart home coupling and the screens
- @subpage connectivity -- Wi-Fi, BLE, MQTT and the cloud account, Matter, USB network, mDNS, TLS and key storage
- @subpage http-api -- Server, access control and the complete endpoint catalogue
- @subpage cli -- Transports and every command
- @subpage updater -- Bundle format, update flow, recovery and factory reset
- @subpage file_formats -- The animation file format

# Feature map

| Feature | Where it lives |
| --- | --- |
| Focus timer with themes, sounds and status lights | `applications/main/busy`, `applications/services/busy_timer` |
| Clock | `applications/main/clock` |
| Settings menu and settings screens | `applications/main/settings_menu`, `applications/settings/*` |
| Two displays and the widget toolkit | `applications/services/gui`, `front_display`, `back_display`, `lib/lvgl_addons` |
| Animations | `lib/anim_file`, `scripts/seq2anim.py` |
| Buttons, mode switch, encoder | `applications/services/input` (both MCUs) |
| Power, charging, USB-PD | `applications/services/power` |
| Wi-Fi | `applications/services/wifi` (both MCUs) |
| BLE with an HTTP tunnel for the mobile application | `applications/services/ble` (both MCUs) |
| Cloud account over MQTT with mutual TLS | `applications/services/mqtt`, `tls_crypto`, `ca_storage` |
| Matter On/Off device | `applications/services/matter` (both MCUs), `lib/matter_glue` |
| USB virtual Ethernet | `applications/services/usb_network` |
| HTTP API, web UI, OpenAPI | `applications/services/web_server`, `assets/frontend` |
| Remote drawing on the displays | `applications/services/canvas` |
| State streaming (WebSocket, BLE, MQTT) | `applications/services/state_publisher`, `assets/proto` |
| JavaScript applications | `applications/services/js_runner`, `applications/system/js_app_launcher`, `lib/js_app` |
| Firmware updates, recovery, factory reset | `applications/system/updater`, `lib/toolbox/update_lib` |
| Command line interface | `lib/cli`, `applications/services/cli_*` |
| Secure key storage and provisioning | `targets/f64/furi_hal/furi_hal_crypto_storage.c`, `applications/system/crypto`, `scripts/*_provision.py` |
| Logging, diagnostics, fault handling | `applications/services/log_storage`, `supervisor`, `device_info` |
