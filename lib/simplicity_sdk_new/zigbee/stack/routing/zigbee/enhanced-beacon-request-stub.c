/***************************************************************************//**
 * @file
 * @brief Implementation of Enhanced Beacon request generation
 * and parsing for Zigbee Pro.
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
#include "stack/include/sl_zigbee_types_internal.h"
#include "stack/include/library.h"
#include "upper-mac.h"

//=============================================================================
// Returns true if the device should continue to process, false if it
// should be dropped.

const sl_zigbee_library_status_t sli_zigbee_enhanced_beacon_request_library_status = SL_ZIGBEE_LIBRARY_IS_STUB;

sli_buffer_manager_buffer_t deviceEnhancedBeaconRequestFilterList = SL_ZIGBEE_NULL_MESSAGE_BUFFER;
uint8_t joinListUpdateId = 0;

void sli_zigbee_mark_enhanced_beacon_buffers(void)
{
}

bool sli_zigbee_process_enhanced_beacon_request(sli_zigbee_packet_header_t header,
                                                sli_802154mac_frame_info_element_parse_result result,
                                                sli_802154mac_info_element_field* infoElementsArray)
{
  (void)header;
  (void)infoElementsArray;
  // If the frame contains MAC Info Elements (IEs) we don't process it.
  if (result == EM_MAC_FRAME_INFO_ELEMENTS_NOT_PRESENT) {
    return true;
  }
  return false;
}

sl_status_t sli_zigbee_send_enhanced_beacon_request(uint8_t mac_index, bool firstTimeJoin)
{
  (void)mac_index;
  (void)firstTimeJoin;
  return SL_STATUS_NOT_AVAILABLE;
}

bool sli_zigbee_check_ebr_filter_list(uint8_t * longId)
{
  (void)longId;
  return true;
}

void sli_zigbee_join_list_add(sl_802154_long_addr_t longId)
{
  (void)longId;
}

void sli_zigbee_join_list_delete(sl_802154_long_addr_t longId)
{
  (void)longId;
}

void sli_zigbee_join_list_clear(void)
{
}

sli_buffer_manager_buffer_t sli_zigbee_get_join_list_pointer(void)
{
  return SL_ZIGBEE_NULL_MESSAGE_BUFFER;
}

int8_t sli_zigbee_find_ebr_power_by_eui64(sl_802154_long_addr_t eui64)
{
  (void)eui64;
  return MAX_RADIO_POWER_USER_PROVIDED;
}

void sli_zigbee_remove_ebr_power_and_eui64_entry(sl_802154_long_addr_t eui64)
{
  (void)eui64;
}

int8_t sli_zigbee_stack_child_power(uint8_t childIndex)
{
  (void)childIndex;
  return MAX_RADIO_POWER_USER_PROVIDED;
}

void sli_zigbee_stack_set_child_power(uint8_t childIndex, int8_t newPower)
{
  (void)childIndex;
  (void)newPower;
}

uint8_t sli_zigbee_get_ebr_power_and_eui_list_pointer(uint8_t **buffer)
{
  (void)buffer;
  return 0;
}

bool sli_zigbee_add_ebr_eui_and_power_entry(sl_802154_long_addr_t eui64, int8_t newPower)
{
  (void)eui64;
  (void)newPower;
  return false;
}

void sli_802154mac_send_enhanced_beacon(uint8_t mac_index)
{
  (void)mac_index;
}
