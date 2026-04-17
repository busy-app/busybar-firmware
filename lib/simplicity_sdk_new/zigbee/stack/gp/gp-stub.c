/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/include/library.h"
#include "event_queue/event-queue.h"

sl_status_t sli_zigbee_gp_init(void)
{
  return SL_STATUS_OK;
}

void sli_zigbee_gp_init_tokens(void)
{
  // stub function
}

void sli_zigbee_gp_proxy_table_init(void)
{
  // stub function
}
void sli_zigbee_gp_incoming_message_handler(sli_zigbee_packet_header_t header)
{
  (void)header;
}
//CGP Stub

// CGP-DATA.request
void sli_zigbee_c_gp_send(bool useCca,
                          sl_zigbee_gp_address_t *src,
                          sl_zigbee_gp_address_t *dst,
                          uint8_t gpdCommandId,
                          sli_buffer_manager_buffer_t asdu)
{
  (void)useCca;
  (void)src;
  (void)dst;
  (void)gpdCommandId;
  (void)asdu;
  return;
}

//CGP-DATA.confirm
void sli_zigbee_c_gp_sent_handler(sl_status_t status, uint8_t gpMpduHandle)
{
  (void)status;
  (void)gpMpduHandle;
  return;
}

//GP-DATA.request
sl_status_t sli_zigbee_stack_d_gp_send(bool action,
                                       bool useCca,
                                       sl_zigbee_gp_address_t *addr,
                                       uint8_t gpdCommandId,
                                       uint8_t gpdAsduLength,
                                       uint8_t const *gpdAsdu,
                                       uint8_t gpepHandle,
                                       uint16_t gpTxQueueEntryLifetimeMs)
{
  (void)action;
  (void)useCca;
  (void)addr;
  (void)gpdCommandId;
  (void)gpdAsduLength;
  (void)gpdAsdu;
  (void)gpepHandle;
  (void)gpTxQueueEntryLifetimeMs;
  return SL_STATUS_NOT_AVAILABLE;
}

void sli_zigbee_read_gp_tokens(void)
{
  return;
}

bool sli_zigbee_is_gp_id_conflict(sl_802154_short_addr_t shortId)
{
  (void)shortId;
  return false;
}

void sli_zigbee_detect_gp_id_conflict_and_resolve(sl_802154_short_addr_t shortId,
                                                  uint8_t *longId)
{
  (void)shortId;
  (void)longId;
}
void sli_zigbee_update_gp_proxy_table(uint8_t *longId, sl_802154_short_addr_t shortId)
{
  (void)longId;
  (void)shortId;
}

void sli_zigbee_gp_tx_event_handler(sli_zigbee_event_t *event)
{
  (void) event;
}

bool sli_zigbee_stack_gp_proxy_table_process_gp_pairing(uint32_t options,
                                                        sl_zigbee_gp_address_t* addr,
                                                        uint8_t commMode,
                                                        uint16_t sinkNwkAddress,
                                                        uint16_t sinkGroupId,
                                                        uint16_t assignedAlias,
                                                        uint8_t* sinkIeeeAddress,
                                                        sl_zigbee_key_data_t* gpdKey,
                                                        uint32_t gpdSecurityFrameCounter,
                                                        uint8_t forwardingRadius)
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

uint8_t sli_zigbee_stack_gp_proxy_table_lookup(sl_zigbee_gp_address_t *addr)
{
  (void)addr;
  return 0xFF;
}
sl_status_t sli_zigbee_stack_gp_proxy_table_get_entry(uint8_t proxyIndex,
                                                      sl_zigbee_gp_proxy_table_entry_t *entry)
{
  (void)proxyIndex;
  (void)entry;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_gp_sink_table_get_entry(uint8_t sinkIndex,
                                                     sl_zigbee_gp_sink_table_entry_t *entry)
{
  (void)sinkIndex;
  (void)entry;
  return SL_STATUS_NOT_AVAILABLE;
}

uint8_t sli_zigbee_stack_gp_sink_table_lookup(sl_zigbee_gp_address_t *addr)
{
  (void)addr;
  return 0xFF;
}

sl_status_t sli_zigbee_stack_gp_sink_table_set_entry(uint8_t sinkTableIndex,
                                                     sl_zigbee_gp_sink_table_entry_t *entry)
{
  (void)sinkTableIndex;
  (void)entry;
  return SL_STATUS_NOT_AVAILABLE;
}

uint8_t sli_zigbee_stack_gp_sink_table_find_or_allocate_entry(sl_zigbee_gp_address_t *addr)
{
  (void)addr;
  return 0xFF;
}

void sli_zigbee_stack_gp_sink_table_remove_entry(uint8_t index)
{
  (void)index;
}

void sli_zigbee_stack_gp_proxy_table_remove_entry(uint8_t index)
{
  (void)index;
}

void sli_zigbee_stack_gp_clear_proxy_table(void)
{
  // Stub function.
}
void sli_zigbee_stack_gp_sink_table_clear_all(void)
{
  // stub function
}
void sli_zigbee_stack_gp_sink_table_init(void)
{
  // stub function
}

void sli_zigbee_stack_gp_sink_table_set_security_frame_counter(uint8_t index,
                                                               uint32_t sfc)
{
  (void)index;
  (void)sfc;
}

uint8_t sli_zigbee_stack_gp_sink_table_get_number_of_active_entries(void)
{
  return 0;
}
