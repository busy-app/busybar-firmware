/***************************************************************************//**
 * @brief API that provides services common to all protocol stacks that use BGAPI
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/


#ifndef SL_BGAPI_SERVICE_API_H
#define SL_BGAPI_SERVICE_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_bgapi.h"


/**
 * @addtogroup sl_bgapi_system BGAPI system services
 * @{
 *
 * @brief BGAPI system services
 *
 * The commands of this class provide system-level services to query information
 * of the BGAPI Protocol.
 */

/* Command and Response IDs */
#define sl_bgapi_cmd_system_get_max_payload_sizes_id                  0x00010030
#define sl_bgapi_rsp_system_get_max_payload_sizes_id                  0x00010030

/***************************************************************************//**
 *
 * Get the maximum payload sizes that BGAPI Protocol is configured to support.
 *
 * @param[out] max_command_payload Unsigned 32-bit integer
 * @param[out] max_response_payload Unsigned 32-bit integer
 * @param[out] max_event_payload Unsigned 32-bit integer
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 *
 ******************************************************************************/
sl_status_t sl_bgapi_system_get_max_payload_sizes(uint32_t *max_command_payload,
                                                  uint32_t *max_response_payload,
                                                  uint32_t *max_event_payload);

/** @} */ // end addtogroup sl_bgapi_system

/**
 * @addtogroup sl_bgapi_trace BGAPI tracing
 * @{
 *
 * @brief BGAPI tracing
 *
 * The events in this class are used by the BGAPI trace tool to record extra
 * metadata in the BGAPI traces. These events are never delivered to the
 * application.
 */

/**
 * @brief Specifies the full type of a BGAPI message.
 */
typedef enum
{
  sl_bgapi_trace_message_type_command  = 0x0, /**< (0x0) The message is a BGAPI
                                                   command */
  sl_bgapi_trace_message_type_response = 0x1, /**< (0x1) The message is a BGAPI
                                                   response */
  sl_bgapi_trace_message_type_event    = 0x2  /**< (0x2) The message is a BGAPI
                                                   event */
} sl_bgapi_trace_message_type_t;

/**
 * @addtogroup sl_bgapi_evt_trace_message_metadata sl_bgapi_evt_trace_message_metadata
 * @{
 * @brief This event is emitted by the BGAPI Trace component to the RTT trace
 * whenever a BGAPI message is output to the trace
 *
 * The message metadata event specifies the full type and the timestamp of the
 * BGAPI message that follows the metadata event.
 */

/** @brief Identifier of the message_metadata event */
#define sl_bgapi_evt_trace_message_metadata_id                        0x000000b0

/***************************************************************************//**
 * @brief Data structure of the message_metadata event
 ******************************************************************************/
PACKSTRUCT( struct sl_bgapi_evt_trace_message_metadata_s
{
  uint8_t  type;         /**< Enum @ref sl_bgapi_trace_message_type_t. The type
                              of the message that follows. Values:
                                - <b>sl_bgapi_trace_message_type_command
                                  (0x0):</b> The message is a BGAPI command
                                - <b>sl_bgapi_trace_message_type_response
                                  (0x1):</b> The message is a BGAPI response
                                - <b>sl_bgapi_trace_message_type_event
                                  (0x2):</b> The message is a BGAPI event */
  uint64_t timestamp_us; /**< Timestamp of the message that follows. The value
                              is a monotonically increasing number of
                              microseconds that have elapsed after the device
                              booted up. */
});

typedef struct sl_bgapi_evt_trace_message_metadata_s sl_bgapi_evt_trace_message_metadata_t;

/** @} */ // end addtogroup sl_bgapi_evt_trace_message_metadata

/**
 * @addtogroup sl_bgapi_evt_trace_custom_message sl_bgapi_evt_trace_custom_message
 * @{
 * @brief This event is emitted when a component or application has called
 * sli_bgapi_trace_log_custom_message to output a custom log message
 */

/** @brief Identifier of the custom_message event */
#define sl_bgapi_evt_trace_custom_message_id                          0x010000b0

/***************************************************************************//**
 * @brief Data structure of the custom_message event
 ******************************************************************************/
PACKSTRUCT( struct sl_bgapi_evt_trace_custom_message_s
{
  uint64_t   timestamp_us; /**< Timestamp of the custom log message. The value
                                is a monotonically increasing number of
                                microseconds that have elapsed after the device
                                booted up. */
  uint8array message;      /**< The custom log message. This is typically a
                                human-readable string, but may also contain
                                binary data. */
});

typedef struct sl_bgapi_evt_trace_custom_message_s sl_bgapi_evt_trace_custom_message_t;

/** @} */ // end addtogroup sl_bgapi_evt_trace_custom_message

/**
 * @addtogroup sl_bgapi_evt_trace_sync sl_bgapi_evt_trace_sync
 * @{
 * @brief This event is emitted by the sli_bgapi_trace_sync API in order to
 * synchronize with the host
 */

/** @brief Identifier of the sync event */
#define sl_bgapi_evt_trace_sync_id                                    0x020000b0

/***************************************************************************//**
 * @brief Data structure of the sync event
 ******************************************************************************/
PACKSTRUCT( struct sl_bgapi_evt_trace_sync_s
{
  uint64_t timestamp_us; /**< Timestamp of the sync message. The value is a
                              monotonically increasing number of microseconds
                              that have elapsed after the device booted up. */
});

typedef struct sl_bgapi_evt_trace_sync_s sl_bgapi_evt_trace_sync_t;

/** @} */ // end addtogroup sl_bgapi_evt_trace_sync

/** @} */ // end addtogroup sl_bgapi_trace

/***************************************************************************//**
 * @addtogroup sl_bgapi_common_types BGAPI_SERVICE Common Types
 * @{
 *  @brief Common types of `bgapi_service` API
 */

/**
 * @brief Data structure of `bgapi_service` API messages
 */
PACKSTRUCT( struct sl_bgapi_msg {
  /** API protocol header consisting of event identifier and data length */
  uint32_t   header;

  /** Union of API event types */
  union {
    uint8_t handle;
    sl_bgapi_evt_trace_message_metadata_t                        evt_trace_message_metadata; /**< Data field for event sl_bgapi_evt_trace_message_metadata_id */
    sl_bgapi_evt_trace_custom_message_t                          evt_trace_custom_message; /**< Data field for event sl_bgapi_evt_trace_custom_message_id */
    sl_bgapi_evt_trace_sync_t                                    evt_trace_sync; /**< Data field for event sl_bgapi_evt_trace_sync_id */
    uint8_t payload[SL_BGAPI_MAX_PAYLOAD_SIZE];
  } data;
});

/**
 * @brief Type definition for the data structure of `bgapi_service` API messages
 */
typedef struct sl_bgapi_msg sl_bgapi_msg_t;

/** @} */ // end addtogroup sl_bgapi_common_types
/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // SL_BGAPI_SERVICE_API_H