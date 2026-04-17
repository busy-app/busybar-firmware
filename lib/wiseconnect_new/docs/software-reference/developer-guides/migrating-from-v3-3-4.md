# Migrating from WiSeConnect™ SDK v3.3.4 to v3.4.0

## Table of Contents

- [Overview](#overview)
- [Migration Steps](#migration-steps)
  - [Update API Calls](#update-api-calls)
  - [Update Files](#update-files)
  - [Update Macros](#update-macros)
  - [Deprecated Macros](#deprecated-macros)
  - [Updated Default Clock Values](#updated-default-clock-values)
  - [Updated GPIO Driver](#updated-gpio-driver)

## Overview

This is a guide for updating an existing application using the WiSeConnect™ SDK v3.3.4 to a v3.4.0 application.

There are few naming and file changes in v3.4.0 as compared to v3.3.4, mostly in order to standardize the names and improve the overall usage experience of the application programming interface (API). Migration requires the names everywhere to be updated in the existing application.

## Migration Steps

In order to convert a WiSeConnect SDK v3.3.4 application to a v3.4.0 application,

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.3.4 names or interfaces with v3.4.0 names or interfaces. 

Refer to the tables in each of the sections that follow which map the v3.3.4 API elements to v3.4.0. In some instances, the differences between v3.3.4 and v3.4.0 are highlighted in **bold** text.
- [Update API Calls](#update-api-calls)
- [Update Files](#update-files)
- [Update Macros](#update-macros)
- [Update Types](#update-types)

### Update API Calls

- For **sl_si91x_select()** API:
  - Until the 3.3.4 release, if the callback argument was not provided (i.e., NULL) in the `sl_si91x_select()` API, the function would return zero on success and -1 on failure. However, starting with the 3.4.0 release, when the callback is not provided, the function now returns the total number of file descriptors in the three descriptor sets upon successful execution. It may return zero if the timeout expires, or -1 in case of errors.

- For **sl_si91x_fwup_load()** API:
  - Before WC-3.4.0, the firmware update process returned a success code of 0x10003 upon successful completion. However, this code had an overlap with an existing error code, specifically the “No AP Found” error. To resolve this issue, the firmware update return codes have been restructured. Starting with WC-3.4.0, a prefix of 0xdd has been added to all firmware update return codes. This ensures a unique and easily identifiable range of return values for firmware updates.

- For **sl_si91x_setsockopt_async** API:
  - The term "async" has been removed from the API as setting an option on a socket is not related to the socket being synchronous/asynchronous. Hence, the name of the API has been updated to `sl_si91x_setsockopt`.

- For **sl_net_host_get_by_name** API:
  - Starting from WC-3.4.0 release, this API has renamed to `sl_net_dns_resolve_hostname`, to enhance the user accessibility.

- For **sl_net_set_credentials** API:
  - Starting from 3.4.0 release, a check has been added to ensure that if the credential is NULL or the credential length is zero, the `sl_net_set_credentials()` API will return an error `SL_STATUS_INVALID_PARAMETER`.

- Removed **sl_si91x_set_custom_sync_sockopt** API:
  - Starting from WC-3.4.0, the `setsockopt` API of BSD can be used to set SiWx91x options that were previously supported by `sl_si91x_set_custom_sync_sockopt`.

- Removed **sl_si91x_get_custom_sync_sockopt** API:
  - Starting from WC-3.4.0, the `getsockopt` API of BSD can be used to get SiWx91x options values that were previously supported by `sl_si91x_get_custom_sync_sockopt`.

- Removed **sl_si91x_m4_sleep_wakeup** API:
  - Starting from WC-3.4.0, the `sl_si91x_power_manager_sleep` API of Power Manager should be used in power-save applications without tickless idle mode that were previously supported by `sl_si91x_m4_sleep_wakeup`.
  - The `sl_si91x_power_manager_sleep` API is defined in the `sl_si91x_power_manager.h` file. Therefore, include this header file in power-save applications.

- Removed `sl_si91x_soc_soft_reset()` API references from the SDK

- Starting from version WC-3.4.0 of the SDK, important changes have been made regarding the management of sockets within the APIs sl_wifi_deinit(), sl_wifi_disconnect(), and sl_wifi_stop_ap(), as well as in cases where the Device Under Test (DUT) station encounters rejoin failures.
The SDK no longer automatically forcefully closes any open sockets in these situations. As a result, it is now the user's or application's responsibility to ensure that sockets are properly closed. If this step is overlooked, the sockets will remain in a disconnected or open state, which could result in unintended behavior in the application.


### Update Files

- NA

### Update Macros

| **Module**  |     **v3.3.4**                |         **v3.4.0**             |
|-------------| ------------------------------| -------------------------------|
| Wi-Fi       | `SO_CERT_INDEX`               | `SL_SO_CERT_INDEX`             |
| Wi-Fi       | `SO_HIGH_PERFORMANCE_SOCKET`  | `SL_SO_HIGH_PERFORMANCE_SOCKET`|
| Wi-Fi       | `SO_TLS_SNI`                  | `SL_SO_TLS_SN`                 |
| Wi-Fi       | `SO_TLS_ALPN`                 | `SL_SO_TLS_ALPN`               |
| Crypto      | `SL_SI91x_SHA_LEN_INVALID`    | `SL_SI91X_SHA_LEN_INVALID`     |
- Note: The custom socket option macros have been renamed with the prefix "SL" to enhance readability.
- Note: The crypto macro has been updated in compliance with Silicon Labs naming conventions.

| **Module**  | **v3.3.4**                             | **v3.4.0**                                    |
|-------------|----------------------------------------|-----------------------------------------------|
| Wi-Fi       | `SL_TRANSCEIVER_CHANNEL_NO`            | `SL_WIFI_TRANSCEIVER_CHANNEL_NO`              |
| Wi-Fi       | `SL_TRANSCEIVER_TX_POWER`              | `SL_WIFI_TRANSCEIVER_TX_POWER`                |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_RETRANSMIT_COUNT` | `SL_WIFI_TRANSCEIVER_DEFAULT_RETRANSMIT_COUNT`|
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_BE_CWMIN`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BE_CWMIN`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_BE_CWMAX`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BE_CWMAX`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_BE_AIFSN`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BE_AIFSN`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_BK_CWMIN`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BK_CWMIN`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_BK_CWMAX`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BK_CWMAX`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_BK_AIFSN`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_BK_AIFSN`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_VI_CWMIN`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VI_CWMIN`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_VI_CWMAX`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VI_CWMAX`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_VI_AIFSN`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VI_AIFSN`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_VO_CWMIN`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VO_CWMIN`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_VO_CWMAX`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VO_CWMAX`    |
| Wi-Fi       | `DEFAULT_TRANSCEIVER_QOS_VO_AIFSN`     | `SL_WIFI_TRANSCEIVER_DEFAULT_QOS_VO_AIFSN`    |
- Note: The transceiver configuration macros have been renamed with the prefix "SL_WIFI" to ensure consistency 
and improve readability.

### Update Typedefs
| **Module**  |     **v3.3.4**                      |         **v3.4.0**                                 |
|-------------| ------------------------------------| ---------------------------------------------------|
| Wi-Fi       | `receive_data_callback`             | `sl_si91x_socket_receive_data_callback_t`          |
| Wi-Fi       | `accept_callback`                   | `sl_si91x_socket_accept_callback_t`                |
| Wi-Fi       | `data_transfer_complete_handler`    | `sl_si91x_socket_data_transfer_complete_handler_t` |
| Wi-Fi       | `select_callback`                   | `sl_si91x_socket_select_callback_t`                |
| Wi-Fi       | `remote_socket_termination_callback`| `sl_si91x_socket_remote_termination_callback_t`    |
- Note: The above typedefs were updated in compliance with Silicon Labs coding standards.

### Update Types

| **Module** | **v3.3.4** | **v3.4.0** |
|------------|------------|------------|
| Wi-Fi   | `sl_si91x_performance_profile_t` | The `sl_si91x_performance_profile_t` enum variables have been renamed for clarity, with `STANDBY_POWER_SAVE` updated to `DEEP_SLEEP_WITHOUT_RAM_RETENTION` and `STANDBY_POWER_SAVE_WITH_RAM_RETENTION` updated to `DEEP_SLEEP_WITH_RAM_RETENTION`.   |
| Crypto | `sl_si91x_crypto_sha_mode_t`      | The `sl_si91x_crypto_sha_mode_t` enum constants have been renamed to adhere to naming conventions with `SL_SI91x_SHA_xxx` updated to `SL_SI91X_SHA_xxx` where xxx represents modes of SHA.|
| Crypto | `sl_si91x_sha_length_t`           | The `sl_si91x_sha_length_t` enum constants have been renamed to adhere to naming conventions with `SL_SI91x_SHA_xxx_DIGEST_LEN` updated to `SL_SI91X_SHA_xxx_DIGEST_LEN` where xxx represents modes of SHA.|
| Wi-Fi  | `SL_WIFI_STATS_AYSNC_EVENT` enum variable in `sl_wifi_event_t` | `SL_WIFI_STATS_ASYNC_EVENT` enum variable in `sl_wifi_event_t`|
|Memory configuration components |si917_memory_default_config|si91x_memory_default_config|
|Memory configuration components |si917_mem_config_1|si91x_mem_config_1|
|Memory configuration components |si917_mem_config_2|si91x_mem_config_2|
|Memory configuration components |si917_mem_config_3|si91x_mem_config_3|

### Deprecated Macros

- Deprecated **MAX_QUERY_PARAMETERS** and **MAX_HEADER_BUFFER_LENGTH** macros.
  > **Note**: From WC-3.4.0, the `MAX_QUERY_PARAMETERS` and `MAX_HEADER_BUFFER_LENGTH` macros are deprecated. It's recommended to use updated macros to ensure compatibility with future releases.

  | **Module**   | **v3.3.4**                   | **v3.4.0**                                 |
  |--------------|------------------------------|--------------------------------------------|
  | HTTP Server  | `MAX_QUERY_PARAMETERS`       | `SL_HTTP_SERVER_MAX_QUERY_PARAMETERS`      |
  | HTTP Server  | `MAX_HEADER_BUFFER_LENGTH`   | `SL_HTTP_SERVER_MAX_HEADER_BUFFER_LENGTH`  |

 
### Updated Default Clock Values

As part of the migration from WiSeConnect™ SDK v3.3.4 to v3.4.0, there have been changes to the default clock configurations to improve system stability and performance. These changes are as follows:

**High-Frequency Clocks (M4 Core, and PLL)**

  - The default clock source for the M4 Core and Phase-Locked Loop (PLL) has been revised from the ULP_MHZ_RC_CLK oscillator to EXT_40MHZ_CLK crystal oscillator.

**Reason for Change**

  The EXT_40MHZ_CLK crystal oscillator provides a more stable and accurate clock source compared to the ULP_MHZ_RC_CLK oscillator. So, configuration of ULP_MHZ_RC_CLK is removed for custom applications. This change enhances the overall performance and reliability of the system, particularly in applications requiring precise timing and high-frequency operations.

**User Impact**

- In some sample apps, where the default M4 Core clock was configured to 32MHz, it has been modified to 40MHz sourcing from the external crystal. The same is configured as the default M4 core clock in PS3-Powersave mode.
- In user applications where:
  - The M4 Core clock configuration should not be performed within the application.
  - Any peripheral initialization/configuration using 32MHz RC should be re-configured using 40MHz as the source.

**Low-Frequency Clocks (LF_FSM and SysRTC)**

- The default clock source for the Low-Frequency Finite State Machine (LF_FSM) and System Real-Time Clock (SysRTC) has been revised from the KHZ_RC_CLK_SEL to the KHZ_XTAL_CLK_SEL.

**Reason for Change**

The XTAL 32.768 kHz crystal oscillator provides a more stable and accurate clock source compared to the 32 kHz RC oscillator. This adjustment resolves timing inconsistencies observed when using the 32 kHz RC clock, ensuring more reliable and accurate timekeeping for low-frequency operations.

**Note**

If the 32.768 kHz XTAL is not available on the custom board designed with SiWx917 IC:
  - If using any timer peripherals on the 917 MCU, failures will be observed.
  - If the custom application doesn't need accurate timers and plans to continue with 32 kHz RC, please use the following code snippet:

 ```c
    #include "rsi_power_save.h"
    #include "rsi_sysrtc.h"

    /* Configuring 32kHz RC Clock for LF-FSM */
    RSI_PS_FsmLfClkSel(KHZ_RC_CLK_SEL);

    /* Enable 32kHz RC clock to SYSRTC peripheral */
    rsi_sysrtc_clk_set(RSI_SYSRTC_CLK_32kHz_RC, 0u);
```

By following these steps, you can ensure a smooth transition to the new default clock configurations in WiSeConnect™ SDK v3.4.0, resulting in improved system stability and performance.

Please refer [SiWG917 SoC Errata](https://www.silabs.com/documents/public/errata/siwg917-soc-ic-errata.pdf) and [SiWG917 SoC Module Errata](https://www.silabs.com/documents/public/errata/siwg917-soc-module-errata.pdf)  for more details.

### Updated GPIO Driver

As part of the migration from WiSeConnect™ SDK v3.3.4 to v3.4.0, we recommend using the GPIO driver instead of the GPIO peripheral code.

**Prerequisite**

The "GPIO" component must be installed in order to use the GPIO Driver APIs.

**GPIO Application Usage**

In GPIO low level peripheral driver, GPIO IRQ handlers were defined within the application code. However, in v3.4.0, these handlers are now handled by the GPIO Driver. Therefore, it is recommended not to define IRQ handlers in the application. Instead, you should register a callback function, which will be invoked by the GPIO IRQ handler from the driver.


| **Module**                 | **v3.3.4**                     | **v3.4.0**                     |
|----------------------------|--------------------------------|--------------------------------|
| GPIO Interrupt Configuration  | sl_gpio_configure_interrupt(port, pin, int_no, flags);<br>NVIC_EnableIRQ(IRQn);<br>NVIC_SetPriority(IRQn, priority); | sl_gpio_driver_configure_interrupt(*gpio, int_no, flags, gpio_callback, *avl_intr_no)|
| GPIO Interrupt Handler<br>(example for PIN0)  | PIN_IRQ0_Handler(void)  | Register Callback function using configure interrupt API<br>gpio_pin_interrupt0_callback(pin_intr)<br> (example callback function name)|
|                            |                                |

Please refer to the example applications that demonstrate the usage of the GPIO.

  | **Demonstration**          | **Example**                    |
  |----------------------------|--------------------------------|
  | HP GPIO Usage              | sl_si91x_gpio_detailed_example |
  | ULP GPIO Usage             | sl_si91x_gpio_ulp_example      |
  | UULP GPIO Usage            | sl_si91x_gpio_uulp_example     |
  | GPIO Interrupts Usage      | sl_si91x_gpio_example          |
  | GPIO Goup Interrupts Usage | sl_si91x_gpio_group_example    |
  |                            |                                |

**Reason for Change**

These improvements aim to simplify the use of GPIO APIs and enhance usability, particularly with regard to GPIO interrupts. 

