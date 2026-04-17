# Migration Guide {#migration-guide}

This guide shows how to migrate projects from previous SDKs to a newer one.

# 8.0.1 {#migrate-section-8-0-1}

# 8.0.0 {#migrate-section-8-0-0}

## zpal_power_manager APIs migration
Following the removal of zpal_power_manager, here are the APIs you need to remove from your code:
- `zpal_pm_lock`, `zpal_pm_relock`, `zpal_get_max_timeout`, `zpal_pm_lock_is_active`, `zpal_pm_register_domain`, `zpal_pm_lock_cancel`, `zpal_zw_pm_event_handler`, `zpal_pm_halt`
- `zw_power_manager_lock`, `zw_power_manager_relock`, `zw_power_manager_is_active`, `zw_power_manager_lock_cancel`, `zw_power_manager_init`

These APIs can be replaced with three different modules depending on your usage:
- If you want to handle the power manager "state" (e.g., EM state) of the ARM core, you can now use the dedicated platform APIs of [sl_power_manager](https://docs.silabs.com/gecko-platform/3.0/service/api/group-power-manager).
- If you want to maintain the Z-Wave radio in an active state (RX active when no TX is in progress), you should use the new dedicated APIs `zpal_radio_request_stay_awake` or `zpal_radio_update_stay_awake` instead of the previous API: `zpal_pm_lock(ZPAL_PM_TYPE_USE_RADIO)`.
  - To revoke a previous request, use `zpal_radio_revoke_stay_awake`.
- If you want to maintain deep sleep mode, equivalent to the previous `zpal_pm_lock(ZPAL_PM_TYPE_DEEP_SLEEP)`, you can now use the new component module called `zw_shutdown_manager`. The logic of this module works the same way as `sl_power_manager`: adding or subtracting 1 to a global counter based on lock requests. When count == 0, the module considers the device ready to enter shutdown mode (EM4).

APIs of this new module are:
- `zw_shutdown_manager_add_lock` to request a lock
- `zw_shutdown_manager_release_lock` to release a lock
- `zw_shutdown_manager_take_temporary_lock` to take a temporary lock for a given duration

## TxPower type and definitions clarification
Some type and definitions have been renamed to avoid confusion:

| Old name            | New name                  |
|---------------------|---------------------------|
| `zpal_tx_power_t`   | `zpal_tx_power_decidbm_t` |
| `ZW_TX_POWER_10DBM` | `ZW_TX_POWER_100_DDBM`    |
| `ZW_TX_POWER_14DBM` | `ZW_TX_POWER_140_DDBM`    |
| `ZW_TX_POWER_20DBM` | `ZW_TX_POWER_200_DDBM`    |

If your application uses these definitions, you need to replace them with the new names.

## Energy modes

Some peripherals may not work in the newly introduced [EM1P](https://docs.silabs.com/rail/latest/efr32-migration-guide-for-proprietary-apps/04-em1p-on-efr32xg22) energy mode (for example, TIMER0). To address this, an EM1 requirement may be added to the power manager module by the application.

To add an EM1 lock to the application, include the power manager header, and call the following function during initialization.

```C
#include "sl_power_manager.h"

sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
```

Timers used by the PWM component in Led Bulb and Power Strip applications only work in EM1 mode, therefore, updating these applications require the above manual changes to be made.

## Hardware Configuration

A new hardware configuration API has been introduced, allowing to control the Energy Management Unit (EMU).  
The following configuration values, previously defined in the SLC project files (.slcp)
had been hard-coded to specific values in prior releases, regardless of the user-configured values.  
These values can now be set via the Z-Wave Core Component (if using the GUI) or via the configuration header `zw_hardware_config.h`:

| Old configuration name                      | New configuration name        |
|---------------------------------------------|-------------------------------|
| -                                           | `ZW_DCDC_CONFIG`\*            |
| `SL_DEVICE_INIT_EMU_EM4_PIN_RETENTION_MODE` | `ZW_EM4_PIN_RETENTION_MODE`   |
| `SL_DEVICE_INIT_EMU_EM4_STATE`              | `ZW_EM4_INITIAL_STATE`        |
| -                                           | `ZW_EM23_VOLTAGE_SCALE`       |
| `SL_DEVICE_INIT_EMU_EM4_RETAIN_LFXO`        | `ZW_EM4_RETAIN_LFXO`          |
| `SL_DEVICE_INIT_EMU_EM4_RETAIN_LFRCO`       | `ZW_EM4_RETAIN_LFRCO`         |
| `SL_DEVICE_INIT_EMU_EM4_RETAIN_ULFRCO`      | `ZW_EM4_RETAIN_ULFRCO`        |

\* The `ZW_DCDC_CONFIG` configuration value is still hard-coded to `EDCDCMODE_AUTO`, regardless of the value of this setting. A workaround is currently not available.

## RAIL Power Manager Initialization

FLiRS applications (e.g. the Door Lock Keypad application) require the RAIL Power Manager Initialization configuration to be enabled. To do this on an upgraded project, set the value to 1 in the `config/sl_rail_util_power_manager_init_config.h` file:

```
#define SL_RAIL_UTIL_RAIL_POWER_MANAGER_INIT 1
```

# 7.24.1 {#migrate-section-7-24-1}

## How to migrate the Wall Controller sample application

The configuration macro `SHORT_BUTTON_PRESS_DURATION` was renamed to `SHORT_BUTTON_PRESS_DURATION_MS`.

Modify the `app_button_press_btn_0_handler` function to use the new macro:
```diff
- sl_sleeptimer_start_timer_ms(&my_sleeptimer_handle, SHORT_BUTTON_PRESS_DURATION, sleeptimer_cb, NULL, 0, 0);
+ sl_sleeptimer_start_timer_ms(&my_sleeptimer_handle, SHORT_BUTTON_PRESS_DURATION_MS, sleeptimer_cb, NULL, 0, 0);
```

# 7.24.0 {#migrate-section-7-24-0}

## Power Manager
It is strongly recommended that applications rely on the zw_power_manager abstraction and do not use the zpal_pm APIs, see `zw_power_manager_ids.h` for available wrapper.
- @note: an application must call zw_power_manager_init prior to using zw_power_manager APIs. If not, the lock operation will be ignored. This call is introduced in ZAF_Init(). For NCP Serial API (End Device/Controller) applications, it is done in ApplicationInit() as these apps do not depend on ZAF layer. If your application does not depend on ZAF, please add this function call prior to using zw_power_manager APIs.

If the application still needs to use zpal_pm API, then:
Any application code previously using zpal_pm_register, zpal_pm_stay_awake, zpal_pm_cancel are expected to align with the following rules:
- `zpal_pm_register` is deprecated. Instead, a power lock ID must be defined in either ZPAL_PM_AppRadioID or ZPAL_PM_AppDeepSleepID. This ID must be used in all operations of this power lock.
- Any call to zpal_pm_stay_awake, zpal_pm_cancel should now be aligned with zw_power_manager_(lock/relock/lock_cancel)

## Watchdog

Apps are now responsible for the watchdog activation
by calling "zpal_watchdog_init" and "zpal_enable_watchdog" functions consecutively.
(see sample applications for reference).

## Logging System

The Debug Print component was replaced by the ZPAL Log API, which allows filtering
log messages by source component and log level.

To ensure compatibility with the new logging system, some files relying on
Debug Print may have to be modified by including the header `zpal_log.h`,
replacing `DPRINT` and `DPRINTF` with `ZPAL_LOG`
and removing any references to the old component.

To enable debug output, either use the *Configure* button of the *Z-Wave Log* component in Simplicity Studio
or edit **zw_log_config.h** directly in the project's `config` directory:
 - configure the output channels for the 4 log levels (debug, info, warning, error)
   by setting `ZW_LOG_CHANNEL_<log level>` to an existing IO Stream instance (`vcom`, `rtt`, etc.)
 - set the relevant defines `ZPAL_LOG_ENABLE_<component name>` to `1`.

To extend the ZPAL Log API with a new component, add a new entry to the enum `zpal_log_component_t`
and update the helper macros in **zpal_log_internal.h**.

## main.c

The file `main.c` is now generated by the *System Setup (sl_main)* component.
If you previously extended this file with your own initialization functions,
these should be relocated and registered using the Event Handler API.
The `main.c` file should then be removed.

For more information, refer to the documentation for *System Initialization and Action Processing*.

## User Credential Command Class

If your project uses or redefines the `CC_UserCredential_move_credential_and_report` function:
remove the `destination_credential_slot` parameter and rename the
`source_credential_slot` parameter to `credential_slot`.

The following structs were renamed:

| Old name                          | New name                           |
|-----------------------------------|------------------------------------|
| `u3c_user`                        | `u3c_user_t`                       |
| `u3c_credential`                  | `u3c_credential_t`                 |
| `u3c_event_data_validate`         | `u3c_event_data_validate_t`        |
| `u3c_event_data_learn_read_done`  | `u3c_event_data_learn_read_done_t` |
| `u3c_credential_metadata`         | `u3c_credential_metadata_t`        |
| `u3c_credential_learn_event_data` | `u3c_event_data_learn_start_t`     |

## Application Specific Hardware Initialization

In `app.c`, ensure that the function `app_hw_init()` is invoked by the `ApplicationInit` function (after `ZAF_Init`, if present). In an unmodified project, this can be done by removing the `#if` preprocessor directives:

```diff
- #if (!defined(SL_CATALOG_SILICON_LABS_ZWAVE_APPLICATION_PRESENT) && !defined(UNIT_TEST))
    #include "app_hw.h"
- #endif
```

```diff
- #if (!defined(SL_CATALOG_SILICON_LABS_ZWAVE_APPLICATION_PRESENT) && !defined(UNIT_TEST))
    app_hw_init();
- #endif
```
## Waking Button Press Detection for Sleeping Applications

If your application has the *Sleeping Reporting* role type, extend the project's .slcp file with the following configuration and regenerate the project:

```yaml
configuration:
  (...)
  - name: APP_BUTTON_PRESS_WAKEUP_DELAY
    value: 46 # Average time for Sleep Timer to start after a reset when built in ocelot
    condition: [device_sdid_210]
  - name: APP_BUTTON_PRESS_WAKEUP_DELAY
    value: 285 # Average time for Sleep Timer to start after a reset when built in margay
    condition: [device_sdid_235]
```

This will ensure that the application detects the correct button press duration when waking up from a sleeping state via a button press.

## RAIL Power Manager Initialization

The `SL_RAIL_UTIL_RAIL_POWER_MANAGER_INIT` configuration value should be set to `0` for the Door Lock Keypad application and to `1` for all other applications. During the SDK update, this configuration value is automatically set to `1` for all applications, including Door Lock Keypad. Therefore, for the Door Lock Keypad application, it must be set manually to `0`; other applications are handled automatically.

This is necessary for a proper initialization sequence of the power manager component for all applications.

## How to upgrade sample applications

In the following sections, an example is shown about the upgrade steps of each sample application.

### Steps for an SDK upgrade:

1. Upgrade SDK for the selected application.
    - See the corresponding guide: [Upgrading a Project to a New Software Version](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-getting-started/project-upgrade-new-gsdk-version)
2. Make changes according to the steps described in the current document.
3. Regenerate the application (force regeneration).
4. Build the application

> **Note:** This guide is not exhaustive. Please ensure that any changes you make follow the steps outlined above.

### General steps for all applications

#### All .c files

- Remove `#include "DebugPrint.h"` and `#include "DebugPrintConfig.h"`
- Remove any references to DEBUGPRINT because it is deprecated.
- Replace all `DPRINT(...)` and `DPRINTF(...)` API calls to `ZPAL_LOG_DEBUG(ZPAL_LOG_APP, ...)`.
- Replace `DebugPrintf` with `ZPAL_LOG_INFO(ZPAL_LOG_APP, ...)`.
- Remove `DebugPrintConfig(...)` calls.
- Add `#include "zpal_log.h"` if `ZPAL_LOG_...` calls are used.

#### app.c of all applications

- Remove `m_aDebugPrintBuffer` variable and its references.
- Add `zpal_watchdog_init()` before `zpal_enable_watchdog(true)` in ApplicationInit.

#### <project_name>.slcp

- Remove `sl_system` and `sl_system_kernel` components if present.
- Add `sl_main` component.
- Remove main.c from source files.
  - This will be generated by the `sl_main` component after project regeneration.
  - If main.c was modified, it is advised to move custom code to a different file, e.g. `app.c`.
- Set the configuration value `SL_RAIL_UTIL_RAIL_POWER_MANAGER_INIT` to `0` for Door Lock Keypad, and to `1` for all the other applications.
- Add `clock_manager` component.
- Add the following configuration values:
  - `name: SL_CLOCK_MANAGER_WDOG0CLK_SOURCE, value: CMU_WDOG0CLKCTRL_CLKSEL_LFRCO`
- Add the following provides:
```
provides:
  - name: zwave_sample_application
  - name: <project_name> # e.g. zwave_soc_power_strip
```

### NCP Zniffer and Zniffer PTI

NCP Zniffer and Zniffer PTI applications were merged into one application. Now, NCP Zniffer contains both applications' functionalities. It is not possible to upgrade these applications directly. Instead, please create a new project using the latest SDK and manually transfer any changes made in the previous project to the new one.

### NCP Serial API Controller

SDK upgrades are not supported for this application. To migrate, please create a new project using the latest SDK and manually transfer any changes from the previous project to the new one.

### Door Lock Keypad

#### app.c

- Add `#include "app_pm_transition_event.h"`
- Add these lines after `RadioConfig = zaf_get_radio_config();`
  ```
  #ifdef SL_CATALOG_ZW_PM_TRANSITION_EVENT_PRESENT
    // register callback from power manager transitions
    ZW_PmTransitionEventInit();
  #endif
  ```
- Remove `zw_cli_sleeping_util_prevent_sleeping_timeout(ZW_CLI_SLEEPING_WAKEUP_TIME_AFTER_RESET);`
- CLI baud rate was changed to 9600, which enables the device to receive CLI commands in EM2 sleep. There is no need to prevent sleeping for this application. To apply the new configuration on the baud rate, see the section [zwave_soc_door_lock_keypad.slcp](#zwave_soc_door_lock_keypadslcp).

#### app_cli_cc_user_credential.c

- Rename `u3c_user` struct type to `u3c_user_t`.
- Rename `u3c_credential` struct type to `u3c_credential_t`.
- Rename `u3c_event_data_validate` struct type to `u3c_event_data_validate_t`.
- Rename `u3c_event_data_learn_read_done` to `u3c_event_data_learn_read_done_t`.
- Rename `u3c_credential_metadata` to `u3c_credential_metadata_t`.
- Rename `u3c_credential_learn_event_data` to `u3c_event_data_learn_start_t`.
- When calling `CC_UserCredential_move_credential_and_report(...)`, remove `slot` argument.
- In function definition `u3c_move_credential_slot(...)`, use `CC_UserCredential_move_credential(...)` instead of `CC_UserCredential_move_credential_and_report(...)`.
  - According to the specification, this action is no longer supported. The lifeline group cannot be notified about this change.

#### app_credentials.c

- Rename `u3c_user` struct type to `u3c_user_t`
- Rename `u3c_credential` struct type to `u3c_credential_t`
- Rename `u3c_credential_learn_event_data` to `u3c_event_data_learn_start_t`

#### zwave_soc_door_lock_keypad.slcp

- Do the steps described in [General steps for all applications](#general-steps-for-all-applications)
- Additionally, add the following configuration to the .slcp file:
  ```
  - name: SL_IOSTREAM_EUSART_VCOM_BAUDRATE
    value: 9600 # In EM2 the eusart is working only with 9600 baud
  - name: SL_IOSTREAM_EUSART_VCOM_ENABLE_HIGH_FREQUENCY # Disable high frequency mode for EUSART to work in EM2
    value: 0
  - name: SL_CLOCK_MANAGER_WDOG0CLK_SOURCE
    value: CMU_WDOG0CLKCTRL_CLKSEL_LFRCO
  - name: SL_CLOCK_MANAGER_EUSART0CLK_SOURCE
    value: CMU_EUSART0CLKCTRL_CLKSEL_LFRCO
  ```

### Led Bulb

All steps are explained in section [General steps for all applications](#general-steps-for-all-applications).

### Multilevel Sensor

#### app.c

- Add `#include "app_pm_transition_event.h"`.
- Add these lines after `RadioConfig = zaf_get_radio_config();`:
  ```
  #ifdef SL_CATALOG_ZW_PM_TRANSITION_EVENT_PRESENT
    // register callback from power manager transitions
    ZW_PmTransitionEventInit();
  #endif
  ```

#### app_btn_handler.c

- See [General steps for all applications](#general-steps-for-all-applications)

#### app_sleep_callbacks.c

- Add the following include:
  ```
  #include "sl_board_control_config.h"
  #if SL_BOARD_ENABLE_SENSOR_RHT || SL_BOARD_ENABLE_SENSOR_IMU
  #include "sl_board_control.h"
  #endif
  ```
- Add at the end of `void EMU_EM4PresleepHook(void)`:
  ```
  #if SL_BOARD_ENABLE_SENSOR_RHT
    sl_board_disable_sensor(SL_BOARD_SENSOR_RHT);
  #endif
  #if SL_BOARD_ENABLE_SENSOR_IMU
    sl_board_disable_sensor(SL_BOARD_SENSOR_IMU);
  #endif
  ```

#### MultilevelSensor_interface_icm20689.c

See [General steps for all applications](#general-steps-for-all-applications)

#### MultilevelSensor_interface_nosensor.c

See [General steps for all applications](#general-steps-for-all-applications)

#### MultilevelSensor_interface_sensor.c

See [General steps for all applications](#general-steps-for-all-applications)

### Switch On Off

All steps are explained in section [General steps for all applications](#general-steps-for-all-applications).

### Power Strip

All steps are explained in section [General steps for all applications](#general-steps-for-all-applications).

### Sensor PIR

#### app.c

- Add `#include "app_pm_transition_event.h"`.
- Add these lines after `RadioConfig = zaf_get_radio_config();`:
  ```
  #ifdef SL_CATALOG_ZW_PM_TRANSITION_EVENT_PRESENT
    // register callback from power manager transitions
    ZW_PmTransitionEventInit();
  #endif
  ```

#### app_btn_handler.c

- See [General steps for all applications](#general-steps-for-all-applications)

#### app_cli.c

- Add `#include <string.h>` to suppress compiler warning about the implicit declaration of the `strcmp` function.

#### SensorPIR_UserTask_DataAcquisition.c

- See [General steps for all applications](#general-steps-for-all-applications)

### Wall Controller

All steps are explained in section [General steps for all applications](#general-steps-for-all-applications).

# 7.23.0 {#migrate-section-7-23-0}

## How to migrate legacy HMIs to Silicon Labs based solution

### General Information
In the 7.23 Z-Wave release, Silicon Labs changed the legacy button and LED handling to the Silicon Labs platform provided solution. This document describes how to migrate the legacy human-machine interfaces (from 7.19) to the Silicon Labs based solution. This document can be divided into two parts: one will describe the new components that have been introduced in the 7.23 release; and the other part will describe how to migrate a 7.22 project to the 7.23 release.

### New components

#### SL Simple Button
The Simple Button Driver provides an implementation of the Generic Button API. This driver supports both active high and low buttons and configurable debouncing. This component is instantiable, meaning that several named instances can be created. For each instance a configuration is generated along with an initialization struct set according to this configuration. These instance defines and variables are available in a generated header file, **sl_simple_button_instances.h**. Selecting this component will also include the Simple Button Core component, which is the implementation of the Simple Button driver itself. [API Reference](https://docs.silabs.com/gecko-platform/latest/platform-driver/simple-button)

#### SL Simple LED
The Simple LED Driver provides an implementation of the Generic LED API. This driver supports controlling GPIO based on/off type LEDs. This component is instantiable, meaning that several named instances can be created. For each instance a configuration is generated along with an initialization struct set according to this configuration. These instance defines and variables are available in a generated header file, **sl_simple_led_instances.h**. Selecting this component will also include the Simple LED Core component, which is the implementation of the Simple LED driver itself. [API Reference (5.0.2)](https://docs.silabs.com/gecko-platform/5.0.2/platform-driver/simple-led)

#### SL App Button Press
The Button Press component is used for handling the button state, which can be:
- Button is not pressed,
- Button is pressed for a short duration,
- Button is pressed for a medium duration,
- Button is pressed for a long duration,
- Button is pressed for a very long duration and
- Button has just been pressed down.

This component is built on SL Simple Button. Button press duration for various lengths is configurable and each end user can configure it according to their needs.

### Differences in the project structure
In this document, the most complex project will be used as an example, which is the Z-Wave SoC Door Lock Keypad project. In the following table, the differences in the source files can be seen between the 7.22 and 7.23 projects.

| 7.22 Top Project Stucture | 7.23 Top Project Stucture      |
|---------------------------|--------------------------------|
|                           |   app_btn_handler.c            |
|                           |   app_cc_battery_callbacks.c   |
|                           |   app_cc_door_lock_callbacks.c |
|   app_cc_event_handlers   |   app_cc_event_handlers        |
|   app_cli.c               |   app_cli.c                    |
|   app_cli.h               |   app_cli.h                    |
|   app_credentials.h       |   app_credentials.h            |
|   app.c                   |   app.c                        |
|   events.h                |   events.h                     |
|   main.c                  |   main.c                       |

The main differences are:
- app_btn_handler.c (API change)
- app_cc_battery_callbacks.c
- app_cc_door_lock_callbacks.c (API change)

Crucially, with this release, application related sources files like `platform\SiliconLabs\AppsHw\src\DoorLockKeyPad\DoorLockKeyPad_hw.c` were removed. All application related logic was moved to the application project's directory. In this case, **DoorLockKeyPad_hw.c** was split into three separated sources: **app_btn_handler.c**, **app_cc_battery_callbacks.c** and **app_cc_door_lock_callbacks.c**.

#### Migrating the app_btn_handler.c file

In previous releases, button and LED configurations were spread out across multiple files. From the 7.23 release, Silicon labs switched to the standard SiSDK components to handle those peripherals, such as sl_simple_button and sl_simple_led. Because of those components, we can use Simplicity Studio's Pin Tool to remap a button or LED pin in the application. There is no need to do any additional configuration regarding these peripherals.

##### Buttons
Previously, button handling was implemented in `platform\SiliconLabs\AppsHw\src\<application>\<application>_hw.c` source files, in this case **DoorLockKeyPad_hw.c**. Now, this logic can be found in the application's directory, in **app_btn_handler.c**. A helper SiSDK component, called app_button_press, is also used to generate events on button pressing, such as short, medium or long press events. The duration of these events can be configured in **app_button_press_config.h** header file, in the config directory inside of the application project's directory. In case you had any custom logic regarding the button handling in `platform\SiliconLabs\AppsHw\src\<application>\<application>_hw.c`, in this example, **DoorLockKeyPad_hw.c**, please move them to the app_btn_handler.c file in the application project's directory.

In app_btn_handler.c, only the **app_button_press_btn_0_handler** or **app_button_press_btn_1_handler** functions need to be handled, which take the duration as an input parameter. Those callbacks are called automatically when a button event is fired.

##### LEDs

LED handling was simplified, which means LEDs can be manipulated via sl_simple_led API calls. They can also be configured using the Pin Tool. No additional configuration is required. In case you are using functions prefixed with **Board_** to manipulate LEDs or GPIOs, please use the sl_simple_led and em_gpio components' API from 7.23 release.

#### Migrating the app_cc_<command class>_callbacks.c files

There are new source files in every application, named in the pattern `app_cc_<command class name>_callback.c`. In the new structure, each of those sources contain callbacks for a specific command class. These callbacks can be different in every application, e.g. battery report callback, which can be different in a real life application compared to a sample application. From the 7.23 release, such callbacks are exposed on the application level. In this example, battery command class related callbacks are in the application project's directory, named **app_cc_battery_callback.c**. If you had any custom implementation of command class specific callbacks in `platform\SiliconLabs\AppsHw\src\<application>\<application>_hw.c` files, in this case **DoorLockKeyPad_hw.c**, please move those implementations next to the application, into the related source file.
