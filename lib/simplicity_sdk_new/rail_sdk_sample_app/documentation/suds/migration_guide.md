Migration Guide
===============

# RAIL SDK 3.0.0

> **Highlights:**
>
> - Uses RAIL LIB 3.0.0
> - PA configuration APIs updated to use RAIL handle parameter
> - Protocol enum `BTC` renamed to `BPSK`

## Breaking Changes in Version 3.0.0

### PA Configuration API Changes

The mode switch PA configuration functions now require or have modified parameters:

```c
// Version 2.19.0 (OLD)
void init_rail_pa_settings(void);
void update_rail_pa_settings(sl_rail_handle_t rail_handle, uint16_t channel);

// Version 3.0.0 (NEW)
void init_rail_pa_settings(sl_rail_handle_t rail_handle);
void update_rail_pa_settings(sl_rail_handle_t rail_handle);
```

**Migration Steps:**
- Update calls to `init_rail_pa_settings()` to pass the RAIL handle
- Remove the `channel` parameter from calls to `update_rail_pa_settings()`

### Protocol Enum Rename

The protocol enum value `BTC` has been renamed to `BPSK`:

```c
// Version 2.19.0 (OLD)
RAIL_SDK_Protocol_t protocol = BTC;

// Version 3.0.0 (NEW)
RAIL_SDK_Protocol_t protocol = BPSK;
```

### Removed Function

The function `sl_rail_sdk_wmbus_phy_software()` has been removed as software-based PHY processing is no longer required.

---

# RAIL SDK 2.19.0

> **Highlights:**
>
> - Uses RAIL LIB 2.19.0
> - Examples now use `sl_main` instead of `sl_system`

## Detailed API Change

- **API Naming:**
  - Switched from `CamelCase` to `snake_case` for most APIs and typedefs.
  - All public APIs now use `sl_rail_handle_t`, `sl_rail_status_t`, etc., instead of the old `RAIL_Handle_t`, `RAIL_Status_t`, etc.

## Breaking Changes in Version 2.19.0

**Example of API Change:**

```c
// Version 2.18.2 (OLD)
uint16_t unpack_packet(
    uint8_t *rx_destination,
    const RAIL_RxPacketInfo_t *packet_information,
    uint8_t **start_of_payload,
    phy_modulation_e modulation
);

// Version 2.19.0 (NEW)
uint16_t unpack_packet(
    sl_rail_handle_t rail_handle,     // New first parameter
    uint8_t *rx_destination,
    const sl_rail_rx_packet_info_t *packet_information,  // Renamed type
    uint8_t **start_of_payload,
    phy_modulation_e modulation
);
```

**Manual Update for Range Test Projects:**
   - If your project directory name contains `range_test`, you should manually:
     - Remove any lines declaring `static RAIL_Handle_t rail_handle;` from all `.c` and `.h` files.
     - Remove `rail_handle` from function parameter lists, regardless of its position (beginning, middle, or end).
     - Remove any assignments to `rail_handle` and any standalone usages of `rail_handle` in your code, to ensure no obsolete references remain.

---

# RAIL SDK 2.18.0

## Directory and SDK Structure Changes

- **SiSDK-2024.12 Release:**
  - RAIL and Connect SDKs are now separated.
  - The former `FLEX SDK` is split into:
    - `RAIL SDK`
    - `Connect SDK`
  - Directory changes: files moved from `app/flex` to `app/rail` and `app/connect`.

## Component and API Renames

- **Component file names and APIs updated:**
  - Any occurrence of `flex` in APIs or definitions updated to match the new structure.

## Table of Changes

| Old File | New File | Changes | Details |
|---|---|---|---|
| simple_rail_assistance.h | sl_rail_sdk_simple_assistance.h | - | |
| simple_rail_heartbeat.h | Deleted | - | |
| simple_rail_rx.h | Deleted | - | |
| simple_rail_rx_cli.h | Deleted | - | |
| simple_rail_tx.h | Deleted | - | |
| sl_flex_mode_switch.h | sl_rail_sdk_mode_switch.h | Rename API: prepare_package -> prepare_packet | |
| sl_flex_ieee802154_support.h | sl_rail_sdk_ieee802154_support.h | Change part of names: FLEX -> RAIL_SDK , flex -> rail_sdk | |
| sl_flex_rail_package_assistant.h | sl_rail_sdk_packet_assistant.h | Rename API:  prepare_package -> prepare_packet | |
| sl_flex_packet_asm.h | sl_rail_sdk_packet_asm.h | Change part of names: FLEX -> RAIL_SDK, flex -> rail_sdk | |
| sl_flex_rail_sleep.h | sl_rail_sdk_sleep.h | Rename API: sl_flex_rail_sleep_init -> sl_rail_sdk_sleep_init | |
| sl_flex_util_802154_init.h | sl_rail_sdk_util_802154_init.h | Change part of names: sl_flex_util_init -> sl_rail_sdk_util_init, sl_flex_util_get_handle -> sl_rail_sdk_util_get_handle | |
| sl_flex_util_ble_init.h | sl_rail_sdk_util_ble_init.h | Change part of names: sl_flex_util_init -> sl_rail_sdk_util_init, sl_flex_util_get_handle -> sl_rail_sdk_util_get_handle | |
| sl_flex_util_ble_protocol.h | sl_rail_sdk_util_ble_protocol.h | Rename include sl_flex_util_ble_protocol_config -> sl_rail_sdk_util_ble_protocol_config | |
| sl_rail_simple_cpc.h | sl_rail_sdk_simple_cpc.h | Change part of names: rail -> rail_sdk | |
| sl_wmbus_packet_assembler.h | sl_rail_sdk_wmbus_packet_assembler.h | Change part of names: wmbus -> rail_sdk_wmbus | |
| sl_wmbus_sensor_pulse_counter.h | sl_rail_sdk_wmbus_sensor_core.h | Change part of names: wmbus -> rail_sdk_wmbus | |
| sl_wmbus_sensor_pulse_counter.h | sl_rail_sdk_wmbus_sensor_pulse_counter.h | Change part of names: wmbus -> rail_sdk_wmbus | |
| sl_wmbus_sensor_thermometer.h | sl_rail_sdk_wmbus_sensor_thermometer.h | Change part of names: wmbus -> rail_sdk_wmbus | |
| sl_wmbus_sensor_virtual_water_meter.h | sl_rail_sdk_wmbus_sensor_virtual_water_meter.h | Change part of names: wmbus -> rail_sdk_wmbus | |
