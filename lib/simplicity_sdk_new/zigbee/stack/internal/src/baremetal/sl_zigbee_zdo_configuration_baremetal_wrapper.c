/***************************************************************************//**
 * @file sl_zigbee_zdo_configuration_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_zdo_configuration' as a thin-wrapper
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
// automatically generated from sl_zigbee_zdo_configuration.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_configuration.h"
#include "stack/internal/inc/sl_zigbee_zdo_configuration_internal_def.h"

sl_status_t sl_zigbee_zdo_get_configuration_req(sl_802154_short_addr_t device_short,
                                                bool encrypt,
                                                uint8_t *tag_ids,
                                                uint8_t count)
{
  return sli_zigbee_stack_zdo_get_configuration_req(device_short,
                                                    encrypt,
                                                    tag_ids,
                                                    count);
}

sl_status_t sl_zigbee_zdo_set_add_configuration(uint8_t tag_id,
                                                size_t zdo_config_arg_len,
                                                const uint8_t *zdo_config_arg)
{
  return sli_zigbee_stack_zdo_set_add_configuration(tag_id,
                                                    zdo_config_arg_len,
                                                    zdo_config_arg);
}

sl_status_t sl_zigbee_zdo_set_send_configuration_req(sl_802154_short_addr_t device_short,
                                                     bool encrypt)
{
  return sli_zigbee_stack_zdo_set_send_configuration_req(device_short,
                                                         encrypt);
}
