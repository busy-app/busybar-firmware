/***************************************************************************//**
 * @file
 * @brief Stub implementation of additional key functionality that is
 * necessary for a Full Zigbee Pro Stack.
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

const sl_zigbee_library_status_t sli_zigbee_security_link_keys_library_status = SL_ZIGBEE_LIBRARY_IS_STUB;

void sli_zigbee_aps_security_read_frame_counters(void)
{
}

sl_status_t sli_zigbee_get_key_table_entry(uint8_t index, sl_zigbee_key_struct_t *result)
{
  (void)index;
  (void)result;
  return SL_STATUS_NOT_AVAILABLE;
}

uint8_t sli_zigbee_stack_find_key_table_entry(sl_802154_long_addr_t address, bool linkKey)
{
  (void)address;
  (void)linkKey;
  return 0xFF;
}

sl_status_t sli_zigbee_af_set_key_table_entry(bool erase,
                                              uint8_t index,
                                              sl_zigbee_key_struct_t* keyStruct)
{
  (void)erase;
  (void)index;
  (void)keyStruct;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_erase_key_table_entry(uint8_t index)
{
  (void)index;
  return SL_STATUS_NOT_AVAILABLE;
}

bool sli_zigbee_process_application_link_key(sl_802154_long_addr_t partnerEui64,
                                             bool amInitiator,
                                             sl_zigbee_key_data_t* keyData)
{
  (void)partnerEui64;
  (void)amInitiator;
  (void)keyData;
  return false;
}

sl_status_t sli_zigbee_update_key_state(uint8_t index,
                                        uint8_t setFlags,
                                        uint8_t clearFlags)
{
  (void)index;
  (void)setFlags;
  (void)clearFlags;
  return SL_STATUS_NOT_AVAILABLE;
}

void sli_zigbee_note_sleepy_device_in_key_table(sl_802154_long_addr_t eui64, bool sleepy)
{
  (void)eui64;
  (void)sleepy;
}

sl_status_t sli_zigbee_stack_clear_key_table(void)
{
  return SL_STATUS_NOT_AVAILABLE;
}

uint8_t sli_zigbee_find_key_table_entry(sl_802154_long_addr_t address, bool linkKey, uint8_t bitmask)
{
  (void)address;
  (void)linkKey;
  (void)bitmask;
  return 0xFF;
}
