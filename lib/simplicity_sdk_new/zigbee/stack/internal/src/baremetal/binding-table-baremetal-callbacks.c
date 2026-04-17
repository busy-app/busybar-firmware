/***************************************************************************//**
 * @file binding-table-baremetal-callbacks.c
 * @brief internal dispatch for 'binding-table' callbacks as a thin-wrapper
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
// automatically generated from binding-table.h.  Do not manually edit
#include "stack/include/binding-table.h"
#include "stack/internal/inc/binding-table-internal-def.h"

void sli_zigbee_stack_remote_delete_binding_handler(uint8_t index,
                                                    sl_zigbee_zdo_status_t status)
{
  sl_zigbee_remote_delete_binding_handler(index,
                                          status);
}

void sli_zigbee_stack_remote_set_binding_handler(sl_zigbee_binding_table_entry_t *entry,
                                                 sl_zigbee_zdo_status_t status)
{
  sl_zigbee_remote_set_binding_handler(entry,
                                       status);
}
