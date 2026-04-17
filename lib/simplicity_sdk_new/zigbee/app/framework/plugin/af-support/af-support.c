/***************************************************************************//**
 * @file af-support.c
 * @brief Application Framework Support code
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "af-support.h"
#include "app/framework/util/attribute-storage.h"

void sli_zb_af_support_init_callback(uint8_t init_level)
{
  UNUSED_VAR(init_level);
  // configure endpoint data
  sl_zigbee_af_endpoint_configure();
}

bool sl_zigbee_af_pre_message_received_cb(sl_zigbee_af_incoming_message_t* incomingMessage)
{
  // Should only handle ZCL message for GP endpoint
  if (sl_zigbee_af_process_message(incomingMessage->apsFrame,
                                   incomingMessage->type,
                                   incomingMessage->message,
                                   incomingMessage->msgLen,
                                   incomingMessage->source,
                                   NULL)) { // no inter-pan header
  }
  // Always return true so sli_zigbee_af_incoming_message_handler() should not continue to process message.
  return true;
}

bool sli_zb_af_support_incoming_message_callback(sl_zigbee_aps_frame_t *apsFrame)
{
  if (apsFrame->destinationEndpoint == SL_ZIGBEE_GP_ENDPOINT
      || ((apsFrame->destinationEndpoint == SL_ZIGBEE_BROADCAST_ENDPOINT) \
          && (apsFrame->clusterId == ZCL_GREEN_POWER_CLUSTER_ID))) {
    return true;
  }
  return false;
}

extern bool sli_zigbee_find_endpoint_cluster(uint8_t endpoint,
                                             uint16_t requestedClusterId,
                                             sl_zigbee_cluster_list_id_t listId);

bool sl_zigbee_af_contains_server_wrapper(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId)
{
  return sli_zigbee_find_endpoint_cluster(endpoint, clusterId, SL_ZIGBEE_INPUT_CLUSTER_LIST);
}

bool sl_zigbee_af_contains_client_wrapper(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId)
{
  return sli_zigbee_find_endpoint_cluster(endpoint, clusterId, SL_ZIGBEE_OUTPUT_CLUSTER_LIST);
}

extern bool sli_zigbee_get_endpoint_info(uint8_t endpoint,
                                         uint8_t *returnNetworkIndex,
                                         uint16_t *returnProfileId);

bool sl_zigbee_af_get_endpoint_info_cb(uint8_t endpoint,
                                       uint8_t *returnNetworkIndex,
                                       sl_zigbee_af_endpoint_info_struct_t *returnEndpointInfo)
{
  return sli_zigbee_get_endpoint_info(endpoint, returnNetworkIndex, &(returnEndpointInfo->profileId));
}

bool sl_zigbee_af_ncp_d_gp_sent_handler(sl_status_t status, uint8_t gpepHandle)
{
  // Return true then sli_zigbee_af_d_gp_sent_callback will not send EZSP to host
  UNUSED_VAR(status);
  UNUSED_VAR(gpepHandle);
  return true;
}

bool sl_zigbee_af_ncp_gpep_incoming_message_handler(sl_zigbee_gp_params_t *param)
{
  // Return true then sli_zigbee_af_gpep_incoming_message_callback will not send EZSP to host
  UNUSED_VAR(param);
  return true;
}
