/***************************************************************************//**
 * @file
 * @brief This is code for a test harness running on a host connected to an NCP.
 *   It allows the host to control non-standard behavior on the NCP to test that a
 *   DUT device is behaving in a compliant manner.  Put another way, it lets the host
 *   do wonky stuff that is normally only accessible through Ember-internal 'em' calls
 *   when on the SOC platform.
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

#include "stack/include/sl_zigbee.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"
#include "app/xncp-test-harness/xncp-test-harness.h"

#include "hal/micro/token.h"

// -----------------------------------------------------------------------------
// External declarations

sl_status_t sl_zigbee_af_xncp_send_custom_ezsp_message(uint8_t length, uint8_t *payload);
void sli_zigbee_reset_aps_frame_counter(void);
void sli_zigbee_test_harness_advance_aps_frame_counter(void);

// -----------------------------------------------------------------------------

void sl_zigbee_af_xncp_get_xncp_information(uint16_t *manufacturerId,
                                            uint16_t *versionNumber)
{
  *versionNumber = SL_ZIGBEE_XNCP_TEST_HARNESS_VERSION_NUMBER;
  *manufacturerId = SL_ZIGBEE_MANUFACTURER_ID;
}

// -----------------------------------------------------------------------------
// Init

void sl_zigbee_af_main_init_cb(void)
{
}

// -----------------------------------------------------------------------------
// Handlers and callbacks
bool sl_zigbee_af_xncp_permit_ncp_to_host_frame_cb(uint16_t frameId,
                                                   uint8_t payloadLength,
                                                   uint8_t *payload)
{
  UNUSED_VAR(frameId);
  UNUSED_VAR(payloadLength);
  UNUSED_VAR(payload);
  return true;
}

bool sl_zigbee_af_xncp_permit_host_to_ncp_frame_cb(uint16_t frameId,
                                                   uint8_t payloadLength,
                                                   uint8_t *payload)
{
  UNUSED_VAR(frameId);
  UNUSED_VAR(payloadLength);
  UNUSED_VAR(payload);
  return true;
}

// The returned status is always the first byte of the EZSP response.
sl_status_t sl_zigbee_af_xncp_incoming_custom_frame_cb(uint8_t messageLength,
                                                       uint8_t *messagePayload,
                                                       uint8_t *replyPayloadLength,
                                                       uint8_t *replyPayload)
{
  UNUSED_VAR(messageLength);
  UNUSED_VAR(replyPayloadLength);
  UNUSED_VAR(replyPayload);

  // In the sample custom EZSP protocol, the command ID is always the first byte
  // of a custom EZSP command.
  uint8_t commandId = messagePayload[0];

  switch (commandId) {
    case SL_ZIGBEE_XNCP_TEST_HARNESS_COMMAND_RESET_APS_FRAME_COUNTER: {
      sli_zigbee_reset_aps_frame_counter();
      return SL_STATUS_OK;
    }
    case SL_ZIGBEE_XNCP_TEST_HARNESS_COMMAND_ADVANCE_APS_FRAME_COUNTER: {
      sli_zigbee_test_harness_advance_aps_frame_counter();
      return SL_STATUS_OK;
    }
    default:
      break;
  }

  return SL_STATUS_INVALID_STATE;
}

bool sl_zigbee_af_xncp_incoming_message_cb(sl_zigbee_incoming_message_type_t type,
                                           sl_zigbee_aps_frame_t *apsFrame,
                                           sl_zigbee_rx_packet_info_t *packetInfo,
                                           uint8_t messageLength,
                                           uint8_t *message)
{
  UNUSED_VAR(type);
  UNUSED_VAR(apsFrame);
  UNUSED_VAR(packetInfo);
  UNUSED_VAR(messageLength);
  UNUSED_VAR(message);
  return false;
}
