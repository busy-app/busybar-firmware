/***************************************************************************//**
 * @file
 * @brief This file contains a function that processes cluster-specific
 * ZCL message.
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

// this file contains all the common includes for clusters in the zcl-util
#include "app/framework/util/common.h"

// for pulling in defines dealing with EITHER server or client
#include "af-main.h"
#include <stdlib.h>

// the EM260 host needs to include the config file
#ifdef EZSP_HOST
  #include "app/framework/util/config.h"
#endif

//------------------------------------------------------------------------------

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_cluster_specific_command_parse(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zigbee_af_zcl_request_status_t zcl_status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_service_function_entry_t *service_entry = sl_service_function_get_first_entry();
  sl_service_function_context_t service_context;
  service_context.data = (void*)cmd;

  while (service_entry != NULL) {
    // Call the service function if:
    // - the service function is a ZCL_COMMAND service function
    // - the cluster ID matches
    // - if the manufacturer ID is present, the manufacturer ID matches
    // - the command direction matches
    if (service_entry->type == SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND
        && service_entry->key == cmd->apsFrame->clusterId
        && (service_entry->subkey & 0xFFFF) == (cmd->mfgSpecific ? cmd->mfgCode : 0xFFFF)
        && (service_entry->subkey >> 16) == cmd->direction) {
      zcl_status = (service_entry->function)(SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, &service_context);

      // Keep going through the list of the service function as there may be
      // a service function that can handle this cluster command
      if (zcl_status != SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND) {
        break;
      }
    }
    service_entry = sl_service_function_get_next_entry(service_entry);
  }

  return zcl_status;
}

bool sli_zigbee_af_process_cluster_specific_command(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zigbee_af_zcl_request_status_t zcl_status = SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_CLUSTER;

  // if we are disabled then we can only respond to read or write commands
  // or identify cluster (see device enabled attr of basic cluster)
  if (!sl_zigbee_af_is_device_enabled(cmd->apsFrame->destinationEndpoint)
      && cmd->apsFrame->clusterId != ZCL_IDENTIFY_CLUSTER_ID) {
    sl_zigbee_af_core_println("%sd, dropping ep 0x%02X clus 0x%04X cmd 0x%02X",
                              "disable",
                              cmd->apsFrame->destinationEndpoint,
                              cmd->apsFrame->clusterId,
                              cmd->commandId);
    sl_zigbee_af_send_default_response(cmd, SL_ZIGBEE_ZCL_STATUS_FAILURE);
    return true;
  }

  if ((cmd->direction == (uint8_t)ZCL_DIRECTION_SERVER_TO_CLIENT
       && sl_zigbee_af_contains_client_with_mfg_code(cmd->apsFrame->destinationEndpoint,
                                                     cmd->apsFrame->clusterId,
                                                     cmd->mfgCode))
      || (cmd->direction == (uint8_t)ZCL_DIRECTION_CLIENT_TO_SERVER
          && sl_zigbee_af_contains_server_with_mfg_code(cmd->apsFrame->destinationEndpoint,
                                                        cmd->apsFrame->clusterId,
                                                        cmd->mfgCode))) {
    zcl_status = sl_zigbee_af_cluster_specific_command_parse(cmd);
  }

  if (!sli_command_or_default_response_submitted
      && zcl_status != SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED) {
    sl_zigbee_af_send_default_response(cmd, (sl_zigbee_af_status_t)zcl_status);
  }

  return true;
}

sl_status_t sl_zigbee_subscribe_to_zcl_commands(uint16_t cluster_id,
                                                uint16_t manufacturer_id,
                                                uint8_t direction,
                                                sl_service_function_t service_function)
{
  sl_service_function_block_t *block;
  sl_service_function_entry_t *entry;

  if (service_function == NULL
      || (direction != ZCL_DIRECTION_CLIENT_TO_SERVER
          && direction != ZCL_DIRECTION_SERVER_TO_CLIENT)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  block = (sl_service_function_block_t*)malloc(sizeof(sl_service_function_block_t));

  if (block == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  entry = (sl_service_function_entry_t*)malloc(sizeof(sl_service_function_entry_t));

  if (entry == NULL) {
    free(block);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  block->count = 1;
  block->entries = entry;

  entry->type = SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND;
  entry->function = service_function;
  entry->key = cluster_id;
  // Set the manufacturer ID in the 2 least significant bytes of the subkey.
  entry->subkey = manufacturer_id;

  // Write the direction in the 2 most significant bytes of the subkey
  entry->subkey |= (direction << 16);

  sl_service_function_register_block(block);

  return SL_STATUS_OK;
}
