/***************************************************************************//**
 * @brief SL_BGAPI_SERVICE_API commands for NCP host
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_bgapi_service_api.h"
#include "sli_bgapi_service_api.h"

void sl_bgapi_host_handle_command();
void sl_bgapi_host_handle_command_noresponse();
extern sl_bgapi_msg_t *sl_bgapi_cmd_msg;
extern sl_bgapi_msg_t *sl_bgapi_rsp_msg;

sl_status_t sl_bgapi_system_get_max_payload_sizes(uint32_t *max_command_payload,
                                                  uint32_t *max_response_payload,
                                                  uint32_t *max_event_payload)
{
    struct sl_bgapi_packet *cmd = (struct sl_bgapi_packet *)sl_bgapi_cmd_msg;
    struct sl_bgapi_packet *rsp = (struct sl_bgapi_packet *)sl_bgapi_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bgapi_system_class_id,
                                       sli_bgapi_system_get_max_payload_sizes_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bgapi_service,
                                       cmd_payload_len);
    sl_bgapi_host_handle_command();

    if (max_command_payload) {
        *max_command_payload = rsp->data.rsp_system_get_max_payload_sizes.max_command_payload;
    }
    if (max_response_payload) {
        *max_response_payload = rsp->data.rsp_system_get_max_payload_sizes.max_response_payload;
    }
    if (max_event_payload) {
        *max_event_payload = rsp->data.rsp_system_get_max_payload_sizes.max_event_payload;
    }

    return rsp->data.rsp_system_get_max_payload_sizes.result;
}