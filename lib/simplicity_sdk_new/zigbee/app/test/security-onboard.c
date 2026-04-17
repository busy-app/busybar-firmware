/***************************************************************************//**
 * @file
 * @brief This file handles 250/2420 routines for applications
 * that implement basic Security.
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
#include "stack/core/sl_zigbee_stack.h"               // Main stack definitions.
#include "core/sl_zigbee_multi_network.h"

// HAL.
#include "hal/hal.h"

// Application utilities.
#include "serial/serial.h"
#include "app/util/common/common.h"

#include "security-common.h"

// Unreleased security policy stack features.
#include "stack/zigbee/aps-security-policy.h"

//------------------------------------------------------------------------------
// External Declarations

void sli_zigbee_get_key_from_core(uint8_t* keyPointer);
bool sli_zigbee_get_trust_center_eui64(sl_802154_long_addr_t address);
bool sli_zigbee_get_security_token_data(sl_zigbee_initial_security_state_t* state);
void printStringWithYesOrNo(const char * formattedString, uint16_t yes);

//------------------------------------------------------------------------------
// Globals

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------

bool setTrustCenterJoinDecision(uint8_t decision)
{
  if ( decision <= SL_ZIGBEE_NO_ACTION ) {
    joinDecision = decision;
    return true;
  }

  return false;
}

//------------------------------------------------------------------------------

uint8_t getSecurityLevel(void)
{
  return sl_zigbee_security_level();
}

//------------------------------------------------------------------------------
// For the Em250/2420 we can gain more insight into the actual token values
// for security, so we print that info.

void printSecurityInfo(void)
{
  sl_zigbee_initial_security_state_t security;
  sl_802154_long_addr_t trustCenterAddress;

  sli_zigbee_get_security_token_data(&security);

  sl_zigbee_core_debug_print("TC Address: ");
  if ( sli_zigbee_get_trust_center_eui64(trustCenterAddress) ) {
    printLittleEndianEui64(serialPort, trustCenterAddress);
  } else {
    sl_zigbee_core_debug_print("<unknown>");
  }
  printCarriageReturn();
  (void) sli_legacy_serial_wait_send(serialPort);

  printStringWithYesOrNo("TC Uses Pre-config Keys: ",
                         (joinDecision == SL_ZIGBEE_USE_PRECONFIGURED_KEY));

  sl_zigbee_core_debug_println("Bitmask " "0x%04X", security.bitmask);
  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------

sl_status_t sendKeyUpdateToTarget(sl_802154_short_addr_t targetShort,
                                  sl_802154_long_addr_t targetLong,
                                  sl_zigbee_key_data_t* newKey)
{
  return sl_zigbee_send_unicast_network_key_update(targetShort,
                                                   targetLong,
                                                   newKey);
}
