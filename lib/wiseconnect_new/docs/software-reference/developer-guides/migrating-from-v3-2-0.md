# Migrating from WiSeConnect™ SDK v3.2.0 to v3.3.0

## Table of Contents

- [Overview](#overview)
- [Migration Steps](#migration-steps)
  - [Update Files](#update-files)
  - [Update Macros](#update-macros)

## Overview

This is a guide for updating an existing application using the WiSeConnect™ SDK v3.2.0 to a v3.3.0 application.

There are few naming and file changes in v3.3.0 as compared to v3.2.0, mostly in order to standardize the names and improve the overall usage experience of the application programming interface (API). Migration requires the names everywhere to be updated in the existing code of an application.

## Migration Steps

In order to convert a WiSeConnect SDK v3.2.0 application to a v3.3.0 application,

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.2.0 names or interfaces with v3.3.0 names or interfaces. 

Refer to the tables in each of the sections that follow which map the v3.2.0 API elements to v3.3.0. In some instances, the differences between v3.2.0 and v3.3.0 are highlighted in **bold** text.
- [Update Files](#update-files)
- [Update Macros](#update-macros)
- [Update Types](#update-types)

### Update Files

- Removed the rsi_chip.h header file from the source. This header should not be used in the example files. If it is already included, please remove it. Ideally, the examples should compile without this file. If any issues arise due to the removal of rsi_chip.h, please add the necessary header files
- The method for ULP-timer instantiation has been updated. Consequently, new instance files "sl_ulp_timer_instances.h" and "sl_si91x_ulp_timer_common_config.h" have been added, while "sl_si91x_ulp_timer_init.h" has been removed.
- For the ADC peripheral, the instance names have been updated from version 3.2.0 (channel_0 to channel_15) to version 3.3.0 (channel_1 to channel_16). As a result, sl_si91x_adc_init_channel_0_config file has been removed from components\board\silabs\config\common_config, and the sl_si91x_adc_init_channel_16_config file has been added. alternatively, user can simply uninstall ADC component and install to resolve this.
- Replaced ROMDRIVER_PRESENT macro with peripheral specific macros and removed SLI_SI91X_MCU_MOV_ROM_API_TO_FLASH macro dependency. User is not expected to set any of these macros and would be taken up by individual drivers.
- New driver-level checks for I2S have been added. User can find detailed information about these checks in the Note section under the About Example Code section of the I2S primary and secondary example readmes.
- ULP I2S instance changed from I2S to ULP_I2S
- UART source clock changed to INTF_PLL and M4 Core source clock changed to SOC_PLL. On application beginning, both these PLLs are configured to run at 180MHz with XTAL 40MHz as reference 
- The RSI components are hidden in Studio because users need to use the SL components for a unified user experience. If necessary, to make a component visible, remove the following lines from the respective component(.slcc)
  ```C
  ui_hints: 
  visibility: never
  ```
- The SoC clock initialization has been decoupled from the driver files and moved to applications. New component added for clock configuration user needs to install Clock Manager component. Refer any peripheral sample app for clock configurations. 
- For **switch_m4_frequency()** api:
  - Ensure TA is not in powersave mode
  - For clock scaling, it is recommended to install Power manager component and use sl_si91x_power_manager_set_clock_scaling(sl_clock_scaling_t mode)
- For the **Sockets** module:
  - The `sl_si91x_socket_config_t` structure needs to be relocated from `sl_si91x_protocol_types.h` to `sl_si91x_socket_utility.h`.
  - The `sl_si91x_sock_info_query_t` and `sl_si91x_socket_info_response_t` structures needs to be relocated from `sl_si91x_protocol_types.h` to `sl_si91x_socket_support.h`.

### Update Macros

| **Module** | **v3.2.0** | **v3.3.0** |
|-------------| -----------| ---------- |
| Blink | SL_CATALOG_LED_PRESENT | SL_CATALOG_LED_LED0_PRESENT |
| Blink | SL_CATALOG_SIMPLE_LED_LED0_PRESENT | SL_CATALOG_SIMPLE_LED_PRESENT |
| button |  SL_CATALOG_BUTTON_BTN0_PRESENT | SL_CATALOG_BUTTON_PRESENT |
| button | SL_CATALOG_SIMPLE_BUTTON_BTN0_PRESENT | SL_CATALOG_SIMPLE_BUTTON_PRESENT |
| SiWx91x Device Management | SL_SI91X_TCP_IP_FEAT_HTTP_SERVER | N/A (removed from SDK) |
| SiWx91x Device Management | SL_SI91X_TCP_IP_FEAT_HTTPS_SERVER | N/A (removed from SDK) |
| SiWx91x Device Management | SL_SI91X_CUSTOM_FEAT_HTTP_SERVER_CRED_TO_HOST | N/A (removed from SDK) |
| SiWx91x Device Management | SL_SI91X_EXT_FEAT_TELEC_SUPPORT | N/A (removed from SDK) |
| SiWx91x Device Management | SL_SI91X_EXT_TCP_IP_HTTP_SERVER_BYPASS | N/A (removed from SDK) |
| I2S0 | SL_I2S_RESOLUTION | SL_I2S0_RESOLUTION |
| I2S0 | SL_I2S_SAMPLING_RATE | SL_I2S0_SAMPLING_RATE |
| I2S1 | SL_I2S_RESOLUTION | SL_ULP_I2S_RESOLUTION |
| I2S1 | SL_I2S_SAMPLING_RATE | SL_ULP_I2S_SAMPLING_RATE | 
| RTE_USART0_CLK_SRC | USART_ULPREFCLK | USART_INTFPLLCLK |
| RTE_UART1_CLK_SRC | USART_ULPREFCLK | USART_INTFPLLCLK |
### Update Types

| **Module** | **v3.2.0** | **v3.3.0** |
|------------|------------|------------|
|  Wi-Fi | sl_wifi_advanced_client_configuration_t  | Removal  of **eap\_flags** member |
|  MQTT  | sl_mqtt_client_connection_state_t        | Removal  of **SL\_MQTT\_CLIENT\_REMOTE\_TERMINATE** member |
