/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "green-power-stack-wrapper.h"

const sli_zigbee_af_zigbee_pro_network sli_zigbee_af_zigbee_pro_networks[] = { { 2, 5 } };

#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_GP_STACK == 1)

WEAK(bool sl_zigbee_gp_proxy_table_process_gp_pairing_wrapper(uint32_t options,
                                                              sl_zigbee_gp_address_t* addr,
                                                              uint8_t commMode,
                                                              uint16_t sinkNwkAddress,
                                                              uint16_t sinkGroupId,
                                                              uint16_t assignedAlias,
                                                              uint8_t* sinkIeeeAddress,
                                                              sl_zigbee_key_data_t* gpdKey,
                                                              uint32_t gpdSecurityFrameCounter,
                                                              uint8_t forwardingRadius))
{
  (void)options;
  (void)addr;
  (void)commMode;
  (void)sinkNwkAddress;
  (void)sinkGroupId;
  (void)assignedAlias;
  (void)sinkIeeeAddress;
  (void)gpdKey;
  (void)gpdSecurityFrameCounter;
  (void)forwardingRadius;
  return false;
}

WEAK(sl_status_t sli_zigbee_gp_proxy_table_set_entry_wrapper(uint8_t proxyIndex,
                                                             sl_zigbee_gp_proxy_table_entry_t *entry))
{
  (void)proxyIndex;
  (void)entry;
  return SL_STATUS_INVALID_STATE;
}

WEAK(void sli_zigbee_gp_proxy_table_init_wrapper(void))
{
}

WEAK(sl_status_t sl_zigbee_gp_proxy_table_get_entry_wrapper(uint8_t proxyIndex,
                                                            sl_zigbee_gp_proxy_table_entry_t *entry))
{
  (void)proxyIndex;
  (void)entry;
  return SL_STATUS_INVALID_STATE;
}
WEAK(uint8_t sli_zigbee_gp_proxy_table_get_free_entry_index_wrapper(void))
{
  return 0xff;
}
WEAK(uint8_t sl_zigbee_gp_proxy_table_lookup_wrapper(sl_zigbee_gp_address_t *addr))
{
  (void)addr;
  return 0xff;
}
WEAK(uint8_t sli_zigbee_gp_proxy_table_find_or_allocate_entry_wrapper(sl_zigbee_gp_address_t *addr))
{
  (void)addr;
  return 0xff;
}
WEAK(void sli_zigbee_gp_proxy_table_add_sink_wrapper(uint8_t index,
                                                     uint8_t commMode,
                                                     const uint8_t* sinkIeeeAddress,
                                                     sl_802154_short_addr_t sinkNwkAddress,
                                                     uint16_t sinkGroupId,
                                                     uint16_t assignedAlias
                                                     ))
{
  (void)index;
  (void)commMode;
  (void)sinkIeeeAddress;
  (void)sinkNwkAddress;
  (void)sinkGroupId;
  (void)assignedAlias;
}
WEAK(void sli_zigbee_gp_proxy_table_remove_sink_wrapper(uint8_t index, const uint8_t* sinkIeeeAddress, uint16_t sinkGroupId, uint16_t assignedAlias))
{
  (void)index;
  (void)sinkIeeeAddress;
  (void)sinkGroupId;
  (void)assignedAlias;
}
WEAK(void sl_zigbee_gp_proxy_table_remove_entry_wrapper(uint8_t index))
{
  (void)index;
}
WEAK(void sli_zigbee_gp_proxy_table_set_key_wrapper(uint8_t index, const uint8_t * gpdKey, sl_zigbee_gp_key_type_t securityKeyType))
{
  (void)index;
  (void)gpdKey;
  (void)securityKeyType;
}
WEAK(void sli_zigbee_gp_proxy_table_get_key_wrapper(uint8_t index, sl_zigbee_key_data_t *key))
{
  (void)index;
  (void)key;
}
WEAK(bool sli_zigbee_gp_address_match_wrapper(const sl_zigbee_gp_address_t *a1, const sl_zigbee_gp_address_t *a2))
{
  (void)a1;
  (void)a2;
  return false;
}

WEAK(void sl_zigbee_gp_sink_table_init_wrapper(void))
{
}
WEAK(sl_status_t sl_zigbee_gp_sink_table_set_entry_wrapper(uint8_t sinkTableIndex,
                                                           sl_zigbee_gp_sink_table_entry_t *entry))
{
  (void)sinkTableIndex;
  (void)entry;
  return SL_STATUS_INVALID_STATE;
}
WEAK(sl_status_t sl_zigbee_gp_sink_table_get_entry_wrapper(uint8_t sinkTableIndex,
                                                           sl_zigbee_gp_sink_table_entry_t *entry))
{
  (void)sinkTableIndex;
  (void)entry;
  return SL_STATUS_INVALID_STATE;
}
WEAK(uint8_t sl_zigbee_gp_sink_table_entry_in_use_wrapper(uint8_t sinkTableIndex))
{
  (void)sinkTableIndex;
  return 0xff;
}
WEAK(uint8_t sl_zigbee_gp_sink_table_get_free_entry_index_wrapper(void))
{
  return 0xff;
}

WEAK(uint8_t sl_zigbee_gp_sink_table_find_or_allocate_entry_wrapper(sl_zigbee_gp_address_t *addr))
{
  (void)addr;
  return 0xff;
}

WEAK(void sl_zigbee_gp_sink_table_add_group_wrapper(uint8_t index,
                                                    uint16_t sinkGroupId,
                                                    uint16_t assignedAlias))
{
  (void)index;
  (void)sinkGroupId;
  (void)assignedAlias;
}
WEAK(bool sl_zigbee_gp_sink_table_remove_group_wrapper(uint8_t index,
                                                       uint16_t sinkGroupId,
                                                       uint16_t assignedAlias))
{
  (void)index;
  (void)sinkGroupId;
  (void)assignedAlias;
  return false;
}
WEAK(uint8_t sl_zigbee_gp_sink_table_lookup_wrapper(sl_zigbee_gp_address_t *addr))
{
  (void)addr;
  return 0xff;
}
WEAK(void sl_zigbee_gp_sink_table_remove_entry_wrapper(uint8_t index))
{
  (void)index;
}
WEAK(void sl_zigbee_gp_sink_table_set_security_frame_counter_wrapper(uint8_t index,
                                                                     uint32_t sfc))
{
  (void)index;
  (void)sfc;
}

WEAK(uint8_t sl_zigbee_gp_sink_table_get_number_of_active_entries_wrapper(void))
{
  return 0;
}

WEAK(void sl_zigbee_gp_clear_proxy_table_wrapper(void))
{
  // Stub function.
}

#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_GP_STACK == 1)
