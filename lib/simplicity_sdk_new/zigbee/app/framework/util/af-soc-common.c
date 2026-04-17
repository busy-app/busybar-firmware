/***************************************************************************//**
 * @file af-soc-common.c
 * @brief common APIs used for both SoC and NCP application when af support
 * component is enabled on NCP.
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

#include "app/framework/include/af.h"
#include "af-main.h"
#include "app/framework/util/attribute-storage.h"

//------------------------------------------------------------------------------
// must return the endpoint desc of the endpoint specified
bool sli_zigbee_af_get_endpoint_description(uint8_t endpoint,
                                            sl_zigbee_endpoint_description_t *result)
{
  if (sl_zigbee_af_get_endpoint_description_cb(endpoint, result)) {
    return true;
  }
  uint8_t endpointIndex = sl_zigbee_af_index_from_endpoint(endpoint);
  if (endpointIndex == 0xFF
#if MAX_ENDPOINT_COUNT > 0
      || (endpointIndex >= MAX_ENDPOINT_COUNT)
#endif
      || (sl_zigbee_af_network_index_from_endpoint_index(endpointIndex)
          != sl_zigbee_get_callback_network())) {
    return false;
  }
  result->profileId          = sl_zigbee_af_profile_id_from_index(endpointIndex);
  result->deviceId           = sl_zigbee_af_device_id_from_index(endpointIndex);
  result->deviceVersion      = sl_zigbee_af_device_version_from_index(endpointIndex);
  result->inputClusterCount  = sl_zigbee_af_cluster_count(endpoint, true);
  result->outputClusterCount = sl_zigbee_af_cluster_count(endpoint, false);
  return true;
}

// must return the clusterId at listIndex in the list specified for the
// endpoint specified
uint16_t sli_zigbee_af_get_endpoint_cluster(uint8_t endpoint,
                                            sl_zigbee_cluster_list_id_t listId,
                                            uint8_t listIndex)
{
  sl_zigbee_af_cluster_t *cluster = NULL;
  uint8_t endpointIndex = sl_zigbee_af_index_from_endpoint(endpoint);
  if (endpointIndex == 0xFF
#if MAX_ENDPOINT_COUNT > 0
      || (endpointIndex >= MAX_ENDPOINT_COUNT)
#endif
      || (sl_zigbee_af_network_index_from_endpoint_index(endpointIndex)
          != sl_zigbee_get_callback_network())) {
    return 0xFFFF;
  } else if (listId == SL_ZIGBEE_INPUT_CLUSTER_LIST) {
    cluster = sl_zigbee_af_get_nth_cluster(endpoint, listIndex, true);
  } else if (listId == SL_ZIGBEE_OUTPUT_CLUSTER_LIST) {
    cluster = sl_zigbee_af_get_nth_cluster(endpoint, listIndex, false);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
  return (cluster == NULL ? 0xFFFF : cluster->clusterId);
}

uint8_t sli_zigbee_af_get_endpoint(uint8_t index)
{
  uint8_t endpoint = SL_ZIGBEE_AF_INVALID_ENDPOINT;
  if (sl_zigbee_af_get_endpoint_by_index_cb(index, &endpoint)) {
    return endpoint;
  }
  return (((sl_zigbee_af_network_index_from_endpoint_index(index)
            == sl_zigbee_get_callback_network())
           && sl_zigbee_af_endpoint_index_is_enabled(index))
          ? sl_zigbee_af_endpoint_from_index(index)
          : 0xFF);
}

//------------------------------------------------------------------------------
// Internal APIs

sl_status_t sli_zigbee_af_send(sl_zigbee_outgoing_message_type_t type,
                               uint16_t indexOrDestination,
                               sl_zigbee_aps_frame_t *apsFrame,
                               uint8_t messageLength,
                               uint8_t *message,
                               uint16_t *messageTag,
                               sl_802154_short_addr_t alias,
                               uint8_t sequence)
{
  sl_status_t status;

  *messageTag = sli_zigbee_af_calculate_message_tag_hash(message, messageLength);
  uint8_t nwkRadius = ZA_MAX_HOPS;
  sl_802154_short_addr_t nwkAlias = SL_ZIGBEE_NULL_NODE_ID;

  switch (type) {
    case SL_ZIGBEE_OUTGOING_DIRECT:
    case SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE:
    case SL_ZIGBEE_OUTGOING_VIA_BINDING:
      status = sl_zigbee_send_unicast(type,
                                      indexOrDestination,
                                      apsFrame,
                                      *messageTag,
                                      messageLength,
                                      message,
                                      NULL);
      break;
    case SL_ZIGBEE_OUTGOING_MULTICAST:
    case SL_ZIGBEE_OUTGOING_MULTICAST_WITH_ALIAS:
    {
      if (type == SL_ZIGBEE_OUTGOING_MULTICAST_WITH_ALIAS
          || (apsFrame->sourceEndpoint == SL_ZIGBEE_GP_ENDPOINT
              && apsFrame->destinationEndpoint == SL_ZIGBEE_GP_ENDPOINT
              && apsFrame->options & SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER)) {
        nwkRadius = apsFrame->radius;
        nwkAlias = alias;
      }
      status = sl_zigbee_send_multicast(apsFrame,
                                        nwkRadius,      //radius
                                        0,      // broadcast Addr
                                        nwkAlias,
                                        sequence,
                                        *messageTag,
                                        messageLength,
                                        message,
                                        NULL);
      break;
    }
    case SL_ZIGBEE_OUTGOING_BROADCAST:
    case SL_ZIGBEE_OUTGOING_BROADCAST_WITH_ALIAS:
    {
      if (type == SL_ZIGBEE_OUTGOING_BROADCAST_WITH_ALIAS
          || (apsFrame->sourceEndpoint == SL_ZIGBEE_GP_ENDPOINT
              && apsFrame->destinationEndpoint == SL_ZIGBEE_GP_ENDPOINT
              && apsFrame->options & SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER)) {
        nwkRadius = apsFrame->radius;
        nwkAlias = alias;
      }
      status = sl_zigbee_send_broadcast(nwkAlias,
                                        indexOrDestination,
                                        sequence,
                                        apsFrame,
                                        nwkRadius, // radius
                                        (type == SL_ZIGBEE_OUTGOING_BROADCAST) ? *messageTag : 0, // tag
                                        messageLength,
                                        message,
                                        NULL);
      break;
    }
    default:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }

  return status;
}

#define INVALID_MESSAGE_TAG 0x0000
uint16_t sli_zigbee_af_calculate_message_tag_hash(uint8_t *messageContents,
                                                  uint8_t messageLength)
{
  uint8_t temp[SL_ZIGBEE_ENCRYPTION_KEY_SIZE];
  uint16_t hashReturn = 0;
  sl_zigbee_aes_hash_simple(messageLength, messageContents, temp);
  for (uint8_t i = 0; i < SL_ZIGBEE_ENCRYPTION_KEY_SIZE; i += 2) {
    hashReturn ^= *((uint16_t *)(temp + i));
  }
  if (hashReturn == INVALID_MESSAGE_TAG) {
    hashReturn = 1;
  }
  return hashReturn;
}
