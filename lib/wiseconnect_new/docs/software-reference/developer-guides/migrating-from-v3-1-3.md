# Migrating from WiSeConnect™ SDK v3.1.3 to v3.1.4

## Table of Contents

- [Overview](#overview)
- [Migration Steps](#migration-steps)
  - [Update API Calls](#update-api-calls)
  - [Update Types](#update-types)
  - [Update Constants](#update-constants)
  - [Update Component Usage](#update-component-usage)
  - [Update Macros](#update-macros)

## Overview

This is a guide for updating an existing application using the WiSeConnect™ SDK v3.1.3 to a v3.1.4 application.

There are few naming and interface changes in v3.1.4 as compared to v3.1.3, mostly in order to standardize the names and improve the overall usage experience of the application programming interface (API). Migration requires the names everywhere to be updated in the existing code of an application.

## Migration Steps

In order to convert a WiSeConnect SDK v3.1.3 application to a v3.1.4 application,

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.1.3 names or interfaces with v3.1.4 names or interfaces. 

Refer to the tables in each of the sections that follow which map the v3.1.3 API elements to v3.1.4. In some instances, the differences between v3.1.3 and v3.1.4 are highlighted in **bold** text.
- [Update API Calls](#update-api-calls)
- [Update Types](#update-types)
- [Update Constants](#update-constants)
- [Update Component Usage](#update-component-usage)
- [Update Macros](#update-macros)

### Update API Calls

| **Module**   | **v3.1.3** | **v3.1.4** |
|--------------|------------|------------|
| Wi-Fi | int16_t si91x_get_socket_mss(int32_t socketIndex)|int16_t **sl\_si91x\_get\_socket\_mss**(int32_t socketIndex)|

### Update Types

| **Module** | **v3.1.3** | **v3.1.4** |
|------------|------------|------------|
|  Wi-Fi | sl_wifi_device_configuration_t | Addition of **efuse\_data\_type** member |

### Update Constants

| **Module** | **v3.1.3** | **v3.1.4** |
|------------|------------|------------|
| Wi-Fi | sl_wifi_transmit_test_configuration | sl_wifi_default_transmit_test_configuration |

### Update Macros

| **v3.1.3** | **v3.1.4** |
|----------------|----------------|
| SL_SI91X_EXT_FEAT_FRONT_END_VIRTUAL_SWITCH | SL_SI91X_EXT_FEAT_FRONT_END_INTERNAL_SWITCH |