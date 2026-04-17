/***************************************************************************//**
 * @file
 * @brief Code for manipulating the APS/NWK groups specific to the onboard
 * application hardware platforms (250, 2420).
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

// Ember stack and related utilities.
#include "stack/include/sl_zigbee.h"               // Main stack definitions.
#include "stack/internal/inc/internal-defs-patch.h"
// HAL.
#include "hal/hal.h"

// Application utilities.
#include "serial/serial.h"
#include "app/util/common/common.h"

//------------------------------------------------------------------------------
// Globals

//------------------------------------------------------------------------------
// Functions

uint8_t getGroupTableSize(void)
{
  return sl_zigbee_get_multicast_table_size();
}

//------------------------------------------------------------------------------

bool initializeGroupsTable(void)
{
  memset(sl_zigbee_get_multicast_table(),
         0,
         sizeof(sl_zigbee_multicast_table_entry_t) * sl_zigbee_get_multicast_table_size());
  return true;
}

//------------------------------------------------------------------------------

bool eraseGroup(uint8_t index)
{
  sl_zigbee_multicast_table_entry_t* table = sl_zigbee_get_multicast_table();
  memset(&table[index],
         0,
         sizeof(sl_zigbee_multicast_table_entry_t));
  return true;
}

//------------------------------------------------------------------------------

bool getGroup(uint8_t index, sl_zigbee_multicast_table_entry_t* returnData)
{
  if ( index >= sl_zigbee_get_multicast_table_size() ) {
    return false;
  }

  sl_zigbee_multicast_table_entry_t* table = sl_zigbee_get_multicast_table();
  memmove(returnData,
          &table[index],
          sizeof(sl_zigbee_multicast_table_entry_t));
  return true;
}

//------------------------------------------------------------------------------

bool setGroup(uint8_t index, sl_zigbee_multicast_table_entry_t* data)
{
  sl_zigbee_multicast_table_entry_t* table = sl_zigbee_get_multicast_table();
  memmove(&table[index],
          data,
          sizeof(sl_zigbee_multicast_table_entry_t));
  return true;
}

//------------------------------------------------------------------------------
