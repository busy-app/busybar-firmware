/***************************************************************************//**
 * @file
 * @brief ZigBee Light Link function stubs.
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
#include "stack/include/zll-types.h"
#include "stack/include/library.h"
#include "event_queue/event-queue.h"
#include "mac-phy.h"

//------------------------------------------------------------------------------
// Globals

sli_zigbee_event_t sli_zigbee_zll_touch_link_event;

const sl_zigbee_library_status_t sli_zigbee_zll_library_status = SL_ZIGBEE_LIBRARY_IS_STUB;

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------

sl_status_t sli_zigbee_zll_send_scan_request(void)
{
  return SL_STATUS_NOT_AVAILABLE;
}

uint8_t sli_zigbee_get_next_scan_channel(uint8_t currentChannel)
{
  return currentChannel + 1;
}

bool sli_zigbee_go_to_next_scan_channel(uint8_t currentChannel)
{
  (void)currentChannel;
  return true;
}

bool sli_zigbee_zll_scan_complete(void)
{
  return false;
}

void sli_zigbee_stack_zll_clear_tokens(void)
{
}

bool sli_zigbee_process_zll_interpan_stack_message(const sl_zigbee_mac_filter_match_struct_t* macFilterMatchStruct)
{
  (void)macFilterMatchStruct;
  return false;
}

bool sli_zigbee_zll_stack_status_handler(sl_status_t status)
{
  (void)status;
  return false;
}

bool sli_zigbee_zll_waiting_for_interpan_response(void)
{
  return false;
}

void sli_zigbee_zll_device_announce_handler(sl_802154_short_addr_t source, sl_802154_short_addr_t nodeId, uint8_t capabilities)
{
  (void)source;
  (void)nodeId;
  (void)capabilities;
}

bool sli_zigbee_stack_is_zll_network(void)
{
  return false;
}

void sli_zigbee_zll_init(void)
{
}

sl_status_t sli_zigbee_stack_zll_form_network(sl_zigbee_zll_network_t* networkInfo,
                                              int8_t radioTxPower)
{
  (void)networkInfo;
  (void)radioTxPower;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_zll_join_target(const sl_zigbee_zll_network_t* targetNetworkInfo)
{
  (void)targetNetworkInfo;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_zll_set_security_state_without_key(const sl_zigbee_zll_initial_security_state_t *securityState)
{
  (void)securityState;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_zll_set_initial_security_state(const sl_zigbee_key_data_t *networkKey,
                                                            const sl_zigbee_zll_initial_security_state_t *securityState)
{
  (void)networkKey;
  (void)securityState;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_zll_start_scan(uint32_t channelMask,
                                            int8_t radioPowerForScan,
                                            sl_zigbee_node_type_t nodeType)
{
  (void)channelMask;
  (void)radioPowerForScan;
  (void)nodeType;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_zll_set_rx_on_when_idle(uint32_t durationMs)
{
  (void)durationMs;
  return SL_STATUS_NOT_AVAILABLE;
}

void sli_zigbee_stack_zll_set_radio_idle_mode(sl_zigbee_radio_power_mode_t mode)
{
  (void)mode;
}

uint8_t sli_zigbee_stack_zll_get_radio_idle_mode(void)
{
  // ZLL does stuff when the radio mode is power off. To stop that, the stub
  // reports the radio power on always
  return SL_ZIGBEE_RADIO_POWER_MODE_RX_ON;
}

void sli_zigbee_stack_set_zll_node_type(sl_zigbee_node_type_t nodeType)
{
  (void)nodeType;
}

void sli_zigbee_stack_set_zll_additional_state(uint16_t mask)
{
  (void)mask;
}

void sli_zigbee_stack_zll_get_tokens_stack_zll(sl_zigbee_tok_type_stack_zll_data_t *data,
                                               sl_zigbee_tok_type_stack_zll_security_t *security)
{
  (void)data;
  (void)security;
}

void sli_zigbee_stack_zll_set_token_stack_zll_data(sl_zigbee_tok_type_stack_zll_data_t *token)
{
  (void)token;
}

void sli_zigbee_stack_zll_set_non_zll_network(void)
{
}

sl_zigbee_zll_policy_t sli_zigbee_stack_zll_get_policy(void)
{
  return SL_ZIGBEE_ZLL_POLICY_DISABLED;
}

sl_status_t sli_zigbee_stack_zll_set_policy(sl_zigbee_zll_policy_t policy)
{
  (void)policy;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_zigbee_node_type_t sli_zigbee_zll_get_node_type(void)
{
  return SL_ZIGBEE_UNKNOWN_DEVICE;
}

bool sli_zigbee_stack_zll_operation_in_progress(void)
{
  return false;
}

bool sli_zigbee_stack_zll_rx_on_when_idle_get_active(void)
{
  return false;
}

void sli_zigbee_zll_suspend_rx_on_when_idle(bool mode)
{
  (void)mode;
}

uint32_t sli_zigbee_stack_get_zll_primary_channel_mask(void)
{
  return 0;
}

uint32_t sli_zigbee_stack_get_zll_secondary_channel_mask(void)
{
  return 0;
}

void sli_zigbee_stack_set_zll_primary_channel_mask(uint32_t mask)
{
  (void)mask;
}

void sli_zigbee_stack_set_zll_secondary_channel_mask(uint32_t mask)
{
  (void)mask;
}

void sli_zigbee_mark_zll_buffers(void)
{
}

sli_buffer_manager_buffer_t sli_build_zll_scan_request_payload(void)
{
  return NULL_BUFFER;
}
void sli_zigbee_stack_zll_scanning_complete(void)
{
}

// these public API wrappers are needed because of exceptional case that these APIs are called from soc plugin apps
sl_status_t sl_zigbee_zll_set_policy(sl_zigbee_zll_policy_t policy)
{
  (void)policy;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_zigbee_zll_policy_t sl_zigbee_zll_get_policy(void)
{
  return SL_ZIGBEE_ZLL_POLICY_DISABLED;
}
