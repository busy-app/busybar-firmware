# Important Changes {#important-changes}

The changes described in this file will possibly break the build and/or functionality of an
existing application. The description serves the purpose of helping to fix the failing build.

# 8.0.1 {#section-8-0-1}

## Documentation update

This document is discontinued.
Information about important changes can now be found in the [Release Notes](https://docs.silabs.com/z-wave/8.0.1/sisdk-zwave-release-notes).

# 8.0.0 {#section-8-0-0}

Version has been bumped to 8.0.0 to reflect the major changes in the stack. In the future the stack version will follow the semantic versioning convention.

## v255 firmware images

From this release v255 firmware images are no longer available in Simplicity Studio. Manual generation of v255 firmware images is still possible from the example projects.

## Frequently Listening Node Inclusion

Fixed a bug where the controller would not store beam routes during inclusion, which could cause frequently listening nodes to become unjoinable if they stopped the inclusion process early. The controller now stores beam routes immediately during inclusion. As a result, the controller may beam the frequently listening node during inclusion even when not strictly necessary, potentially adding a slight delay to the inclusion process.

## Firmware Update - Storage Slot Erase at OTA Start

Starting with this version, when receiving the first **COMMAND_CLASS_FIRMWARE_UPDATE_MD (0x7A) / FIRMWARE_UPDATE_MD_REQUEST_GET (0x03)** command at the beginning of an OTA update, the device will immediately erase the firmware storage slot to prepare for the new firmware image. This operation ensures a clean storage area before writing the new firmware data.

**Impact on Response Time**: The storage erase operation can introduce a noticeable delay in the device's response time, which varies significantly depending on the type of memory used:

- **Internal SRAM/Flash storage**: Erase time is typically very fast (~300 ms per erase operation). With this configuration, the delay should be negligible for most applications.

- **External Flash storage**: Erase time is significantly longer (~6000 ms for erasing a full slot, ~8000 ms for erasing an empty/already erased slot). This can result in a delay of several seconds before the device responds to the first MD GET command.

**Benchmark Results** (256 KB storage slot):
- Internal storage (BRD4401C with bootloader v3.3.2): 302-305 ms (full or empty)
- External storage (BRD4401C with bootloader v3.3.0): 5848-5994 ms (full), 8034-8229 ms (empty)

Controllers initiating OTA updates should be prepared to handle longer response times, especially when updating devices using external flash storage.

## Radio Layer Regulatory Compliance Update

The radio layer has been refactored to enforce strict compliance with regional regulatory requirements. Clear Channel Assessment (CCA) thresholds and procedures have been updated to meet the specifications defined by regulatory bodies in all supported regions:

- **EU regions**: CCA threshold set to -77 dBm (-69 dBm for LR), with 160 µs duration, in accordance with ETSI EN 300.220-2
- **Japan**: CCA threshold aligned with ARIB T-108 requirements
- **Korea**: CCA threshold and duration configured per local regulations
- **Other regions**: Compliance with Z-Wave PHY Layer specifications (-80 dBm for classic channels, -60 dBm for LR channels)

These stricter regulatory requirements impact the radio transmission characteristics and may affect fragmented beam transmission performance in high-interference environments.

## Fragmented Beam Transmission - Robustness Limitations

The fragmented beam transmission feature in this release has been implemented with basic functionality but has known limitations regarding robustness to RF interference. The stricter regulatory compliance requirements applied to the radio layer (see Radio Layer Regulatory Compliance Update above) contribute to these limitations. In environments with high interference levels, devices may experience transmission failures where the Beam ACK frame is not successfully sent, potentially resulting in the device being unable to complete the beam transmission sequence.

An enhanced solution addressing these robustness limitations is under validation. Improvements are expected to be integrated in a future release to provide more reliable fragmented beam transmission in challenging RF conditions.

## Power Management Commands - Lock Behavior

- The permanent I/O lock can only be acquired once. Subsequent requests with timeout=0 will be ignored if the lock is already held.
- Only one temporary lock can be held at a time. Requesting a second temporary lock will cancel and replace the first one.
- Temporary locks cannot be revoked using `FUNC_ID_POWER_MANAGEMENT_CANCEL`. They can only expire naturally after their timeout period.
- If a permanent lock is already held, requesting a temporary lock will not prevent the temporary lock mechanism from working - they operate independently.

## PTI Configuration
- Starting with this version, the PTI feature is mainly configured in sl_rail_util_pti component.
- The whole PTI configuration is loaded from rail_util_pti component. 
- PTI is disabled by default in all sample applications, except for the Zniffer. It can be overridden using the ENABLE_RADIO_DEBUG param in zw_config_rf.h.
- It's not possible to disable PTI on the Zniffer application.
- The user is now free to align PTI pins with the pinout of their board.

## Removal of zpal_power_manager module
- The zpal_power_manager module is removed from Z-Wave. To handle the power management state of the platform, refer to the Silicon Labs [`sl_power_manager`](https://docs.silabs.com/gecko-platform/3.0/service/api/group-power-manager) module APIs.

- The Z-Wave stack is now independent from the EM state of the platform, meaning that using `sl_power_manager` APIs will not interfere with Z-Wave stack logic.

- Previous to this release, the EM1 state was required during radio operations (RX). Now, the [EM1P](https://docs.silabs.com/rail/latest/efr32-migration-guide-for-proprietary-apps/04-em1p-on-efr32xg22) energy mode may be maintained during radio RX operations. This could improve the overall power consumption of the application.

  - Note: there may be peripherals that do not work in EM1P mode. For example, timers used by the PWM component in Led Bulb and Power Strip applications only work in EM1 mode. Due to this, an EM1 requirement was added to all always-on applications (see `app.c` of these apps). This may require manual steps during migration of old projects, or it can be removed to reduce power consumption if EM1P mode is sufficient and no peipherals are affected. See also at the [migration guide](migration_guide.md#energy-modes).

- A new `zw_shutdown_manager` component module has been introduced to manage shutdown mode (EM4) locks, replacing the deep sleep functionality previously provided by zpal_power_manager.

- New radio stay awake APIs have been introduced: `zpal_radio_request_stay_awake`, `zpal_radio_update_stay_awake`, and `zpal_radio_revoke_stay_awake` replace the previous `zpal_pm_lock(ZPAL_PM_TYPE_USE_RADIO)` mechanism for keeping the radio in receive mode.

## Removal of enterPowerDown() and exitPowerDown() hooks
FreeRTOS power down hooks (`configPRE_SLEEP_PROCESSING` and `configPOST_SLEEP_PROCESSING`) related to the zpal_power_manager module have been removed. With the removal of zpal_power_manager, applications should now use the Silicon Labs `sl_power_manager` API directly to manage power states, or use `zw_shutdown_manager` for controlling entry into shutdown mode.

## CLI
- Now that all applications can make use of EM1P, the CLI has been configured to rely on the LFRCO clock (which implies a lower baud rate of 9600) for SoC applications. Therefore, the CLI will remain active in EM1P.
  - Exceptions are the sample applications for BRD4204A board. The EUSART pins used for VCOM on this board don't operate in low-energy mode. Therefore, on this board, the default configuration of sample applications is the following:
    - Always-on apps stay in EM1 (as stated in [Removal of zpal_power_manager module](#removal-of-zpal_power_manager-module)).
    - Frequently listening app's CLI is configured to use a high frequency clock by default, which will keep the app in EM1.
    - Sleeping reporting apps enable the newly added ZW_CLI_SLEEPING_WAKEUP_EM1 configuration.
- New configuration value was added in zw_cli_sleeping component. When ZW_CLI_SLEEPING_WAKEUP_EM1 is enabled, the application will stay in EM1 during wakeup or when sleeping is disabled through CLI command.

## TX POWER
The ADJUST_RAIL_TX_POWER (1.4 dBm) empirical offset in the zpal_radio layer has been removed. In the case of OTA updates from versions prior to 8.0 to higher versions, TX power values need to be adjusted.

Moreover, the zpal_radio layer has been refactored to comply strictly with regulatory rules, including TX power levels for EU and US regions, and LBT and duty cycle rules for JP and KR regions.

## TX Power types and definition
ZPAL and Z-Wave stack manipulate RSSI level and Tx Power in 3 different unit (enum, dbm, deci-debm). To avoid confusion, clarify some type and definitions.
zpal_tx_power_t > zpal_tx_power_decidbm_t
ZW_TX_POWER_xxDBM  > ZW_TX_POWER_xx0DDBM

## ENTROPY GENERATOR
The zpal_entropy API is now based entirely on the RNG, and radio is no longer used to generate random data.

## ZPAL Zniffer API
The zpal_radio_get_last_beam_info API has been added to allow beam management from the stack layer in the Zniffer application.

## Noise Detection Module Refactoring
Noise detection management has been moved from the Z-Wave stack layer (`ZW_noise_detect`) to the ZPAL layer (`zpal_noise`). This refactoring improves modularity and allows for better platform-specific implementations.

## New Dynamic Tx Power algorithm
The new algorithm stores the output power for all LR nodes. Output power is stored in RAM only.
The controller and end device manage their power independently to be able to adapt to different noise levels.
To reduce latency, the third transmission always uses maximum output power.
This standard algorithm can be overridden by the customer to suit their specific use case.

## Tx Power in LR ACK frame
The Tx Power in LR ACK frame is now the tx power used to send the ACK frame instead of the tx power of
the singlecast frame. It might be different especially when the singlecast frame was sent with a higher
power than the capability of the device (e.g. The singlecast frame was sent at 19dBm, but the ACK frame
is sent at the device's maximum output: 14dBm).

## Improve frame transmission validation
Nodes won't be able to send frames with their own node ID as the destination node ID anymore.

# 7.24.1 {#section-7-24-1}

## User Credential Command Class

The return type of function `CC_UserCredential_delete_all_credentials_of_type`
has been changed from `void` to `bool` to indicate whether any credentials were deleted.  
If you override this function, update your implementation to return a boolean value.

# 7.24.0 {#section-7-24-0}

## Improve Clear Channel Assessment
Change the clear channel assessment configuration to improve performance.

## S2V2 covered commands update according to specification

The following Z-Wave protocol command classes are removed from the NLS coverage list:
- `0x22 ZWAVE_CMD_SET_NWI_MODE`
- `0x23 ZWAVE_CMD_EXCLUDE_REQUEST`

## Restore NVM migration script
Fix the issue about missing NVM migration script in the controller.

## Restore NVM handler after backup/restore operations
To avoid NVM acces by the stack or the application during backup/restore operations, the NVM3 handler is closed by the SAPI command NVMBackupRestoreOperationOpen. However it was never restored.
The NVM3 handler of the stack is restored by the SAPI command NVMBackupRestoreOperationClose. It should no longer be required to reset the controller after a backup operation. The controller must always be restarted after a restore operation.
The SAPI command NVM backup close now send an answer with the result of the operation (as specified). In case of error when restoring the NVM3 handler, the controller wil answer an error. In this case, the controller should be reset.
*Breaking change*
zpal_nvm_backup_close return a zpal_status_t instead of void.

## Power Manager new implementation

### APIs update
*Breaking change*  
- Changes at the application level
  - Power Manager operations are now divided into three domains: STACK, PAL, and APP.
  - New APIs (zw_power_manager_(lock/relock/is_active/lock_cancel)) have been introduced to manage power operations within the APP domain, serving as a wrapper to PAL code.
  **Important**: we strongly recommend any application code to use this wrappper instead of calling ZPAL APIs.
  - Each power lock is now identified by a unique ID. These lock IDs are defined in the zw_power_manager_ids.h header file through enums. For the APP domain, ZPAL_PM_APP_RADIO_APPLICATION_ID is used for radio locks and ZPAL_PM_APP_DEEP_SLEEP_APPLICATION_ID is used for DeepSleep locks. If additional locks are needed at the application level, new IDs must be defined in zw_power_manager_ids.h.
- Changes in the ZPAL
zpal_pm_register API have been deprecated and zpal_pm_register_domain introduced
zpal_pm_stay_awake has been deprecated for zpal_pm_lock (change in behavior, see zpal_power_manager.h for more details).
zpal_pm_cancel has been moved to zpal_pm_lock_cancel
zpal_pm_relock, zpal_get_max_timeout and zpal_pm_lock_is_active, zpal_pm_lock_type_is_active API has been introduced

Notable changes:
- *locks are now registered  per domain: APP, STACK and PAL*
- possible operations on those power locks are defined as follows:
- a power lock can be acquired initially with a lock operation (zpal_pm_lock) with timeout_ms being of value 0 (permanent) or non-zero (timer).
  - if already acquired (!= INACTIVE state) a lock operation on that given power lock will fail.
- relock operation does the same as lock but enforces the acquisition of a given lock from any state.
- lock_cancel will cancel the last operation on a given lock resulting in a INACTIVE state.
- zpal_get_max_timeout has been introduced for one to check maximum allowed value for timeout_ms.
- zpal_pm_lock_is_active has been introduced for one to check current state of individual power lock. This represents the current state (ACTIVE_PERMANENT, ACTIVE_TIMER, or INACTIVE) of a lock.
- zpal_pm_lock_type_is_active has been introduced for one to check global current state of power lock type: RADIO or DEEP_SLEEP.

## Introduce firmware update migration callback for Command Classes

New macro `REGISTER_CC_V6()` has been created and can be used to initialize Command Classes. This macro extends `REGISTER_CC_V5()` in two ways:
  - Introduced `migrate()` callback function similar to `init()` or `reset()`. `migrate()` can be used to execute any migration that is necessary for a command class during firmware upgrade. `migrate()` is called by Firmware Update Command Class, after the new firmware is loaded, but before the Firmware Update Status Report is transmitted.
  - V6 allows the command class to choose the `cc_api_handler_version`. This allows command classes to update its registration to `REGISTER_CC_V6()` without the need to change the CC handlers. For example:
    - `REGISTER_CC_V5(...)` is equivalent to `REGISTER_CC_V6(CC_API_HANDLER_V3, ..., NULL)`
    - `REGISTER_CC_V4(...)` is equivalent to `REGISTER_CC_V6(CC_API_HANDLER_V2, ..., NULL)`

## New ZAF File ID for User Code CC

`ZAF_FILE_ID_CC_USER_CODE_HEADER` has been created to store a new database for the User Code Command Class. The database stores flags about the validity of the corresponding User Code, which makes the NVM initialization significantly faster when many user codes are used. This removes the limitation of `CC_USER_CODE_MAX_IDS`, increasing its range to 255.

## Controller's NVM application version
In controller's NVM, application's datas have their own version number (instead of the application version).
ZAF_FILE_ID_APP_VERSION store the app version and an integer with the data version (like FILE_ID_ZW_VERSION).
From Z-Wave Alliance https://github.com/Z-Wave-Alliance/z-wave-stack/issues/441


## Door Lock Keypad default bolt state
The default bolt state of the Door Lock Keypad application was changed to unlocked.  
Unlocking the door bolt is no longer possible when the device is not included into a network.

## CLI change in DoorLockKeypad
For the Door Lock Keypad application, the CLI command `sleeping <enable|disable>` was removed, because the CLI can now be invoked even when the device is in the EM2 sleep state. If the CLI is used from Simplicity Commander, no modifications are required. If the CLI is used from a serial terminal, make sure that the chosen baud rate is 9600 bps.

On Development Kits, if the board was flashed with an application whose baud rate is not 9600 bps, the CLI will fail. To fix this, press the reset button on the development kit. This will reset the WSTK/WPK firmware as well, and the automatic baud rate will be recognized automatically.
Remotely, the reset can be triggered by the Simplicity Commander command `commander device reset -s <jlink_serial>`. Alternatively, the baud rate can be set through the development kit's admin port. The admin port can be accessed via Silink, via the Simplicity Commander over ethernet, or via Simplicity Studio (right click on the board and `Launch Console`). To set the baud rate, use the command `serial vcom config speed <baud_rate>`.

## ZPAL Watchdog
New module initialization function, zpal_watchdog_init(), added to zpal WDT module.
- The user needs to call this function before enable the watchdog.

## Implement new EU_LR frequency
Shifted EU_LR frequency from 864.4/866.4 Mhz to 864.0/866.0 Mhz to comply with ETSI regulation rules.


## Exclusion from foreign network
After an exclusion from a foreign network, the controller sent nops. However, nops were sent on the controller's network instead of the foreign network.
So according to the network specification (NWK:01C8.1 & LR-NWK:0078.1), the controller no longer sends nops in case of foreign exclusion.

## Logging System
*Breaking change*  
The macros `DPRINT` and `DPRINTF` have been replaced by `ZPAL_LOG` which has two additional parameters:
the originating component and the log level severity.
Logs can now be enabled selectively per component and messages can be routed to different channels based on the log level (debug, info, warning, error).
To configure log messages, use the *Configure* button of the *Z-Wave Log* component in Simplicity Studio
or edit `zw_log_config.h` directly in the project's `config` directory.

## Removal of main.c from Sample Applications
`main.c` was removed from all sample applications and is now generated automatically by the *System Setup (sl_main)* component during project generation.
For more information, refer to the documentation for *System Initialization and Action Processing*.

## User Credential Command Class
*Breaking change*  
The specification for the *User Credential Association Set* and
*User Credential Association Report* commands was updated in the 2025A certification program:
the option to move a credential to a different slot was removed.  
The end device implementation was aligned to the new specification.
As a result, the API for `CC_UserCredential_move_credential_and_report` changed.  
Moving a credential to a different slot is still possible via the CLI;
however, the lifeline group can no longer be notified about a credential slot change.

# 7.23.0 {#section-7-23-0}

## S2V2 Alpha
A first implementation of the Security 2 Version 2 specification.
Alpha release implements the following NLS commands:
- `KEX Report NLS Support Bit`
- `NLS State Get`
- `NLS State Set`
- `NLS State Report`
and the following SerialAPI commands:
- `Transfer Protocol Command Class (0x69)`
- `Enable Node NLS (0x6A)`
- `Get Node NLS State (0x6B)`
- `Request Protocol Command Class Encryption (0x6C)`
- `Controller Node Send Protocol Data (0xAC)`
Alpha release will not support:
- virtual nodes
- supervision encapsulation of protocol commands
- NLS commands `NLS Node List Get` and `NLS Node List Report`

## Migration guide

See the new [migration guide](migration_guide.md) on how to migrate projects to the latest SDK.

## Expansion board removal
The expansion board was removed, so the functionalities are remapped to the *Wireless Starter Kit Mainboard*/*Wireless Pro Kit Mainboard*. Because of the reduced number of LEDs and buttons, some functionalities are only available through the CLI. For more information, please refer to the README file of each example application.

## Change zwave_soc_sensor_pir notification sending
SensorPIR's behavior has been changed slightly. Now, if the button is pressed for medium time (check readme for exact duration), then the device will send a `NOTIFICATION_EVENT_HOME_SECURITY_MOTION_DETECTION_UNKNOWN_LOCATION` notification and start a timer with a 10 second timeout. If the timeout event occurs or the user sends the `motion_detected deactivate` command through the CLI, then the device will send a `NOTIFICATION_EVENT_HOME_SECURITY_NO_EVENT` notification.

## Enable all examples for all boards
All examples are now enabled for all boards. Previously, some example applications required an RGB LED or multiple buttons to access all of the features. Now, the CLI is enabled by default in all example applications, so every feature can be controlled using the CLI. The functionality of each application is described in the README file in each application's respective folder, with the available commands. This section contains a summary of the changes. For more detailed information, please refer to the README file of each example application.

### zwave_soc_door_lock_keypad
Previously, this example required four buttons to control all the features. Now, the user code is no longer simulated, the user can enter/modify the user code manually. The door handle is also accessible through the CLI.

The default PIN code for this application was changed from 1234 to 3494 to comply with the rules recommended by the specification for the User Credential Command class.

### zwave_soc_led_bulb
Previously, this example required an RGB LED to control the color of the LED. Now, the color can be read through the CLI. If the RGB LED is not available, then a monochrome LED is used to represent the overall brightness of the set color, which will be calculated from the RGB color with the following formula:
```
uint16_t sum_colors = ((rgb_led_attributes.color.red * rgb_led_attributes.intensity) / multilevel_switch_max)
                        + ((rgb_led_attributes.color.green * rgb_led_attributes.intensity) / multilevel_switch_max)
                        + ((rgb_led_attributes.color.blue * rgb_led_attributes.intensity) / multilevel_switch_max);
// The PWM duty cycle is a percentage in the range of 0-100
uint16_t monochrome_value = (100 * sum_colors) / (3 * 255);
```

### zwave_soc_power_strip
Previously, this example displayed the Multilevel Switch value on the RGB LED. Now, if an RGB LED is available, the brightness of that LED
is controlled by the Multilevel Switch value. Otherwise, a monochrome LED is used to display this value.

## Reduce the number of demo applications
The number of the demo applications has been reduced. From now on, two different variants are supported for each SoC and SerialAPI demo application:
- Default: The default firmware with the default region, which is the EU region.
- V255: The default firmware with 255.0.0 application version. This demo is available to test the OTA and OTW firmware update.

For Zniffer applications, only the default firmware is available.

The default region for this firmware is EU. This region is only applied by the Zniffer PTI and the SoC applications if the dedicated MFG_ZWAVE_COUNTRY_FREQ token is empty (0xFF) when the application first starts up. If another region is needed, then the MFG_ZWAVE_COUNTRY_FREQ token must be set with Simplicity Commander using the following command: `commander flash --tokengroup znet --token MFG_ZWAVE_COUNTRY_FREQ:{region_hex}`, where the {region_hex} is the desired region. The available regions can be found in the `zpal_radio.h` file, such as REGION_EU.

The Long-Range capability can be enabled for any of the demo applications by setting the region to US_LR or EU_LR.

The region of the Serial API and NCP Zniffer applications can be set by the host application.

## RF Configuration in Apps
The range for APP_MAX_TX_POWER_LR & APP_MAX_TX_POWER values is now set at their highest (200 deci-dBm) by default for all chips.

## Removing support of deprecated SAPI commands
Remove handling of SAPI commands `Replication Send Data (0x44)` and `Replication Command Complete (0x45)`

## New serial API commands
Add new serial API commands: Z-Wave API Setup Get Supported Regions Sub Command (0x15) & Z-Wave API Setup Regions Info (0x16).

## Non-Listening and Frequently Listening Apps behavior using CLI
The behavior is changed on Non-Listening (Sensor PIR, Multilevel Sensor) and Frequently Listening (Door Lock Keypad) applications in case of installed CLI component. After pressing the reset button (or use `commander reset`), the device will wake up for 10 seconds. This allows the user to interact with the device and disable sleeping. The wakeup time can be configurable through the `zw_cli_sleeping` component.

## Removal of postbuild.sh
The postbuild.sh script has been removed from the SDK. This script was used to sign the applications with the Z-Wave SDK's sample keys and generate GBL files. The script is no longer needed as the signing is now done by the SLC's postbuild profile (SLPB). The keys can be set in the projects' YAML file or interactively in the Studio's GUI. For key generation check the UG489 User Guide.

## CC Configurator moved
The CC Configurator has been moved from the `ZAF/tool/cc_configurator`. The new location is `DevTools/cc_configurator`.

## Multilevel sensor
Multilevel sensor can now be assigned to an endpoint, also, multiple multilevel sensors can be used in a single end device. The cc_config file has been changed to follow this format. Endpoint is now part of the "ID" of a given sensor. The sensor structure changed to include information about the assigned endpoint.

## Workspace usage
The workspace solutions are now signing the bootloader and application binaries with the same key, which is stored by the `application` post build configuration.
The signing keys can be freely set by the developer in the SLPB files. The solution will generate three gbl files with the same keys, one for the bootloader, one for the application and one for the combined bootloader and application.

## Door Lock Keypad with U3C Beta Application
The Door Lock Keypad with U3C Beta application has been removed and its functionalities has been merged into Door Lock Keypad application. For certain devices, the Door Lock Keypad application now supports and uses the User Credential Command Class. See application README file for details.

# 7.22.0 {#section-7-22-0}

## RF Configuration in Apps
prior to this release, RF configuration was stored in two different files, depending on whether or not the chip supported the  20dBm TX level. These files have now been merged. The range for APP_MAX_TX_POWER_LR & APP_MAX_TX_POWER values now has a high value of 200 for all chips. It needs to be increased to allow 20dBm transmission on supported chips.

## LR Channel configuration management refactored
Prior to the implementation of the Europe Long Range frequency, management of LR channel configuration has been refactored (US_LR products are impacted). There should be no behavior change.

## Europe Long Range frequency - Alpha
Implementation of the Europe long range frequency in Alpha state.

## Version Management
ZAF_BUILD_NO, SDK_VERSION_[MAJOR|MINOR|PATCH], ZAF_VERSION_[MAJOR|MINOR|PATCH] are not available in Applications anymore, in order to access these values you must include and use the accessor functions defined in "ZAF_version.h".

## Region change and Custom LBT Threshold value
Custom values for LBT Threshold level are now cancelled during region switch. It's now necessary to reapply them after the switch.

## EResetReason_t removal
The EResetReason_t enum has been removed. You can now use directly zpal_reset_reason_t. Values are aligned with old EResetReason_t.

## Z-Wave specific section
Simplicity SDK require that all sections are
placed in the linkerscript. If using software components, they are contributed by the Z-Wave software components. For developers not using the software component mechanism, they should contribute the following sections to their linker file.
* Contributed by the zw_core.slcc software component:
  - zw_protocol_cmd_handlers_lr
  - zw_protocol_cmd_handlers
* Contributed by the zw_apputilities.slcc software component:
  - zw_cc_handlers_v3
  - zw_zaf_cc_config
* Contributed by the zaf_event_distributor_soc.slcc software component:
  - zw_zaf_event_distributor_cc_event_handler
* Contributed by the zwave_ncp_serial_api_controller.slcp software component:
  - zw_cmd_handlers

## Extended NVM backup and restore
New command is implemented in the serial API: Extended NVM backup and restore. Basically, it is the same command as NVM backup and
restore, with the address now stored on 4 bytes instead of the previous 2. For more details on this command, refer to the Serial API specification.

## Removal of default debug traces

The default debug traces were removed from stack components. These traces were not enabled by default.

# 7.21.2 {#section-7-21-2}

## Fix migration process from 7.17 to 7.18+
The migration process of the controller is broken since 7.18.1. Restore this migration process.
This process is used when a controller is updated from a version older than 7.18 to 7.18 or newer.

# 7.21.1 {#section-7-21-1}

## Support
Footprint (heap, stack, ROM) has been optimized to support low-end devices (ZG14).

## Fix saving host data in controller
Fixed an issue on 700 series controllers, where the host was no longer able to save data in the controller's NVM.
Increased largest NVM object size to match host's limitation (from 204 to 512 bytes currently used by ZPC or ZGW)
This is to satisfy the design of the 700 series and allow the host to store data on the controller's NVM.

## ZPAL Power Manager
Relocated zpal_pm_set_device_type to apps (e.g. sensor is non-listening).
Reworked notifications (deep sleep, power down) related to EM4, EM3, EM2 states.

# 7.21.0 {#section-7-21-0}

## SPAN synchronization
Unsollicited request from end devices will now have S2_TXOPTION_VERIFY_DELIVERY enabled by default to allow SPAN synchronization.

## Portable Controller Application
The portable controller application has been removed in this version. Last version supporting this application is 7.20.

## ZPAL Power Manager
The maximum number of power manager handlers has been reduced from 20 to 17. This number includes both stack-side and application-side handlers.

## Application user-specified versioning
A new boolean parameter is introduced into Simplicity Studio. When the user creates a new application,
the user can choose whether their application uses the Z-Wave SDK Version, or a specific version
he is free to define.

## OTW Update of Serial API controller
In version 7.20 of the Z-Wave SDK, The Serial API Controller application's
Product ID, used for validation during OTW updates had been changed to include
a Generic and Specific Device Type.\
This prevented updating devices running version 7.19 or lower of the Serial
API Controller to more recent versions.

To address this incompatibility, the Specific and Generic Device Type bytes of
the Product ID, defined in `application_properties_config.h`
(used by the struct `sl_app_properties` of the bootloader)
have been reverted to their previous values of 0.\
This only affects the Product ID; the application still reports the new Device
Type values in its Node Information Frame.\
Note that these values can be changed on the fly using the Serial API command `SERIAL_API_APPL_NODE_INFORMATION`.

The following update paths for this application are not possible using the
pre-built binaries:

- 7.19 or lower to 7.20
- 7.20 to 7.21 or higher

However, firmwares that are not affected by this incompatibility can be created manually, as described in the README of the application.

## Z-Wave NVM instance configurable from Simplicity Studio
A configurable component (ZPAL Z-Wave NVM instance) has been created to help user to configure the Z-Wave NVM stack instance from Simplicity Studio. This NVM instance is used on series 700 to store ZAF and user application's data. GSDK and Z-Wave stack data are stored in the default NVM instance (which is configurable from the NVM3 Default Config component).\
On other series, all data are stored in the NVM default instance. So this component is available only in projects using series 700.

## Improve NVM management on series 700
Add periodic repacks for Z-Wave NVM instance on series 700 in zpal. Avoid to repack this NVM instance on NVM write (which could create latency in wireless communication).\
NVM default instance already has a periodic repack.

## Controller's NVM data update
Controller's data in NVM have been updated in order to save the nodeId length (Classic Z-Wave or Long Range Z-Wave). A migration script is required to update data in NVM.
This script is enable by installing slc component "Migration from V7.20 (to V7.21)" in the controller project.

# 7.20.2 {#section-7-20-2}

## zwave_soc_led_bulb Moved to NonCertifiableApps
The zwave_soc_led_bulb (LED Bulb) application is no longer certified, so this application
is moved from `Apps` to `NonCertifiableApps` folder.

# 7.20.1 {#section-7-20-1}

## ZAF Retention Register
Added module for retention registers. Application can now use `ZAF_retention_register_read` and `ZAF_retention_register_write` to read and write retention registers not used by ZAF, protocol and PAL. `ZAF_retention_register_count` can be used to get the number of registers available for application use. Register index starts from 0.

# 7.20.0 {#section-7-20-0}

## ZAF Application Name
Storing the name of applications is now handled by the `zaf_appname` component.\
The name is stored as a string by default.
By installing an additional component, `zaf_appname_nvm`, the name can be saved
to the non-volatile memory as well.\
When multiple Z-Wave devices are connected, this feature can speed up the
process of identifying which application is flashed on each device.

## ZAF Job Helper
This module is no longer recommended for use in new applications. The main
use case for this module was to transmit different frames for the same event.
The new applications should leverage the ZAF Transport Queue instead.
For applications that still wants to use this module it has been moved to its
own component: `zaf_job_helper.slcc`

## ZAF Transport Module

ZW_TransportEndpoint was refactored and much local data has been removed.
Functions `Transport_SendRequestEP()` and `Transport_SendResponseEP()`
were replaced with a common function `ZAF_Transmit()`
The type of `callback_function` in `ZAF_Transmit(buffer, len, txOptions, callback_function)`
is now `ZAF_TX_Callback_t`.
Note that `ZAF_Transmit()` does not set `S2_TXOPTION_VERIFY_DELIVERY` by default,
as it was with `Transport_SendRequestEP()`. This flag can be set in
`ZAF_Transmit()`, by enabling it in `TransmitSecurityOptions`.
It is not recommended to use this API directly, applications should use the ZAF
Transport Queue to ensure that only one frame is sent from the application to
the protocol at one time.

### ZAF_Transmit migrated to zaf_transport_tx
In order to ensure a single entry point for frames from the application to the
protocol, ZAF_Transmit is not being called directly by Command Classes.

## ZAF Transport Queue
This module implements a transport queue in the application level. This queue
ensures that there is only a single entry point for a frame from the
application. This adds a new feature to applications, the ability to "send and
forget", which means the user only has to send to the queue, and the rest of the
process is taken care of. i.e. Send Central Scene Notification and Send Basic
Set in sequence. The queue size is configurable and it can bet set by
`ZAF_TRANSPORT_CONFIG_QUEUE_SIZE`, the default is 2.


## ZAF TX Mutex Removed
This module was used to ensure that a shared buffer frame was only used by one
module at the same time. It was very relevant for 500 series to reduce the RAM
usage as no stack was available for the tasks, however it became irrelevant for
700 series and forward. This module was only being used by the ZW Transport
Multicast module at this point. The buffer was moved to the module itself and
the TX Mutex module was removed, therefore the `ZAF_tx_mutex.h` no longer
exists.

## Merge agi.c/h into CC_AssociationGroupInfo.c/h
The agi.c/h module does not exist anymore as the contents
of the files have been merged into CC_AssociationGroupInfo. This should reduce some complexity
since data does not have to be passed between the different modules, and should reduce flash and ram consumption.

## ZAF Event Helper module removed
This module was only used by the event distributor soc to enqueue events into
the application event queue therefore this functionality was moved into the
event distributor soc.

The interface changes are listed below:

| **Old name**                          |      | **New name**                                         |
|---------------------------------------|------|------------------------------------------------------|
| `ZAF_EventHelperInit`                 | `->` | Not needed anymore                                   |
| `ZAF_EventHelperIsInitialized`        | `->` | Not needed anymore                                   |
| `ZAF_EventHelperEventEnqueue`         | `->` | `zaf_event_distributor_enqueue_app_event`            |
| `ZAF_EventHelperEventEnqueueFromISR`  | `->` | `zaf_event_distributor_enqueue_app_event_from_isr`   |

The functions that still exist takes the same arguments as before. They are
defined in `zaf_event_distributor_soc.h`

## Application Name Configurable
Each application's name is configurable. It can be changed by specifying the
configuration entry named `ZAF_APP_NAME` in the SLC project description.
Printing the application's name, reset reason, and SDK version was moved to ZAF_PrintAppInfo().

## Eliminating the config_app.h files
config_app.h files were eliminated from sample applications, and the content of the headers
were moved to the project source files. Exceptions to this are version numbers, previously stored in these files.
Version numbers have been moved to a configurable SLC component.
The APP_VERSION_MAJOR, APP_VERSION_MINOR, and APP_VERSION_PATCH numbers
are now configurable via the zw_version component in the studio GUI.
Users still get the original Silicon Labs version numbers by default,
but it is also possible to change them as described above.

## Common hardware functions
Each application would implement each own `<App>_hw_init` function and have
their own `<App>_hw.h` header. The applications that go to deep sleep also had
their own `<APP>_hw_deep_sleep_wakeup_handler` function. These two common
functions have been consolidate in a `app_hw.h` as `app_hw_init` and
`app_hw_deep_sleep_wakeup_handler`. `app_hw_init` MUST be implemented by the
application while `app_hw_deep_sleep_wakeup_handler` should only be implemented
by applications that can go into deep sleep.

## Application's Source Code

Application's source codes were moved from \<application_name\>.c to app.c file.
The app.h headers were also removed with the function `app_init()`. Because of this
the function calls were removed from the main.c file.

## Board Indicator Moved into ZAF

Board indicator LED is initialized in ZAF_Init() and the default idle status is set also in it.
`Board_IndicateStatus()` is available so it can be used freely in the code.

## Command Classes

### New Version of Command Classes Handlers

New macro `REGISTER_CC_V5()` has been created and can be used to initialize Command Classes.
This macro requires a CC handler that takes only input and output structure as arguments.
See `CC_BinarySwitch_handler(cc_handler_input_t * input,  cc_handler_output_t * output)` as an example.
This makes it possible for CC to exit with status WORKING, without a need to pass duration parameter to Supervision CC,
or to call Supervision Report directly, as was the case so far.

`REGISTER_CC_V5()` is supported in all Command Classes that support timed change, and their handlers were updated accordingly.
List of handlers compatible with `REGISTER_CC_V5()`:
 * CC_BinarySwitch_handler()
 * CC_ColorSwitch_handler()
 * CC_DoorLock_handler()
 * CC_MultilevelSwitch_handler()
 * CC_MultiChannel_handler()

### Folder structure and private/public headers
Some command classes have more than two source files and header files. This is
because the command class contains a lot of logic therefore splitting into
more than one source and one header file makes sense for readability.
In order to allow interaction between two source files, private header were
introduced and to avoid confusion a new folder structure is used. The source
files and private header are placed in `src` while the public headers are
placed in `inc`.

All Command Classes have been structured to have `inc`, `src` and in some cases, `config` folder.
Header files located in `config` folder contain values configurable in Simplicity Studio.

### CC Basic

`CC_Basic_Set_handler()` was removed. This function is no longer required by
applications as ZAF handles Basic Command Class mapping (assuming all relevant
Command Classes implement a Basic Set mapper function).

### CC Central Scene
Configuration of Command Class Central Scene was migrated to Z-Wave Command Class
Configurator and must be configured in \<app\>.cc_config. More details can be found
[here](ZAF/tool/cc_configurator/README.md).


The configuration of cc_central_scene_get_supported_key_attributes was removed
from `zw_cc_central_scene` component. Values can either be set using Z-Wave Command Class
Configurator, or they are automatically calculated by Command Class.

- Function `cc_central_scene_set_configuration` was removed since this operation is only done internally in the Command Class.
- `cc_central_scene_get_supported_key_attributes` were made static as they are only used internally.
- `cc_central_scene_get_configuration` was removed as obsoleted. Slow refresh value
  is now used only internally in CC_CentralScene.c and can be accessed directly.
- Refactored function `CommandClassCentralSceneNotificationTransmit()`:
  - Renamed to `cc_central_scene_notification_tx()`
  - Removed `sourceEndpoint` argument. The function will always use `ROOT` as source endpoint.

### CC Door Lock
It is now the responsibility of the Command Class to ensure that the TSE actions are managed.
All TSE-related functionalities have been moved from the applications to the Door Lock CC.
This includes functions `cc_door_lock_handle_working_state()` and `CC_DoorLock_operation_report_notifyWorking()`

Hardware handling has been moved out of CC Door Lock, leaving the command class with the software logic only.

#### Supervision Handling Moved to CC
Handling of Supervision Get for Door Lock Operation Set has been moved from application
to Command Class.

Function `cc_supervision_get_received_handler` no longer needs to be implemented in the
app.

New `REGISTER_CC_V5` made it possible to remove `CmdClassSupervisionReportSend()` with status Working
and leave it to CC Supervision to handle the working case.

### CC Multi Channel
Configuration of Command Class Multi Channel (endpoints) was migrated to Z-Wave Command Class
Configurator and must be configured in \<app\>.cc_config. More details can be found
[here](ZAF/tool/cc_configurator/README.md).

Configuring whether endpoints are identical or not must be done in Studio.

As a result of the migration, Transport_AddEndpointSupport() was removed.

To comply with the naming scheme, the following two SLC components were renamed:
* zw_cc_multichannelcontrol -> zw_cc_multi_channel_control
* zw_cc_multichannelsupport -> zw_cc_multi_channel_support

### CC Notification
The source code in notification.c has been moved to CC_Notification.c and notification.c has been
removed.

Notification types have been moved into `cc_notification_types.h` and states/events have been
moved into `cc_notification_events.h`. This is done to prepare for automatic generation of
these two files based on the types and states/events defined in the Z-Wave specification.

CC_NOTIFICATION_MAX_EVENTS_PER_NOTIFICATION was removed as the number of configured events/states
can be derived from the CC configuration.

The CC configuration syntax was simplified to no longer require `state_event:` for each
state/event listed under `state_events:`. This is a breaking change and existing `cc_config`
files must be updated. More details can be found [here](ZAF/tool/cc_configurator/README.md).

Existing syntax:
```yaml
state_events:
  - state_event: NOTIFICATION_EVENT_POWER_MANAGEMENT_OVERLOADED_DETECTED
  - state_event: NOTIFICATION_EVENT_POWER_MANAGEMENT_LOAD_ERROR
```

New syntax:
```yaml
state_events:
  - NOTIFICATION_EVENT_POWER_MANAGEMENT_OVERLOADED_DETECTED
  - NOTIFICATION_EVENT_POWER_MANAGEMENT_LOAD_ERROR
```

### CC Wake Up
Applications that included the Command Class Wake Up had to call three functions
in response to the `EVENT_SYSTEM_LEARNMODE_FINISHED` event. These functions were
`CC_WakeUp_stayAwake10s`, `CC_WakeUp_AutoStayAwakeAfterInclusion` and
`CC_WakeUp_startWakeUpNotificationTimer` if the inclusion state is not excluded.
Now this is handled by ZAF. This is done by implementing the weak function
`zaf_learn_mode_finished`.

## Application Events Abstracted
Some sample applications contained events that contained hardware-specific names.
Those names were based on how the Silicon Labs port would generate those events.

Many events had the BUTTON keyword and it was removed from event names so
`EVENT_APP_BUTTON_<event>` becomes `EVENT_APP_<event>`. Below is the list
of renamed events for each application that did **NOT** follow this pattern.

### zwave_soc_switch_on_off

| **Old name**                          |      | **New name**           |
|---------------------------------------|------|------------------------|
| `EVENT_APP_BUTTON_TOGGLE_SHORT_PRESS` | `->` | `EVENT_APP_TOGGLE_LED` |
| `EVENT_APP_BUTTON_TOGGLE_HOLD`        | `->` | `EVENT_APP_SEND_NIF`   |

### zwave_soc_key_fob

| **Old name**                                    |      | **New name**                               |
|-------------------------------------------------|------|--------------------------------------------|
| `EVENT_APP_BUTTON_UP_ASSOCIATION_GROUP_ADD`     | `->` | `EVENT_APP_ASSOCIATION_GROUP_ADD_START`    |
| `EVENT_APP_BUTTON_UP_ASSOCIATION_GROUP_REMOVE`  | `->` | `EVENT_APP_ASSOCIATION_GROUP_REMOVE_START` |

## New ZAF Responsibilities
In order to make apps smaller and easier to start with, all implementation common to all apps
that did not require additional configuration from the user has been moved to ZAF.

### Features moved out of apps to ZAF
All sample application would keep a local pointer to the `AppHandles` passed by
the `ApplicationTask`. This parameter is already passed to ZAF in the `ZAF_Init`
therefore the Applications don't need to keep a local pointer anymore. They
should use the functions from `ZAF_Common_interface.h` to retrieve the data
needed from the `AppHandles`

They also had to call `zaf_event_distributor_init` in the `ApplicationTask`.
This is not necessary anymore as this functions is called by ZAF in the
`ZAF_Init`

The sample applications that support the CC Firmware Update also had to call
`cc_firmware_update_send_status_report` in `ApplicationTask` after `ZAF_Init`.
This is not needed anymore because the CC Firmware Update handles it internally.

### New weak function on EVENT_SYSTEM_LEARNMODE_FINISHED
A new weak function was introduced in ZAF, `zaf_learn_mode_finished`. This
function can be used by Command Classes that need to execute an action in
response to this event. Currently only the Command Class Wake Up utilizes this
functionality.

# 7.19.1 {#section-7-19-1}

## Command Classes

### CC Configuration
The YAML configuration of CC Configuration in \<app\>.cc_config now requires "number" and "file_id"
for each parameter. This requirement was added to allow addition/removal of parameters across
different application firmware versions. See MultilevelSensor.cc_config for an example.

The output array generated in cc_configuration_config.c is now sorted by "number". The
Z-Wave Command Class Configurator (Z3C) will automatically sort the items, but in case a previously
auto-generated file is manually modified, make sure that the array stays sorted by "number".
The sorting is not required as of this release, but will be required later as it opens for some
flash usage optimizations in CC_Configuration.c.

### CC Central Scene

`cc_central_scene_properties` was renamed to `cc_central_scene_identical`. If you used this variable in your code, please change any occurrences of it to the new name.

# Important Changes in 7.19.0 {#section-7-19-0}

In general, these changes are designed to allow the user to configure their projects in the Simplicity Studio Component Editor,
rather than by updating code or header files. A new Z-Wave Command Class Configurator has been added to Simplicity Studio
to accommodate the more advanced Z-Wave command classes that require complex configurations. More details can be found [here](ZAF/tool/cc_configurator/README.md).

In order to make applications smaller and easier to start with, all implementation that was common to all applications
and that did not require additional configuration from the user has been moved to the Z-Wave Application Framework (ZAF).

## Configurable Parameters Have Been Moved from config_app.h

A number of parameters, previously located in `config_app.h`, that were mandatory for application configuration
now can be configured using the Simplicity Studio GUI and are moved out of this header file.
This makes it easier to set up all mandatory configurations at the moment when the new project is created in Studio.

See [Command Classes](#cc-id) for more details about specific command class configuration.

### Role Type and Device Type

`APP_ROLE_TYPE` was moved to `zaf_config_role_type.h`.
`DEVICE_OPTION_MASK` was renamed to `ZAF_CONFIG_DEVICE_OPTION_MASK` and it was also
moved to `zaf_config_role_type.h`.
Both defines are automatically set by `role_type` components for end devices.

Generic and Specific type can be configured in the GUI:
- `GENERIC_TYPE` was renamed to `ZAF_CONFIG_GENERIC_TYPE`
- `SPECIFIC_TYPE` was renamed to `ZAF_CONFIG_SPECIFIC_TYPE`

They are moved to `zaf_config.h` and can be configured in the "Z-Wave ZAF Component" component.

### Requested Security Keys

The configuration of security keys to request from the controller, previously defined as
`REQUESTED_SECURITY_KEYS`, was moved from config_app.h to `zaf_config.h`.
Instead of using `REQUESTED_SECURITY_KEYS`, it is now possible to select any combination of keys in "Z-Wave ZAF Component":

- `ZAF_CONFIG_REQUEST_KEY_S0`
- `ZAF_CONFIG_REQUEST_KEY_S2_UNAUTHENTICATED`
- `ZAF_CONFIG_REQUEST_KEY_S2_AUTHENTICATED`
- `ZAF_CONFIG_REQUEST_KEY_S2_ACCESS`

### Number of End Points

The configuration of number of end points, previously defined as `NUMBER_OF_ENDPOINTS`,
was renamed to `ZAF_CONFIG_NUMBER_OF_END_POINTS` and moved  to `zaf_config.h`.

## Application Initialization

`ZAF_setApplicationData()` was removed. The parameters of this function must instead be passed to `ZAF_Init()`.

For Serial API specifically, `SetTaskHandle()` (and `GetTaskHandle()`) was introduced and must be used
instead of `ZAF_setApplicationData()`. `ZAF_setAppProtocolConfig()` must be used instead of `ZAF_Init()` to set
the protocol configuration.

`ZAF_setAppHandle()` was moved to `ZAF_Init()` and no longer has to be invoked from the application.

### ZAF_ApplicationEvents.h

Application events like `EAPPLICATIONEVENT_APP` were moved to `ZAF_ApplicationEvents.h` as they are all
the same for most apps. Additional events can be added to the file if desired.

### ZAF_Init()

The parameters of `ZAF_Init()` have changed. See the description of `ZAF_Init()` for information
about which parameters it takes.

`ZAF_Init()` MUST be invoked from the task function passed to `ZW_ApplicationRegisterTask()`.
For the Z-Wave apps included in the SDK, this function is named `ApplicationTask()`.

### AssociationInitEndpointSupport()

In a previous release, the need for `AssociationInitEndpointSupport()` was removed as ZAF takes care
of the association root group mapping. The function was kept for backwards capability, but is now
removed.

### Initialization of Command Classes

Command Classes manage their own initialization and so the application no longer needs to do it.
Each Command Class uses `REGISTER_CC_V4()` to register its init and reset functions.
This ensures that the initialization and reset are performed by ZAF.

### Read/Write to NVM

The functions used to read/write from Non-Volatile Memory `zpal_nvm_read(handle, key, object, object_size)`
and `zpal_nvm_write(handle, key, object, object_size)` are replaced by `ZAF_nvm_app_read(key, object, object_size)`
and `ZAF_nvm_app_write(key, object, object_size)`. The new functions work without needing a pointer to
the file system as an input. The application file system must be opened by calling `ZAF_nvm_app_init()`
at initialization so it is avaliable to be used by `ZAF_nvm_app_read()` and `ZAF_nvm_app_write()`.

ZAF filesystem can be used with `ZAF_nvm` module. `ZAF_FILE_ID_BASE` was removed. Now ZAF file IDs start from 0.

### Removed LED_RGB and Timer Components

Timer components were used to control `RGB_LED`, but these components are replaced by the Silicon Labs component "SL_SIMPLE_RGB_PWM_LED".
After this release these components are no longer available.

The following functions were removed from *platform/SiliconLabs/AppsHW/inc/board.h*:

- `Board_RgbLedInitPwmTimer()`
- `Board_RgbLedEnablePwm()`
- `Board_RgbLedDisablePwm()`
- `Board_RgbLedSetPwm()`

## New ZAF Responsibilities

In order to make applications smaller and easier to start with, all implementation that was common to all applications
and that did not require additional configuration from the user has been moved to ZAF.

### Features Moved Out of Apps to ZAF

#### Learn Mode

ZAF is now responsible for the handling of Learn Mode.

#### Device Reset Local

ZAF is also responsible for handling the device reset using the "NVM SOC" component.

#### Removed State Machine from Applications

All sample applications had a state machine implemented into them. It is no longer needed, making
the applications simpler and allowing more responsibility to be given to ZAF.
Applications can still implement a state machine if the use case requires but our sample applications
use cases do not require state machines.

#### Stay Awake

An application no longer has to pass a stay awake function to ZAF as the stay awake functionality
is now handled by ZAF. The callback parameter was removed from `ZAF_Init()` and from
`Transport_OnApplicationInitSW()`.

Also, `EZWAVERECEIVETYPE_NODE_UPDATE` and `EZWAVERECEIVETYPE_STAY_AWAKE` need not trigger a call
to a stay awake function as this is now handled inside ZAF.

ZAF will determine the stay awake time from the role type and from whether CC Wake Up is linked or
not.

`ZAF_FLiRS_StayAwake()` and `CC_WakeUp_stayAwakeIfActive()` must no longer be used and are replaced by
`zaf_stay_awake()`.

### Event Distributor

All sample applications implemented their own event distributor component. However, it was the same
between all sample applications, with the only difference being if the application allowed for pooling
or full event driven. This logic has been moved into ZAF in the component event distributor and the
logic has not changed.

Two types of distributor have been implemented: NCP and SOC

#### NCP

This is the event distributor used in Serial API and it is the event distributor that should be
used in all applications that run the host in a separated chip.

#### SOC

This is the event distributor used in all other sample applications and it should be used in all
applications that run controller and host on the same chip.

#### NVM SOC

All SOC sample applications use this component as it contains all the NVM logic expected to be
common among them. It is responsible for resetting the APP NVM, Load Default Configuration and
Setting Default Configuration. It contains handlers that can be used for custom behaviors in
the application level.

### AppPowerDownCallBack

This function replaces the ZAF Power Module Wrapper for more optimized code. Instead of registering a callback
which was called as the last step just before the chip entered deep sleep hibernate, a new weakly-defined function
`ZW_AppPowerDownCallBack` is provided.

### ZAF Power Module Wrapper - Deprecated

The ZAF Power Module Wrapper is no longer supported. It was responsible for registering functions that would
be called as the last step just before the chip enters deep sleep hibernate.

It is important to highlight that the protocol still supports the `EZWAVECOMMANDTYPE_PM_SET_POWERDOWN_CALLBACK`
for registering such callbacks at run time. However, we recommend that the weakly-defined function
`ZW_AppPowerDownCallBack` is used instead. Since the DeepSleepTimer is no longer using the protocol
command, we reduced the `MAX_POWERDOWN_CALLBACKS` from 4 to 3.

## Components Renamed

The following component IDs were renamed in order to comply with the snake case naming convention

- `zw_cc_centralscene` renamed to `zw_cc_central_scene`
- `zw_cc_colorswitch` renamed to `zw_cc_color_switch`
- `zw_cc_doorlock` renamed to `zw_cc_door_lock`
- `zw_cc_multilevelsensor` renamed to `zw_cc_multilevel_sensor`
- `zw_cc_multilevelswitchsupport` renamed to `zw_cc_multilevel_switch_support`

## Command Classes {#cc-id}

### Z-Wave Command Class Configurator

An advanced configuration was introduced in order to accommodate the more advanced z-wave command
classes that require complex configurations. More details can be found [here](ZAF/tool/cc_configurator/README.md)

### Command Class List Generator

An application no longer needs to pass the Command Classes lists to the protocol and ZAF. ZAF
has new components that are capable of generating these lists from the linked command classes.
For end points a new macro was introduced:

```c
#define ZAF_CC_REGISTER_CONFIG(cc, config, index)
```

This macro needs to be called for each element in a configuration array that contains
endpoints. For example:

```c
static cc_multilevel_switch_t s_switches[] = {
  {
    .endpoint = 2,
  },
  {
    .endpoint = 1,
  },
};

...

ZAF_CC_REGISTER_CONFIG(COMMAND_CLASS_SWITCH_MULTILEVEL, &s_switches[0], 0);
ZAF_CC_REGISTER_CONFIG(COMMAND_CLASS_SWITCH_MULTILEVEL, &s_switches[1], 1);
```

This is automatically handled in the command classes provided in the SDK but command
classes not from the SDK need to follow this standard. This requirement is similar to the
`REGISTER_CC`.

### I/O Interface for CCs

Some Command Classes have a new level of abstraction, the I/O interface. This interface has the
functions used to read, write, and sometimes migrate the data. This level of abstraction allows the
user to disable and enable data persistency seamlessly.

CCs that support this interface:

- Battery
- Central Scene
- Color Switch
- Configuration
- Door Lock
- Multilevel Switch
- Notification
- User Code

### CC Association

The `AssociationInit()` function was removed and the initialization performed by this function was split into two
separate functions: `init()` and `reset()`. These two functions are now passed as callbacks to the `REGISTER_CC_V4` macro.
This ensures that the initialization and reset are performed by ZAF, instead of the user code.

`MAX_ASSOCIATION_GROUPS` and `MAX_ASSOCIATION_IN_GROUP` have been moved from `config_app.h` to `cc_association_config.h`
and renamed to `CC_ASSOCIATION_MAX_GROUPS_PER_ENDPOINT` and `CC_ASSOCIATION_MAX_NODES_IN_GROUP` respectively.
These values can be set in Simplicity Studio in the "Association Command Class" module.

### CC AGI

The Association Group Information Command Class has been removed from the `config_app.h` header and implemented in the config generator and the application slcp files.
This refactoring enables the user to easily configure the device's group and node information in the Simplicity Studio GUI instead of a source file.
All the command classes, utilities, applications, and tests have been refactored to use the automatically-generated headers containing the AGI group and node
configuration. The names of the defines have also been refactored to make them easy to understand.

The `AGI_ResourceGroupSetup` function has been deprecated. In addition, the association group information has been moved from the `config_app.h` header to the `cc_config` yaml file.
Redundant duplication of the root device groups is removed.
In order to define the device groups and endpoints, developers have to add / modify the zw_cc_association section in the respective app cc_config file. Developers can look at at WallController, SensorPIR, or PowerStrip applications as a starting point for defining their own groups and endpoints. The configurator will use this information to generate the agi groups and tables in the cc_association_config.c source file for the specific application.

### CC Basic

Basic Get mapping is now done automatically by ZAF. It does require relevant command classes to
implement a certain function. See [How to implement a new command class](ZAF/doc/content/how-to-implement-a-new-command-class.md) for more information.

True Status was removed from CC Basic as True Status must be handled by the command class mapped to Basic Set.

As a result of the above changes, the following functions were removed from CC Basic:

1. CC_Basic_report_stx()
2. CC_Basic_prepare_zaf_tse_data()
3. CC_Basic_report_notifyWorking()
4. CC_Basic_GetCurrentValue_handler()
5. CC_Basic_GetTargetValue_handler()
6. CC_Basic_GetDuration_handler()

Additionally, a Supervision flag was added to CC_Basic_Set_tx() making it possible for the
application to decide whether Supervision must be used or not for the given transmission of Basic
Set. The use of Supervision was previously hardcoded in CC_BasicController.c.

### CC Binary Switch

The Binary Switch command class was rewritten to use ZAF Actuator and take care of reporting the
true status so that the application does not have to.

The configuration of this CC must be done in the `*.cc_config` file. Example for root device:
```
zw_cc_binaryswitch:
  binary_switches:
    - endpoint: 0
```
See PowerStrip.cc_config for an example of endpoints.

### CC Central Scene

Three Central Scene config APIs were moved from the application to the Command Class:

- `cc_central_scene_set_configuration` (used to be `setAppCentralSceneConfiguration`)
- `cc_central_scene_get_configuration` (used to be `getAppCentralSceneConfiguration`)
- `cc_central_scene_get_supported_key_attributes` (used to be `getAppCentralSceneReportData`)

The configuration of `cc_central_scene_get_supported_key_attributes` can be done from the Simplicity Studio GUI in the "zw_cc_central_scene" component.

Non-Volatile Memory can be used to store the configuration values by installing the "zw_cc_central_scene_nvm" component.

### CC Color Switch

The Command Class Color Switch was refactored to no longer require initialization on the application
level. The initialization is now handled internally by ZAF on `ZAF_Init`.

The color configuration is now also handled by the Command Class and not the application.
The configuration is available using the `Z-Wave Command Class Configurator`. The array of
all colors supported by the application is no longer needed since it is created by the Command Class
based on the configuration.

The functionality that each color component had in its callback function has been removed. Now
all color components call the `cc_color_switch_cb` with the color component as parameter.
In the `CC_ColorSwitch_Init` one of the parameters was a callback function. This function is now a weak
function defined in the command class `cc_color_switch_refresh_cb`.

### CC Door Lock

The Door Lock Command Class has been refactored so that all logic related to the Command Class is handled
internally by it. The Command Class had five extern functions that had to be implemented by the application.
`CC_DoorLock_OperationSet_handler`, `CC_DoorLock_OperationGet_handler`, `CC_DoorLock_ConfigurationSet_handler`, `CC_DoorLock_ConfigurationGet_handler` and
`CC_DoorLock_CapabilitiesGet_handler`. These functions are now implemented inside the command class
and six new functions were created:

- `CC_DoorLock_Toggle` to toggle the door lock mode.
- `cc_door_lock_latch_status_handler` to be called when the latch status changes (HW specific function).
- `cc_door_lock_bolt_status_handler` to be called when the bolt status changes (HW specific function).
- `CC_DoorLock_update` to set door conditions (LED) from door handle mode and state.
- `CC_DoorLock_SetOutsideDoorHandleState` to set the Outside Door Handle State for a specific handle.
- `CC_DoorLock_ClearOutsideDoorHandleState` to clear the Outside Door Handle State for a specific handle.

The configuration is now also handled by the Command Class and not the application. The configuration is available
using the `Z-Wave Command Class Configurator`.

All type redefinitions and function redefinitions kept for backwards compatibility were removed.

The Door Lock data is now owned by the Command Class and not the application. In our sample
application the data was stored in `FILE_ID_APPLICATIONDATA` and code to migrate this information
from the application into the command class was added with this assumption. If an application is storing
this information in another file ID, the define `DOOR_LOCK_DATA_LEGACY_FILE_ID` in `cc_door_lock_nvm.c`
needs to be changed to match this other file ID

### CC Firmware Update Metadata

`APP_FIRMWARE_ID`, previously defined in `config_app.h`, has been obsoleted, as well as function `handleFirmWareIdGet()`.
Instead, firmware ID (16 bit) is hard-coded in ota_util.c to consist of product type ID LSB and product
ID LSB.

### CC Indicator

`CC_Indicator_Init()` was removed and, as a replacement for the callback parameter, a weakly-
defined function was added: `cc_indicator_handler()`. This function should be defined
by the platform-specific code to override the weakly-defined function.

If an existing function was previously passed to `CC_Indicator_Init()`, simply rename that
function to `cc_indicator_handler()`.

### CC Manufacturer Specific

CC Manufacturer Specific was changed so that all applications can be built without this CC. This
was done to enable building an even smaller application for updating Secure Element.

As a result, the following files are removed from the SDK:

- CC_ManufacturerSpecific.h
- CC_ManufacturerSpecific_config.c
- CC_ManufacturerSpecific_config.h

This includes following functions:

- `CC_ManufacturerSpecific_ManufacturerSpecificGet_handler()`
- `CC_ManufacturerSpecific_DeviceSpecificGet_handler()`

`APP_MANUFACTURER_ID`, `APP_PRODUCT_ID` and `PRODUCT_TYPE_ID` have been moved to `zaf_config.h`
and renamed to `ZAF_CONFIG_MANUFACTURER_ID`,`ZAF_CONFIG_PRODUCT_ID`, and `ZAF_CONFIG_PRODUCT_TYPE_ID`, respectively.

Manufacturer ID, product type ID, and product ID must be set in Simplicity Studio in the Software
Components tab.

Device Specific Report contains the chip serial number per default as this is what Silicon Labs
recommends. If anything else is desired, CC_ManufacturerSpecific.c can be altered.

### CC Multilevel Sensor

The sensor configuration is now also handled by the Command Class and not the application.
The configuration is available using the `Z-Wave Command Class Configurator`.
The `sensor_interface_t` and `cc_configuration_t` in the application are no longer needed.

In `sensor_interface_t` three callbacks are now weak functions in the Command Class:
`cc_multilevel_sensor_{{sensor_instance}}_interface_init`, `cc_multilevel_sensor_{{sensor_instance}}_interface_deinit`
and `cc_multilevel_sensor_{{sensor_instance}}_interface_read_value`. The functions' behavior and parameters
were not changed so renaming them should be enough.

In the `cc_configuration_init` function one of the parameters was the `cc_configuration_io_interface_t`,
which defined two callback functions for reading and writing. These functions are now two weak functions
defined inside the Command Class: `cc_configuration_io_write` and `cc_configuration_io_read`.
The functions' behavior and parameters were not changed so renaming them should be enough.

### CC Multilevel Switch

The Command Class Multilevel Switch was refactored to no longer require initialization on the application
level. The initialization is now handled internally by ZAF on `ZAF_Init`.

The switch configuration is now also handled by the Command Class and not the application.
The configuration is available using the `Z-Wave Command Class Configurator`. The array of
all switches supported by the application is no longer needed since it is created by the Command Class
based on the configuration.

In the `cc_multilevel_switch_init` one of the parameters was a callback function. This function is now a weak
function defined in the command class `cc_multilevel_switch_support_cb`.

### CC Notification

The Command Class Notification was refactored to no longer require initialization on the application
level. The initialization is now handled internally by ZAF on `ZAF_Init` and `ZAF_Reset`.
`MAX_NOTIFICATION` define was removed from `config_app.h` and moved to `cc_notification_config.h`
This value can be configured in the Simplicity Studio GUI.

- Functions `NotificationEventTrigger()` and `UnsolicitedNotificationAction()` have been merged into a single function named `CC_Notification_TriggerAndTransmit()`.
- Notification CC now handles triggering of TSE by itself.
- Configuration of CC Notification is now handled by the `Z-Wave Command Class Configurator` and functions
`AddNotification()` and `DefaultNotificationStatus()` have been obsoleted.
- Polling mode has been removed.

  - Implementation of deprecated polling mode has been removed from CC Notification.
  - This includes removing enum NOTIFICATION_STATUS_SET. Since poll mode is not supported anymore, notification status is the same for all commands.
  - CC Notification now supports push mode only.

### CC Supervision

The Command Class Supervision is refactored to no longer require initialization on the application
level. The initialization is now handled internally by ZAF on `ZAF_Init`.

The status updates can be enabled/disabled in the Simplicity Studio GUI.

The `cc_supervision_get_received_handler(...)` and `cc_supervision_report_recived_handler(...)` handler callbacks defined in `cc_supervision_handlers.h` have a dummy weakly-defined implementation and can be redefined by the applications.

### CC User Code

The User Code Command Class has been refactored so that all logic related to the Command Class is handled
internally by it. The Command Class had four extern functions that had to be implemented by the application.
`CC_UserCode_Set_handler`, `CC_UserCode_getId_handler`, `CC_UserCode_Report_handler` and
`CC_UserCode_UsersNumberReport_handler`. These functions are now implemented inside the command class
and two new functions were created:

- `CC_UserCode_reset_data` to reset all the data persisted by the
command class.
- `CC_UserCode_Validate` that contains all the validation logic.

Two configuration parameters related to this Command Class that belonged in the application now belong in the Command Class: `CC_USER_CODE_MAX_IDS` (formerly known as `USER_ID_MAX`)
and `CC_USER_CODE_DEFAULT` (formerly known as `DEFAULT_USERCODE`).

### CC Wake Up

The Command Class Wake Up was refactored to no longer require initialization on the application
level. The initialization is now handled internally by ZAF on `ZAF_Init`.

The `CC_WakeUp_setConfiguration()` API was removed from the applications. These parameters can be configured in the Simplicity Studio GUI under the Wake Up Command Class's component.

### CC Z-Wave Plus Info

The icon types for the root device and optionally for the endpoints is now configurable. The configuration
is handled by the Command Class.

The user icon type and installer icon type for the root device are defined in ZAF config and can be configured in Simplicity Studio.

If an application supports Command Class Multi Channel, the icon types of an endpoint must be configured in the app.cc_config file. See the configuration of Power Strip for an example.

## 7.18.x and Previous Releases

See the corresponding release notes.
