/***************************************************************************//**
 * @file gp-types-baremetal-wrapper.c
 * @brief internal implementations for 'gp-types' as a thin-wrapper
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
// automatically generated from gp-types.h.  Do not manually edit
#include "stack/include/gp-types.h"
#include "stack/internal/inc/gp-types-internal-def.h"

sl_status_t sl_zigbee_d_gp_send(bool action,
                                bool useCca,
                                sl_zigbee_gp_address_t *addr,
                                uint8_t gpdCommandId,
                                uint8_t gpdAsduLength,
                                const uint8_t *gpdAsdu,
                                uint8_t gpepHandle,
                                uint16_t gpTxQueueEntryLifetimeMs)
{
  return sli_zigbee_stack_d_gp_send(action,
                                    useCca,
                                    addr,
                                    gpdCommandId,
                                    gpdAsduLength,
                                    gpdAsdu,
                                    gpepHandle,
                                    gpTxQueueEntryLifetimeMs);
}

uint16_t sl_zigbee_get_gp_max_tx_q_list_count(void)
{
  return sli_zigbee_stack_get_gp_max_tx_q_list_count();
}

uint16_t sl_zigbee_get_gp_tx_q_list_count(void)
{
  return sli_zigbee_stack_get_gp_tx_q_list_count();
}

sli_buffer_manager_buffer_t sl_zigbee_gp_add_gp_tx_queue_entry_with_payload(sl_zigbee_gp_tx_queue_entry_t *txQueue,
                                                                            uint8_t *data,
                                                                            uint16_t dataLength)
{
  return sli_zigbee_stack_gp_add_gp_tx_queue_entry_with_payload(txQueue,
                                                                data,
                                                                dataLength);
}

void sl_zigbee_gp_clear_tx_queue(void)
{
  sli_zigbee_stack_gp_clear_tx_queue();
}

sli_buffer_manager_buffer_t sl_zigbee_gp_get_tx_queue_entry_from_queue(sl_zigbee_gp_tx_queue_entry_t *txQueue,
                                                                       uint8_t *data,
                                                                       uint16_t *dataLength,
                                                                       uint16_t allocatedDataLength)
{
  return sli_zigbee_stack_gp_get_tx_queue_entry_from_queue(txQueue,
                                                           data,
                                                           dataLength,
                                                           allocatedDataLength);
}

sl_status_t sl_zigbee_gp_get_tx_queue_entry_from_queue_index(uint8_t index,
                                                             sl_zigbee_gp_tx_queue_entry_t *txQueue,
                                                             uint8_t *payload,
                                                             uint16_t *payload_len)
{
  return sli_zigbee_stack_gp_get_tx_queue_entry_from_queue_index(index,
                                                                 txQueue,
                                                                 payload,
                                                                 payload_len);
}

sli_zigbee_message_buffer_queue_t * sl_zigbee_gp_get_tx_queue_head(void)
{
  return sli_zigbee_stack_gp_get_tx_queue_head();
}

bool sl_zigbee_gp_remove_from_tx_queue(sl_zigbee_gp_tx_queue_entry_t *txQueue)
{
  return sli_zigbee_stack_gp_remove_from_tx_queue(txQueue);
}

void sl_zigbee_gp_set_max_tx_queue_entry(uint16_t maxEntries)
{
  sli_zigbee_stack_gp_set_max_tx_queue_entry(maxEntries);
}
