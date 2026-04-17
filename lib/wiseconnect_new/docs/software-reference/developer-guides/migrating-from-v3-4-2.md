# Migrating from WiSeConnect™ SDK v3 .4.2 to v3 .5.0

## Table of Contents

- [Table of Contents](#table-of-contents)
- [Overview](#overview)
- [Migration Steps](#migration-steps)
  - [Updated default TCP/IP thread priority in LWIP stack:](#updated-default-tcpip-thread-priority-in-lwip-stack)
  - [Updated-user-gain-table-API](#updated-user-gain-table-api)
  - [Condition Introduced for Keeping Files in .data Section Based on pm\_ps2\_component](#condition-introduced-for-keeping-files-in-data-section-based-on-pm_ps2_component)
  - [API Naming Corrections](#api-naming-corrections)
  - [Changes to Reserved Fields](#changes-to-reserved-fields)
  - [Updated Structures](#updated-structures)
  - [Deprecated APIs](#deprecated-apis)
  - [Deprecated Macros](#deprecated-macros)
  - [Deprecated Enumerators](#deprecated-enumerators)
  - [Deprecated Structures/API's](#deprecated-structuresapis)
  - [Updated Status Codes](#updated-status-codes)

## Overview

This is a guide for updating an existing application using the WiSeConnect™ SDK v3.4.2 to a v3.5.0 application.

There are few naming and file changes in v3.5.0 as compared to v3.4.2, mostly in order to standardize the names and improve the overall usage experience of the application programming interface (API). Migration requires the names everywhere to be updated in the existing application.

## Migration Steps

In order to convert a WiSeConnect SDK v3.4.2 application to a v3.5.0 application, complete the following steps:

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.4.2 names or interfaces with v3.5.0 names or interfaces.

Refer to the tables in each of the sections that follow that map the v3.4.2 API elements to v3.5.0. In some instances, the differences between v3.4.2 and v3.5.0 are highlighted in **bold** text.


### Updated default TCP/IP thread priority in LWIP stack

As part of the migration from WiSeConnect™ SDK v3.4.2 to v3.5.0, there have been changes to the TCP/IP thread priority in the LWIP stack to ensure faster and more responsive handling of network operations.

**Reason for Change**

Currently, the TCP/IP thread priority is at osPriorityIdle. The TCP/IP thread's idle priority is causing issues with UDP data transfer, because it is unable to process the ARP response. This occurs because the application thread, with its higher priority, does not allow the TCP/IP thread sufficient processing time.

The ideal priority for the TCP/IP thread in the LWIP stack should be higher than application threads but lower than critical system threads such as the bus and event threads.
To address this, we have updated the TCP/IP thread in LWIP priority to osPriorityNormal.

**User Impact**

Increasing the LWIP TCP/IP thread priority can affect the functionality of application threads, because their priorities are lower compared to the TCP/IP thread.

### Updated User Gain Table API

As part of the migration from WiSeConnect™ SDK v3.4.2 to v3.5.0, a new API `sl_wifi_update_su_gain_table` is added for loading Wi-Fi user gain table values to get more gain in 11ax SU operations. 

**Reason for Change**
This request aims to support at least two 11ax Power tables (SU and TB), because currently only one power table is supported. Due to this limitation, we are using lower TB-based power levels in 11ax, which reduces overall 11ax performance.

With current tests:

- ETSI/MIC TX powers are reduced by about ~6 dB for 11ax (limited by 26Tone RU - PSD).
- FCC Band edge for 26Tone RU is worse by up to ~2 dB compared to other 11ax cases.

  - With the new power table, we will be able to support both TB and SU side by side with two different power levels, without compromising the SU power table.
  - We should be able to support different levels or backoffs for various tones (26, 52, 106, etc.).
  - Supporting different backoffs based on the placement of 26-tone or 52-tone (whether center, edge of 20 MHz, and so on) is optional.

**User Impact**

Increased gain values as per mentioned above.

### Condition Introduced for Keeping Files in .data Section Based on pm_ps2_component

A condition has been introduced in release **3.5.0** to keep specific files in the `.data` section (RAM) **only if** the `pm_ps2_component` is installed.

- **Component Requirement:**  
  The `pm_ps2_component` must be installed when the application is designed to switch or manage PS2 power states.

- **Reason for Change:**  
  In previous releases, when the ULP wakeup sources components were used or installed, files related to PS2 functionality were unnecessarily placed in the `.data` section, causing **increased RAM usage** in applications that did not require PS2 features. This change optimizes RAM usage by including these files only when necessary.

- **Action Required:**  
  If your application uses PS2 power management, do one of the following:
  - Install the `pm_ps2_component` in **Simplicity Studio**,  
    **OR**
  - Add it to the `components` section of your `.slcp` file manually.
  
- **Note:**  
  Without this component, related files will no longer be moved to `.data` section, helping reduce RAM footprint for applications that do not use PS2 features.

### API Naming Corrections

| **Module** | **v3.4.2** | **v3.5.0** |
|------------|------------|------------|
| Wi-Fi | **sl_si91x_network_params_response_t** | typo error corrected as **sli_si91x_network_params_response_t** |

### Changes to Reserved Fields

| **Module** | **v3.4.2** | **v3.5.0** |
|------------|------------|------------|
| Wi-Fi | reserved **BIT 6** in **ctrl_flags** member variable of **sl_wifi_transceiver_tx_data_control_t** | used for **Extended Information** |
| Wi-Fi | reserved **BIT 7** in **ctrl_flags** member variable of **sl_wifi_transceiver_tx_data_control_t** | used for **Immediate Transfer** |
| Wi-Fi | **reserved1** member variable of **sl_wifi_transceiver_tx_data_control_t** | renamed to **ctrl_flags1** |

### Updated Structures

| **Module** | **v3.4.2** | **v3.5.0** |
|------------|------------|------------|
| Wi-Fi | The **status** member variable in **sl_wifi_transceiver_tx_data_confirmation_t** is of type **sl_status_t** | The **status** member variable in **sl_wifi_transceiver_tx_data_confirmation_t** has been changed to **uint32_t** |
| Wi-Fi | The **status** member variable in **sl_wifi_transceiver_rx_data_t** is of type **sl_status_t** | The **status** member variable in **sl_wifi_transceiver_rx_data_t** has been changed to **uint32_t** |

### Deprecated APIs

**Note:** The following elements are applicable to both NCP and SoC modes.

- Deprecated **sl_wifi_update_gain_table** API.
  > **Note**: Starting from WC-3.5.0, the sl_wifi_update_gain_table API is deprecated. It is recommended to use the updated API to ensure compatibility with future releases.

  | **Module**| **v3.4.2**                  | **v3.5.0**                     |
  |-----------|-----------------------------|--------------------------------|
  | Wi-Fi     | `sl_wifi_update_gain_table` | `sl_wifi_update_su_gain_table` |

### Deprecated Macros

**Note:** The following elements are applicable to both NCP and SoC modes.

- Deprecated **SL_SI91X_TCP_IP_FEAT_POP3_CLIENT** macro.
  > **Note**: Starting from v3.5.0, the SL_SI91X_TCP_IP_FEAT_POP3_CLIENT macro is deprecated. It is recommended to use the updated macros to ensure compatibility with future releases.

  | **Module**| **v3.4.2**                                       | **v3.5.0**                                           |
  |-----------|--------------------------------------------------|------------------------------------------------------|
  | Wi-Fi     | `SL_SI91X_TCP_IP_FEAT_POP3_CLIENT`               | `SL_SI91X_TCP_IP_FEAT_DTLS_THREE_SOCKETS`            |

### Deprecated Enumerators

**Note:** The following elements are applicable to both NCP and SoC modes.

- Deprecated **SL_NET_INVALID_CREDENTIAL_ID** enumerator.
  > **Note**: Starting from v3.5.0, the enumerator SL_NET_INVALID_CREDENTIAL_ID is deprecated. It is recommended to use the updated enumerator to ensure compatibility with future releases.

  | **Module**| **v3.4.2**                                       | **v3.5.0**                                           |
  |-----------|--------------------------------------------------|------------------------------------------------------|
  | Network Manager     | `SL_NET_INVALID_CREDENTIAL_ID`                   | `SL_NET_NO_CREDENTIAL_ID`            |

### Deprecated Structures/APIs

**Note:** The following elements are applicable to both NCP and SoC modes.

- Deprecated APIs and structures.
  > **Note**: Starting from WiSeConnect 3.5.0, it is recommended to use the v2 versions of APIs and structures to ensure compatibility with future releases.

  | **Module**| **v3.4.2**                                       | **v3.5.0**                                           |
  |-----------|--------------------------------------------------|------------------------------------------------------|
  | Wi-Fi     | `sl_wifi_performance_profile_t`                  | `sl_wifi_performance_profile_v2_t`                   |
  | Wi-Fi     | `sl_wifi_listen_interval_t`                      | `sl_wifi_listen_interval_v2_t`                       |
  | Wi-Fi     | `sl_wifi_set_performance_profile`                | `sl_wifi_set_performance_profile_v2`                 |
  | Wi-Fi     | `sl_wifi_get_performance_profile`                | `sl_wifi_get_performance_profile_v2`                 |
  | Wi-Fi     | `sl_wifi_set_listen_interval`                    | `sl_wifi_set_listen_interval_v2`                     |
  | Wi-Fi     | `sl_wifi_get_listen_interval`                    | `sl_wifi_get_listen_interval_v2`                     |
  | Wi-Fi     | `sl_si91x_set_listen_interval`                   | `sl_wifi_set_listen_interval_v2`                     |

### Updated Status Codes

As part of the migration from WiSeConnect™ SDK version v3.4.2 to v3.5.0, the status codes in `sl_additional_status.h` have been updated to avoid overlapping with the status codes in `sl_status.h`. The new status code values now include a prefix (`0x16`) for better categorization and to avoid conflicts.

**Reason for Change**

The status codes in `sl_additional_status.h` previously overlapped with those in `sl_status.h`, which could lead to conflicts and incorrect status handling. To resolve this, the status code values in `sl_additional_status.h` have been updated to use the range `0x16000` to `0x16FFF`.

**User Impact**

Applications using the old status codes will need to update their references to the new status code values. This change ensures compatibility with the v3.5.0 SDK and prevents conflicts between `sl_status.h` and `sl_additional_status.h`.

#### **Status Code Mapping**

| **Status Description**                          | **v3.4.2 Status Code** | **v3.5.0 Status Code** |
|------------------------------------------------|------------------------|------------------------|
| SL_STATUS_OS_OPERATION_FAILURE                            | `0x0051`              | `0x16051`             |
| SL_STATUS_BOOTUP_OPTIONS_NOT_SAVED                  | `0x0052`              | `0x16052`             |
| SL_STATUS_BOOTUP_OPTIONS_CHECKSUM_FAILURE      | `0x0053`              | `0x16053`             |
| SL_STATUS_BOOTLOADER_VERSION_MISMATCH     | `0x0054`              | `0x16054`             |
| SL_STATUS_WAITING_FOR_BOARD_READY              | `0x0055`              | `0x16055`             |
| SL_STATUS_VALID_FIRMWARE_NOT_PRESENT                      | `0x0056`              | `0x16056`             |
| SL_STATUS_INVALID_OPTION                     | `0x0057`              | `0x16057`             |
| SL_STATUS_SPI_BUSY                          | `0x0058`              | `0x16058`             |
| SL_STATUS_CARD_READY_TIMEOUT           | `0x0059`              | `0x16059`             |
| SL_STATUS_FW_LOAD_OR_UPGRADE_TIMEOUT   | `0x005A`              | `0x1605A`             |