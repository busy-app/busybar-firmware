/***************************************************************************//**
 * @file
 * @brief Default RSSI setup
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "sl_rail.h"
#include "sl_rail_util_rssi.h"
#include "sl_rail_util_rssi_config.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

static int8_t rssi_offset_db = SL_RAIL_UTIL_RSSI_OFFSET;

#if (SL_RAIL_UTIL_RSSI_SUPPORTS_NVM_DATA \
     && defined(SL_CATALOG_NVM3_DEFAULT_PRESENT))
#define SLI_RSSI_NVM_ENABLED 1
#else
#define SLI_RSSI_NVM_ENABLED 0
#endif

#if     SLI_RSSI_NVM_ENABLED

#include "nvm3_default.h"

sl_status_t sl_rail_util_write_nvm_rssi(int8_t rssiOffsetDb)
{
  // As NVM uninitialized has 0xFF but -1 is a valid RSSI offset
  // Use 16bits to represent 8 bits of rssiOffsetDb
  // Lower byte has the data and Upper byte has one's
  // complement of the data.

  // Intent is to apply this rssiOffsetDb at next boot
  // So, sl_rail_util_rssi_init() is not called here.
  rssi_offset_db = rssiOffsetDb;
  uint16_t writeNvmRssiOffsetDb = (((~(uint8_t)rssiOffsetDb) << 8U)
                                   | (uint8_t)rssiOffsetDb);
  sl_status_t status = nvm3_writeData(nvm3_defaultHandle,
                                      SL_RAIL_UTIL_RSSI_NVM_DATA_TAG,
                                      &writeNvmRssiOffsetDb,
                                      sizeof(writeNvmRssiOffsetDb));
  return status;
}

int8_t sl_rail_util_read_nvm_rssi(void)
{
  uint16_t readNvmRssiOffset;
  sl_status_t status = nvm3_readData(nvm3_defaultHandle,
                                     SL_RAIL_UTIL_RSSI_NVM_DATA_TAG,
                                     &readNvmRssiOffset,
                                     sizeof(readNvmRssiOffset));
  if ((status != SL_STATUS_OK)
      || ((~(readNvmRssiOffset) & 0xFFU) != ((readNvmRssiOffset >> 8U) & 0xFFU))) {
    // If data is invalid then use default value
    rssi_offset_db = SL_RAIL_UTIL_RSSI_OFFSET;
  } else {
    rssi_offset_db = (int8_t)(readNvmRssiOffset & 0xFFU);
  }
  return rssi_offset_db;
}

#else//!SLI_RSSI_NVM_ENABLED

sl_status_t sl_rail_util_write_nvm_rssi(int8_t rssiOffsetDb)
{
  (void) rssiOffsetDb;
  return SL_STATUS_NOT_AVAILABLE;
}

int8_t sl_rail_util_read_nvm_rssi(void)
{
  return SL_RAIL_UTIL_RSSI_OFFSET;
}

#endif//SLI_RSSI_NVM_ENABLED

void sl_rail_util_rssi_init(void)
{
#if     SLI_RSSI_NVM_ENABLED
  (void)sl_rail_util_read_nvm_rssi(); // Sets rssi_offset_db
#endif
  (void)sl_rail_set_rssi_offset(SL_RAIL_EFR32_HANDLE, rssi_offset_db);
}
