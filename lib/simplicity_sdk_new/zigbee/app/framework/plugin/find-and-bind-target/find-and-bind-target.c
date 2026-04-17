/***************************************************************************//**
 * @file
 * @brief Routines for the Find and Bind Target plugin.
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_FIND_AND_BIND_TARGET_PRESENT
#include "find-and-bind-target-config.h"
#else
#include "config/find-and-bind-target-config.h"
#endif

#include "find-and-bind-target.h"

#ifdef SL_ZIGBEE_SCRIPTED_TEST
  #include "../find-and-bind-initiator/find-and-bind-test-configuration.h"
#endif

// -----------------------------------------------------------------------------
// Public API

sl_zigbee_af_status_t sl_zigbee_af_find_and_bind_target_start(uint8_t endpoint)
{
  // Write the identify time.
  uint16_t identifyTime = SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;

  if (sl_zigbee_af_contains_server(endpoint, ZCL_IDENTIFY_CLUSTER_ID)) {
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_IDENTIFY_CLUSTER_ID,
                                                 ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                                 (uint8_t *)&identifyTime,
                                                 ZCL_INT16U_ATTRIBUTE_TYPE);
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_TARGET_PLUGIN_NAME,
                            "Start target",
                            status);

  return status;
}
