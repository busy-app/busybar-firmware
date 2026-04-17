/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Zigbee Dynamic Node Type plugin.
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

#ifndef SILABS_DYNAMIC_NODE_TYPE_STACK_H
#define SILABS_DYNAMIC_NODE_TYPE_STACK_H

#include "sl_status.h"
#include "stack/include/sl_zigbee_types.h"

sl_status_t sli_zigbee_stack_switch_role_sleepy_end_device(uint8_t end_device_configuration);
sl_status_t sli_zigbee_stack_switch_role_router(void);

#endif
