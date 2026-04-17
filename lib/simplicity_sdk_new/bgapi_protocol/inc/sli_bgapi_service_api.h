/***************************************************************************//**
 * @brief BGAPI Service internal types and functions
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


#ifndef SLI_BGAPI_SERVICE_API_H
#define SLI_BGAPI_SERVICE_API_H

#include "sl_bgapi_service_api.h"
#include "sli_bgapi.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Silicon Labs internal functions for BGAPI Service initialization

/**
 * @brief Register the BGAPI Service device.
 *
 * This function is used internally by BGAPI Service initialization. Do not call
 * this function directly from the application.
 *
 * @param[in] device_info Information about the BGAPI device
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 */
sl_status_t sli_bgapi_service_register_bgapi_device(const sli_bgapi_device_info_t *device_info);

// -----------------------------------------------------------------------------
// Class and command IDs for `bgapi_service` API

enum sli_bgapi_class_id
{
    sli_bgapi_system_class_id = 0x01,
    sli_bgapi_trace_class_id = 0x00,
};

enum sli_bgapi_command_id
{
    sli_bgapi_system_get_max_payload_sizes_command_id = 0x00,
};

enum sli_bgapi_response_id
{
    sli_bgapi_system_get_max_payload_sizes_response_id = 0x00,
};

enum sli_bgapi_event_id
{
    sli_bgapi_trace_message_metadata_event_id = 0x00,
    sli_bgapi_trace_custom_message_event_id = 0x01,
    sli_bgapi_trace_sync_event_id = 0x02,
};

// -----------------------------------------------------------------------------
// Command and response structures for `bgapi_service` API

PACKSTRUCT( struct sl_bgapi_rsp_system_get_max_payload_sizes_s
{
    uint16_t result;
    uint32_t max_command_payload;
    uint32_t max_response_payload;
    uint32_t max_event_payload;
});

typedef struct sl_bgapi_rsp_system_get_max_payload_sizes_s sl_bgapi_rsp_system_get_max_payload_sizes_t;

// -----------------------------------------------------------------------------
// BGAPI message union for all message types of `bgapi_service` API

PACKSTRUCT( struct sl_bgapi_packet {
  uint32_t   header;
  union {
    uint8_t handle;
    sl_bgapi_rsp_error_t                                         rsp_error;
    sl_bgapi_rsp_system_get_max_payload_sizes_t                  rsp_system_get_max_payload_sizes;
    sl_bgapi_evt_trace_message_metadata_t                        evt_trace_message_metadata;
    sl_bgapi_evt_trace_custom_message_t                          evt_trace_custom_message;
    sl_bgapi_evt_trace_sync_t                                    evt_trace_sync;
    uint8_t payload[SL_BGAPI_MAX_PAYLOAD_SIZE];
  } data;
});

#ifdef __cplusplus
}
#endif

#endif // SLI_BGAPI_SERVICE_API_H