/***************************************************************************//**
 * @file raw-message-baremetal-wrapper.c
 * @brief internal implementations for 'raw-message' as a thin-wrapper
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
// automatically generated from raw-message.h.  Do not manually edit
#include "stack/include/raw-message.h"
#include "stack/internal/inc/raw-message-internal-def.h"

sl_802154_short_addr_t sl_zigbee_get_embernet_passthrough_source_address(void)
{
  return sli_zigbee_stack_get_embernet_passthrough_source_address();
}

sl_zigbee_mac_passthrough_type_t sl_zigbee_get_mac_passthrough_flags(void)
{
  return sli_zigbee_stack_get_mac_passthrough_flags();
}

sl_status_t sl_zigbee_send_raw_message(const uint8_t *message,
                                       uint8_t message_length,
                                       sl_zigbee_transmit_priority_t priority,
                                       bool useCca)
{
  return sli_zigbee_stack_send_raw_message(message,
                                           message_length,
                                           priority,
                                           useCca);
}

sl_status_t sl_zigbee_send_raw_message_with_tag(const uint8_t *message,
                                                uint8_t message_length,
                                                sl_zigbee_transmit_priority_t priority,
                                                bool useCca,
                                                uint8_t messageTag)
{
  return sli_zigbee_stack_send_raw_message_with_tag(message,
                                                    message_length,
                                                    priority,
                                                    useCca,
                                                    messageTag);
}

void sl_zigbee_set_embernet_passthrough_source_address(sl_802154_short_addr_t address)
{
  sli_zigbee_stack_set_embernet_passthrough_source_address(address);
}

sl_status_t sl_zigbee_set_mac_filter_match_list(const sl_zigbee_mac_filter_match_data_t *macFilterMatchList,
                                                uint8_t listLength)
{
  return sli_zigbee_stack_set_mac_filter_match_list(macFilterMatchList,
                                                    listLength);
}

void sl_zigbee_set_mac_passthrough_flags(sl_zigbee_mac_passthrough_type_t type)
{
  sli_zigbee_stack_set_mac_passthrough_flags(type);
}
