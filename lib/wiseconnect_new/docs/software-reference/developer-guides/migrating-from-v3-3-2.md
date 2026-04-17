# Migrating from WiSeConnect™ SDK v3.3.2 to v3.3.3

## Table of Contents

- [Overview](#overview)
- [Migration Steps](#migration-steps)
  - [Update API Calls](#update-api-calls)
  - [Update Files](#update-files)
  - [Update Macros](#update-macros)

## Overview

This is a guide for updating an existing application using the WiSeConnect™ SDK v3.3.2 to a v3.3.3 application.

There are few naming and file changes in v3.3.3 as compared to v3.3.2, mostly in order to standardize the names and improve the overall usage experience of the application programming interface (API). Migration requires the names everywhere to be updated in the existing code of an application.

## Migration Steps

In order to convert a WiSeConnect SDK v3.3.2 application to a v3.3.3 application,

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.3.2 names or interfaces with v3.3.3 names or interfaces. 

Refer to the tables in each of the sections that follow which map the v3.3.2 API elements to v3.3.3. In some instances, the differences between v3.3.2 and v3.3.3 are highlighted in **bold** text.

- [Update API Calls](#update-api-calls)
- [Update Files](#update-files)
- [Update Macros](#update-macros)
- [Update Types](#update-types)

### Update Files

- By default, the sleep timer is used to run RTOS when tickless mode is enabled, and it internally operates SYSRTC timer. If SYSRTC component is installed while tickless mode is enabled, it may cause conflicts. To prevent this, avoid installing the SYSRTC component when tickless mode is enabled.
- To enable host PAD selection for GPIO pin numbers 25–30, use the function `sl_si91x_gpio_driver_enable_host_pad_selection()`. This function allows selecting and configuring the PAD for these specific GPIO pins, ensuring proper setup for host-side PADs.
- Use `sl_si91x_soc_nvic_reset()` API for system soft reset rather than the
`sl_si91x_soc_soft_reset()`
function, since this uses the WDT for soft reset, which is specifically intended for system reset recovery
- Explicitly configuring the clock for the Watchdog timer is no longer required, as it is now handled as part of default clock configurations

- | **Module** | **v3.3.2** | **v3.3.3** |
  |-------------| -----------| ---------- |
  | DEBUG UC | ULP_UART is being used for Debug prints | No migration required, just install and use "Debug UC" component if want to change instance |
  | DEBUG UC | USART0/UART1 is being used for Debug prints | ULP_UART instance becomes default for Debug prints. Install and use "Debug UC" component to change instance |
  | DEBUG UC | If user wants to select DEBUG instance, user has to go to rsi_debug.c file and change manually | Install "Debug UC" component.  By default, ULP UART instance is used. Provision for changing baudrate is also provided |

### Update Macros

| **Module** | **v3.3.2** | **v3.3.3** |
|-------------| -----------| ---------- |
|NONE|

### Update Types

| **Module** | **v3.3.2** | **v3.3.3** |
|------------|------------|------------|
| Wi-Fi | si91x_socket_type_length_value_t | sl_si91x_socket_type_length_value_t |