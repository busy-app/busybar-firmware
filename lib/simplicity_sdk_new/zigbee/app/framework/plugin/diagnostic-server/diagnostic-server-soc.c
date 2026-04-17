/***************************************************************************//**
 * @file
 * @brief Routines for the Diagnostic Server plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app/framework/plugin/counters/counters.h"
#include "diagnostic-server.h"
#include "app/framework/util/attribute-storage.h"
#include "app/util/common/common.h"
#include "stack/config/sl_zigbee_token_defines.h"

bool sl_zigbee_af_read_diagnostic_attribute(
  sl_zigbee_af_attribute_metadata_t *attributeMetadata,
  uint8_t *buffer)
{
  uint8_t sl_zigbee_counter = SL_ZIGBEE_COUNTER_TYPE_COUNT;
  sl_status_t status;

  switch (attributeMetadata->attributeId) {
    case ZCL_MAC_RX_BCAST_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_MAC_RX_BROADCAST;
      break;
    case ZCL_MAC_TX_BCAST_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_MAC_TX_BROADCAST;
      break;
    case ZCL_MAC_RX_UCAST_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_MAC_RX_UNICAST;
      break;
    case ZCL_MAC_TX_UCAST_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_SUCCESS;
      break;
    case ZCL_MAC_TX_UCAST_RETRY_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_RETRY;
      break;
    case ZCL_MAC_TX_UCAST_FAIL_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_FAILED;
      break;
    case ZCL_APS_RX_BCAST_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_DATA_RX_BROADCAST;
      break;
    case ZCL_APS_TX_BCAST_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_DATA_TX_BROADCAST;
      break;
    case ZCL_APS_RX_UCAST_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_DATA_RX_UNICAST;
      break;
    case ZCL_APS_UCAST_SUCCESS_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_SUCCESS;
      break;
    case ZCL_APS_TX_UCAST_RETRY_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY;
      break;
    case ZCL_APS_TX_UCAST_FAIL_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_FAILED;
      break;
    case ZCL_ROUTE_DISC_INITIATED_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_ROUTE_DISCOVERY_INITIATED;
      break;
    case ZCL_NEIGHBOR_ADDED_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_NEIGHBOR_ADDED;
      break;
    case ZCL_NEIGHBOR_REMOVED_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_NEIGHBOR_REMOVED;
      break;
    case ZCL_NEIGHBOR_STALE_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_NEIGHBOR_STALE;
      break;
    case ZCL_JOIN_INDICATION_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_JOIN_INDICATION;
      break;
    case ZCL_CHILD_MOVED_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_CHILD_REMOVED;
      break;
    case ZCL_NWK_FC_FAILURE_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_NWK_FRAME_COUNTER_FAILURE;
      break;
    case ZCL_APS_FC_FAILURE_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_FRAME_COUNTER_FAILURE;
      break;
    case ZCL_APS_UNAUTHORIZED_KEY_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_LINK_KEY_NOT_AUTHORIZED;
      break;
    case ZCL_NWK_DECRYPT_FAILURE_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_NWK_DECRYPTION_FAILURE;
      break;
    case ZCL_APS_DECRYPT_FAILURE_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_APS_DECRYPTION_FAILURE;
      break;
    case ZCL_PACKET_BUFFER_ALLOC_FAILURES_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_ALLOCATE_PACKET_BUFFER_FAILURE;
      break;
    case ZCL_RELAYED_UNICAST_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_RELAYED_UNICAST;
      break;
    case ZCL_PHY_TO_MAC_QUEUE_LIMIT_REACHED_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_PHY_TO_MAC_QUEUE_LIMIT_REACHED;
      break;
    case ZCL_PACKET_VALIDATE_DROP_COUNT_ATTRIBUTE_ID:
      sl_zigbee_counter = SL_ZIGBEE_COUNTER_PACKET_VALIDATE_LIBRARY_DROPPED_COUNT;
      break;
    default:
      break;
  }
  if (sl_zigbee_counter < SL_ZIGBEE_COUNTER_TYPE_COUNT) {
    if (attributeMetadata->size == 2) {
      sl_util_store_low_high_int16u(buffer, sl_zigbee_counters[sl_zigbee_counter]);
    } else if (attributeMetadata->size == 4) {
      sl_util_store_low_high_int32u(buffer, sl_zigbee_counters[sl_zigbee_counter]);
    }
    return true;
  }
  // code for handling diagnostic attributes that need to be computed
  switch (attributeMetadata->attributeId) {
    case ZCL_NUMBER_OF_RESETS_ATTRIBUTE_ID:
    {
      tokTypeStackBootCounter rebootCounter;

      uint16_t rebootCounter16;

      (void)sl_token_manager_get_data(COMMON_TOKEN_STACK_BOOT_COUNTER, (void *)&rebootCounter, sizeof(tokTypeStackBootCounter));

      // The token is a uint32, but the attribute is a uint16
      rebootCounter16 = (uint16_t) rebootCounter;

      sl_util_store_low_high_int16u(buffer, rebootCounter16);

      return true;
    }
    break;
    case ZCL_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE_ID:
    {
      uint32_t scratch;
      uint16_t macRetriesPerAps;

      scratch = sl_zigbee_counters[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_SUCCESS]
                + sl_zigbee_counters[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_FAILED];
      if (scratch > 0) {
        scratch = sl_zigbee_counters[SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_RETRY]
                  / scratch;
      }

      macRetriesPerAps = (uint16_t) scratch;
      sl_util_store_low_high_int16u(buffer, macRetriesPerAps);
      return true;
    }
    break;
    case ZCL_LAST_MESSAGE_LQI_ATTRIBUTE_ID:
      status = sli_zigbee_af_get_last_hop_lqi(buffer);
      assert(status == SL_STATUS_OK);
      return true;
      break;
    case ZCL_LAST_MESSAGE_RSSI_ATTRIBUTE_ID:
    {
      int8_t rssi;
      status = sli_zigbee_af_get_last_hop_rssi(&rssi);
      assert(status == SL_STATUS_OK);
      buffer[0] = (uint8_t) rssi;
      return true;
    }
    break;
    default:
      break;
  }

  return false;
}

/** @brief External Attribute Read
 *
 * Like sl_zigbee_af_external_attribute_write_cb above, this function is called
 * when the framework needs to read an attribute that is not stored within the
 * Application Framework's data structures.
        All of the important
 * information about the attribute itself is passed as a pointer to an
 * sl_zigbee_af_attribute_metadata_t struct, which is stored within the application and
 * used to manage the attribute. A complete description of the
 * sl_zigbee_af_attribute_metadata_t struct is provided in
 * app/framework/include/af-types.h
        This function assumes that the
 * application is able to read the attribute, write it into the passed buffer,
 * and return immediately. Any attributes that require a state machine for
 * reading and writing are not really candidates for externalization at the
 * present time. The Application Framework does not currently include a state
 * machine for reading or writing attributes that must take place across a
 * series of application ticks. Attributes that cannot be read in a timely
 * manner should be stored within the Application Framework and updated
 * occasionally by the application code from within the
 * sl_zigbee_af_main_tick_cb.
        If the application was successfully able
 * to read the attribute and write it into the passed buffer, it should return
 * a value of SL_ZIGBEE_ZCL_STATUS_SUCCESS. Any other return value indicates the
 * application was not able to read the attribute.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeMetadata   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param buffer   Ver.: always
 */
sl_zigbee_af_status_t sl_zigbee_af_stack_diagnostic_attribute_read_cb(uint8_t endpoint,
                                                                      sl_zigbee_af_cluster_id_t clusterId,
                                                                      sl_zigbee_af_attribute_metadata_t * attributeMetadata,
                                                                      uint16_t manufacturerCode,
                                                                      uint8_t * buffer)
{
  UNUSED_VAR(endpoint);
  UNUSED_VAR(clusterId);
  UNUSED_VAR(manufacturerCode);
  if (sl_zigbee_af_read_diagnostic_attribute(attributeMetadata, buffer)) {
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  } else {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }
}

bool sl_zigbee_af_pre_message_received_cb(sl_zigbee_af_incoming_message_t* incomingMessage)
{
#if defined (ZCL_USING_DIAGNOSTICS_CLUSTER_LAST_MESSAGE_LQI_ATTRIBUTE)   \
  || defined (ZCL_USING_DIAGNOSTICS_CLUSTER_LAST_MESSAGE_RSSI_ATTRIBUTE) \
  || defined (ZCL_USING_DIAGNOSTICS_CLUSTER_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE)

  sl_zigbee_af_status_t afStatus;
  bool foundValidEndpoint = true;
  uint8_t endpoint = incomingMessage->apsFrame->destinationEndpoint;

  if (sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_DIAGNOSTICS_CLUSTER_ID) == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    foundValidEndpoint = false;
    for (uint8_t i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
      uint8_t ep = sl_zigbee_af_endpoint_from_index(i);
      if (ep != endpoint && sl_zigbee_af_find_cluster_server_endpoint_index(ep, ZCL_DIAGNOSTICS_CLUSTER_ID) != SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
        foundValidEndpoint = true;
        endpoint = ep;
        break;
      }
    }
  }

  if (!foundValidEndpoint) {
    return false;
  }

  // Write values to the framework's implemented optional cluster attributes.
#if defined (ZCL_USING_DIAGNOSTICS_CLUSTER_LAST_MESSAGE_LQI_ATTRIBUTE)
  {
    uint8_t lqi;
    sl_status_t status = sli_zigbee_af_get_last_hop_lqi(&lqi);
    if (SL_STATUS_OK == status) {
      afStatus = sl_zigbee_af_write_server_attribute(
        endpoint,
        ZCL_DIAGNOSTICS_CLUSTER_ID,
        ZCL_LAST_MESSAGE_LQI_ATTRIBUTE_ID,
        &lqi,
        ZCL_INT8U_ATTRIBUTE_TYPE);

      if (SL_ZIGBEE_ZCL_STATUS_SUCCESS != afStatus) {
        sl_zigbee_af_core_println("Diagnostic Server (SOC): failed to write value 0x%02X "
                                  " to cluster 0x%04X attribute ID 0x%04X: error 0x%02X",
                                  lqi,
                                  ZCL_DIAGNOSTICS_CLUSTER_ID,
                                  ZCL_LAST_MESSAGE_LQI_ATTRIBUTE_ID,
                                  afStatus);
      }
    } else {
      sl_zigbee_af_core_println("Diagnostic Server (SOC): failed to get last hop LQI: "
                                "error 0x%02X", status);
    }
  }
#endif // ZCL_USING_DIAGNOSTICS_CLUSTER_LAST_MESSAGE_LQI_ATTRIBUTE

#if defined (ZCL_USING_DIAGNOSTICS_CLUSTER_LAST_MESSAGE_RSSI_ATTRIBUTE)
  {
    int8_t rssi;
    sl_status_t status = sli_zigbee_af_get_last_hop_rssi(&rssi);
    if (SL_STATUS_OK == status) {
      afStatus = sl_zigbee_af_write_server_attribute(
        endpoint,
        ZCL_DIAGNOSTICS_CLUSTER_ID,
        ZCL_LAST_MESSAGE_RSSI_ATTRIBUTE_ID,
        (uint8_t *) &rssi,
        ZCL_INT8S_ATTRIBUTE_TYPE);

      if (SL_ZIGBEE_ZCL_STATUS_SUCCESS != afStatus) {
        sl_zigbee_af_core_println("Diagnostic Server (SOC): failed to write value 0x%02X "
                                  " to cluster 0x%04X attribute ID 0x%04X: error 0x%02X",
                                  (uint8_t)rssi,
                                  ZCL_DIAGNOSTICS_CLUSTER_ID,
                                  ZCL_LAST_MESSAGE_RSSI_ATTRIBUTE_ID,
                                  afStatus);
      }
    } else {
      sl_zigbee_af_core_println("Diagnostic Server (SOC): failed to get last hop RSSI: "
                                "error 0x%02X", status);
    }
  }
#endif // ZCL_USING_DIAGNOSTICS_CLUSTER_LAST_MESSAGE_RSSI_ATTRIBUTE

#if defined (ZCL_USING_DIAGNOSTICS_CLUSTER_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE)
  {
    uint32_t scratch;

    scratch = sl_zigbee_counters[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_SUCCESS]
              + sl_zigbee_counters[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_FAILED];
    if (scratch > 0) {
      scratch = sl_zigbee_counters[SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_RETRY]
                / scratch;
    }

    uint16_t macRetriesPerAps = (uint16_t)scratch;
    afStatus = sl_zigbee_af_write_server_attribute(
      endpoint,
      ZCL_DIAGNOSTICS_CLUSTER_ID,
      ZCL_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE_ID,
      (uint8_t *) &macRetriesPerAps,
      ZCL_INT16U_ATTRIBUTE_TYPE);

    if (SL_ZIGBEE_ZCL_STATUS_SUCCESS != afStatus) {
      sl_zigbee_af_core_println("Diagnostic Server (SOC): failed to write value 0x%04X "
                                " to cluster 0x%04X attribute ID 0x%04X: error 0x%02X",
                                macRetriesPerAps,
                                ZCL_DIAGNOSTICS_CLUSTER_ID,
                                ZCL_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE_ID,
                                afStatus);
    }
  }
#endif // ZCL_USING_DIAGNOSTICS_CLUSTER_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE
#else
  UNUSED_VAR(incomingMessage);
#endif // all

  return false;
}
