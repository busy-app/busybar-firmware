/***************************************************************************/ /**
 * @file
 * @brief CPC Internal Definitions
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

#ifndef SLI_CPC_H
#define SLI_CPC_H

#include <stddef.h>
#include <stdarg.h>

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sl_status.h"
#include "sl_enum.h"
#include "sl_slist.h"
#include "sli_cpc_types.h"
#include "sli_cpc_timer.h"
#include "sl_cpc.h"
#include "sl_cpc_config.h"
#include "sli_cpc_system_common.h"
#if defined(SL_CATALOG_CPC_SECURITY_PRESENT)
#include "sl_cpc_security.h"
#include "sl_cpc_security_config.h"
#include "sli_cpc_security.h"
#endif

#if defined(SL_CATALOG_CPC_JOURNAL_PRESENT)
#include "sl_cpc_journal.h"
#else
#define SL_CPC_JOURNAL_RECORD_ERROR(string, value) ((void)(string), (void)(value))
#define SL_CPC_JOURNAL_RECORD_WARNING(string, value) ((void)(string), (void)(value))
#define SL_CPC_JOURNAL_RECORD_INFO(string, value) ((void)(string), (void)(value))
#define SL_CPC_JOURNAL_RECORD_DEBUG(string, value) ((void)(string), (void)(value))
#endif

#if !defined(SL_CATALOG_EMLIB_CORE_PRESENT) && !defined(SL_CATALOG_SL_CORE_PRESENT)
#include "mcu.h"
#else
#if defined(SL_CATALOG_EMLIB_CORE_PRESENT)
#include "em_core.h"
#elif defined(SL_CATALOG_SL_CORE_PRESENT)
#include "sl_core.h"
#endif
#include "sl_atomic.h"

#define MCU_DECLARE_IRQ_STATE   CORE_DECLARE_IRQ_STATE
#define MCU_ENTER_CRITICAL      CORE_ENTER_CRITICAL
#define MCU_EXIT_CRITICAL       CORE_EXIT_CRITICAL
#define MCU_ATOMIC_SECTION      CORE_ATOMIC_SECTION
#define MCU_ENTER_ATOMIC        CORE_ENTER_ATOMIC
#define MCU_EXIT_ATOMIC         CORE_EXIT_ATOMIC
#define MCU_ATOMIC_LOAD         sl_atomic_load
#define MCU_ATOMIC_STORE        sl_atomic_store
#endif

#define SLI_CPC_PROTOCOL_VERSION            (6)

#define SLI_CPC_ENDPOINT_SYSTEM             (1)

#if (!defined(SL_CATALOG_CPC_SECURITY_PRESENT))
#define SL_CPC_ENDPOINT_SECURITY_ENABLED    (0)
#else
#if (SL_CPC_SECURITY_ENABLED >= 1)
#define SL_CPC_ENDPOINT_SECURITY_ENABLED    (1)
#else
#define SL_CPC_ENDPOINT_SECURITY_ENABLED    (0)
#endif
#endif

#if defined(SL_CATALOG_BLUETOOTH_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_BLUETOOTH_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_BLUETOOTH_ENABLED  (0)
#endif

#if defined(SL_CATALOG_RAIL_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_RAIL_ENABLED (2)
#else
#define SLI_CPC_ENDPOINT_RAIL_ENABLED (0)
#endif

#if defined(SL_CATALOG_ZIGBEE_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_ZIGBEE_ENABLED (1)
#else
#define SLI_CPC_ENDPOINT_ZIGBEE_ENABLED (0)
#endif

#if defined(SL_CATALOG_ZWAVE_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_ZWAVE_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_ZWAVE_ENABLED  (0)
#endif

#if defined(SL_CATALOG_CONNECT_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_CONNECT_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_CONNECT_ENABLED  (0)
#endif

#if defined(SL_CATALOG_GPIO_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_GPIO_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_GPIO_ENABLED  (0)
#endif

#if defined(SL_CATALOG_OPENTHREAD_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_OPENTHREAD_ENABLED (1)
#else
#define SLI_CPC_ENDPOINT_OPENTHREAD_ENABLED (0)
#endif

#if defined(SL_CATALOG_WISUN_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_WISUN_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_WISUN_ENABLED  (0)
#endif

#if defined(SL_CATALOG_WIFI_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_WIFI_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_WIFI_ENABLED  (0)
#endif

#if defined(SL_CATALOG_15_4_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_15_4_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_15_4_ENABLED  (0)
#endif

#if defined(SL_CATALOG_CLI_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_CLI_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_CLI_ENABLED  (0)
#endif

#if defined(SL_CATALOG_CLI_NCP_PRESENT)
#define SLI_CPC_ENDPOINT_CLI_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_CLI_ENABLED  (0)
#endif

#if defined(SL_CATALOG_BLUETOOTH_RCP_PRESENT)
#define SLI_CPC_ENDPOINT_BLUETOOTH_RCP_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_BLUETOOTH_RCP_ENABLED  (0)
#endif

#if defined(SL_CATALOG_ACP_PRESENT)
#define SLI_CPC_ENDPOINT_ACP_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_ACP_ENABLED  (0)
#endif

#if defined(SL_CATALOG_SE_CPC_PRESENT)
#define SLI_CPC_ENDPOINT_SE_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_SE_ENABLED  (0)
#endif

#if defined(SL_CATALOG_CPC_NVM3_PRESENT)
#define SLI_CPC_ENDPOINT_NVM3_ENABLED  (1)
#else
#define SLI_CPC_ENDPOINT_NVM3_ENABLED  (0)
#endif

// Frame Flags
// I-Frame is enabled by default; this flag MUST be set to disable the I-Frame support by the endpoint
#define SL_CPC_ENDPOINT_FLAG_IFRAME_DISABLE             (0x01 << 0)
// U-Frame is disabled by default; this flag MUST be set to enable U-Frame support by the endpoint
#define SL_CPC_ENDPOINT_FLAG_UFRAME_ENABLE              (0x01 << 1)
// Disable non-poll/final U-Frame
#define SL_CPC_ENDPOINT_FLAG_UFRAME_INFORMATION_DISABLE (0x01 << 2)
// Disable encryption
// SL_CPC_ENDPOINT_FLAG_DISABLE_ENCRYPTION              (0x01 << 3) is defined in public header
// Endpoint was opened using legacy API
#define SL_CPC_ENDPOINT_FLAG_LEGACY_API                 (0x01 << 4)

// Legacy Flags
#define SL_CPC_OPEN_ENDPOINT_FLAG_IFRAME_DISABLE        SL_CPC_ENDPOINT_FLAG_IFRAME_DISABLE
#define SL_CPC_OPEN_ENDPOINT_FLAG_UFRAME_ENABLE         SL_CPC_ENDPOINT_FLAG_UFRAME_ENABLE
#define SL_CPC_OPEN_ENDPOINT_FLAG_UFRAME_INFORMATION_DISABLE SL_CPC_ENDPOINT_FLAG_UFRAME_INFORMATION_DISABLE
// SL_CPC_OPEN_ENDPOINT_FLAG_DISABLE_ENCRYPTION         is defined in public header

// SL_CPC_FLAG_NO_BLOCK = 0x01 << 0
#define SL_CPC_FLAG_UNNUMBERED_INFORMATION      (0x01 << 1)
#define SL_CPC_FLAG_UNNUMBERED_POLL_FINAL       (0x01 << 2)
#define SL_CPC_FLAG_UNNUMBERED_ACKNOWLEDGE      (0x01 << 3)
#define SL_CPC_FLAG_INFORMATION_POLL_FINAL      (0x01 << 4)
#define SL_CPC_FLAG_BLOCKING_WRITE              (0x01 << 5)

/// @brief Enumeration representing the possible id
SL_ENUM(sl_cpc_service_endpoint_id_t){
  SL_CPC_ENDPOINT_SYSTEM = 0,          ///< System control
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  SL_CPC_ENDPOINT_SECURITY = 1,        ///< Security - related functionality
#endif
#if (SLI_CPC_ENDPOINT_BLUETOOTH_ENABLED >= 1)
  SL_CPC_ENDPOINT_BLUETOOTH = 2,       ///< Bluetooth endpoint
#endif
#if (SLI_CPC_ENDPOINT_RAIL_ENABLED >= 1)
  SL_CPC_SLI_CPC_ENDPOINT_RAIL_DOWNSTREAM = 3, ///< RAIL downstream endpoint
  SL_CPC_SLI_CPC_ENDPOINT_RAIL_UPSTREAM = 4,   ///< RAIL upstream endpoint
#endif
#if (SLI_CPC_ENDPOINT_ZIGBEE_ENABLED >= 1)
  SL_CPC_ENDPOINT_ZIGBEE = 5,          ///< ZigBee EZSP endpoint
#endif
#if (SLI_CPC_ENDPOINT_ZWAVE_ENABLED >= 1)
  SL_CPC_ENDPOINT_ZWAVE = 6,           ///< Z-Wave endpoint
#endif
#if (SLI_CPC_ENDPOINT_CONNECT_ENABLED >= 1)
  SL_CPC_ENDPOINT_CONNECT = 7,         ///< Connect endpoint
  #endif
#if (SLI_CPC_ENDPOINT_GPIO_ENABLED >= 1)
  SL_CPC_ENDPOINT_GPIO = 8,            ///< GPIO endpoint for controlling GPIOs on SECONDARYs
#endif
#if (SLI_CPC_ENDPOINT_OPENTHREAD_ENABLED >= 1)
  SL_CPC_ENDPOINT_OPENTHREAD = 9,      ///< Openthread Spinel endpoint
#endif
#if (SLI_CPC_ENDPOINT_WISUN_ENABLED >= 1)
  SL_CPC_ENDPOINT_WISUN = 10,           ///< WiSun endpoint
#endif
#if (SLI_CPC_ENDPOINT_WIFI_ENABLED >= 1)
  SL_CPC_ENDPOINT_WIFI = 11,           ///< WiFi endpoint(main control)
#endif
#if (SLI_CPC_ENDPOINT_15_4_ENABLED >= 1)
  SL_CPC_ENDPOINT_15_4 = 12,           ///< 802.15.4 endpoint
#endif
#if (SLI_CPC_ENDPOINT_CLI_ENABLED >= 1)
  SL_CPC_ENDPOINT_CLI = 13,            ///< Ascii based CLI for stacks / applications
#endif
#if (SLI_CPC_ENDPOINT_BLUETOOTH_RCP_ENABLED >= 1)
  SL_CPC_ENDPOINT_BLUETOOTH_RCP = 14,  ///< Bluetooth RCP endpoint
#endif
#if (SLI_CPC_ENDPOINT_ACP_ENABLED >= 1)
  SL_CPC_ENDPOINT_ACP = 15,            ///< ACP endpoint
#endif
#if (SLI_CPC_ENDPOINT_SE_ENABLED >= 1)
  SL_CPC_ENDPOINT_SE = 16,             ///< Secure Engine endpoint
#endif
#if (SLI_CPC_ENDPOINT_NVM3_ENABLED >= 1)
  SL_CPC_ENDPOINT_NVM3 = 17,           ///< NVM3 endpoint
#endif
  SL_CPC_ENDPOINT_LAST_ID_MARKER,      // DO NOT USE THIS ENDPOINT ID
};

#define SLI_CPC_SERVICE_ENDPOINT_MAX_COUNT    (SLI_CPC_ENDPOINT_SYSTEM + SL_CPC_ENDPOINT_SECURITY_ENABLED + SLI_CPC_ENDPOINT_BLUETOOTH_ENABLED          \
                                               + SLI_CPC_ENDPOINT_RAIL_ENABLED + SLI_CPC_ENDPOINT_ZIGBEE_ENABLED + SLI_CPC_ENDPOINT_ZWAVE_ENABLED       \
                                               + SLI_CPC_ENDPOINT_CONNECT_ENABLED + SLI_CPC_ENDPOINT_GPIO_ENABLED + SLI_CPC_ENDPOINT_OPENTHREAD_ENABLED \
                                               + SLI_CPC_ENDPOINT_WISUN_ENABLED + SLI_CPC_ENDPOINT_WIFI_ENABLED + SLI_CPC_ENDPOINT_15_4_ENABLED         \
                                               + SLI_CPC_ENDPOINT_CLI_ENABLED + SLI_CPC_ENDPOINT_BLUETOOTH_RCP_ENABLED + SLI_CPC_ENDPOINT_ACP_ENABLED   \
                                               + SLI_CPC_ENDPOINT_SE_ENABLED + SLI_CPC_ENDPOINT_NVM3_ENABLED)

#define SLI_CPC_SERVICE_ENDPOINT_ID_START     ((uint8_t)SL_CPC_ENDPOINT_SYSTEM)
#define SLI_CPC_SERVICE_ENDPOINT_ID_END       ((uint8_t)SL_CPC_ENDPOINT_LAST_ID_MARKER - 1)

/***************************************************************************//**
 * The maximum size of a system endpoint command buffer.
 *
 * @note
 *   For the moment, this value must be manually set.
 *
 * @note : it is set to the size of the payload of a
 * CMD_PROPERTY_GET::PROP_ENDPOINT_STATES.
 *
 ******************************************************************************/
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define SL_CPC_ENDPOINT_MAX_COUNT  256

#ifndef SLI_CPC_SYSTEM_COMMAND_BUFFER_COUNT
#define SLI_CPC_SYSTEM_COMMAND_BUFFER_COUNT 5
#endif

#ifndef SLI_CPC_SYSTEM_COMMAND_HANDLE_COUNT
#define SLI_CPC_SYSTEM_COMMAND_HANDLE_COUNT 2
#endif

// Length of the maximum CPC payload that can be received. This take into
// account: user payload + CRC (2B) [ + Security Tag (8B) ]
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  #define SLI_CPC_RX_DATA_MAX_LENGTH_CFG(_user_payload_max) (_user_payload_max \
                                                             + 2               \
                                                             + SLI_SECURITY_TAG_LENGTH_BYTES)
#else
  #define SLI_CPC_RX_DATA_MAX_LENGTH_CFG(_user_payload_max) (_user_payload_max \
                                                             + 2)
#endif

// Number of payload to allocate to send HDLC reject reasons
#define SLI_CPC_HDLC_REJECT_MAX_COUNT_CFG(_user_rx_buffer_max)         ((_user_rx_buffer_max) / 2 + 1)

// Maximum number of RX queue item. It is _user_rx_buffer_max - 1 to make sure
// there is one RX buffer available to receive an acknowledgement from the
// remote, otherwise the bus could end up deadlock
#define SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT_CFG(_user_rx_buffer_max)       ((_user_rx_buffer_max) - 1)

// Maximum number of S-Frame, it matches the number of RX queue item to make
// sure they can be all acked
#define SLI_CPC_TX_QUEUE_ITEM_SFRAME_MAX_COUNT_CFG(_rx_queue_item_max) (_rx_queue_item_max)

// Maximum number of buffer handles. There must be enough for all TX queue
// item (user data), RX buffer and TX S-Frames, so it's just the sum of that.
#define SLI_CPC_BUFFER_HANDLE_MAX_COUNT_CFG(_user_tx_queue_item_max, _user_rx_buffer_max, _tx_queue_item_sframe_max) \
  (_user_tx_queue_item_max + _user_rx_buffer_max + _tx_queue_item_sframe_max)

// Maximum number of HDLC header. There is one header per buffer handle.
#define SLI_CPC_HDLC_HEADER_MAX_COUNT_CFG(_buffer_handle_max_cnt)      (_buffer_handle_max_cnt)

// Maximum frame size, ie. the maximum size for a full CPC frame on the bus
// (header + user payload)
#define SLI_CPC_RX_FRAME_MAX_LENGTH_CFG(_rx_data_max_length)           ((uint16_t)((_rx_data_max_length) + SLI_CPC_HDLC_HEADER_RAW_SIZE))

// define for the size that must be allocated to have an sli_cpc_rx_buffer_t
// with a data array of size "_rx_frame_max_length"
#define SLI_CPC_RX_BUFFER_MAX_LENGTH_CFG(_rx_frame_max_length)         ((_rx_frame_max_length) + sizeof(sli_cpc_rx_buffer_t))

// Signal count max = closing signal per endpoint + tx queue items + rx queue items
#define SLI_CPC_EVENT_SIGNAL_MAX_COUNT_CFG(_user_tx_queue_item_max, _rx_queue_item_max) \
  (SL_CPC_ENDPOINT_MAX_COUNT + _user_tx_queue_item_max + _rx_queue_item_max)

#define SLI_CPC_RE_TRANSMIT                    (10)
#define SLI_CPC_INIT_RE_TRANSMIT_TIMEOUT_MS    (100)
#define SLI_CPC_MAX_RE_TRANSMIT_TIMEOUT_MS     (5000)
#define SLI_CPC_MIN_RE_TRANSMIT_TIMEOUT_MS     (5)
#define SLI_CPC_DISCONNECTION_NOTIFICATION_TIMEOUT_MS  (1000)
#define SLI_CPC_MIN_RE_TRANSMIT_TIMEOUT_MINIMUM_VARIATION_MS (5)
#define SLI_CPC_SYSTEM_CMD_TIMEOUT_MS           (10000)

#define SLI_CPC_TEMPORARY_ENDPOINT_ID_START   100
#define SLI_CPC_TEMPORARY_ENDPOINT_ID_END     255
#define SLI_CPC_ENDPOINT_TEMPORARY_MAX_COUNT  (SLI_CPC_TEMPORARY_ENDPOINT_ID_END - SLI_CPC_TEMPORARY_ENDPOINT_ID_START + 1)

#define SLI_CPC_ENDPOINT_MIN_COUNT  (SLI_CPC_ENDPOINT_SYSTEM + SL_CPC_ENDPOINT_SECURITY_ENABLED)

SL_ENUM(sl_cpc_signal_type_t) {
  SL_CPC_SIGNAL_RX,
  SL_CPC_SIGNAL_TX,
  SL_CPC_SIGNAL_CLOSED,
  SL_CPC_SIGNAL_SYSTEM,
};

SL_ENUM(sl_cpc_reject_reason_t){
  SL_CPC_REJECT_NO_ERROR = 0,
  SL_CPC_REJECT_CHECKSUM_MISMATCH,
  SL_CPC_REJECT_SEQUENCE_MISMATCH,
  SL_CPC_REJECT_OUT_OF_MEMORY,
  SL_CPC_REJECT_SECURITY_ISSUE,
  SL_CPC_REJECT_UNREACHABLE_ENDPOINT,
  SL_CPC_REJECT_ERROR
};

SL_ENUM(sl_cpc_buffer_handle_type_t) {
  SL_CPC_UNKNOWN_BUFFER_HANDLE,
  SL_CPC_RX_INTERNAL_BUFFER_HANDLE,
  SL_CPC_RX_USER_BUFFER_HANDLE,
  SL_CPC_TX_REJECT_BUFFER_HANDLE,
  SL_CPC_TX_DATA_BUFFER_HANDLE,
  SL_CPC_TX_SFRAME_BUFFER_HANDLE,
};

typedef void (*sl_cpc_dispatcher_fnct_t)(void *data);

/**
 * @brief This structure defines cpc dispatcher handle.
 */
typedef struct {
  sl_slist_node_t node;                            ///< node
  sl_cpc_dispatcher_fnct_t fnct;                   ///< fnct
  sli_cpc_instance_t *instance;                    ///< instance
  void *data;                                      ///< data
  bool submitted;                                  ///< submitted
} sl_cpc_dispatcher_handle_t;

/**
 * @brief This structure defines cpc buffer handle.
 */
typedef struct {
  uint8_t *hdlc_header;                            ///< hdlc header
  uint8_t *data;                                   ///< data
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  void *security_tag;
#endif
  uint16_t data_length;                            ///< data_length
  uint16_t fcs;                                    ///< fcs
  uint8_t control;                                 ///< control
  uint8_t address;                                 ///< address
  uint8_t ref_count;                               ///< ref count
  sl_cpc_buffer_handle_type_t type;                ///< buffer handle type
  sl_cpc_endpoint_t *endpoint;                     ///< endpoint
  sli_cpc_instance_t *instance;                    ///< instance
  sl_cpc_reject_reason_t reason;                   ///< reason
  void *arg;                                       ///< arg
  bool on_write_complete_pending;                  ///< on write complete pending
  sl_status_t write_status;                        ///< write status
  sl_slist_node_t driver_node;                     ///< driver node
  sl_slist_node_t core_node;                       ///< core node
} sl_cpc_buffer_handle_t;

/**
 * @brief This structure defines cpc receive queue item.
 */
typedef struct {
  sl_slist_node_t node;                            ///< node
  void *data;                                      ///< data
  uint16_t data_length;                            ///< data length
  sl_cpc_buffer_handle_type_t buffer_type;         ///< buffer handle type
} sl_cpc_receive_queue_item_t;

#ifdef __cplusplus
extern "C"
{
#endif
// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************//**
 * Return the instance associated with endpoint
 *
 * @param[in] id  Endpoint ID.
 *
 * @return Instance pointer, might be NULL.
 ******************************************************************************/
sli_cpc_instance_t* sli_cpc_get_instance(uint8_t ep_id);

/***************************************************************************//**
 * Dynamically allocate and initialize Silicon Labs Internal Service endpoint.
 *
 * @param[in] endpoint_handle  Endpoint Handle.
 *
 * @param[in] id  Endpoint ID.
 *
 * @param[in] flags  Initialization flags. Reserved for future used
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_init_service_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                          sl_cpc_service_endpoint_id_t id,
                                          uint8_t flags);

/***************************************************************************//**
 * Dynamically allocate and initialize temporary endpoint.
 *
 * @param[in] endpoint_handle  Endpoint Handle.
 *
 * @param[out] id  Endpoint ID.
 *
 * @param[in] flags  Initialization flags. Reserved for future used
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_init_temporary_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                            uint8_t *id,
                                            uint8_t flags);

/***************************************************************************//**
 * Open Silicon Labs Internal Service endpoint.
 *
 * @param[in] endpoint_handle  Endpoint Handle.
 *
 * @param[in] id  Endpoint ID.
 *
 * @param[in] flags  Endpoint flags.
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_NONE                Default behaviors
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_DISABLE_ENCRYPTION  Disable encryption on the endpoint
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_IFRAME_DISABLE      Discard I - frame received on the endpoint
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_UFRAME_ENABLE       Enable reception of U-frame on the endpoint
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_UFRAME_INFORMATION_DISABLE  Only allow U-frame of type P/F
 *
 * @param[in] tx_window_size  Endpoint TX Window size.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_open_service_endpoint (sl_cpc_endpoint_handle_t *endpoint_handle,
                                           sl_cpc_service_endpoint_id_t id,
                                           uint8_t flags,
                                           uint8_t tx_window_size);

/***************************************************************************//**
 * Open Silicon Labs Internal Service endpoint with an explicit instance to
 * operate on. This is reserved for the system endpoint and MUST NOT be used
 * outside of CPC.
 *
 * @param[in] endpoint_handle  Endpoint Handle.
 * @param[in] inst             Instance pointer.
 * @param[in] id               Endpoint ID.
 * @param[in] flags  Endpoint flags.
 * @param[in] tx_window_size  Endpoint TX Window size.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_open_service_endpoint_in_instance(sl_cpc_endpoint_handle_t *endpoint_handle,
                                                      sli_cpc_instance_t *inst,
                                                      sl_cpc_service_endpoint_id_t id,
                                                      uint8_t flags,
                                                      uint8_t tx_window_size);

/***************************************************************************//**
 * Open temporary endpoint.
 *
 * @param[in] endpoint_handle  Endpoint Handle.
 *
 * @param[out] id  Endpoint ID.
 *
 * @param[in] flags  Endpoint flags:
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_NONE                Default behaviors
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_DISABLE_ENCRYPTION  Disable encryption on the endpoint
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_IFRAME_DISABLE      Discard I-frame received on the endpoint
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_UFRAME_ENABLE       Enable reception of u-frame on the endpoint
 *                      SL_CPC_OPEN_ENDPOINT_FLAG_UFRAME_INFORMATION_DISABLE  Only allow U-frame of type P/F
 *
 * @param[in] tx_window_size  Endpoint TX Window size.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_open_temporary_endpoint(sl_cpc_endpoint_handle_t *endpoint_handle,
                                            uint8_t *id,
                                            uint8_t flags,
                                            uint8_t tx_window_size);

/***************************************************************************//**
 * Signal process needed.
 ******************************************************************************/
void sli_cpc_signal_event(sli_cpc_instance_t *inst, sl_cpc_signal_type_t signal_type);

/***************************************************************************//**
 * Initialize the dispatcher handle.
 *
 * @param[in] handle  Dispatch queue node.
 ******************************************************************************/
void sli_cpc_dispatcher_init_handle(sl_cpc_dispatcher_handle_t *handle,
                                    sli_cpc_instance_t *inst);

/***************************************************************************//**
 * Push function in dispatch queue along with the data to be passed when
 * dispatched.
 *
 * @param[in] handle  Dispatch queue node.
 * @param[in] fnct    Function to be dispatched.
 * @param[in] data    Data to pass to the function.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_dispatcher_push(sl_cpc_dispatcher_handle_t *handle,
                                    sl_cpc_dispatcher_fnct_t fnct,
                                    void *data);

/***************************************************************************//**
 * Remove function from dispatch queue along with the data to be passed when
 * dispatched.
 *
 * @param[in] handle  Dispatch queue node.
 ******************************************************************************/
void sli_cpc_dispatcher_cancel(sl_cpc_dispatcher_handle_t *handle);

/***************************************************************************//**
 * Process the dispatch queue.
 *
 * @brief
 *   This function empty the dispatch queue by calling all the functions
 *   registered.
 ******************************************************************************/
void sli_cpc_dispatcher_process(sli_cpc_instance_t* inst);

/***************************************************************************//**
 * Notify that remote is connecting to this endpoint if ready.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_instance_on_remote_ep_connecting(sli_cpc_instance_t *inst,
                                                     uint8_t endpoint_id,
                                                     sl_cpc_endpoint_state_t *reply_state);

/***************************************************************************//**
 * Notify the user that an endpoint was opened by the host/primary.
 ******************************************************************************/
void sli_cpc_instance_on_remote_ep_connected(sli_cpc_instance_t *inst,
                                             uint8_t endpoint_id,
                                             sl_status_t status);

/***************************************************************************//**
 * Notify the core that the remote has requested a connection shutdown.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_instance_on_remote_ep_shutdown(sli_cpc_instance_t *inst,
                                                   uint8_t endpoint_id,
                                                   sl_cpc_endpoint_state_t *reply_state);

/***************************************************************************//**
 * Completed the endpoint disconnection handshake.
 *
 * @note Endpoint must be locked by caller.
 ******************************************************************************/
void sli_cpc_instance_on_remote_ep_disconnected(sli_cpc_instance_t *inst, uint8_t endpoint_id);

/***************************************************************************//**
 * Notify the user that an endpoint on the host has closed.
 ******************************************************************************/
void sli_cpc_instance_on_remote_ep_terminated(sli_cpc_instance_t *inst,
                                              uint8_t endpoint_id,
                                              sl_cpc_endpoint_state_t *reply_state);

/***************************************************************************//**
 * Called on re-transmition of frame.
 ******************************************************************************/
void sli_cpc_on_frame_retransmit(sl_cpc_buffer_handle_t* frame);

/***************************************************************************//**
 * Called on expiration of retransmit timer
 ******************************************************************************/
void sli_cpc_on_retransmit_timer_expiration(sl_cpc_buffer_handle_t *frame);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************//**
 * Notify the core that the remote sent an unsolicited opening notification.
 ******************************************************************************/
void sli_cpc_remote_open_unsolicited(uint8_t id);

/***************************************************************************//**
 * Notify the core that the remote sent an unsolicited shutdown request.
 ******************************************************************************/
void sli_cpc_remote_shutdown_unsolicited(uint8_t id);
#endif

/***************************************************************************//**
 * Called when the secondary has reset.
 ******************************************************************************/
void sli_cpc_reset(sli_cpc_instance_t *inst);

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
/***************************************************************************//**
 * Set the security counters for a given endpoint
 ******************************************************************************/
sl_status_t sli_cpc_set_security_counters(uint8_t endpoint_id, uint32_t primary_rx_counter, uint32_t primary_tx_counter);
#endif

/***************************************************************************//**
 * Set the maximum payload length that the remote can receive
 ******************************************************************************/
void sli_cpc_set_remote_tx_max_payload_length(sli_cpc_instance_t *inst, uint16_t remote_tx_max_payload_length);
// -----------------------------------------------------------------------------
// Driver to core notifications

/***************************************************************************//**
 * Notifies core of rx completion.
 ******************************************************************************/
void sli_cpc_notify_rx_data_from_drv(sli_cpc_instance_t *inst);

/***************************************************************************//**
 * Notifies core of tx completion by the driver.
 *
 * @param buffer_handle Pointer to the buffer handle that was transmitted.
 ******************************************************************************/
void sli_cpc_notify_tx_data_by_drv(sl_cpc_buffer_handle_t *buffer_handle);

#if defined(SL_CATALOG_KERNEL_PRESENT)
/***************************************************************************//**
 * Notifies core of a freed transmit item
 ******************************************************************************/
void sli_cpc_notify_tx_item_freed(void);

/***************************************************************************//**
 * Notifies core of a freed buffer handle
 ******************************************************************************/
void sli_cpc_notify_buffer_handle_freed(void);

/***************************************************************************//**
 * Main task in charge of handling an instance.
 ******************************************************************************/
void sl_cpc_task(void *arg);

#endif

/** @} (end addtogroup cpc) */

#ifdef __cplusplus
}
#endif

#endif // SLI_CPC_H
