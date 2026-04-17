/***************************************************************************//**
 * @file binding-table-baremetal-wrapper.c
 * @brief internal implementations for 'binding-table' as a thin-wrapper
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
// automatically generated from binding-table.h.  Do not manually edit
#include "stack/include/binding-table.h"
#include "stack/internal/inc/binding-table-internal-def.h"

bool sl_zigbee_binding_is_active(uint8_t index)
{
  return sli_zigbee_stack_binding_is_active(index);
}

sl_status_t sl_zigbee_clear_binding_table(void)
{
  return sli_zigbee_stack_clear_binding_table();
}

void sl_zigbee_clear_binding_table_on_leave(bool clear)
{
  sli_zigbee_stack_clear_binding_table_on_leave(clear);
}

sl_status_t sl_zigbee_delete_binding(uint8_t index)
{
  return sli_zigbee_stack_delete_binding(index);
}

sl_status_t sl_zigbee_get_binding(uint8_t index,
                                  sl_zigbee_binding_table_entry_t *result)
{
  return sli_zigbee_stack_get_binding(index,
                                      result);
}

sl_802154_short_addr_t sl_zigbee_get_binding_remote_node_id(uint8_t index)
{
  return sli_zigbee_stack_get_binding_remote_node_id(index);
}

uint8_t sl_zigbee_get_binding_table_size(void)
{
  return sli_zigbee_stack_get_binding_table_size();
}

sl_status_t sl_zigbee_note_senders_binding(uint8_t index,
                                           sl_802154_short_addr_t source)
{
  return sli_zigbee_stack_note_senders_binding(index,
                                               source);
}

sl_status_t sl_zigbee_set_binding(uint8_t index,
                                  sl_zigbee_binding_table_entry_t *value)
{
  return sli_zigbee_stack_set_binding(index,
                                      value);
}

void sl_zigbee_set_binding_remote_node_id(uint8_t index,
                                          sl_802154_short_addr_t id)
{
  sli_zigbee_stack_set_binding_remote_node_id(index,
                                              id);
}

sl_status_t sl_zigbee_set_binding_table_size(uint8_t size)
{
  return sli_zigbee_stack_set_binding_table_size(size);
}

sl_status_t sl_zigbee_set_reply_binding(uint8_t index,
                                        sl_zigbee_binding_table_entry_t *entry,
                                        sl_802154_short_addr_t source)
{
  return sli_zigbee_stack_set_reply_binding(index,
                                            entry,
                                            source);
}
