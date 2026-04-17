/***************************************************************************//**
 * @file
 * @brief A sample of custom EZSP protocol.
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

#include "custom-ezsp.h"

#include "app/framework/include/af.h"
#include "app/xncp/xncp-sample-custom-ezsp-protocol.h"

void sl_zigbee_ezsp_custom_frame_handler(uint8_t payloadLength, uint8_t* payload)
{
  uint8_t commandId;
  assert(payloadLength > 0);

  commandId = payload[0];

  switch (commandId) {
    case SL_ZIGBEE_CUSTOM_EZSP_CALLBACK_REPORT:
      sl_zigbee_af_core_println("Got report, count=0x%02X",
                                HIGH_LOW_TO_INT(payload[2],
                                                payload[1]));
      break;
  }
}
