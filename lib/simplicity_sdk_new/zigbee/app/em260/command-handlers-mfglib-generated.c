/*****************************************************************************/
/**
 * Copyright 2021 Silicon Laboratories, Inc.
 *
 *****************************************************************************/
//
// *** Generated file. Do not edit! ***
//
// Description: Handlers for the EZSP frames that directly correspond to Ember
// API calls.

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/internal/inc/internal-defs-patch.h"
#include "ezsp-enum.h"
#include "app/em260/command-context.h"
#include "stack/include/cbke-crypto-engine.h"
#include "stack/include/zigbee-security-manager.h"
#include "stack/internal/inc/mfglib_internal_def.h"
#include "stack/include/binding-table.h"
#include "stack/include/message.h"
#include "stack/include/mac-layer.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"
#include "app/em260/command-handlers-cbke.h"
#include "app/em260/command-handlers-binding.h"
#include "app/em260/command-handlers-mfglib.h"
#include "app/em260/command-handlers-security.h"
#include "app/em260/command-handlers-zll.h"
#include "app/em260/command-handlers-zigbee-pro.h"
#include "child.h"
#include "message.h"
#include "zll-api.h"
#include "security.h"
#include "stack-info.h"
#include "network-formation.h"
#include "zigbee-device-stack.h"
#include "sl_zigbee_duty_cycle.h"
#include "multi-phy.h"
#include "stack/include/gp-sink-table.h"
#include "stack/include/gp-proxy-table.h"
#include "stack/include/source-route.h"
#include "stack/include/multi-network.h"
#include "stack/include/sl_zigbee_dhc.h"
#include "app/framework/plugin/dhc/dhc-ncp.h"

bool sli_zigbee_af_process_ezsp_command_mfglib(uint16_t commandId)
{
  switch (commandId) {
//------------------------------------------------------------------------------

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START: {
      sl_status_t status;
      bool rxCallback;
      rxCallback = fetchInt8u();
      status = sl_zigbee_af_ezsp_mfglib_internal_start_command_cb(rxCallback);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_END: {
      sl_status_t status;
      status = mfglibInternalEnd();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START_TONE: {
      sl_status_t status;
      status = mfglibInternalStartTone();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_STOP_TONE: {
      sl_status_t status;
      status = mfglibInternalStopTone();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START_STREAM: {
      sl_status_t status;
      status = mfglibInternalStartStream();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_STOP_STREAM: {
      sl_status_t status;
      status = mfglibInternalStopStream();
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SEND_PACKET: {
      sl_status_t status;
      uint8_t packetLength;
      uint8_t *packetContents;
      packetLength = fetchInt8u();
      packetContents = (uint8_t *)fetchInt8uPointer(packetLength);
      status = sl_zigbee_af_ezsp_mfglib_internal_send_packet_command_cb(packetLength, packetContents);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SET_CHANNEL: {
      sl_status_t status;
      uint8_t channel;
      channel = fetchInt8u();
      status = mfglibInternalSetChannel(channel);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_GET_CHANNEL: {
      uint8_t channel;
      channel = mfglibInternalGetChannel();
      appendInt8u(channel);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SET_POWER: {
      sl_status_t status;
      uint16_t txPowerMode;
      int8_t power;
      txPowerMode = fetchInt16u();
      power = fetchInt8();
      status = mfglibInternalSetPower(txPowerMode, power);
      appendInt32u(status);
      break;
    }

    case SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_GET_POWER: {
      int8_t power;
      power = mfglibInternalGetPower();
      appendInt8(power);
      break;
    }

//------------------------------------------------------------------------------

    default: {
      return false;
    }
  }

  return true;
}
