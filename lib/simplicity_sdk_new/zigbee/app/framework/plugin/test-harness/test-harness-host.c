/***************************************************************************//**
 * @file
 * @brief Test harness code specific to the SOC.
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

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "test-harness.h"

#include "app/xncp-test-harness/xncp-test-harness.h"

//------------------------------------------------------------------------------
// Globals

// For easier readability
#define RESET_FRAME_COUNTER false
#define ADVANCE_FRAME_COUNTER true

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Functions

// Returns false if XNCP is not the right software and doesn't support our
// custom EZSP frames, true if it does.
static bool checkXncpSoftware(void)
{
  uint16_t manufacturerId;
  uint16_t versionNumber;

  sl_status_t status;

  status  = sl_zigbee_ezsp_get_xncp_info(&manufacturerId, &versionNumber);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("Error: XNCP test harness software not present on NCP.");
    return false;
  } else if (manufacturerId != SL_ZIGBEE_MANUFACTURER_ID
             || versionNumber != SL_ZIGBEE_XNCP_TEST_HARNESS_VERSION_NUMBER) {
    sl_zigbee_af_core_println("Error: Wrong XNCP software loaded on NCP.");
    sl_zigbee_af_core_println("  Wanted Manuf ID: 0x%04X, Version: 0x%04X",
                              SL_ZIGBEE_MANUFACTURER_ID,
                              SL_ZIGBEE_XNCP_TEST_HARNESS_VERSION_NUMBER);
    sl_zigbee_af_core_println("     Got Manuf ID: 0x%04X, Version: 0x%04X",
                              manufacturerId,
                              versionNumber);
    return false;
  }

  return true;
}

static void resetOrAdvanceApsFrameCounter(bool advance)
{
  uint8_t customEzspMessage[1];
  uint8_t reply[1];
  uint8_t replyLength = 1;
  sl_status_t status;

  customEzspMessage[0] = (advance
                          ? SL_ZIGBEE_XNCP_TEST_HARNESS_COMMAND_ADVANCE_APS_FRAME_COUNTER
                          : SL_ZIGBEE_XNCP_TEST_HARNESS_COMMAND_RESET_APS_FRAME_COUNTER);

  if (!checkXncpSoftware()) {
    return;
  }

  status = sl_zigbee_ezsp_custom_frame(1, // length of custom EZSP message
                                       customEzspMessage,
                                       &replyLength,
                                       reply);
  sl_zigbee_af_core_println("%s APS Frame counter: %s (0x%02X)",
                            (advance
                             ? "Advancing"
                             : "Reset"),
                            ((status == SL_STATUS_OK)
                             ? "Success"
                             : "FAILED"),
                            status);
}

void sli_zigbee_af_test_harness_reset_aps_frame_counter(void)
{
  // Reseting the outgoing APS frame counter is non-standard and not
  // a good idea, especially on the TC.  This is necessary for Smart Energy
  // Key establihsment tests 15.39 and 15.40.  It is only necessary for a test
  // harness device.

  // In the case of the Host, the XNCP test harness software must be loaded
  // or this cannot be done.  Since resetting frame counters is non-standard
  // and can break things, it is not part of the default NCP binary.

  resetOrAdvanceApsFrameCounter(RESET_FRAME_COUNTER);
}

void sli_zigbee_af_test_harness_advance_aps_frame_counter(void)
{
  resetOrAdvanceApsFrameCounter(ADVANCE_FRAME_COUNTER);
}

//------------------------------------------------------------------------------
