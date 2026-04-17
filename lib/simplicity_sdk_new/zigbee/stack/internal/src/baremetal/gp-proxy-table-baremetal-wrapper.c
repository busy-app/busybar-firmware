/***************************************************************************//**
 * @file gp-proxy-table-baremetal-wrapper.c
 * @brief internal implementations for 'gp-proxy-table' as a thin-wrapper
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
// automatically generated from gp-proxy-table.h.  Do not manually edit
#include "stack/include/gp-proxy-table.h"
#include "stack/internal/inc/gp-proxy-table-internal-def.h"

void sl_zigbee_gp_clear_proxy_table(void)
{
  sli_zigbee_stack_gp_clear_proxy_table();
}

sl_status_t sl_zigbee_gp_proxy_table_get_entry(uint8_t proxyIndex,
                                               sl_zigbee_gp_proxy_table_entry_t *entry)
{
  return sli_zigbee_stack_gp_proxy_table_get_entry(proxyIndex,
                                                   entry);
}

uint8_t sl_zigbee_gp_proxy_table_lookup(sl_zigbee_gp_address_t *addr)
{
  return sli_zigbee_stack_gp_proxy_table_lookup(addr);
}

bool sl_zigbee_gp_proxy_table_process_gp_pairing(uint32_t options,
                                                 sl_zigbee_gp_address_t *addr,
                                                 uint8_t commMode,
                                                 uint16_t sinkNwkAddress,
                                                 uint16_t sinkGroupId,
                                                 uint16_t assignedAlias,
                                                 uint8_t *sinkIeeeAddress,
                                                 sl_zigbee_key_data_t *gpdKey,
                                                 uint32_t gpdSecurityFrameCounter,
                                                 uint8_t forwardingRadius)
{
  return sli_zigbee_stack_gp_proxy_table_process_gp_pairing(options,
                                                            addr,
                                                            commMode,
                                                            sinkNwkAddress,
                                                            sinkGroupId,
                                                            assignedAlias,
                                                            sinkIeeeAddress,
                                                            gpdKey,
                                                            gpdSecurityFrameCounter,
                                                            forwardingRadius);
}

void sl_zigbee_gp_proxy_table_remove_entry(uint8_t index)
{
  sli_zigbee_stack_gp_proxy_table_remove_entry(index);
}
