/***************************************************************************//**
 * @file zigbee-dynamic-node-type-baremetal-wrapper.c
 * @brief internal implementations for 'zigbee-dynamic-node-type' as a thin-wrapper
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
// automatically generated from zigbee-dynamic-node-type.h.  Do not manually edit
#include "stack/include/zigbee-dynamic-node-type.h"
#include "stack/internal/inc/zigbee-dynamic-node-type-internal-def.h"

sl_status_t slx_zigbee_switch_role_router(void)
{
  return slxi_zigbee_stack_switch_role_router();
}

sl_status_t slx_zigbee_switch_role_sleepy_end_device(uint8_t end_device_configuration)
{
  return slxi_zigbee_stack_switch_role_sleepy_end_device(end_device_configuration);
}
