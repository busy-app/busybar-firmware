/***************************************************************************//**
 * @file
 * @brief Onboard specific commands for the Application Test Profile #2.
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

// HAL.
#include "hal/hal.h"

// Application utilities.
#include "serial/serial.h"
#include "app/util/common/common.h"
#include "app/test/misc-common.h"
#include "app/test/security-common.h"
#include "fragmentation.h"
#include "app/test/test-profile.h"

//------------------------------------------------------------------------------
// Globals

//------------------------------------------------------------------------------
// External Declarations

extern uint8_t sl_zigbee_fragment_window_size;
extern uint8_t sli_zigbee_fragment_delay_ms;
extern void sl_mac_set_mac_tx_mode(uint8_t);
//------------------------------------------------------------------------------
// Functions

sl_status_t initializeFragmentation(uint8_t maxTotalMessageLength,
                                    uint8_t windowSize,
                                    uint8_t fragmentDelayMs,
                                    uint8_t missedBlocks)
{
  maxMessageLength = maxTotalMessageLength;
  sl_zigbee_fragment_window_size = windowSize;
  sli_zigbee_fragment_delay_ms = fragmentDelayMs;
  sli_zigbee_af_fragmentation_artificially_drop_block_number = missedBlocks;

  return SL_STATUS_OK;
}
///---sTODO: this definition is probably different from Host version, but not sure
sl_status_t setTxMode(uint8_t mode)
{
  UNUSED_VAR(mode);
  //sl_mac_set_mac_tx_mode(mode);
  return SL_STATUS_OK;
}
