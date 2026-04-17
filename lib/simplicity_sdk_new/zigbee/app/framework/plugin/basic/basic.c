/***************************************************************************//**
 * @file
 * @brief Implementation for the Basic Server Cluster plugin.
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
#include "basic.h"
#include "app/framework/util/attribute-storage.h"
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_REPORTING_PRESENT
#include "app/framework/plugin/reporting/reporting.h"
#endif // SL_CATALOG_ZIGBEE_REPORTING_PRESENT
#else // !SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_ZIGBEE_AF_PLUGIN_REPORTING
#include "app/framework/plugin/reporting/reporting.h"
#define SL_CATALOG_ZIGBEE_REPORTING_PRESENT
#endif // SL_ZIGBEE_AF_PLUGIN_REPORTING
#endif // SL_COMPONENT_CATALOG_PRESENT

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_basic_cluster_reset_to_factory_defaults_cb(void)
{
  sl_zigbee_af_basic_cluster_println("RX: ResetToFactoryDefaultsCallback");
  sl_zigbee_af_reset_attributes(sl_zigbee_af_current_endpoint());
  sl_zigbee_af_basic_reset_to_factory_defaults_cb(sl_zigbee_af_current_endpoint());
#ifdef SL_CATALOG_ZIGBEE_REPORTING_PRESENT
  sli_zigbee_af_reporting_get_last_value_all();
#endif // SL_CATALOG_ZIGBEE_REPORTING_PRESENT
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

uint32_t sl_zigbee_af_basic_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                         sl_service_function_context_t *context)
{
  (void)opcode;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  if (!cmd->mfgSpecific && cmd->commandId == ZCL_RESET_TO_FACTORY_DEFAULTS_COMMAND_ID) {
    status = sl_zigbee_af_basic_cluster_reset_to_factory_defaults_cb();
  }

  return status;
}
