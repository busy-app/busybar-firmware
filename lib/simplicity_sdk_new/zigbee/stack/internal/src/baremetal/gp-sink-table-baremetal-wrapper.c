/***************************************************************************//**
 * @file gp-sink-table-baremetal-wrapper.c
 * @brief internal implementations for 'gp-sink-table' as a thin-wrapper
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
// automatically generated from gp-sink-table.h.  Do not manually edit
#include "stack/include/gp-sink-table.h"
#include "stack/internal/inc/gp-sink-table-internal-def.h"

void sl_zigbee_gp_sink_table_add_group(uint8_t index,
                                       uint16_t sinkGroupId,
                                       uint16_t assignedAlias)
{
  sli_zigbee_stack_gp_sink_table_add_group(index,
                                           sinkGroupId,
                                           assignedAlias);
}

void sl_zigbee_gp_sink_table_clear_all(void)
{
  sli_zigbee_stack_gp_sink_table_clear_all();
}

uint8_t sl_zigbee_gp_sink_table_entry_in_use(uint8_t sinkTableIndex)
{
  return sli_zigbee_stack_gp_sink_table_entry_in_use(sinkTableIndex);
}

uint8_t sl_zigbee_gp_sink_table_find_or_allocate_entry(sl_zigbee_gp_address_t *addr)
{
  return sli_zigbee_stack_gp_sink_table_find_or_allocate_entry(addr);
}

sl_status_t sl_zigbee_gp_sink_table_get_entry(uint8_t sinkTableIndex,
                                              sl_zigbee_gp_sink_table_entry_t *entry)
{
  return sli_zigbee_stack_gp_sink_table_get_entry(sinkTableIndex,
                                                  entry);
}

uint8_t sl_zigbee_gp_sink_table_get_free_entry_index(void)
{
  return sli_zigbee_stack_gp_sink_table_get_free_entry_index();
}

uint8_t sl_zigbee_gp_sink_table_get_number_of_active_entries(void)
{
  return sli_zigbee_stack_gp_sink_table_get_number_of_active_entries();
}

void sl_zigbee_gp_sink_table_init(void)
{
  sli_zigbee_stack_gp_sink_table_init();
}

uint8_t sl_zigbee_gp_sink_table_lookup(sl_zigbee_gp_address_t *addr)
{
  return sli_zigbee_stack_gp_sink_table_lookup(addr);
}

void sl_zigbee_gp_sink_table_remove_entry(uint8_t index)
{
  sli_zigbee_stack_gp_sink_table_remove_entry(index);
}

bool sl_zigbee_gp_sink_table_remove_group(uint8_t index,
                                          uint16_t sinkGroupId,
                                          uint16_t assignedAlias)
{
  return sli_zigbee_stack_gp_sink_table_remove_group(index,
                                                     sinkGroupId,
                                                     assignedAlias);
}

sl_status_t sl_zigbee_gp_sink_table_set_entry(uint8_t sinkTableIndex,
                                              sl_zigbee_gp_sink_table_entry_t *entry)
{
  return sli_zigbee_stack_gp_sink_table_set_entry(sinkTableIndex,
                                                  entry);
}

void sl_zigbee_gp_sink_table_set_security_frame_counter(uint8_t index,
                                                        uint32_t sfc)
{
  sli_zigbee_stack_gp_sink_table_set_security_frame_counter(index,
                                                            sfc);
}
