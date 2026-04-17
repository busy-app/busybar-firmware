/***************************************************************************/ /**
 * @file  sl_wifi_callback_framework.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#pragma once

#include "sl_wifi_device.h" // To access the device specific structs
#include "sli_wifi_callback_framework.h"
#include <stdint.h>
#ifndef __ZEPHYR__
#include "sli_cmsis_os2_ext_task_register.h"

/// External variable representing the index of the thread local array at which the firmware status will be stored.
extern sli_task_register_id_t sli_fw_status_storage_index;
#endif

/** \addtogroup WIFI_CALLBACK_FRAMEWORK Callback Framework
  * \ingroup SL_WIFI
  * @{ */

/// Generic macro for callback functions to check if the event has failed.
#define SL_WIFI_CHECK_IF_EVENT_FAILED(event) ((event & SL_WIFI_EVENT_FAIL_INDICATION) ? true : false)

/**
 * @typedef sl_wifi_callback_function_t
 * @brief Generic callback for Wi-Fi group events of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles Wi-Fi events of type @ref sl_wifi_event_t.
 *   The callback is invoked when a Wi-Fi event occurs, providing the event details and any associated data.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t. This parameter indicates the specific Wi-Fi event that triggered the callback.
 * @param data
 *   Pointer to the data received. The type and content of this data depend on the specific event.
 * @param data_length
 *   Length of the data received in bytes.
 * @param optional_arg
 *   Optional user-provided argument passed in @ref sl_wifi_set_callback. This parameter allows the user to pass additional context or information to the callback function.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter will be of type `sl_status_t`, and the `data_length` parameter can be ignored.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_callback_function_v2_t](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-callback-function-v2-t) API. This is retained for backward compatibility.
 */
typedef sl_status_t (*sl_wifi_callback_function_t)(sl_wifi_event_t event,
                                                   void *data,
                                                   uint32_t data_length,
                                                   void *optional_arg);

/**
 * @typedef sl_wifi_scan_callback_t
 * @brief Callback for SL_WIFI_SCAN_RESULT_EVENTS group event of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles Wi-Fi scan result events of type @ref sl_wifi_event_t.
 *   The callback is triggered when a Wi-Fi module tries to scan and receive the response, providing the event details and any associated scan results.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t. This parameter indicates the specific Wi-Fi event that triggered the callback.
 *   | @ref sl_wifi_event_t                 | Description                                                          |
 *   |:-------------------------------------|:---------------------------------------------------------------------|
 *   | SL_WIFI_SCAN_RESULT_EVENTS           | It is an indication to host that the scan was successful or failed   |
 * @param data
 *   Pointer to the scan results of type @ref sl_wifi_scan_result_t. This parameter provides the scan results obtained from the Wi-Fi scan operation.
 * @param data_length
 *   Length of the scan results data received in bytes.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_scan_callback](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-scan-callback).
 *
 * @pre Wi-Fi module must call @ref sl_wifi_start_scan to receive SL_WIFI_SCAN_RESULT_EVENTS event.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In case of event failure, SL_WIFI_FAIL_EVENT_STATUS_INDICATION bit is set in the event.
 *   When this bit is set, the `data` parameter will be of type `sl_status_t`, and the `data_length` parameter can be ignored.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_scan_callback_v2_t](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-scan-callback-v2-t) API. This is retained for backward compatibility.
 */
typedef sl_status_t (*sl_wifi_scan_callback_t)(sl_wifi_event_t event,
                                               sl_wifi_scan_result_t *data,
                                               uint32_t data_length,
                                               void *optional_arg);

/**
 * @typedef sl_wifi_stats_callback_t
 * @brief Callback for SL_WIFI_STATS_RESPONSE_EVENTS group events of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles Wi-Fi statistics response events of type @ref sl_wifi_event_t.
 *   The callback is invoked when a Wi-Fi statistics response event occurs, providing the event details and any associated data.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t that triggered the callback.
 *   Individual Wi-Fi events related to SL_WIFI_STATS_RESPONSE_EVENTS is as follows:
 *   | @ref sl_wifi_event_t                 | DataType                                    |
 *   |:-------------------------------------|:--------------------------------------------|
 *   | SL_WIFI_STATS_EVENT                  | Not supported in current release            |
 *   | SL_WIFI_STATS_ASYNC_EVENT            | [sl_wifi_async_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-async-stats-response-t)        |
 *   | SL_WIFI_STATS_ADVANCE_EVENT          | Not supported in current release            |
 *   | SL_WIFI_STATS_TEST_MODE_EVENT        | Not supported in current release            |
 *   | SL_WIFI_STATS_MODULE_STATE_EVENT     | [sl_wifi_module_state_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-module-state-stats-response-t) |
 * @param data
 *   Pointer to the payload received.
 * @param data_length
 *   Length of the payload received in bytes.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_stats_callback](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-stats-callback).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 *  SL_WIFI_STATS_MODULE_STATE_EVENT messages are used to indicate module state to the host. These messages are enabled by setting the 10th bit of the custom feature bitmap in opermode.
 *  For the event SL_WIFI_STATS_MODULE_STATE_EVENT response structure refer [sl_wifi_module_state_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-module-state-stats-response-t).
 * - state_code of this response  (1 byte), indicates the state of the module. `state_code` contains two parts, the upper nibble and lower nibble.
 *  The state code is formed by combining the upper and the lower nibbles using a bitwise OR operation, that is, State code = upper nibble | lower nibble
 *  For example, if the state code is 82 but is not found in the table, it can be divided as follows: state_code = 80 | 02, where 80 is the upper nibble and 02 is the lower nibble.
 *
 *  The upper nibble indicates the state of the rejoin process.
 *  The following table documents the possible values of the upper nibble of the state_code.
 *  | Module state code                   | upper nibble | Description                                                                |
 *  |:------------------------------------|------------- |----------------------------------------------------------------------------|
 *  |     Scan Trigger (State I)          | 0x00         | Startup (Initial state or idle state).                                     |
 *  | Indicates the reason for the scan   |--------------|----------------------------------------------------------------------------|
 *  |         triggered.                  | 0x10         | Beacon Loss (Failover Roam).                                               |
 *  |                                     |--------------|----------------------------------------------------------------------------|
 *  |                                     | 0x20         | De-authentication from AP.                                                 |
 *  |-------------------------------------|--------------|----------------------------------------------------------------------------|
 *  |   Scan Result/Decision (State II)   | 0x50         | Current AP is best.                                                        |
 *  | Indicates a state change based on   |--------------|--------------------------------------------------------------------------- |
 *  | the scan result.                    | 0x60         | Better AP found while roaming.                                             |
 *  |                                     |--------------|----------------------------------------------------------------------------|
 *  |                                     | 0x70         | No AP found.                                                               |
 *  |-------------------------------------|--------------|----------------------------------------------------------------------------|
 *  | Final Connection or Join (State III)| 0x80         | Associated or joined to an Access point.                                   |
 *  |Indicates the connection state       |--------------|----------------------------------------------------------------------------|
 *  |change                               | 0x90         | Unassociated(Disconnected from host or join failure).                      |
 *
 * @note
 *  The lower nibble of state_code indicates the reason for a state change.
 *  The table below lists the possible values of the lower nibble of the state_code.
 *  | Module state code lower nibble | Description                                   |
 *  |:-------------------------------|:----------------------------------------------|
 *  | 0x00                           | No reason specified (Initial state or idle state). |
 *  | 0x01                           | No response from AP for authentication request(Authentication denial).   |
 *  | 0x02                           | Association denial (Association timeout or Association failure due to unknown reasons). |
 *  | 0x03                           | User configured AP is not present.            |
 *  | 0x05                           | Four-way Handshake failure.                   |
 *  | 0x06                           | Deauthentication from user.                   |
 *  | 0x07                           | PSK not configured.                           |
 *  | 0x08                           | key-handshake failure during rejoin/roaming/after connection(Disconnection from supplicant). |
 *  | 0x09                           | Roaming not enabled.                          |
 *
 * @note
 *  | Module reason code | Description                                                |
 *  |:-------------------|:-----------------------------------------------------------|
 *  | 0x00               | No reason specified (Initial state or idle state).         |
 *  | 0x01               | No response from AP for authentication request(Authentication denial). |
 *  | 0x02               | Association denial (caused by Association timeout or Association failure due to unknown reasons). |
 *  | 0x03               | User configured AP is not present.                         |
 *  | 0x05               | Four-way Handshake failure.                                |
 *  | 0x06               | Deauthentication from user.                                |
 *  | 0x07               | PSK not configured.                                        |
 *  | 0x08               | key-handshake failure during rejoin/roaming/after connection(Disconnection from supplicant). |
 *  | 0x09               | Roaming not enabled                                        |
 *  | 0x10               | Beacon Loss (Failover Roam).                               |
 *  | 0x20               | De-authentication from AP.                                 |
 *  | 0x28               | TLS CA Cert not present                                    |
 *  | 0x29               | TLS PRIVATE key not present.                               |
 *  | 0x2A               | TLS Client Cert not present.                               |
 *  | 0x2B               | TLS no Cert present.                                       |
 *  | 0x2C               | PEAP CA Cert not present.                                  |
 *  | 0x2D               | Server Cert Invalid Key Type.                              |
 *  | 0x2E               | Server Intermediate CA Invalid Key Type.                   |
 *  | 0x2F               | Server Root CA Invalid Key Type.                           |
 *  | 0x30               | Client Cert Invalid Key Type.                              |
 *  | 0x31               | Client Root CA Invalid Key Type.                           |
 *  | 0x37               | Server Cert 4096-bit length support is not enabled.        |
 *  | 0x38               | Server Intermediate CA 4096-bit length support is not enabled. |
 *  | 0x39               | Server Root CA 4096-bit length support is not enabled.     |
 *  | 0x3A               | Client Cert 4096-bit length support is not enabled.        |
 *  | 0x3B               | Client Root CA 4096-bit length support is not enabled.     |
 *  | 0x3C               | Server Cert Invalid Sign Alg.                              |
 *  | 0x3D               | Server Intermediate CA Invalid Sign Alg.                   |
 *  | 0x3E               | Server Root CA Invalid Sign Length.                        |
 *  | 0x3F               | Client Cert Invalid Sign Alg.                              |
 *  | 0x40               | Client Root CA Invalid Sign Length.                        |
 *  | 0x41               | Server Intermediate CA not Present.                        |
 *  | 0x42               | Server Root CA Parse Error.                                |
 *  | 0x43               | Server Intermediate Root CA Parse Error.                   |
 *  | 0x44               | Sever Cert Parse Error.                                    |
 *  | 0x45               | Client Cert Parse Error.                                   |
 *  | 0x46               | Incorrect Private Key Password.                            |
 *  | 0x47               | EAP Failure Received.                                      |
 *  | 0x48               | Client Cert Bad Date Error.                                |
 *  | 0x49               | Server Cert Bad Date Error.                                |
 *  | 0x4A               | Server Root CA Bad Date Error.                             |
 *  | 0x4B               | Client Root CA Bad Date Error.                             |
 *  | 0x4C               | Server Intermediate Root CA Bad Date Error.                |
 *  | 0x4D               | Pem Header Error.                                          |
 *  | 0x4E               | Pem Footer Error.                                          |
 *  | 0x4F               | Client Intermediate CA Invalid Sign Length.                |
 *  | 0x50               | Client Intermediate CA Invalid Length.                     |
 *  | 0x52               | Client Intermediate CA invalid Key Type.                   |
 *  | 0x53               | Pem Error.                                                 |
 *  | 0x54               | Pathlen certificate is Invalid.                            |
 *
 * @note
 *   In addition to the above, the reason code received in the Deauthentication/Disassociation frame from the AP is modified by setting the Most Significant Bit (MSB) of the reason code. If the MSB is set in the reason code, it should be masked with 0x7F to extract the actual reason code received in the Deauthentication/Disassociation frame. Refer to the "Reason Codes Table" section in the IEEE 802.11 standard to learn about the de-authentication or disassociation reason codes.
 *   Pem Header Error (0x4D) and Pem Footer Error (0x4E) apply only when certificates are loaded individually.
 *   If certificates are loaded together in a single file, only the Pem Error (0x53) triggers for any header or footer errors.
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter will be of type `sl_status_t`, and the `data_length` parameter can be ignored.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_stats_callback_v2_t](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-stats-callback-v2-t) API. This is retained for backward compatibility.
 */
typedef sl_status_t (*sl_wifi_stats_callback_t)(sl_wifi_event_t event,
                                                void *data,
                                                uint32_t data_length,
                                                void *optional_arg);

/**
 * @typedef sl_wifi_join_callback_t
 * @brief Callback for SL_WIFI_JOIN_EVENTS group events of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles Wi-Fi join events of type @ref sl_wifi_event_t.
 *   The callback is invoked when a Wi-Fi join event occurs, providing the event details and any associated data.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t that triggered the callback.
 * @param data
 *   Pointer to the response data received (1 byte).
 *   The possible values and their descriptions are as follows:
 *   | Data received   | Description                |
 *   |:----------------|:---------------------------|
 *   | C               | Module connection success  |
 *   | F               | Module connection failed   |
 * @param data_length
 *   Length of the data received in bytes. This parameter indicates the size of the data buffer pointed to by the `data` parameter.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_join_callback](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-join-callback).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note This is valid in WiFi client mode only
 * @note
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter will be of type `sl_status_t`, and the `data_length` parameter can be ignored.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_join_callback_v2_t](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-join-callback-v2-t) API. This is retained for backward compatibility.
 */
typedef sl_status_t (*sl_wifi_join_callback_t)(sl_wifi_event_t event,
                                               char *data,
                                               uint32_t data_length,
                                               void *optional_arg);

/**
 * @typedef sl_wifi_twt_config_callback_t
 * @brief Callback for SL_WIFI_TWT_RESPONSE_EVENTS group events
 *
 *   This typedef defines a callback function that handles Wi-Fi Target Wake Time (TWT) configuration response events.
 *   The callback is invoked when a TWT response event occurs, providing the event details and any associated data.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t that triggered the callback.
 *   Individual Wi-Fi events related to SL_WIFI_TWT_RESPONSE_EVENTS is as follows:
 *   | @ref sl_wifi_event_t                          | Description                                             |
 *   |:----------------------------------------------|:--------------------------------------------------------|
 *   | SL_WIFI_TWT_UNSOLICITED_SESSION_SUCCESS_EVENT | Unsolicited TWT session was successfully established.   |
 *   | SL_WIFI_TWT_AP_REJECTED_EVENT                 | TWT request was rejected by the Access Point (AP).      |
 *   | SL_WIFI_TWT_OUT_OF_TOLERANCE_EVENT            | TWT response out of tolerance limits                    |
 *   | SL_WIFI_TWT_RESPONSE_NOT_MATCHED_EVENT        | TWT response did not match the request.                 |
 *   | SL_WIFI_TWT_UNSUPPORTED_RESPONSE_EVENT        | TWT response is unsupported.                            |
 *   | SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT            | TWT session teardown was successful.                    |
 *   | SL_WIFI_TWT_AP_TEARDOWN_SUCCESS_EVENT         | AP successfully tore down the TWT session.              |
 *   | SL_WIFI_TWT_FAIL_MAX_RETRIES_REACHED_EVENT    | Reached maximum number of retries for TWT setup.        |
 *   | SL_WIFI_TWT_INACTIVE_DUE_TO_ROAMING_EVENT     | TWT session became inactive due to roaming.             |
 *   | SL_WIFI_TWT_INACTIVE_DUE_TO_DISCONNECT_EVENT  | TWT session became inactive due to disconnection.       |
 *   | SL_WIFI_TWT_INACTIVE_NO_AP_SUPPORT_EVENT      | Connected AP does not support TWT                       |
 *   | SL_WIFI_RESCHEDULE_TWT_SUCCESS_EVENT          | TWT session was successfully rescheduled.               |
 *   | SL_WIFI_TWT_INFO_FRAME_EXCHANGE_FAILED_EVENT  | TWT information frame exchange failed.                  |
 * @param data
 *   Pointer to the data received of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
 *   This parameter provides detailed information about the TWT response event. The structure contains various fields that describe the TWT session parameters and status.
 * @param data_length
 *   Length of the data received in bytes.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_twt_config_callback](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-twt-config-callback).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter will be of type `sl_status_t`, and the `data_length` parameter can be ignored.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_twt_config_callback_v2_t](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-twt-config-callback-v2-t) API. This is retained for backward compatibility.
 */
typedef sl_status_t (*sl_wifi_twt_config_callback_t)(sl_wifi_event_t event,
                                                     sl_wifi_twt_response_t *data,
                                                     uint32_t data_length,
                                                     void *optional_arg);

/**
 * @typedef sl_wifi_transceiver_callback_t
 * @brief Callback for SL_WIFI_TRANSCEIVER_EVENTS group events
 *
 *   This typedef defines a callback function that handles Wi-Fi transceiver events.
 *   The callback is invoked when a transceiver event occurs, providing the event details and any associated data.
 *
 * @param[out] event
 *   Wi-Fi event of type @ref sl_wifi_event_t. This parameter indicates the specific Wi-Fi event that triggered the callback.
 *   Individual Wi-Fi events related to SL_WIFI_TRANSCEIVER_EVENTS are as follows:
 *   | @ref sl_wifi_event_t                   | Description                                                 |
 *   |:---------------------------------------|:------------------------------------------------------------|
 *   | SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB | Indicates that data has been received by the transceiver.   |
 *   | SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB  | Indicates the status of the data sent by the transceiver.   |
 * @param[out] data
 *   - Data received is of type @ref sl_wifi_transceiver_rx_data_t for SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB event.
 *   - Data received is of type @ref sl_wifi_transceiver_tx_data_confirmation_t for SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB event.
 * @param[out] data_length
 *   Reserved.
 * @param[out] optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_transceiver_callback](../pages/wifi-callback-framework#sl-wifi-set-transceiver-callback)
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/4.1/common/api/group-status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note This API is only supported in Wi-Fi Transceiver opermode (7).
 * @note
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter will be of type `sl_status_t`, and the `data_length` parameter can be ignored.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_transceiver_callback_v2_t](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-transceiver-callback-v2-t) API. This is retained for backward compatibility.
 */
typedef sl_status_t (*sl_wifi_transceiver_callback_t)(sl_wifi_event_t event,
                                                      void *data,
                                                      uint32_t data_length,
                                                      void *optional_arg);

/**
 * @typedef sl_wifi_command_engine_status_callback_t
 * @brief Callback for SL_WIFI_COMMAND_ENGINE_STATUS_EVENTS group events of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles command engine status events of type @ref sl_wifi_event_t.
 *   The callback is invoked when a command engine status event occurs, providing the status details.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t that triggered the callback.
 * @param status_code
 *   Status code of type sl_status_t. This parameter always indicates a failure event and contains the failure status code.
 *   See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @param data
 *   Unused parameter. Reserved for future use.
 * @param data_length
 *   Unused parameter. Reserved for future use.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_command_engine_status_handler](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-command-engine-status-handler).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [Additional Status Codes](../wiseconnect-api-reference-guide-err-codes/sl-additional-status-errors) for details.
 * @note
 *   The @p data and @p data_length parameters are currently unused and should be ignored by the callback implementation.
 */
typedef sl_status_t (*sl_wifi_command_engine_status_callback_t)(sl_wifi_event_t event,
                                                                sl_status_t status_code,
                                                                void *data,
                                                                uint32_t data_length,
                                                                void *optional_arg);

/**
 * @typedef sl_wifi_callback_function_v2_t
 * @brief Generic callback for Wi-Fi group events of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles Wi-Fi events of type @ref sl_wifi_event_t.
 *   The callback is invoked when a Wi-Fi event occurs, providing the event details, status code and any associated data.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t. This parameter indicates the specific Wi-Fi event that triggered the callback.
 * @param status_code
 *   Status code of type sl_status_t. On success/regular events always indicates SL_STATUS_OK, on failure events indicates the status code of failure.
 *   See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @param data
 *   Pointer to the data received. The type and content of this data depend on the specific event.
 * @param data_length
 *   Length of the data received in bytes.
 * @param optional_arg
 *   Optional user-provided argument passed in @ref sl_wifi_set_callback_v2. This parameter allows the user to pass additional context or information to the callback function.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` and the `data_length` parameter can be ignored.
 */
typedef sl_status_t (*sl_wifi_callback_function_v2_t)(sl_wifi_event_t event,
                                                      sl_status_t status_code,
                                                      void *data,
                                                      uint32_t data_length,
                                                      void *optional_arg);

/**
 * @typedef sl_wifi_scan_callback_v2_t
 * @brief Callback for SL_WIFI_SCAN_RESULT_EVENTS group event of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles Wi-Fi scan result events of type @ref sl_wifi_event_t.
 *   The callback is triggered when a Wi-Fi module tries to scan and receive the response, providing the event details and any associated scan results.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t. This parameter indicates the specific Wi-Fi event that triggered the callback.
 *   | @ref sl_wifi_event_t                 | Description                                                          |
 *   |:-------------------------------------|:---------------------------------------------------------------------|
 *   | SL_WIFI_SCAN_RESULT_EVENTS           | It is an indication to host that the scan was successful or failed   |
 * @param status_code
 *   Status code of type sl_status_t. On successful scan event always indicates SL_STATUS_OK, on failure indicates the status code of failure.
 *   See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @param data
 *   Pointer to the scan results of type @ref sl_wifi_scan_result_t. This parameter provides the scan results obtained from the Wi-Fi scan operation.
 * @param data_length
 *   Length of the scan results data received in bytes.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_scan_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-scan-callback-v2).
 *
 * @pre Wi-Fi module must call @ref sl_wifi_start_scan to receive SL_WIFI_SCAN_RESULT_EVENTS event.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In case of event failure, SL_WIFI_FAIL_EVENT_STATUS_INDICATION bit is set in the event.
 *   When this bit is set, the `data` parameter and the `data_length` parameter can be ignored.
 *   In the case of SL_WIFI_SCAN_TYPE_EXTENDED, the scan callback is invoked with data set to NULL and data_length
 *   indicating the total size of the scan results (that is, scan count * sizeof(sl_wifi_extended_scan_result_t)). Upon
 *   receiving this callback, the application must allocate a buffer of size data_length and explicitly call the
 *   @ref sl_wifi_get_stored_scan_results() API, passing the allocated buffer to retrieve the complete scan results.
 */
typedef sl_status_t (*sl_wifi_scan_callback_v2_t)(sl_wifi_event_t event,
                                                  sl_status_t status_code,
                                                  sl_wifi_scan_result_t *data,
                                                  uint32_t data_length,
                                                  void *optional_arg);

/**
 * @typedef sl_wifi_stats_callback_v2_t
 * @brief Callback for SL_WIFI_STATS_RESPONSE_EVENTS group events of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles Wi-Fi statistics response events of type @ref sl_wifi_event_t.
 *   The callback is invoked when a Wi-Fi statistics response event occurs, providing the event details and any associated data.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t that triggered the callback.
 *   Individual Wi-Fi events related to SL_WIFI_STATS_RESPONSE_EVENTS is as follows:
 *   | @ref sl_wifi_event_t                 | DataType                                    |
 *   |:-------------------------------------|:--------------------------------------------|
 *   | SL_WIFI_STATS_EVENT                  | Not supported in current release            |
 *   | SL_WIFI_STATS_ASYNC_EVENT            | [sl_wifi_async_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-async-stats-response-t)        |
 *   | SL_WIFI_STATS_ADVANCE_EVENT          | Not supported in current release            |
 *   | SL_WIFI_STATS_TEST_MODE_EVENT        | Not supported in current release            |
 *   | SL_WIFI_STATS_MODULE_STATE_EVENT     | [sl_wifi_module_state_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-module-state-stats-response-t) |
 * @param status_code
 *   Status code of type sl_status_t. On successful events always indicates SL_STATUS_OK, on failure events indicates the status code of failure.
 *   See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @param data
 *   Pointer to the payload received.
 * @param data_length
 *   Length of the payload received in bytes.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_stats_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-stats-callback-v2).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 *  SL_WIFI_STATS_MODULE_STATE_EVENT messages are used to indicate module state to the host. These messages are enabled by setting the 10th bit of the custom feature bitmap in opermode.
 *  For the event SL_WIFI_STATS_MODULE_STATE_EVENT response structure refer [sl_wifi_module_state_stats_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-module-state-stats-response-t).
 * - state_code of this response  (1 byte), indicates the state of the module. `state_code` contains two parts, the upper nibble and lower nibble.
 *  The state code is formed by combining the upper and the lower nibbles using a bitwise OR operation, that is, State code = upper nibble | lower nibble
 *  For example, if the state code is 82 but is not found in the table, it can be divided as follows: state_code = 80 | 02, where 80 is the upper nibble and 02 is the lower nibble.
 *
 *  The upper nibble indicates the state of the rejoin process.
 *  The following table documents the possible values of the upper nibble of the state_code.
 *  | Module state code                   | upper nibble | Description                                                                |
 *  |:------------------------------------|------------- |----------------------------------------------------------------------------|
 *  |     Scan Trigger (State I)          | 0x00         | Startup (Initial state or idle state).                                     |
 *  | Indicates the reason for the scan   |--------------|----------------------------------------------------------------------------|
 *  |         triggered.                  | 0x10         | Beacon Loss (Failover Roam).                                               |
 *  |                                     |--------------|----------------------------------------------------------------------------|
 *  |                                     | 0x20         | De-authentication from AP.                                                 |
 *  |-------------------------------------|--------------|----------------------------------------------------------------------------|
 *  |   Scan Result/Decision (State II)   | 0x50         | Current AP is best.                                                        |
 *  | Indicates a state change based on   |--------------|--------------------------------------------------------------------------- |
 *  | the scan result.                    | 0x60         | Better AP found while roaming.                                             |
 *  |                                     |--------------|----------------------------------------------------------------------------|
 *  |                                     | 0x70         | No AP found.                                                               |
 *  |-------------------------------------|--------------|----------------------------------------------------------------------------|
 *  | Final Connection or Join (State III)| 0x80         | Associated or joined to an Access point.                                   |
 *  |Indicates the connection state       |--------------|----------------------------------------------------------------------------|
 *  |change                               | 0x90         | Unassociated(Disconnected from host or join failure).                      |
 *
 * @note
 *  The lower nibble of state_code indicates the reason for a state change.
 *  The table below lists the possible values of the lower nibble of the state_code.
 *  | Module state code lower nibble | Description                                   |
 *  |:-------------------------------|:----------------------------------------------|
 *  | 0x00                           | No reason specified (Initial state or idle state). |
 *  | 0x01                           | No response from AP for authentication request(Authentication denial).   |
 *  | 0x02                           | Association denial (Association timeout or Association failure due to unknown reasons). |
 *  | 0x03                           | User configured AP is not present.            |
 *  | 0x05                           | Four-way Handshake failure.                   |
 *  | 0x06                           | Deauthentication from user.                   |
 *  | 0x07                           | PSK not configured.                           |
 *  | 0x08                           | key-handshake failure during rejoin/roaming/after connection(Disconnection from supplicant). |
 *  | 0x09                           | Roaming not enabled.                          |
 *
 * @note
 *  | Module reason code | Description                                                |
 *  |:-------------------|:-----------------------------------------------------------|
 *  | 0x00               | No reason specified (Initial state or idle state).         |
 *  | 0x01               | No response from AP for authentication request(Authentication denial). |
 *  | 0x02               | Association denial (caused by Association timeout or Association failure due to unknown reasons). |
 *  | 0x03               | User configured AP is not present.                         |
 *  | 0x05               | Four-way Handshake failure.                                |
 *  | 0x06               | Deauthentication from user.                                |
 *  | 0x07               | PSK not configured.                                        |
 *  | 0x08               | key-handshake failure during rejoin/roaming/after connection(Disconnection from supplicant). |
 *  | 0x09               | Roaming not enabled                                        |
 *  | 0x10               | Beacon Loss (Failover Roam).                               |
 *  | 0x20               | De-authentication from AP.                                 |
 *  | 0x28               | TLS CA Cert not present                                    |
 *  | 0x29               | TLS PRIVATE key not present.                               |
 *  | 0x2A               | TLS Client Cert not present.                               |
 *  | 0x2B               | TLS no Cert present.                                       |
 *  | 0x2C               | PEAP CA Cert not present.                                  |
 *  | 0x2D               | Server Cert Invalid Key Type.                              |
 *  | 0x2E               | Server Intermediate CA Invalid Key Type.                   |
 *  | 0x2F               | Server Root CA Invalid Key Type.                           |
 *  | 0x30               | Client Cert Invalid Key Type.                              |
 *  | 0x31               | Client Root CA Invalid Key Type.                           |
 *  | 0x37               | Server Cert 4096-bit length support is not enabled.        |
 *  | 0x38               | Server Intermediate CA 4096-bit length support is not enabled. |
 *  | 0x39               | Server Root CA 4096-bit length support is not enabled.     |
 *  | 0x3A               | Client Cert 4096-bit length support is not enabled.        |
 *  | 0x3B               | Client Root CA 4096-bit length support is not enabled.     |
 *  | 0x3C               | Server Cert Invalid Sign Alg.                              |
 *  | 0x3D               | Server Intermediate CA Invalid Sign Alg.                   |
 *  | 0x3E               | Server Root CA Invalid Sign Length.                        |
 *  | 0x3F               | Client Cert Invalid Sign Alg.                              |
 *  | 0x40               | Client Root CA Invalid Sign Length.                        |
 *  | 0x41               | Server Intermediate CA not Present.                        |
 *  | 0x42               | Server Root CA Parse Error.                                |
 *  | 0x43               | Server Intermediate Root CA Parse Error.                   |
 *  | 0x44               | Sever Cert Parse Error.                                    |
 *  | 0x45               | Client Cert Parse Error.                                   |
 *  | 0x46               | Incorrect Private Key Password.                            |
 *  | 0x47               | EAP Failure Received.                                      |
 *  | 0x48               | Client Cert Bad Date Error.                                |
 *  | 0x49               | Server Cert Bad Date Error.                                |
 *  | 0x4A               | Server Root CA Bad Date Error.                             |
 *  | 0x4B               | Client Root CA Bad Date Error.                             |
 *  | 0x4C               | Server Intermediate Root CA Bad Date Error.                |
 *  | 0x4D               | Pem Header Error.                                          |
 *  | 0x4E               | Pem Footer Error.                                          |
 *  | 0x4F               | Client Intermediate CA Invalid Sign Length.                |
 *  | 0x50               | Client Intermediate CA Invalid Length.                     |
 *  | 0x52               | Client Intermediate CA invalid Key Type.                   |
 *  | 0x53               | Pem Error.                                                 |
 *  | 0x54               | Pathlen certificate is Invalid.                            |
 *
 * @note
 *   In addition to the above, the reason code received in the Deauthentication/Disassociation frame from the AP is modified by setting the Most Significant Bit (MSB) of the reason code.If the MSB is set in the reason code, it should be masked with 0x7F to extract the actual reason code received in the Deauthentication/Disassociation frame. Please refer to the "Reason Codes Table" section in the IEEE 802.11 standard to learn about the de-authentication or disassociation reason codes.
 *   Pem Header Error (0x4D) and Pem Footer Error (0x4E) apply only when certificates are loaded individually.
 *   If certificates are loaded together in a single file, only the Pem Error (0x53) triggers for any header or footer errors.
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter and the `data_length` parameter can be ignored.
 */
typedef sl_status_t (*sl_wifi_stats_callback_v2_t)(sl_wifi_event_t event,
                                                   sl_status_t status_code,
                                                   void *data,
                                                   uint32_t data_length,
                                                   void *optional_arg);

/**
 * @typedef sl_wifi_join_callback_v2_t
 * @brief Callback for SL_WIFI_JOIN_EVENTS group events of type @ref sl_wifi_event_group_t.
 *
 *   This typedef defines a callback function that handles Wi-Fi join events of type @ref sl_wifi_event_t.
 *   The callback is invoked when a Wi-Fi join event occurs, providing the event details and any associated data.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t that triggered the callback.
 * @param status_code
 *   Status code of type sl_status_t. On successful events always indicates SL_STATUS_OK, on failure events indicates the status code of failure.
 *   See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @param data
 *   Pointer to the response data received (1 byte).
 *   The possible values and their descriptions are as follows:
 *   | Data received   | Description                |
 *   |:----------------|:---------------------------|
 *   | C               | Module connection success  |
 *   | F               | Module connection failed   |
 * @param data_length
 *   Length of the data received in bytes. This parameter indicates the size of the data buffer pointed to by the `data` parameter.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_join_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-join-callback-v2).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note This is valid in WiFi client mode only
 * @note
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter and the `data_length` parameter can be ignored.
 */
typedef sl_status_t (*sl_wifi_join_callback_v2_t)(sl_wifi_event_t event,
                                                  sl_status_t status_code,
                                                  char *data,
                                                  uint32_t data_length,
                                                  void *optional_arg);

/**
 * @typedef sl_wifi_twt_config_callback_v2_t
 * @brief Callback for SL_WIFI_TWT_RESPONSE_EVENTS group events
 *
 *   This typedef defines a callback function that handles Wi-Fi Target Wake Time (TWT) configuration response events.
 *   The callback is invoked when a TWT response event occurs, providing the event details and any associated data.
 *
 * @param event
 *   Wi-Fi event of type @ref sl_wifi_event_t that triggered the callback.
 *   Individual Wi-Fi events related to SL_WIFI_TWT_RESPONSE_EVENTS is as follows:
 *   | @ref sl_wifi_event_t                          | Description                                             |
 *   |:----------------------------------------------|:--------------------------------------------------------|
 *   | SL_WIFI_TWT_UNSOLICITED_SESSION_SUCCESS_EVENT | Unsolicited TWT session was successfully established.   |
 *   | SL_WIFI_TWT_AP_REJECTED_EVENT                 | TWT request was rejected by the Access Point (AP).      |
 *   | SL_WIFI_TWT_OUT_OF_TOLERANCE_EVENT            | TWT response out of tolerance limits                    |
 *   | SL_WIFI_TWT_RESPONSE_NOT_MATCHED_EVENT        | TWT response did not match the request.                 |
 *   | SL_WIFI_TWT_UNSUPPORTED_RESPONSE_EVENT        | TWT response is unsupported.                            |
 *   | SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT            | TWT session teardown was successful.                    |
 *   | SL_WIFI_TWT_AP_TEARDOWN_SUCCESS_EVENT         | AP successfully tore down the TWT session.              |
 *   | SL_WIFI_TWT_FAIL_MAX_RETRIES_REACHED_EVENT    | Reached maximum number of retries for TWT setup.        |
 *   | SL_WIFI_TWT_INACTIVE_DUE_TO_ROAMING_EVENT     | TWT session became inactive due to roaming.             |
 *   | SL_WIFI_TWT_INACTIVE_DUE_TO_DISCONNECT_EVENT  | TWT session became inactive due to disconnection.       |
 *   | SL_WIFI_TWT_INACTIVE_NO_AP_SUPPORT_EVENT      | Connected AP does not support TWT                       |
 *   | SL_WIFI_RESCHEDULE_TWT_SUCCESS_EVENT          | TWT session was successfully rescheduled.               |
 *   | SL_WIFI_TWT_INFO_FRAME_EXCHANGE_FAILED_EVENT  | TWT information frame exchange failed.                  |
 * @param status_code
 *   Status code of type sl_status_t. On successful events always indicates SL_STATUS_OK, on failure events indicates the status code of failure.
 *   See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @param data
 *   Pointer to the data received of type [sl_wifi_twt_response_t](../wiseconnect-api-reference-guide-si91x-driver/sl-si91x-twt-response-t).
 *   This parameter provides detailed information about the TWT response event. The structure contains various fields that describe the TWT session parameters and status.
 * @param data_length
 *   Length of the data received in bytes.
 * @param optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_twt_config_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-twt-config-callback-v2).
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter and the `data_length` parameter can be ignored.
 */
typedef sl_status_t (*sl_wifi_twt_config_callback_v2_t)(sl_wifi_event_t event,
                                                        sl_status_t status_code,
                                                        sl_wifi_twt_response_t *data,
                                                        uint32_t data_length,
                                                        void *optional_arg);

/**
 * @typedef sl_wifi_transceiver_callback_v2_t
 * @brief Callback for SL_WIFI_TRANSCEIVER_EVENTS group events
 *
 *   This typedef defines a callback function that handles Wi-Fi transceiver events.
 *   The callback is invoked when a transceiver event occurs, providing the event details and any associated data.
 *
 * @param[out] event
 *   Wi-Fi event of type @ref sl_wifi_event_t. This parameter indicates the specific Wi-Fi event that triggered the callback.
 *   Individual Wi-Fi events related to SL_WIFI_TRANSCEIVER_EVENTS are as follows:
 *   | @ref sl_wifi_event_t                   | Description                                                 |
 *   |:---------------------------------------|:------------------------------------------------------------|
 *   | SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB | Indicates that data has been received by the transceiver.   |
 *   | SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB  | Indicates the status of the data sent by the transceiver.   |
 * @param status_code
 *   Status code of type sl_status_t. On successful events always indicates SL_STATUS_OK, on failure events indicates the status code of failure.
 *   See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @param[out] data
 *   - Data received is of type @ref sl_wifi_transceiver_rx_data_t for SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB event.
 *   - Data received is of type @ref sl_wifi_transceiver_tx_data_confirmation_t for SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB event.
 * @param[out] data_length
 *   Reserved.
 * @param[out] optional_arg
 *   Optional user-provided argument passed in [sl_wifi_set_transceiver_callback_v2](../pages/wifi-callback-framework#sl-wifi-set-transceiver-callback-v2)
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/4.1/common/api/group-status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note This API is only supported in Wi-Fi Transceiver opermode (7).
 * @note
 *   In case of event failure, the `SL_WIFI_FAIL_EVENT_STATUS_INDICATION` bit is set in the `event` parameter.
 *   When this bit is set, the `data` parameter and the `data_length` parameter can be ignored.
 */
typedef sl_status_t (*sl_wifi_transceiver_callback_v2_t)(sl_wifi_event_t event,
                                                         sl_status_t status_code,
                                                         void *data,
                                                         uint32_t data_length,
                                                         void *optional_arg);

/***************************************************************************/
/**
 * @brief
 *   Register a callback for a selected event group. All the individual Wi-Fi events related to specific group would be triggered via this group callback.
 *
 *   This function allows the user to register a callback function for a specific group of Wi-Fi events @ref sl_wifi_event_group_t.
 *   When any event within the specified group occurs, the registered callback function will be invoked, providing the event details and any associated data.
 *
 * @param[in] group
 *   Group ID of the event for which the callback is registered. See @ref sl_wifi_event_group_t for possible values.
 * @param[in] function
 *   Function pointer to callback of type @ref sl_wifi_callback_function_t that would be invoked when an event in the specified group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument to pass additional context or information to the callback function. This would be passed back to callback handler of type @ref sl_wifi_callback_function_t.
 * @pre Pre-conditions:
 * - The Wi-Fi module must be initialized by calling @ref sl_wifi_init before this API can be used.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   Callbacks can be set only for event groups defined in @ref sl_wifi_event_group_t, not for individual events defined in @ref sl_wifi_event_t.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_set_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-callback-v2) API. This is retained for backward compatibility.
 ******************************************************************************/
sl_status_t sl_wifi_set_callback(sl_wifi_event_group_t group,
                                 sl_wifi_callback_function_t function,
                                 void *optional_arg) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief
 *   Register a callback for a selected event group. All the individual Wi-Fi events related to specific group would be triggered via this group callback.
 *
 *   This function allows the user to register a callback function for a specific group of Wi-Fi events @ref sl_wifi_event_group_t.
 *   When any event within the specified group occurs, the registered callback function will be invoked, providing the event details and any associated data.
 *
 * @param[in] group
 *   Group ID of the event for which the callback is registered. See @ref sl_wifi_event_group_t for possible values.
 * @param[in] function
 *   Function pointer to callback of type @ref sl_wifi_callback_function_v2_t that would be invoked when an event in the specified group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument to pass additional context or information to the callback function. This would be passed back to callback handler of type @ref sl_wifi_callback_function_v2_t.
 * @pre Pre-conditions:
 * - The Wi-Fi module must be initialized by calling @ref sl_wifi_init before this API can be used.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   Callbacks can be set only for event groups defined in @ref sl_wifi_event_group_t, not for individual events defined in @ref sl_wifi_event_t.
 ******************************************************************************/
sl_status_t sl_wifi_set_callback_v2(sl_wifi_event_group_t group,
                                    sl_wifi_callback_function_v2_t function,
                                    void *optional_arg);

/***************************************************************************/ /**
 * @brief
 *   Default Wi-Fi event handler to be passed to @ref sl_wifi_init. This event handler would dispatch all the Wi-Fi events and invoke respective Wi-Fi group event.
 *
 *   This function serves as the default event handler for Wi-Fi events.
 *   When passed to the @ref sl_wifi_init function, it would handle incoming Wi-Fi events by dispatching them to the appropriate group event functions.
 *
 * @param[in] event
 *   Wi-Fi event of type of @ref sl_wifi_event_t.
 * @param[in] buffer
 *   Buffer containing raw data from NWP firmware
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 *   Passing the event handler is optional. User can implement their own event dispatching handler if they prefer.
 ******************************************************************************/
extern sl_status_t sl_wifi_default_event_handler(sl_wifi_event_t event, sl_wifi_buffer_t *buffer);

/***************************************************************************/
/**
 * @brief
 *   Register a callback for the SL_WIFI_SCAN_RESULT_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_SCAN_RESULT_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data.
 *
 * @param[in] function
 *   Callback function to register. This parameter specifies the callback function of type @ref sl_wifi_scan_callback_t that would be invoked when an event in the SL_WIFI_SCAN_RESULT_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This would be passed back to callback handler of type @ref sl_wifi_scan_callback_t.
 * @pre Pre-conditions:
 *   - @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group would be triggered via this callback.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_set_scan_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-scan-callback-v2) API. This is retained for backward compatibility.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_scan_callback(sl_wifi_scan_callback_t function, void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_SCAN_RESULT_EVENTS, (sl_wifi_callback_function_t)function, NULL, optional_arg);
}

/***************************************************************************/
/**
 * @brief
 *   Register a callback for SL_WIFI_JOIN_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_JOIN_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data.
 *
 * @param[in] function
 *   Function pointer to callback of type @ref sl_wifi_join_callback_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_JOIN_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This parameter allows the user to pass additional context or information to the callback function. This would be passed back to callback handler of type @ref sl_wifi_twt_config_callback_t.
 * @pre Pre-conditions:
 *   @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group will be triggered via this callback.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_set_join_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-join-callback-v2) API. This is retained for backward compatibility.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_join_callback(sl_wifi_join_callback_t function, void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_JOIN_EVENTS, (sl_wifi_callback_function_t)function, NULL, optional_arg);
}
/***************************************************************************/
/**
 * @brief
 *   Register a callback for SL_WIFI_TWT_RESPONSE_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_TWT_RESPONSE_EVENTS group.
 *   When any event within this group occurs, the registered callback function will be invoked, providing the event details and any associated data.
 *
 * @param[in] function
 *   Function pointer to the callback of type @ref sl_wifi_twt_config_callback_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_TWT_RESPONSE_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This would be passed back to callback handler.
 * @pre Pre-conditions:
 *   @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group would be triggered via this callback.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_set_twt_config_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-twt-config-callback-v2) API. This is retained for backward compatibility.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_twt_config_callback(sl_wifi_twt_config_callback_t function, void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_TWT_RESPONSE_EVENTS, (sl_wifi_callback_function_t)function, NULL, optional_arg);
}

/***************************************************************************/
/**
 * @brief
 *   Register a callback for SL_WIFI_STATS_RESPONSE_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_STATS_RESPONSE_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data.
 *
 * @param[in] function
 *   Function pointer to the callback of type @ref sl_wifi_stats_callback_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_STATS_RESPONSE_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This parameter allows the user to pass additional context or information to the callback function. This would be passed back to callback handler.
 * @pre Pre-conditions:
 *   @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group would be triggered via this callback.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_set_stats_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-stats-callback-v2) API. This is retained for backward compatibility.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_stats_callback(sl_wifi_stats_callback_t function, void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_STATS_RESPONSE_EVENTS,
                               (sl_wifi_callback_function_t)function,
                               NULL,
                               optional_arg);
}

/***************************************************************************/ /**
 * @brief
 *   Register a callback for SL_WIFI_TRANSCEIVER_EVENTS group event of tye @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_TRANSCEIVER_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data.
 *
 * @param[in] function
 *   Function pointer to the callback of type @ref sl_wifi_transceiver_callback_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_TRANSCEIVER_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This parameter allows the user to pass additional context or information to the callback function. This would be passed back to callback handler.
 * @pre Pre-conditions:
 * - @ref sl_wifi_init should be called before this API.
 * @return
*   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/4.1/common/api/group-status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group will be triggered via this callback.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_set_transceiver_callback_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-callback-framework#sl-wifi-set-transceiver-callback-v2) API. This is retained for backward compatibility.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_transceiver_callback(sl_wifi_transceiver_callback_t function, void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_TRANSCEIVER_EVENTS, (sl_wifi_callback_function_t)function, NULL, optional_arg);
}

/***************************************************************************/
/**
 * @brief
 *   Register a callback for the SL_WIFI_SCAN_RESULT_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_SCAN_RESULT_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data or failure code.
 *
 * @param[in] function
 *   Callback function to register. This parameter specifies the callback function of type @ref sl_wifi_scan_callback_v2_t that would be invoked when an event in the SL_WIFI_SCAN_RESULT_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This would be passed back to callback handler of type @ref sl_wifi_scan_callback_v2_t.
 * @pre Pre-conditions:
 *   - @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group would be triggered via this callback.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_scan_callback_v2(sl_wifi_scan_callback_v2_t function, void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_SCAN_RESULT_EVENTS,
                               NULL,
                               (sl_wifi_callback_function_v2_t)function,
                               optional_arg);
}

/***************************************************************************/
/**
 * @brief
 *   Register a callback for SL_WIFI_JOIN_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_JOIN_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data or failure code.
 *
 * @param[in] function
 *   Function pointer to callback of type @ref sl_wifi_join_callback_v2_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_JOIN_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This parameter allows the user to pass additional context or information to the callback function. This would be passed back to callback handler of type @ref sl_wifi_join_callback_v2_t.
 * @pre Pre-conditions:
 *   @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group will be triggered via this callback.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_join_callback_v2(sl_wifi_join_callback_v2_t function, void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_JOIN_EVENTS, NULL, (sl_wifi_callback_function_v2_t)function, optional_arg);
}

/***************************************************************************/
/**
 * @brief
 *   Register a callback for SL_WIFI_TWT_RESPONSE_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_TWT_RESPONSE_EVENTS group.
 *   When any event within this group occurs, the registered callback function will be invoked, providing the event details and any associated data or failure code.
 *
 * @param[in] function
 *   Function pointer to the callback of type @ref sl_wifi_twt_config_callback_v2_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_TWT_RESPONSE_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This would be passed back to callback handler.
 * @pre Pre-conditions:
 *   @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group would be triggered via this callback.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_twt_config_callback_v2(sl_wifi_twt_config_callback_v2_t function,
                                                             void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_TWT_RESPONSE_EVENTS,
                               NULL,
                               (sl_wifi_callback_function_v2_t)function,
                               optional_arg);
}

/***************************************************************************/
/**
 * @brief
 *   Register a callback for SL_WIFI_STATS_RESPONSE_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_STATS_RESPONSE_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data or failure code.
 *
 * @param[in] function
 *   Function pointer to the callback of type @ref sl_wifi_stats_callback_v2_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_STATS_RESPONSE_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This parameter allows the user to pass additional context or information to the callback function. This would be passed back to callback handler.
 * @pre Pre-conditions:
 *   @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group would be triggered via this callback.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_stats_callback_v2(sl_wifi_stats_callback_v2_t function, void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_STATS_RESPONSE_EVENTS,
                               NULL,
                               (sl_wifi_callback_function_v2_t)function,
                               optional_arg);
}

/***************************************************************************/ /**
 * @brief
 *   Register a callback for SL_WIFI_TRANSCEIVER_EVENTS group event of tye @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_TRANSCEIVER_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data or failure code.
 *
 * @param[in] function
 *   Function pointer to the callback of type @ref sl_wifi_transceiver_callback_v2_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_TRANSCEIVER_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This parameter allows the user to pass additional context or information to the callback function. This would be passed back to callback handler.
 * @pre Pre-conditions:
 * - @ref sl_wifi_init should be called before this API.
 * @return
*   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/4.1/common/api/group-status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   All the individual Wi-Fi events related to this group will be triggered via this callback.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_transceiver_callback_v2(sl_wifi_transceiver_callback_v2_t function,
                                                              void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_TRANSCEIVER_EVENTS,
                               NULL,
                               (sl_wifi_callback_function_v2_t)function,
                               optional_arg);
}

#ifndef __ZEPHYR__
/***************************************************************************/ /**
 * @brief 
 *   Retrieves the saved thread-specific firmware status value.
 *  
 * @details
 *   This function fetches the firmware status value that is specific to the current thread.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
static inline sl_status_t sl_wifi_get_saved_firmware_status(void)
{
  sl_status_t status = SL_STATUS_FAIL;

  sli_osTaskRegisterGetValue(NULL, sli_fw_status_storage_index, &status);
  return status;
}
#endif

/***************************************************************************/
/**
 * @brief
 *   Register a callback for SL_WIFI_COMMAND_ENGINE_STATUS_EVENTS group event from @ref sl_wifi_event_group_t.
 *
 *   This function allows the user to register a callback function for the SL_WIFI_COMMAND_ENGINE_STATUS_EVENTS group.
 *   When any event within this group occurs, the registered callback function would be invoked, providing the event details and any associated data.
 *
 * @param[in] function
 *   Function pointer to callback of type @ref sl_wifi_command_engine_status_callback_t. This parameter specifies the callback function that would be invoked when an event in the SL_WIFI_COMMAND_ENGINE_STATUS_EVENTS group occurs.
 * @param[in] optional_arg
 *   Optional user-provided argument. This parameter allows the user to pass additional context or information to the callback function. This would be passed back to callback handler of type @ref sl_wifi_command_engine_status_callback_t.
 * @pre Pre-conditions:
 *   @ref sl_wifi_init should be called before this API.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [Additional Status Codes](../wiseconnect-api-reference-guide-err-codes/sl-additional-status-errors) for details.
 * @note
 *   All the individual Wi-Fi events related to this group will be triggered via this callback.
 ******************************************************************************/
static inline sl_status_t sl_wifi_set_command_engine_status_handler(sl_wifi_command_engine_status_callback_t function,
                                                                    void *optional_arg)
{
  return sli_wifi_set_callback(SL_WIFI_COMMAND_ENGINE_STATUS_EVENTS,
                               NULL,
                               (sl_wifi_callback_function_v2_t)function,
                               optional_arg);
}

/** @} */
