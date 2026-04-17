/***************************************************************************//**
 * @file
 * @brief Zigbee Dynamic Node Type API
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

#include "app/framework/include/af.h"
#include "dynamic-node-type.h"
#include "stack/include/zigbee-dynamic-node-type.h"

extern sli_zigbee_af_zigbee_pro_network *sli_zigbee_af_current_zigbee_pro_network;

sl_status_t sl_zigbee_af_switch_role_sleepy_end_device(uint8_t end_device_configuration)
{
  (void)end_device_configuration;
  sli_zigbee_af_current_zigbee_pro_network->nodeType = SL_ZIGBEE_SLEEPY_END_DEVICE;
  slx_zigbee_switch_role_sleepy_end_device(end_device_configuration);
  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_af_switch_role_router(void)
{
  sli_zigbee_af_current_zigbee_pro_network->nodeType = SL_ZIGBEE_ROUTER;
  slx_zigbee_switch_role_router();
  return SL_STATUS_OK;
}
