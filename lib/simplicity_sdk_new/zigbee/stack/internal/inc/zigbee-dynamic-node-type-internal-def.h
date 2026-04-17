/***************************************************************************//**
 * @file zigbee-dynamic-node-type-internal-def.h
 * @brief internal names for 'zigbee-dynamic-node-type' declarations
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
#ifndef ZIGBEE_DYNAMIC_NODE_TYPE_INTERNAL_DEF_H
#define ZIGBEE_DYNAMIC_NODE_TYPE_INTERNAL_DEF_H

#include "stack/include/zigbee-dynamic-node-type.h"

// Command Indirection

sl_status_t slxi_zigbee_stack_switch_role_router(void);

sl_status_t slxi_zigbee_stack_switch_role_sleepy_end_device(uint8_t end_device_configuration);

#endif // ZIGBEE_DYNAMIC_NODE_TYPE_INTERNAL_DEF_H
