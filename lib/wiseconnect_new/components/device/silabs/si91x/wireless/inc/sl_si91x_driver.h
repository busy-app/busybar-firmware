/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_status.h"
#include "sli_wifi_types.h"
#include "sl_wifi_device.h"
#include "firmware_upgradation.h"
#include "sl_wifi_host_interface.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_types.h"
#include "sl_rsi_utility.h"
#include "sl_si91x_constants.h"
#include "sl_constants.h"
#include "cmsis_os2.h"

//! @cond Doxygen_Suppress
#define SLI_VERIFY_STATUS(s) \
  do {                       \
    if (s != SL_STATUS_OK)   \
      return s;              \
  } while (0)

/**
 * All flags used in si91x event mask
 * There are three main groups of flags, each with SL_SI91X_COMMAND_TYPE_COUNT number of unique flags
 */
#define SL_SI91X_TX_PENDING_FLAG(x)  (1 << (x))
#define SL_SI91X_RESPONSE_FLAG(x)    (1 << (SI91X_CMD_MAX + x))
#define SL_SI91X_EXTRA_EVENT_FLAG(x) (1 << (SI91X_CMD_MAX + SI91X_CMD_MAX + x))

//! TX Flags
#define SL_SI91X_COMMON_TX_PENDING_EVENT         SL_SI91X_TX_PENDING_FLAG(SLI_WIFI_COMMON_CMD)
#define SL_SI91X_WLAN_TX_PENDING_EVENT           SL_SI91X_TX_PENDING_FLAG(SLI_WIFI_WLAN_CMD)
#define SL_SI91X_NETWORK_TX_PENDING_EVENT        SL_SI91X_TX_PENDING_FLAG(SLI_SI91X_NETWORK_CMD)
#define SL_SI91X_BT_TX_PENDING_EVENT             SL_SI91X_TX_PENDING_FLAG(SLI_SI91X_BT_CMD)
#define SL_SI91X_GENERIC_SOCKET_TX_PENDING_EVENT SL_SI91X_TX_PENDING_FLAG(SLI_SI91X_SOCKET_CMD)

// Indicates RX response received for COMMON command type
#define NCP_HOST_COMMON_RESPONSE_EVENT SL_SI91X_RESPONSE_FLAG(SLI_WIFI_COMMON_CMD)

// Indicates synchronous RX response received for WLAN command type
#define NCP_HOST_WLAN_RESPONSE_EVENT SL_SI91X_RESPONSE_FLAG(SLI_WIFI_WLAN_CMD)

// Indicates synchronous RX response received for NETWORK command type
#define NCP_HOST_NETWORK_RESPONSE_EVENT SL_SI91X_RESPONSE_FLAG(SLI_SI91X_NETWORK_CMD)

// Indicates RX response received for SOCKET command type
#define NCP_HOST_SOCKET_RESPONSE_EVENT SL_SI91X_RESPONSE_FLAG(SLI_SI91X_SOCKET_CMD)

// Indicates RX response received for BLE command type
#define NCP_HOST_BT_RESPONSE_EVENT SL_SI91X_RESPONSE_FLAG(SLI_SI91X_BT_CMD)

// Triggered by IRQ to indicate something to read
#define SL_SI91X_NCP_HOST_BUS_RX_EVENT           SL_SI91X_EXTRA_EVENT_FLAG(0)
#define SL_SI91X_SOCKET_DATA_TX_PENDING_EVENT    SL_SI91X_EXTRA_EVENT_FLAG(1)
#define SL_SI91X_SOCKET_COMMAND_TX_PENDING_EVENT SL_SI91X_EXTRA_EVENT_FLAG(2)
#define SL_SI91X_GENERIC_DATA_TX_PENDING_EVENT   SL_SI91X_EXTRA_EVENT_FLAG(3)
#define SL_SI91X_TA_BUFFER_FULL_CLEAR_EVENT      SL_SI91X_EXTRA_EVENT_FLAG(4)

#define SL_SI91X_ALL_TX_PENDING_COMMAND_EVENTS                                                                       \
  (SL_SI91X_COMMON_TX_PENDING_EVENT | SL_SI91X_WLAN_TX_PENDING_EVENT | SL_SI91X_NETWORK_TX_PENDING_EVENT             \
   | SL_SI91X_SOCKET_DATA_TX_PENDING_EVENT | SL_SI91X_BT_TX_PENDING_EVENT | SL_SI91X_GENERIC_SOCKET_TX_PENDING_EVENT \
   | SL_SI91X_SOCKET_COMMAND_TX_PENDING_EVENT | SL_SI91X_GENERIC_DATA_TX_PENDING_EVENT)

typedef enum { SL_NCP_NORMAL_POWER_MODE, SL_NCP_LOW_POWER_MODE, SL_NCP_ULTRA_LOW_POWER_MODE } sl_si91x_power_mode_t;

typedef struct sl_si91x_power_configuration sl_si91x_power_configuration_t;
//! @endcond

/***************************************************************************/ /**
 * @brief
 *   Initialize the driver.
 * @param[in] config
 *   @ref sl_wifi_device_configuration_t Pointer to device configuration.
 * @param[in] event_handler
 *   [sl_wifi_event_handler_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-event-handler-t) Function pointer to receive asynchronous events.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_driver_init(const sl_wifi_device_configuration_t *config, sl_wifi_event_handler_t event_handler);

/***************************************************************************/ /**
 * @brief
 *   De-initialize the driver.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_driver_deinit(void);

//! @cond Doxygen_Suppress
/***************************************************************************/ /**
 * @brief
 *   Register a function and optional argument for scan results callback.
 * @param[in] command
 *   Command type to be sent to NWP firmware.
 * @param[in] queue_type
 *   @ref sli_wifi_command_type_t Command type
 * @param[in] data
 *   Command packet to be sent to the NWP firmware.
 * @param[in] data_length
 *   Length of command packet.
 * @param[in] wait_period
 *   @ref sli_wifi_wait_period_t Timeout for the command response.
 * @param[in] sdk_context
 *   Pointer to the context.
 * @param[in] data_buffer
 *   [sl_wifi_buffer_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-buffer-t) Pointer to a data buffer pointer for the response data to be returned in.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sli_si91x_driver_send_command(uint32_t command,
                                          sli_wifi_command_type_t queue_type,
                                          const void *data,
                                          uint32_t data_length,
                                          sli_wifi_wait_period_t wait_period,
                                          void *sdk_context,
                                          sl_wifi_buffer_t **data_buffer);

/***************************************************************************/ /**
 * @brief
 *   Register a function and optional argument for scan results callback.
 * @param[in] command
 *   Command type to be sent to NWP firmware.
 * @param[in] data
 *   Command packet to be sent to the NWP firmware.
 * @param[in] data_length
 *   Length of command packet.
 * @param[in] wait_period
 *   @ref sli_wifi_wait_period_t Timeout for the command response.
 * @pre 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_driver_send_side_band_crypto(uint32_t command,
                                                  const void *data,
                                                  uint32_t data_length,
                                                  sli_wifi_wait_period_t wait_period);

/***************************************************************************/ /**
 * @brief
 * Send commands to the NWP; whose response needs to be handled asynchronously.
 * Note: This function doesn't acquire "command_in_flight" boolean
 * @param[in] command
 *   Command type to be sent to NWP firmware.
 * @param[in] queue_type
 *   @ref sli_wifi_command_type_t Command type
 * @param[in] data
 *   Command packet to be sent to the NWP firmware.
 * @param[in] data_length
 *   Length of the command packet.
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sli_si91x_driver_send_async_command(uint32_t command,
                                                sli_wifi_command_type_t queue_type,
                                                void *data,
                                                uint32_t data_length);
/***************************************************************************/ /**
 * @brief
 *   Wait for a command response.
 * @param[in] command
 *   @ref sli_wlan_cmd_request_t Command type to wait .
 * @param[in] wait_period
 *   @ref sli_wifi_wait_period_t Wait time in milliseconds to wait for command response.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_driver_wait_for_response(sli_wlan_cmd_request_t command, sli_wifi_wait_period_t wait_period);

/***************************************************************************/ /**
 * @brief
 *   Send a socket command.
 * @param[in] request
 *   @ref sli_si91x_socket_send_request_t Pointer to socket command packet.
 * @param[in] data
 *   Pointer to socket data.
 * @param[in] wait_time
 *   Timeout  for the command response.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sli_si91x_driver_send_socket_data(const sli_si91x_socket_send_request_t *request,
                                              const void *data,
                                              uint32_t wait_time);

/***************************************************************************/ /**
 * @brief
 *   Send a Bluetooth command.
 * @param[in] command
 *   @ref sli_wlan_cmd_request_t Command type to be sent.
 * @param[in] queue_type
 *   @ref sli_wifi_command_type_t Command type.
 * @param[in] data
 *   [sl_wifi_buffer_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-buffer-t) Pointer to Bluetooth data.
 * @param[in] sync_command
 *   Sync or Async command.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sli_si91x_driver_send_bt_command(sli_wlan_cmd_request_t command,
                                             sli_wifi_command_type_t command_type,
                                             sl_wifi_buffer_t *data,
                                             uint8_t sync_command);
//! @endcond

/***************************************************************************/ /**
 * @brief
 *   Load a certificate into a specified index.
 * @param[in] certificate_type
 *   Type of certificate being loaded
 * @param[in] certificate_index
 *   Index where the certificate is to be loaded.
 * @param[in] buffer
 *   Pointer to the buffer containing the certificate to be loaded.
 * @param[in] certificate_length
 *   Length of the certificate buffer.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_wifi_set_certificate_index(uint8_t certificate_type,
                                                uint8_t certificate_index,
                                                const uint8_t *buffer,
                                                uint32_t certificate_length);

/** \addtogroup SL_SI91X_TYPES
  * @{ */
/// Firmware version information
typedef struct {
  uint8_t chip_id;          ///< Chip ID
  uint8_t rom_id;           ///< ROM ID
  uint8_t major;            ///< Major version number
  uint8_t minor;            ///< Minor version number
  uint8_t security_version; ///< Security enabled or disabled
  uint8_t patch_num;        ///< Patch number
  uint8_t customer_id;      ///< Customer ID
  uint16_t build_num;       ///< Build number
} sl_si91x_firmware_version_t;

/// Network wirless processor (NWP) configuration structure
typedef struct {
  uint32_t code; ///< Configuration code. The possible values are:
                 ///< - SL_SI91X_XO_CTUNE_FROM_HOST
                 ///< - SL_SI91X_ENABLE_NWP_WDT_FROM_HOST
                 ///< - SL_SI91X_DISABLE_NWP_WDT_FROM_HOST
                 ///< - SL_SI91X_SET_XTAL_GOOD_TIME_FROM_HOST
                 ///< - SL_SI91X_SET_PMU_GOOD_TIME_FROM_HOST
  union {
    uint16_t config_val; /**< 
                      * @brief Configuration value as per the code selected above
                      * 
                      * |  code  | config_val range for SoC  | config_val range for NCP |
                      * |--------|---------------------------|---------------------------|
                      * |SL_SI91X_XO_CTUNE_FROM_HOST| 0 - 255 | 0 - 255 |
                      * |SL_SI91X_ENABLE_NWP_WDT_FROM_HOST| NA | NA |
                      * |SL_SI91X_DISABLE_NWP_WDT_FROM_HOST| NA | NA |
                      * |SL_SI91X_SET_XTAL_GOOD_TIME_FROM_HOST| 600µs - 5000µs | 600µs - 5000µs |
                      * |SL_SI91X_SET_PMU_GOOD_TIME_FROM_HOST|  900µs - 2000µs | 600µs - 2000µs |
                      * 
                      * @note
                      * The default values for SoC and NCP are set to the minimum value from the range.
                      */
    // Below structure is used in case of SL_SI91X_ENABLE_NWP_WDT_FROM_HOST
    struct {
      uint8_t wdt_timer_val;    ///< Timer value in seconds for the watchdog timer.
      uint8_t wdt_enable_in_ps; ///< Enable watchdog timer in power save mode.
    };
  } values; ///< Values
} sl_si91x_nwp_configuration_t;

/// NWP get configuration structure
typedef struct {
  uint32_t sub_command_type; ///< Requested configuration. Currently, only `GET_OPN_BOARD_CONFIG` is supported.
} sl_si91x_nwp_get_configuration_t;

/// Assertion structure. It is a structure that contains information about the assertion type and level, which are used to determine the output of debug logs.
typedef struct {
  sl_si91x_assertion_type_t assert_type;   ///< Assertion type. It must be in the range of 0 to 15 (both included).
  sl_si91x_assertion_level_t assert_level; ///< Assertion level. It must be in the range of 0 to 15 (both included).
} sl_si91x_assertion_t;
/** @} */

/** \addtogroup SI91X_DRIVER_FUNCTIONS 
 * \ingroup SL_SI91X_API
 * @{ */
/***************************************************************************/ /**
 * @brief
 *   Sets the Real Time Clock (RTC) of the module. 
 * 
 * @details
 *   This function sets the RTC time of the module using the provided @ref sl_si91x_module_rtc_time_t structure.
 *   It is a blocking call, meaning it will wait until the operation is complete before returning.
 *  
 * @param[in]   timer
 *  Pointer to an @ref sl_si91x_module_rtc_time_t structure that contains the RTC time to be set.
 * 
 * @pre 
 *  Pre-conditions:
 *  - The [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) API must be called prior to this API.
 *  - The @ref SL_SI91X_CUSTOM_FEAT_RTC_FROM_HOST bit must be enabled in the @ref SI91X_CUSTOM_FEATURE_BITMAP during the [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) process.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 *   Ensure that the Real-Time Clock (RTC) timer is configured to enable SSL certificate validation.
 ******************************************************************************/
sl_status_t sl_si91x_set_rtc_timer(const sl_si91x_module_rtc_time_t *timer);

/***************************************************************************/ /**
 * @brief
 *   Retrieves the current time from the module's Real Time Clock (RTC).
 * 
 * @details
 *   This function fetches the current time from the module's RTC and stores it in the provided @ref sl_si91x_module_rtc_time_t structure.
 *   It is a blocking call, meaning it will wait until the operation is complete before returning.
 * 
 * @param[out]  response
 *  Pointer to an @ref sl_si91x_module_rtc_time_t structure where the RTC's current time will be stored.
 * @pre 
 *  Pre-conditions:
 *  - The @ref sl_si91x_set_rtc_timer() API must be called to set the RTC time before attempting to retrieve it.
 *  - The @ref SL_SI91X_CUSTOM_FEAT_RTC_FROM_HOST bit must be enabled in the @ref SI91X_CUSTOM_FEATURE_BITMAP during the [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) process to allow RTC time setting and retrieval from the host.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_si91x_get_rtc_timer(sl_si91x_module_rtc_time_t *response);

#if defined(SLI_SI91X_MCU_INTERFACE) || defined(DOXYGEN)
/*==============================================*//**
 * @brief
 *   Sends M4 specific commands to NWP to access shared resources such as flash, XTAL etc.
 *
 * @details
 *   This function sends M4 specific commands using the specified sub-command type and input data.
 *   
 *   This is a blocking API.
 * 
 *   In SoC mode, this API only sends commands from the M4 core to the TA core.
 * 
 * @pre Pre-conditions:
 * - 
 *   [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @param[in] sub_cmd_type
 *  Specifies the sub-command type for the secure handshake.
 *
 * @param[in] input_data
 *   Pointer to the input data that contains the information used during the secure handshake.
 *
 * @param[in] input_len
 *   Specifies the length of the input data. 
 *
 * @param[in] output_len
 *   Specifies the length of the output data.
 *
 * @param[out] output_data
 *   Pointer to a buffer where the response data will be stored after the secure handshake process.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 * 
 * @note
 *  The `output_len` and `output_data` parameters are currently unused and reserved for future use.
 * @note
 *  This API is only applicable in SoC mode.
 * @note
 *   - Currently, only `SL_SI91X_ENABLE_XTAL` is supported for `sub_cmd_type`. 
 *   - `SL_SI91X_ENABLE_XTAL` is of `uint8_t` datatype and user can pass 1 to enable and 0 to disable it.
 *   - For alarm based sleep wakeup applications, `SL_SI91X_ENABLE_XTAL` is used to enable/disable the XTAL.
 ***************************************************/
sl_status_t sl_si91x_m4_ta_secure_handshake(uint8_t sub_cmd_type,
                                            uint8_t input_len,
                                            const uint8_t *input_data,
                                            uint8_t output_len,
                                            const uint8_t *output_data);

/***************************************************************************/ /**
 * @brief
 *   Configure M4 RAM address for NWP timestamp updates.
 * 
 * @details
 *   This function configures an M4 RAM address where the Network Processor (NWP) 
 *   will continuously write timestamp updates. The NWP will keep the specified 
 *   memory location updated with the current timestamp value.
 * 
 * @param[in] addr_len
 *   Length of address data in bytes.
 * @param[in] address
 *   Pointer to the variable that holds the M4 RAM address where NWP should write timestamps.
 * 
 * @pre Pre-conditions:
 * - @ref sl_si91x_driver_init should be called before this API.
 * 
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_configure_timestamp_memory_location(uint8_t addr_len, const uint32_t *address);
#endif

/***************************************************************************/ /**
 * @brief
 *   Sets different timeouts given by sl_si91x_timeout_type_t for the Si91x device.
 * 
 * @details
 *   This API configures various timeout settings such as authentication, association, channel active scan, 
 *   channel passive scan timeout, and keep-alive timeout for the module.  
 *   The timeout type determines the specific timeout being set, and the timeout value specifies the duration.
 * 
 * @pre
 *   This API should be called after [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init).
 * 
 * @param[in] timeout_type
 *   Identifies which timeout type to set. Possible values are defined in @ref sl_si91x_timeout_type_t.
 * 
 * @param[in] timeout_value
 *   The timeout value to set. The time resolution depends on the timeout_type.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 * 
 * @note 
 *   - After a successful IP configuration, Gratuitous ARP is used as the periodic WLAN Keep-Alive packet with the configured keep_alive_timeout interval.
 *   - If there is no IP configuration, NULL Data Packets are used as the WLAN Keep-Alive packet. 
 *   - As an alternative, users can use @ref sl_si91x_set_timeout to set all timeouts before calling `sl_wifi_init()`.
 *******************************************************************************/
sl_status_t sl_si91x_configure_timeout(sl_si91x_timeout_type_t timeout_type, uint16_t timeout_value);

/***************************************************************************/ /**
 * @brief
 *   Sets different module timeouts in a single call for the Si91X device.
 * 
 * @details
 *   This API sets all the timeout configurations in a single call. 
 *   It includes settings for the active channel scan timeout, authentication association timeout, and keep-alive timeout for the module.
 * 
 * @param[in] timeout_config
 *   Timeout configuration of type [sl_wifi_timeout_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-timeout-t).
 *  
 * @note
 *   - This API should ONLY be called before [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init).
 *   - Repeated calls to this API will overwrite the timeout values stored in the SDK and will be applied on the next call to [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init).
 *   - As an alternative, users can set individual timeouts using @ref sl_si91x_configure_timeout() after calling `sl_wifi_init()`.
*******************************************************************************/
void sl_si91x_set_timeout(const sl_wifi_timeout_t *timeout_config);

/***************************************************************************/ /**
 * @brief       
 *   Retrieves TA RAM log/dump via Si91x UART/UART2.
 * 
 * @details
 *   This function reads a chunk of data from the specified address in the Si91x module's RAM via UART or UART2. 
 *   It is useful for debugging purposes by allowing access to the RAM log or dump.
 * 
 *  Selection of debug UART can be done by using @ref SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS
 * 
 * @param[in]   address
 *    Address in Si91x module from which to start reading.
 * 
 * @param[in]   length       
 *    Length of the chunk to read from the Si91x module.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 ******************************************************************************/
sl_status_t sl_si91x_get_ram_log(uint32_t address, uint32_t length);

/***************************************************************************/
/**
 * @brief
 *   Allows the Network Processor (NWP) to write content to the common flash from M4.
 * 
 * @details
 *   This function enables the NWP to write data to the common flash memory from the M4 core. It is a blocking API.
 * 
 * @param[in] write_address
 *   The address in the common flash memory where the write operation should begin.
 *   - For the M4 region, the write address should start from 0x8000000. Possible values range from the M4 image end address to the M4 region end address.
 *   - For the NWP region, the write address should range from 0 to (20K-1).
 *   - For sector erase, it should be multiples of 4K.
 *
 * @param[in] write_data
 *   Pointer to the data to be written. 
 * 
 * @param[in] write_data_length
 *   The total length of the data, which should be multiples of 4K for sector erase.
 * 
 * @param[in] flash_sector_erase_enable
 *   Enable or disable sector erase.
 *   - 1: Erases multiples of 4 KB of data.
 *   - 0: Disable, allows writing data onto flash.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 ******************************************************************************/
sl_status_t sl_si91x_command_to_write_common_flash(uint32_t write_address,
                                                   const uint8_t *write_data,
                                                   uint16_t write_data_length,
                                                   uint8_t flash_sector_erase_enable);

/***************************************************************************/ /**
 * @brief
 *   Sends a command to read data from the NWP flash memory of the SI91x wireless device. 
 * 
 * @details
 *   This function sends a command to the SI91x wireless device to read data from the NWP flash memory at the specified address. The read data is stored in the provided output buffer.
 * 
 *   This is a blocking API.
 * 
 * @param[in] read_address
 *   The address in the NWP flash memory to read from. The address should range from 0 to (20K-1).
 * 
 * @param[in] length
 *   The number of bytes to read from the NWP flash memory.
 * 
 * @param[out] output_buffer
 *   Pointer to the buffer where the read data will be stored.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 ******************************************************************************/
sl_status_t sl_si91x_command_to_read_common_flash(uint32_t read_address, size_t length, uint8_t *output_buffer);

/***************************************************************************/ /**
 * @brief 
 *   Reads the status of a specified id.
 *
 * @details
 *   This function reads the status specified by the read_id parameter and stores the result in the location pointed to by the output parameter.
 *   This is a blocking API.
 *
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 *
 * @param[in] read_id 
 *   Status to be read, which is of type @ref sl_si91x_read_status_t.
 * 
 * @param[out] output 
 *   Pointer to a variable where the status value will be stored.
 * 
 * | read_id                                | output | Description                                           |
 * |----------------------------------------|--------|-------------------------------------------------------|
 * | SL_SI91X_READ_NWP_DEBUG_PORT_STATUS    | 0      | Indicates the NWP debug port status is locked.        |
 * |                                        | 1      | Indicates the NWP debug port status is unlocked.      |
 * | SL_SI91X_READ_MCU_DEBUG_PORT_STATUS    | 0      | Indicates the MCU debug port status is locked.        |
 * |                                        | 1      | Indicates the MCU debug port status is unlocked.      |
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 *  This API is only applicable in SoC mode.
 ******************************************************************************/
sl_status_t sl_si91x_read_status(sl_si91x_read_status_t read_id, uint8_t *output);

/***************************************************************************/ /**
 * @brief
 *   Retrieve the firmware version currently installed on the SiWx91x device. 
 * 
 * @details
 *   This function retrieves the firmware version currently installed on the SiWx91x device. The version information is stored in the provided `sl_si91x_firmware_version_t` object.
 * 
 *   This is a blocking API.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @param[out] version
 *   Pointer to an `sl_si91x_firmware_version_t` object that will be populated with the firmware version information.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 ******************************************************************************/
sl_status_t sl_si91x_get_firmware_version(sl_si91x_firmware_version_t *version);

/***************************************************************************/ /**
 * @brief
 *   Retrieve the firmware image size from the firmware image file. 
 * 
 * @details
 *   This function reads the firmware image file from the provided buffer and returns the size of the firmware image.
 * 
 *   This is a non-blocking API.
 * 
 * @param[in] buffer
 *   Pointer to the buffer containing the firmware image file.
 * 
 * @param[out] fw_image_size
 *   Pointer to a variable where the size of the firmware image will be stored. The value returned in this parameter is valid only if this API returns SL_STATUS_OK (0).
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 ******************************************************************************/
sl_status_t sl_si91x_get_firmware_size(const void *buffer, uint32_t *fw_image_size);

/***************************************************************************/
/**
 * @brief
 *   Set configuration to NWP. 
 * 
 * @details
 *   This function sets the configuration for the Network Processor (NWP) based on the provided `sl_si91x_nwp_configuration_t` structure.
 *   
 *   The configuration values are determined by the `code` element of the structure.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * - To set XTAL and PMU good time from host, call this API before [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init).
 * 
 * @param[in] nwp_config
 *   Configuration as identified by @ref sl_si91x_nwp_configuration_t.
 *   Possible values for `config.code` are defined below:
 * - For `SL_SI91X_XO_CTUNE_FROM_HOST`:
 *    - `nwp_config.values.config_val` is used to configure NWP's XO Ctune value.
 * - For `SL_SI91X_ENABLE_NWP_WDT_FROM_HOST`:
 *    - `nwp_config.values.wdt_timer_val` is used to configure the NWP WDT ISR timer, currently set to 32 seconds.
 *    - `nwp_config.values.wdt_enable_in_ps` is used to enable WDT in powersave mode.
 * - For `SL_SI91X_DISABLE_NWP_WDT_FROM_HOST`:
 *    - Disables NWP WDT ISR timer. `nwp_config.values.config_val` is not utilized by the NWP.
 * - For `SL_SI91X_SET_XTAL_GOOD_TIME_FROM_HOST`:
 *    - To configure XTAL(crystal) good time from host. Please check the @ref sl_si91x_nwp_configuration_t for valid range of values.
 * - For `SL_SI91X_SET_PMU_GOOD_TIME_FROM_HOST`:
 *    - To configure PMU good time from host. Please check the @ref sl_si91x_nwp_configuration_t for valid range of values.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 * @note
 * The crystal good time can be configured to any value between 600 and 5000 microseconds, allowing flexibility for fine-tuning based on specific crystal
 * characteristics or application requirements.
 * @note
 * In an ideal environment, the average current consumption in the associated mode increases by approximately 10 µA when the crystal good time is raised
 * from 600 µs to 2000 µs, as observed with the wifi_powersave_standby_associated_soc example.
 * @note
 * However, the @ref SI91X_CONFIG_FEATURE_BITMAP supports only a limited set of predefined values: 600, 1000, 2000, and 3000 microseconds. If both this API and
 * the configuration feature bitmap are used, the Network Processor (NWP) prioritizes the value configured via the API.
 * @note
 * XTAL (crystal) good time or PMU good time configured through this API is applied once during the subsequent call to sl_net_init()/sl_wifi_init().
 ******************************************************************************/
sl_status_t sl_si91x_set_nwp_config_request(sl_si91x_nwp_configuration_t nwp_config);

/***************************************************************************/ /**
 * @brief
 *   Retrieve the configuration value from the firmware based on the requested sub_command_type element of `sl_si91x_nwp_get_configuration_t` structure.
 * @pre Pre-conditions:
 * -
 *   [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * @param[in] nwp_config
 *   Configuration as identified by @ref sl_si91x_nwp_get_configuration_t.
 *   Possible values for nwp_config.sub_command_type are defined below:
 * - For GET_OPN_BOARD_CONFIG:
 *    - nwp_config.sub_command_type is used to get board OPN part number.
 *    - OTP bits should be programmed with the board OPN part number otherwise the API will return NULL characters for OTP bits.
 *    - Currently, all other values are NOT SUPPORTED.
 * @param[out] response
 *  Buffer to hold board OPN part number. The value returned in this param is valid only if this API returns SL_STATUS_OK(0).
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_get_nwp_config(const sl_si91x_nwp_get_configuration_t *nwp_config, uint8_t *response);

/***************************************************************************/ /**
 * @brief
 *   Debug prints on UART interfaces 1 and 2. Host can get 5 types of debug prints based on the assertion level and assertion type. This is a blocking API.
 * @pre Pre-conditions:
 * -
 *   [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * @param[in] assertion
 *   Configuration as identified by @ref sl_si91x_assertion_t.
 * - Assertion type (Possible values are 0 - 15):
 *    - 0000 (LMAC core) 
 *    - 0001 (SME)
 *    - 0010 (UMAC) 
 *    - 0100 (NETX) 
 *    - 1000 (Enables assertion indication and provides ram dump in critical assertion)
 * - Assertion level (Possible values are 0 - 15):
 *    - 0 is only for specific prints & 15 is to enable all prints. 
 *    - 0000 (Assertion required)
 *    - 0010 (Recoverable) 
 *    - 0100 (Information)
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/4.1/common/api/group-status for details.
 ******************************************************************************/
sl_status_t sl_si91x_debug_log(const sl_si91x_assertion_t *assertion);
/** @} */

/***************************************************************************/ /**
 * @brief
 *  Configures the join feature bitmap for the Si91X device.
 *
 * @details
 *  This function sets the join feature bitmap configuration for the specified Wi-Fi interface.
 *
 *  The join feature bitmap determines various connection parameters and behaviors.
 *
 *  By default, the `SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID` bitmap is enabled.
 *
 *  Users can call this API before calling [sl_wifi_connect](../wiseconnect-api-reference-guide-wi-fi/wifi-client-api#sl-wifi-connect), [sl_wifi_start_ap](../wiseconnect-api-reference-guide-wi-fi/wifi-ap-api#sl-wifi-start-ap), [sl_wifi_start_wps](../wiseconnect-api-reference-guide-wi-fi/wifi-wps-api#sl-wifi-start-wps) to overwrite the join feature bitmap.
 *
 * @param[in] interface
 *   The selected Wi-Fi interface. Refer to [sl_wifi_interface_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-interface-t) for possible values.
 *
 * @param[in] join_feature_bitmap
 *   The join feature bitmap configuration. One of the values from @ref SI91X_JOIN_FEATURE_BIT_MAP.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_set_join_configuration](../wiseconnect-api-reference-guide-wi-fi/wifi-radio-api#sl-wifi-set-join-configuration) API. This is retained for backward compatibility.
 *******************************************************************************/
sl_status_t sl_si91x_set_join_configuration(sl_wifi_interface_t interface,
                                            uint8_t join_feature_bitmap) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/ /**
 * @brief
 *   Retrieves the join feature bitmap configuration for the Si91X device.
 *
 * @details
 *   This function gets the current join feature bitmap configuration for the specified Wi-Fi interface.
 *   The join feature bitmap determines various connection parameters and behaviors.
 *
 *   By default, the `SL_WIFI_JOIN_FEAT_LISTEN_INTERVAL_VALID` bitmap is enabled.
 *
 * @param[in] interface
 *   The selected Wi-Fi interface. Refer to [sl_wifi_interface_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-interface-t) for possible values.
 *
 * @param[out] join_feature_bitmap
 *   Pointer to a variable where the current join feature bitmap configuration will be stored. One or more values from @ref SI91X_JOIN_FEATURE_BIT_MAP.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_get_join_configuration](../wiseconnect-api-reference-guide-wi-fi/wifi-radio-api#sl-wifi-get-join-configuration) API. This is retained for backward compatibility.
 *******************************************************************************/
sl_status_t sl_si91x_get_join_configuration(sl_wifi_interface_t interface,
                                            uint8_t *join_feature_bitmap) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/ /**
 * @brief
 *   Trigger an assert in firmware.
 * 
 * @details
 *   This function is used to signal that an assertion has occurred in the firmware. It helps in debugging by indicating that a specific condition has failed.
 * 
 *   This is blocking API.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 *******************************************************************************/
sl_status_t sl_si91x_assert(void);

/***************************************************************************/ /**
 * @brief
 *   Writes calibration data to non-volatile device memory.
 * @param[in] data
 *   @ref si91x_calibration_data_t Pointer to buffer containing calibration data.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_write_calibration_data(const si91x_calibration_data_t *data);

/** \addtogroup SI91X_RADIO_FUNCTIONS 
  * \ingroup SL_SI91X_API
  * @{ */

/***************************************************************************/ /**
 * @brief
 *   Start the transmit test. 
 * 
 * @details
 *   This function starts the transmit test using the provided configuration. 
 * 
 *   This is a blocking API.
 * 
 *   This API is relevant in PER mode
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @param[in] tx_test_info
 *   Pointer to an [sl_wifi_request_tx_test_info_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-request-tx-test-info-t). structure containing the configuration for the transmit test.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 * 
 * @note Before starting Continuous Wave mode, user must start Continuous mode with power and channel values that are intended to be used in Continuous Wave mode i.e. \n
 *       - Start Continuous mode with intended power value and channel values - Pass any valid values for rate and length. 
 *       - Stop Continuous mode 
 *       - Start Continuous Wave mode 
 * @note If user wants to switch continuous wave mode, first need to stop the per mode and again need to give continous wave mode which user wants to switch. 
 ******************************************************************************/
sl_status_t sl_si91x_transmit_test_start(const sl_wifi_request_tx_test_info_t *tx_test_info);

/***************************************************************************/
/**
 * @brief
 *   Stop the transmit test. 
 * 
 * @details
 *   This function stops the ongoing transmit test on the Si91x device. 
 * 
 *   This is a blocking API. 
 * 
 *   This API is relevant in PER mode.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.  
 * 
 * @note 
 *  User should configure a minimum delay (approx. 10 milliseconds) before and after @ref sl_si91x_transmit_test_start API to observe a stable output at requested dBm level.
 ******************************************************************************/
sl_status_t sl_si91x_transmit_test_stop(void);

/***************************************************************************/ /**
 * @brief
 *   Provide feedback of frequency error in KHz. 
 * 
 * @details
 *   This function provides feedback of the frequency error in KHz. The frequency error is specified using the `sl_si91x_freq_offset_t` structure.
 * 
 *   This is a blocking API.
 *  
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @param[in] frequency_calibration
 *   Pointer to an @ref sl_si91x_freq_offset_t structure containing the frequency error in KHz.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 *******************************************************************************/
sl_status_t sl_si91x_frequency_offset(const sl_si91x_freq_offset_t *frequency_calibration);

/***************************************************************************/ /**
 * @brief
 *   Set the device region.
 * 
 * @details
 *   This function sets the operational region of the Si91x device. The region is specified using the `sl_wifi_region_code_t` enumeration.
 * 
 * @param[in] operation_mode
 *   Operation mode of the device, specified by [sl_wifi_operation_mode_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-types#sl-wifi-operation-mode-t).
 * 
 * @param[in] band
 *   Operational band of the device, specified by [sl_wifi_band_mode_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-band-mode-t).
 * 
 * @param[in] region_code
 *   Region code to be set in the device, specified by [sl_wifi_region_code_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-regulatory-region-t).
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note When SL_WIFI_DEFAULT_REGION is specified, the SiWx917 device applies the same settings as SL_WIFI_REGION_US.
 * @note SL_WIFI_REGION_WORLD_DOMAIN is supported only in SL_SI91X_CLIENT_MODE, SL_SI91X_TRANSCEIVER_MODE and SL_SI91X_TRANSMIT_TEST_MODE.
 * @note
 *   In FCC-certified SiWx91x Modules the behavior is as follows
 *      1. For FCC-certified modules, using this API will result in an SL_STATUS_SI91X_FEATURE_NOT_AVAILABLE error unless the module is in SL_SI91X_TRANSMIT_TEST_MODE.
 *      2. STA mode channels 1 to 11 are actively scanned and 12,13,14 are passively scanned.
 *      3. AP mode and Concurrent mode supports only 1 to 11 channels.
 *      4. The AP will not broadcast the Country Information Element (IE).
 *      5. `region_code` parameter will be ignored for modules. The device region for modules parts cannot be manually configured by the user. It automatically updates to align with the region of the connected AP.
 ******************************************************************************/
sl_status_t sl_si91x_set_device_region(sl_wifi_operation_mode_t operation_mode,
                                       sl_wifi_band_mode_t band,
                                       sl_wifi_region_code_t region_code);

/***************************************************************************/ /**
 * @brief
 *   Command the firmware to update the existing Flash/EFuse calibration data. 
 * 
 * @details
 *   This function commands the firmware to update the existing Flash/EFuse calibration data using the provided calibration configuration.
 * 
 *   This is a blocking API.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init), `sl_si91x_transmit_test_start`, and `sl_si91x_frequency_offset` should be called before this API.
 * 
 * @param[in] calib_write
 *   Write calibration configuration of type @ref sl_si91x_calibration_write_t.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note 
 *   Executing this API will overwrite calibration values in certified modules.
 *   In FCC-certified modules, this API will trigger an error SL_STATUS_SI91X_FEATURE_NOT_AVAILABLE if used, except when in SL_SI91X_TRANSMIT_TEST_MODE mode.
 ******************************************************************************/
sl_status_t sl_si91x_calibration_write(sl_si91x_calibration_write_t calib_write);

/***************************************************************************/
/**
 * @brief
 *   Read the calibration data from the Flash/EFuse storage. 
 * 
 * @details
 *   This function reads the calibration data from the specified storage (Flash or EFuse) and provides it in the `calibration_read` parameter.
 * 
 *  This is a blocking API.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @param[in] target
 *   Specifies the storage to read the calibration data from.
 *   - 0: READ_FROM_EFUSE (read calibration data from the EFuse)
 *   - 1: READ_FROM_FLASH (read calibration data from the Flash)
 * 
 * @param[out] calibration_read
 *   Pointer to an @ref sl_si91x_calibration_read_t structure where the read calibration data will be stored.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_si91x_calibration_read(sl_si91x_calibration_read_t target,
                                      sl_si91x_calibration_read_t *calibration_read);

/***************************************************************************/
/**
 * @brief
 *   Provide feedback on the error caused by the EVM offset. 
 * 
 * @details
 *   This function provides feedback on the error caused by the Error Vector Magnitude (EVM) offset. The EVM offset is specified using the `sl_si91x_evm_offset_t` structure.
 * 
 *   This is a blocking API.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @param[in] evm_offset
 *   Pointer to an `sl_si91x_evm_offset_t` structure containing the EVM offset.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In FCC-certified modules, this API will trigger an error SL_STATUS_SI91X_FEATURE_NOT_AVAILABLE if used, except when in SL_SI91X_TRANSMIT_TEST_MODE mode.
 ******************************************************************************/
sl_status_t sl_si91x_evm_offset(const sl_si91x_evm_offset_t *evm_offset);

/***************************************************************************/
/**
 * @brief
 *   Command the firmware to update the existing Flash/EFuse calibration data. 
 * 
 * @details
 *   This function commands the firmware to update the existing Flash/EFuse calibration data using the provided EVM calibration configuration.
 * 
 * This is a blocking API.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init), @ref sl_si91x_evm_offset, and @ref sl_si91x_transmit_test_start should be called before this API.
 * 
 * @param[in] evm_write
 *   Pointer to an @ref sl_si91x_evm_write_t structure containing the EVM calibration configuration.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In FCC-certified modules, this API will trigger an error SL_STATUS_SI91X_FEATURE_NOT_AVAILABLE if used, except when in SL_SI91X_TRANSMIT_TEST_MODE mode.
 ******************************************************************************/
sl_status_t sl_si91x_evm_write(const sl_si91x_evm_write_t *evm_write);

/***************************************************************************/ /**
 * @brief
 *   Read data from Efuse memory location. 
 * 
 * @details
 *   This function reads data from the specified Efuse memory location. The data is read into the provided buffer.
 * 
 *  This is a blocking API.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @param[in] efuse_read
 *   Pointer to an @ref sl_si91x_efuse_read_t structure, which contains the Efuse read address offset and read data length.
 *   - Efuse Read Address Offset: Specifies the starting byte address in the Efuse memory to be read. Valid range: 144 to 255. For more information refer to @ref sl_si91x_efuse_read_t.
 *   - Read Data Length: Specifies the number of bytes to read from the given offset. Please refer to @ref sl_si91x_efuse_read_t for the length in bytes which can be read from each offset.
 * 
 * @param[out] efuse_read_buf
 *   Pointer to a buffer where the read Efuse data will be stored.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   If a valid Efuse is not present on the device, this API returns the error SL_STATUS_SI91X_EFUSE_DATA_INVALID.
 *******************************************************************************/
sl_status_t sl_si91x_efuse_read(const sl_si91x_efuse_read_t *efuse_read, uint8_t *efuse_read_buf);

/***************************************************************************/ /**
 * @brief
 *   Update Flash/EFuse DPD data. 
 * 
 * @details
 *   This function updates the Flash/EFuse DPD (Digital Pre-Distortion) data using the provided DPD calibration data.
 * 
 *   This is a blocking API.
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) and @ref sl_si91x_transmit_test_start should be called before this API.
 * 
 * @param[in] dpd_calib_data
 *   Pointer to an @ref sl_si91x_get_dpd_calib_data_t structure containing the DPD calibration data.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   In FCC-certified modules, this API will trigger an errorSL_STATUS_SI91X_FEATURE_NOT_AVAILABLE if used, except when in SL_SI91X_TRANSMIT_TEST_MODE mode.
 ******************************************************************************/
sl_status_t sl_si91x_dpd_calibration(const sl_si91x_get_dpd_calib_data_t *dpd_calib_data);

/***************************************************************************/ /**
 * @brief
 *   Enable wireless radio.
 * 
 * @details
 *   This function enables the wireless radio on the Si91x device. 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_si91x_enable_radio(void);

/***************************************************************************/
/**
 * @brief
 *   Disable wireless radio.
 * 
 * @details
 *   This function disables the wireless radio on the Si91x device. 
 * 
 * @pre Pre-conditions:
 * - [sl_wifi_init](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_si91x_disable_radio(void);

/***************************************************************************/ /**
 * @brief
 *   Set the Wi-Fi listen interval for the Si91x device.
 * 
 * @details
 *   This function sets the Wi-Fi listen interval for the Si91x device. The listen interval determines how often the device wakes up to listen for beacon frames from the access point.
 * 
 * @param[in] listen_interval
 *   Wi-Fi listen interval in milli seconds.
 * @note
 *   Moving forward, this API will be deprecated. Instead, use the [sl_wifi_set_listen_interval_v2](../wiseconnect-api-reference-guide-wi-fi/wifi-radio-api#sl-wifi-set-listen-interval-v2) API. This is retained for backward compatibility.
 ******************************************************************************/
void sl_si91x_set_listen_interval(uint32_t listen_interval) SL_DEPRECATED_API_WISECONNECT_3_5;

/** @} */

/***************************************************************************/ /**
 * @brief
 *   Send a raw command frame.
 * @param[in] command
 *   Command type to be sent.
 * @param[in] data
 *   Pointer to the command data to be sent.
 * @param[in] data_length
 *   Length of the data length
 * @param[in] wait_time
 *   Wait time for the command response.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_driver_raw_send_command(uint8_t command,
                                             const void *data,
                                             uint32_t data_length,
                                             uint32_t wait_time);

//! @cond Doxygen_Suppress
/***************************************************************************/ /**
 * @brief
 *   Set device power configuration.
 * @param[in] mode
 *   @ref sl_si91x_power_mode_t Power mode to be set to the device.
 * @param[in] config
 *   @ref sl_si91x_power_configuration_t Pointer to structure containing power configuration.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_set_power_mode(sl_si91x_power_mode_t mode, const sl_si91x_power_configuration_t *config);
//! @endcond

/***************************************************************************/ /**
 * @brief     Si91X specific Wi-Fi transceiver mode driver function to send Tx data
 * @param[in] control - Meta data for the payload.
 * @param[in] payload      - Pointer to payload to be sent to LMAC.
 * @param[in] payload_len  - Length of the payload.
 * @param[in] wait_time    - Wait time for the command response.
 * @return    sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 *******************************************************************************/
sl_status_t sl_si91x_driver_send_transceiver_data(sl_wifi_transceiver_tx_data_control_t *control,
                                                  const uint8_t *payload,
                                                  uint16_t payload_len,
                                                  uint32_t wait_time);

//! @cond Doxygen_Suppress
/***************************************************************************/ /**
 * @brief
 *   Register a function and optional argument for scan results callback.
 * @param[in] command
 *   Command type to be sent to NWP firmware.
 * @param[in] queue_type
 *   @ref sli_wifi_command_type_t Command type
 * @param[in] data
 *   Command packet to be sent to the NWP firmware.
 * @param[in] data_length
 *   Length of command packet.
 * @param[in] wait_period
 *   @ref sli_wifi_wait_period_t Timeout for the command response.
 * @param[in] sdk_context
 *   Pointer to the context.
 * @param[in] data_buffer
 *   [sl_wifi_buffer_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-buffer-t) Pointer to a data buffer pointer for the response data to be returned in.
 *  @param[in] custom_host_desc
 *   Custom Variable to send additional data to the firmware through the host descriptor.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_custom_driver_send_command(uint32_t command,
                                                sli_wifi_command_type_t command_type,
                                                const void *data,
                                                uint32_t data_length,
                                                sli_wifi_wait_period_t wait_period,
                                                void *sdk_context,
                                                sl_wifi_buffer_t **data_buffer,
                                                uint8_t custom_host_desc);
//! @endcond

/***************************************************************************/ /**
 * @brief
 *   Wait for response packet
 * @pre Pre-conditions:
 * -
 *   [sl_wifi_init()](../wiseconnect-api-reference-guide-wi-fi/wifi-common-api#sl-wifi-init) should be called before this API.
 * @param[in] queue
 *   Buffer queue
 * @param[in] event_flag
 *   Event flags
 * @param[in] event_mask
 *   Packet id
 * @param[in] packet_id
 *   Packet id
 * @param[in] wait_period
 *   Wait period
 * @param[out] packet_buffer
 *   Pointer that will contain the response packet buffer when the function successfully returns.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sli_si91x_driver_wait_for_response_packet(sli_wifi_buffer_queue_t *queue,
                                                      osEventFlagsId_t event_flag,
                                                      uint32_t event_mask,
                                                      uint16_t packet_id,
                                                      sli_wifi_wait_period_t wait_period,
                                                      sl_wifi_buffer_t **packet_buffer);

/***************************************************************************/ /**
 * @brief
 *   Set the Wi-Fi listen interval and listen interval multiplier for the Si91x device.
 *
 * @details
 *   This function sets the Wi-Fi listen interval and listen interval multiplier for the Si91x device. The listen interval determines how often the device wakes up to listen for beacon frames from the access point.The listen interval is multiplied with listen interval multiplier and advertised in the assoc request.
 *
 * @param[in] listen_interval
 *   Wi-Fi listen interval in milli seconds and listen interval multiplier, specified by sl_wifi_listen_interval_v2_t.
 ******************************************************************************/
void sli_si91x_set_listen_interval(sl_wifi_listen_interval_v2_t listen_interval);

/***************************************************************************/ /**
 * @brief
 *   Si91X specific get listen interval
 *******************************************************************************/
void sli_si91x_get_listen_interval(sl_wifi_listen_interval_v2_t *listen_interval);

/***************************************************************************/
/**
 * @brief
 *   Enqueue a status message to the command engine status queue and set an event.
 *
 * @param[in] status
 *   The status code indicating the error condition to be enqueued.
 *   Must be of type sl_status_t.
 *
 * @note
 *   This function is intended for internal use by the Si91x driver and should not be called directly
 *   by applications. If the queue is full or not initialized, the status will be dropped
 *   and an error will be logged.
 ******************************************************************************/
void sli_command_engine_status_queue_enqueue_and_set_event(sl_status_t status);

/***************************************************************************/ /**
* @brief
*   Initialize the command engine status message queue for the Si91x device.
*
* @details
*   Creates a message queue to store command engine status messages used for error reporting
*   within the Si91x driver. This queue is essential for asynchronous error notifications
*   from the command engine to higher layers of the stack.
*
*   The function:
*   1. Creates a new queue if it doesn't exist already
*   2. Configures the queue with the appropriate size and message format
*   3. Returns an error if the queue already exists or if creation fails
*
*   The queue is used in conjunction with the
*   sli_command_engine_status_queue_enqueue_and_set_event() function to report critical
*   error conditions like memory allocation failures, bus errors, or timeouts.
*
* @return
*   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details
*   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
*
*   Returns:
*   - SL_STATUS_OK if the queue was successfully created
*   - SL_STATUS_ALREADY_INITIALIZED if the queue already exists
*   - SL_STATUS_ALLOCATION_FAILED if the queue creation fails due to memory constraints
******************************************************************************/
sl_status_t sli_command_engine_status_queue_init();

/***************************************************************************/ /**
* @brief
*   Deinitialize the command engine status message queue for Si91x device.
*
* @details
*   Releases resources and deinitializes the command engine status message queue.
*   This should be called when the queue is no longer needed to avoid memory leaks.
*
* @return
*   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
*   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
******************************************************************************/
sl_status_t sli_command_engine_status_queue_deinit();

/***************************************************************************/ /**
 * @brief Retrieves the current timestamp from the NWP.
 *
 * This function queries the Network Processor for its current timestamp value and stores
 * the result in the provided timestamp pointer.
 *
 * @param[out] timestamp Pointer to a variable where the NWP timestamp will be stored.
 *                      Must not be NULL.
 *
 * @return sl_status_t Returns SL_STATUS_OK if the operation is successful,
 *                     or an appropriate error code otherwise.
 ******************************************************************************/
sl_status_t sli_get_nwp_timestamp(uint32_t *timestamp);