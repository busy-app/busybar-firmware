# Migrating from WiSeConnect™ SDK v3.4.1 to v3.4.2

## Table of Contents

- [Overview](#overview)
- [Migration Steps](#migration-steps)
  - [Update Application name](#update-application-name)
  - [Deprecated Macros](#deprecated-macros)

## Overview

This guide is used for updating an existing application using the WiSeConnect™ SDK v3.4.1 to v3.4.2 application.

There are few naming and file changes in v3.4.2 as compared to v3.4.1, mostly in order to standardize the names, and to improve the overall usage experience of the Application Programming Interface (API). Migration requires the names everywhere to be updated in the existing application.

## Migration Steps

In order to convert a WiSeConnect SDK v3.4.1 application to v3.4.2 application,

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.4.1 names or interfaces with v3.4.2 names or interfaces. 

Refer to the tables in respective sections that follow which map v3.4.1 API elements to v3.4.2. In some instances, the differences between v3.4.1 and v3.4.2 are highlighted in **bold** text.




### Update Application name

**Note:** The following elements are applicable to both NCP and SoC modes.

- Starting from the 3.4.2 release, the application **wifi_firmware_flashing_from_host_uart_xmodem_bootloader_ncp** has been renamed to **fw_update_bootloader_xmodem** to reduce the application name length and resolve studio build issues caused by long path names.

### Deprecated Macros

**Note:** The following elements are applicable to both NCP and SoC modes.

- Deprecated **SL_SI91X_EXT_FEAT_AP_BROADCAST_PKT_SND_B4_DTIM** macro.
  > **Note**: Starting from WC-3.4.2, the SL_SI91X_EXT_FEAT_AP_BROADCAST_PKT_SND_B4_DTIM macro is deprecated. It is recommended to use the updated macros to ensure compatibility with future releases.

  | **Module**| **v3.4.1**                                       | **v3.4.2**                                           |
  |-----------|--------------------------------------------------|------------------------------------------------------|
  | Wi-Fi     | `SL_SI91X_EXT_FEAT_AP_BROADCAST_PKT_SND_B4_DTIM` | `SL_SI91X_EXT_FEAT_AP_BROADCAST_PKT_SND_BEFORE_DTIM` |
