/***************************************************************************//**
 * @file
 * @brief Definitions for the Simple Metering Client plugin, which implements
 *        the client side of the Simple Metering cluster.
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

#include "../../include/af.h"
#include "../../util/common.h"
#include "simple-metering-client.h"

#include "simple-metering-client-config.h"
#include "zap-cluster-command-parser.h"

static uint32_t profileIntervals[SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_CLIENT_NUMBER_OF_INTERVALS_SUPPORTED];

uint16_t removeEndpointId = 0xFFFF;

static sl_zigbee_af_zcl_request_status_t clusterRequestCommon(uint8_t responseCommandId)
{
  uint16_t endpointId;
  sl_802154_long_addr_t otaEui;

  if (sl_zigbee_lookup_eui64_by_node_id(sl_zigbee_af_response_destination, otaEui)
      != SL_STATUS_OK) {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  endpointId = (ZCL_REQUEST_MIRROR_RESPONSE_COMMAND_ID == responseCommandId
                ? sl_zigbee_af_simple_metering_client_request_mirror_cb(otaEui)
                : sl_zigbee_af_simple_metering_client_remove_mirror_cb(otaEui));

  if (endpointId == 0xFFFF && ZCL_REQUEST_MIRROR_RESPONSE_COMMAND_ID != responseCommandId) {
    sl_zigbee_af_simple_metering_cluster_println("Invalid endpoint. Sending Default Response");
    return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
  }

  if (responseCommandId == ZCL_MIRROR_REMOVED_COMMAND_ID) {
    removeEndpointId = endpointId;
  }
  (void) sl_zigbee_af_fill_external_buffer(ZCL_CLUSTER_SPECIFIC_COMMAND
                                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                           | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_RESPONSES,
                                           ZCL_SIMPLE_METERING_CLUSTER_ID,
                                           responseCommandId,
                                           "v",
                                           endpointId);
  sl_zigbee_af_send_response();
  if (responseCommandId == ZCL_MIRROR_REMOVED_COMMAND_ID) {
    sl_zigbee_af_endpoint_enable_disable(endpointId, false);
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_request_mirror_cb(void)
{
  sl_zigbee_af_simple_metering_cluster_println("RX: RequestMirror");
  return clusterRequestCommon(ZCL_REQUEST_MIRROR_RESPONSE_COMMAND_ID);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_remove_mirror_cb(void)
{
  sl_zigbee_af_simple_metering_cluster_println("RX: RemoveMirror");
  return clusterRequestCommon(ZCL_MIRROR_REMOVED_COMMAND_ID);
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_profile_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_get_profile_response_command_t cmd_data;
  uint8_t i;

  if (zcl_decode_simple_metering_cluster_get_profile_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_simple_metering_cluster_print("RX: GetProfileResponse 0x%08X, 0x%02X, 0x%02X, 0x%02X",
                                             cmd_data.endTime,
                                             cmd_data.status,
                                             cmd_data.profileIntervalPeriod,
                                             cmd_data.numberOfPeriodsDelivered);
  if (cmd_data.numberOfPeriodsDelivered > SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_CLIENT_NUMBER_OF_INTERVALS_SUPPORTED) {
    cmd_data.numberOfPeriodsDelivered = SL_ZIGBEE_AF_PLUGIN_SIMPLE_METERING_CLIENT_NUMBER_OF_INTERVALS_SUPPORTED;
  }
  for (i = 0; i < cmd_data.numberOfPeriodsDelivered; i++) {
    sl_zigbee_af_simple_metering_cluster_print(" [0x%08X]",
                                               sl_zigbee_af_get_int24u(cmd_data.intervals + i * 3, 0, 3));
    profileIntervals[i] = sl_zigbee_af_get_int24u(cmd_data.intervals + i * 3, 0, 3);
  }
  sl_zigbee_af_simple_metering_cluster_println("");
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_request_fast_poll_mode_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_simple_metering_cluster_request_fast_poll_mode_response_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_request_fast_poll_mode_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_simple_metering_cluster_println("RX: RequestFastPollModeResponse 0x%02X, 0x%08X 0x%08X 0x%08X",
                                               cmd_data.appliedUpdatePeriod,
                                               cmd_data.fastPollModeEndtime,
                                               sl_zigbee_af_get_current_time(),
                                               (cmd_data.fastPollModeEndtime - sl_zigbee_af_get_current_time()));
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_supply_status_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;

  sl_zigbee_af_simple_metering_cluster_println("Supply Status Callback");
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

uint32_t sl_zigbee_af_simple_metering_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                   sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_PROFILE_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_get_profile_response_cb(cmd);
        break;
      }
      case ZCL_REQUEST_MIRROR_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_request_mirror_cb();
        break;
      }
      case ZCL_REMOVE_MIRROR_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_remove_mirror_cb();
        break;
      }
      case ZCL_REQUEST_FAST_POLL_MODE_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_request_fast_poll_mode_response_cb(cmd);
        break;
      }
      case ZCL_SUPPLY_STATUS_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_supply_status_response_cb(cmd);
        break;
      }
    }
  }

  return status;
}
