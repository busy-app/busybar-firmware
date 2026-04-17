/***************************************************************************//**
 * @brief
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

// *** Generated file. Do not edit! ***
// vNCP Version: 1.0

#include "sl-connect-assert.h"
#include "stack/include/ember.h"

#include "csp-format.h"
#include "csp-command-utils.h"
#include "app_framework_callback.h"
#include "callback_dispatcher.h"
#include "csp-api-enum-gen.h"

void emberStackStatusHandler(EmberStatus status)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_STACK_STATUS_HANDLER_IPC_COMMAND_ID,
                                          "u",
                                          status);
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberChildJoinHandler(EmberNodeType nodeType,
                           EmberNodeId nodeId)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_CHILD_JOIN_HANDLER_IPC_COMMAND_ID,
                                          "uv",
                                          nodeType,
                                          nodeId);
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberRadioNeedsCalibratingHandler(void)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_RADIO_NEEDS_CALIBRATING_HANDLER_IPC_COMMAND_ID,
                                          "");
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberMessageSentHandler(EmberStatus status,
                             EmberOutgoingMessage *message)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_MESSAGE_SENT_HANDLER_IPC_COMMAND_ID,
                                          "uuvuulbuw",
                                          status,
                                          message->options,
                                          message->destination,
                                          message->endpoint,
                                          message->tag,
                                          message->length,
                                          message->payload,
                                          message->length,
                                          message->ackRssi,
                                          message->timestamp);
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberIncomingMessageHandler(EmberIncomingMessage *message)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_INCOMING_MESSAGE_HANDLER_IPC_COMMAND_ID,
                                          "uvuulbwu",
                                          message->options,
                                          message->source,
                                          message->endpoint,
                                          message->rssi,
                                          message->length,
                                          message->payload,
                                          message->length,
                                          message->timestamp,
                                          message->lqi);
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberIncomingMacMessageHandler(EmberIncomingMacMessage *message)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_INCOMING_MAC_MESSAGE_HANDLER_IPC_COMMAND_ID,
                                          "uvbuvbuvvuuuuwlbw",
                                          message->options,
                                          message->macFrame.srcAddress.addr.shortAddress,
                                          message->macFrame.srcAddress.addr.longAddress,
                                          EUI64_SIZE,
                                          message->macFrame.srcAddress.mode,
                                          message->macFrame.dstAddress.addr.shortAddress,
                                          message->macFrame.dstAddress.addr.longAddress,
                                          EUI64_SIZE,
                                          message->macFrame.dstAddress.mode,
                                          message->macFrame.srcPanId,
                                          message->macFrame.dstPanId,
                                          message->macFrame.srcPanIdSpecified,
                                          message->macFrame.dstPanIdSpecified,
                                          message->rssi,
                                          message->lqi,
                                          message->frameCounter,
                                          message->length,
                                          message->payload,
                                          message->length,
                                          message->timestamp);
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberMacMessageSentHandler(EmberStatus status,
                                EmberOutgoingMacMessage *message)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_MAC_MESSAGE_SENT_HANDLER_IPC_COMMAND_ID,
                                          "uuvbuvbuvvuuuwlbuw",
                                          status,
                                          message->options,
                                          message->macFrame.srcAddress.addr.shortAddress,
                                          message->macFrame.srcAddress.addr.longAddress,
                                          EUI64_SIZE,
                                          message->macFrame.srcAddress.mode,
                                          message->macFrame.dstAddress.addr.shortAddress,
                                          message->macFrame.dstAddress.addr.longAddress,
                                          EUI64_SIZE,
                                          message->macFrame.dstAddress.mode,
                                          message->macFrame.srcPanId,
                                          message->macFrame.dstPanId,
                                          message->macFrame.srcPanIdSpecified,
                                          message->macFrame.dstPanIdSpecified,
                                          message->tag,
                                          message->frameCounter,
                                          message->length,
                                          message->payload,
                                          message->length,
                                          message->ackRssi,
                                          message->timestamp);
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberIncomingBeaconHandler(EmberPanId panId,
                                EmberMacAddress *source,
                                int8_t rssi,
                                bool permitJoining,
                                uint8_t beaconFieldsLength,
                                uint8_t *beaconFields,
                                uint8_t beaconPayloadLength,
                                uint8_t *beaconPayload)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_INCOMING_BEACON_HANDLER_IPC_COMMAND_ID,
                                          "vvbuuuubub",
                                          panId,
                                          source->addr.shortAddress,
                                          source->addr.longAddress,
                                          EUI64_SIZE,
                                          source->mode,
                                          rssi,
                                          permitJoining,
                                          beaconFieldsLength,
                                          beaconFields,
                                          beaconFieldsLength,
                                          beaconPayloadLength,
                                          beaconPayload,
                                          beaconPayloadLength);
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberActiveScanCompleteHandler(void)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_ACTIVE_SCAN_COMPLETE_HANDLER_IPC_COMMAND_ID,
                                          "");
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberEnergyScanCompleteHandler(int8_t mean,
                                    int8_t min,
                                    int8_t max,
                                    uint16_t variance)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_ENERGY_SCAN_COMPLETE_HANDLER_IPC_COMMAND_ID,
                                          "uuuv",
                                          mean,
                                          min,
                                          max,
                                          variance);
  sendCallbackCommand(callbackCommandBuffer, length);
}
void emberFrequencyHoppingStartClientCompleteHandler(EmberStatus status)
{
  uint8_t *callbackCommandBuffer = allocateCallbackCommandPointer();
  uint16_t length = formatResponseCommand(callbackCommandBuffer,
                                          MAX_STACK_API_COMMAND_SIZE,
                                          EMBER_FREQUENCY_HOPPING_START_CLIENT_COMPLETE_HANDLER_IPC_COMMAND_ID,
                                          "u",
                                          status);
  sendCallbackCommand(callbackCommandBuffer, length);
}
