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

#ifndef _SILABS_GREEN_POWER_STACK_WRAPPER_H_
#define _SILABS_GREEN_POWER_STACK_WRAPPER_H_

#include "zap-command.h"
#include "zap-id.h"
#include "zap-type.h"
#include "zap-enabled-incoming-commands.h"
#if !defined SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#define SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "zap-command-structs.h"
#undef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#endif

#include "sl_service_function.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "zcl-framework-core-config.h"
#endif // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "green-power-adapter-config.h"

#endif // SL_COMPONENT_CATALOG_PRESENT
/***************************************************************************
* Stub functions
* Customer should define these in their own application code
***************************************************************************/

#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_GP_STACK == 1)
bool sl_zigbee_gp_proxy_table_process_gp_pairing_wrapper(uint32_t options,
                                                         sl_zigbee_gp_address_t* addr,
                                                         uint8_t commMode,
                                                         uint16_t sinkNwkAddress,
                                                         uint16_t sinkGroupId,
                                                         uint16_t assignedAlias,
                                                         uint8_t* sinkIeeeAddress,
                                                         sl_zigbee_key_data_t* gpdKey,
                                                         uint32_t gpdSecurityFrameCounter,
                                                         uint8_t forwardingRadius);

sl_status_t sli_zigbee_gp_proxy_table_set_entry_wrapper(uint8_t proxyIndex,
                                                        sl_zigbee_gp_proxy_table_entry_t *entry);
void sli_zigbee_gp_proxy_table_init_wrapper(void);
sl_status_t sl_zigbee_gp_proxy_table_get_entry_wrapper(uint8_t proxyIndex,
                                                       sl_zigbee_gp_proxy_table_entry_t *entry);
uint8_t sli_zigbee_gp_proxy_table_get_free_entry_index_wrapper(void);
uint8_t sl_zigbee_gp_proxy_table_lookup_wrapper(sl_zigbee_gp_address_t *addr);
uint8_t sli_zigbee_gp_proxy_table_find_or_allocate_entry_wrapper(sl_zigbee_gp_address_t *addr);
void sli_zigbee_gp_proxy_table_add_sink_wrapper(uint8_t index,
                                                uint8_t commMode,
                                                const uint8_t* sinkIeeeAddress,
                                                sl_802154_short_addr_t sinkNwkAddress,
                                                uint16_t sinkGroupId,
                                                uint16_t assignedAlias
                                                );
void sli_zigbee_gp_proxy_table_remove_sink_wrapper(uint8_t index, const uint8_t* sinkIeeeAddress, uint16_t sinkGroupId, uint16_t assignedAlias);
void sl_zigbee_gp_proxy_table_remove_entry_wrapper(uint8_t index);
void sli_zigbee_gp_proxy_table_set_key_wrapper(uint8_t index, const uint8_t * gpdKey, sl_zigbee_gp_key_type_t securityKeyType);
void sli_zigbee_gp_proxy_table_get_key_wrapper(uint8_t index, sl_zigbee_key_data_t *key);
bool sli_zigbee_gp_address_match_wrapper(const sl_zigbee_gp_address_t *a1, const sl_zigbee_gp_address_t *a2);
void sl_zigbee_gp_sink_table_init_wrapper(void);
sl_status_t sl_zigbee_gp_sink_table_set_entry_wrapper(uint8_t sinkTableIndex,
                                                      sl_zigbee_gp_sink_table_entry_t *entry);
sl_status_t sl_zigbee_gp_sink_table_get_entry_wrapper(uint8_t sinkTableIndex,
                                                      sl_zigbee_gp_sink_table_entry_t *entry);
uint8_t sl_zigbee_gp_sink_table_entry_in_use_wrapper(uint8_t sinkTableIndex);
uint8_t sl_zigbee_gp_sink_table_get_free_entry_index_wrapper(void);
uint8_t sl_zigbee_gp_sink_table_find_or_allocate_entry_wrapper(sl_zigbee_gp_address_t *addr);
void sl_zigbee_gp_sink_table_add_group_wrapper(uint8_t index,
                                               uint16_t sinkGroupId,
                                               uint16_t assignedAlias);
bool sl_zigbee_gp_sink_table_remove_group_wrapper(uint8_t index,
                                                  uint16_t sinkGroupId,
                                                  uint16_t assignedAlias);
uint8_t sl_zigbee_gp_sink_table_lookup_wrapper(sl_zigbee_gp_address_t *addr);
void sl_zigbee_gp_sink_table_remove_entry_wrapper(uint8_t index);
void sl_zigbee_gp_sink_table_set_security_frame_counter_wrapper(uint8_t index,
                                                                uint32_t sfc);
void sl_zigbee_gp_clear_proxy_table_wrapper(void);

#define sl_zigbee_gp_proxy_table_process_gp_pairing sl_zigbee_gp_proxy_table_process_gp_pairing_wrapper
#define sli_zigbee_gp_proxy_table_set_entry sli_zigbee_gp_proxy_table_set_entry_wrapper
#define sli_zigbee_gp_proxy_table_init sli_zigbee_gp_proxy_table_init_wrapper
#define sl_zigbee_gp_proxy_table_get_entry sl_zigbee_gp_proxy_table_get_entry_wrapper
#define sli_zigbee_gp_proxy_table_get_free_entry_index sli_zigbee_gp_proxy_table_get_free_entry_index_wrapper
#define sl_zigbee_gp_proxy_table_lookup sl_zigbee_gp_proxy_table_lookup_wrapper
#define sli_zigbee_gp_proxy_table_find_or_allocate_entry sli_zigbee_gp_proxy_table_find_or_allocate_entry_wrapper
#define sli_zigbee_gp_proxy_table_add_sink sli_zigbee_gp_proxy_table_add_sink_wrapper
#define sli_zigbee_gp_proxy_table_remove_sink sli_zigbee_gp_proxy_table_remove_sink_wrapper
#define sl_zigbee_gp_proxy_table_remove_entry sl_zigbee_gp_proxy_table_remove_entry_wrapper
#define sli_zigbee_gp_proxy_table_set_key sli_zigbee_gp_proxy_table_set_key_wrapper
#define sli_zigbee_gp_proxy_table_get_key sli_zigbee_gp_proxy_table_get_key_wrapper
#define sli_zigbee_gp_address_match sli_zigbee_gp_address_match_wrapper
#define sl_zigbee_gp_sink_table_init sl_zigbee_gp_sink_table_init_wrapper
#define sl_zigbee_gp_sink_table_set_entry sl_zigbee_gp_sink_table_set_entry_wrapper
#define sl_zigbee_gp_sink_table_get_entry sl_zigbee_gp_sink_table_get_entry_wrapper
#define sl_zigbee_gp_sink_table_entry_in_use sl_zigbee_gp_sink_table_entry_in_use_wrapper
#define sl_zigbee_gp_sink_table_get_free_entry_index sl_zigbee_gp_sink_table_get_free_entry_index_wrapper
#define sl_zigbee_gp_sink_table_find_or_allocate_entry sl_zigbee_gp_sink_table_find_or_allocate_entry_wrapper
#define sl_zigbee_gp_sink_table_add_group sl_zigbee_gp_sink_table_add_group_wrapper
#define sl_zigbee_gp_sink_table_remove_group sl_zigbee_gp_sink_table_remove_group_wrapper
#define sl_zigbee_gp_sink_table_lookup sl_zigbee_gp_sink_table_lookup_wrapper
#define sl_zigbee_gp_sink_table_remove_entry sl_zigbee_gp_sink_table_remove_entry_wrapper
#define sl_zigbee_gp_sink_table_set_security_frame_counter sl_zigbee_gp_sink_table_set_security_frame_counter_wrapper
#define sl_zigbee_gp_sink_table_get_number_of_active_entries sl_zigbee_gp_sink_table_get_number_of_active_entries_wrapper
#define sl_zigbee_gp_clear_proxy_table sl_zigbee_gp_clear_proxy_table_wrapper

#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_GP_STACK == 1)

#endif //_SILABS_GREEN_POWER_STACK_WRAPPER_H_
