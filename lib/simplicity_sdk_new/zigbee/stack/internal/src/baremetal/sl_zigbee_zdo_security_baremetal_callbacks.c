/***************************************************************************//**
 * @file sl_zigbee_zdo_security_baremetal_callbacks.c
 * @brief internal dispatch for 'sl_zigbee_zdo_security' callbacks as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from sl_zigbee_zdo_security.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_security.h"
#include "stack/internal/inc/sl_zigbee_zdo_security_internal_def.h"

void sli_zigbee_stack_get_authentication_level_callback(sl_zigbee_zdo_status_t rsp_status,
                                                        sl_802154_long_addr_t target,
                                                        uint8_t join_method,
                                                        uint8_t link_key_update)
{
  sl_zigbee_get_authentication_level_callback(rsp_status,
                                              target,
                                              join_method,
                                              link_key_update);
}

void sli_zigbee_stack_set_authenticaion_level_callback(sl_802154_long_addr_t target,
                                                       sl_zigbee_initial_join_method *initial_join_method,
                                                       sl_zigbee_active_link_key_type *active_link_key_type)
{
  sl_zigbee_set_authenticaion_level_callback(target,
                                             initial_join_method,
                                             active_link_key_type);
}

void sli_zigbee_stack_zdo_retrieve_authentication_token_complete_callback(sl_status_t status)
{
  sl_zigbee_zdo_retrieve_authentication_token_complete_callback(status);
}
