/***************************************************************************//**
 * @file mfglib_ipc_command_messages.c
 * @brief internal wrappers for 'mfglib' ipc commands
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from mfglib.h.  Do not manually edit
#include "stack/include/mfglib.h"
#include "stack/internal/inc/mfglib_internal_def.h"
#include "stack/internal/src/ipc/mfglib_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void mfglibInternalEnd_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgEnd.response.result = mfglibInternalEnd();
}

void mfglibInternalGetChannel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgGetChannel.response.result = mfglibInternalGetChannel();
}

void mfglibInternalGetCtune_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgGetCtune.response.result = mfglibInternalGetCtune();
}

void mfglibInternalGetOptions_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgGetOptions.response.result = mfglibInternalGetOptions();
}

void mfglibInternalGetPower_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgGetPower.response.result = mfglibInternalGetPower();
}

void mfglibInternalGetSynOffset_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgGetSynOffset.response.result = mfglibInternalGetSynOffset();
}

void mfglibInternalSendPacket_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgSendPacket.response.result = mfglibInternalSendPacket(msg->data.mfgSendPacket.request.packet,
                                                                     msg->data.mfgSendPacket.request.repeat);
}

void mfglibInternalSetChannel_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgSetChannel.response.result = mfglibInternalSetChannel(msg->data.mfgSetChannel.request.chan);
}

void mfglibInternalSetCtune_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgSetCtune.response.result = mfglibInternalSetCtune(msg->data.mfgSetCtune.request.ctune);
}

void mfglibInternalSetOptions_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgSetOptions.response.result = mfglibInternalSetOptions(msg->data.mfgSetOptions.request.options);
}

void mfglibInternalSetPower_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgSetPower.response.result = mfglibInternalSetPower(msg->data.mfgSetPower.request.txPowerMode,
                                                                 msg->data.mfgSetPower.request.power);
}

void mfglibInternalSetSynOffset_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  mfglibInternalSetSynOffset(msg->data.mfgSetSynOffset.request.synOffset);
}

void mfglibInternalStart_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgStart.response.result = mfglibInternalStart(msg->data.mfgStart.request.mfglibRxCallback);
}

void mfglibInternalStartStream_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgStartStream.response.result = mfglibInternalStartStream();
}

void mfglibInternalStartTone_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgStartTone.response.result = mfglibInternalStartTone();
}

void mfglibInternalStopStream_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgStopStream.response.result = mfglibInternalStopStream();
}

void mfglibInternalStopTone_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.mfgStopTone.response.result = mfglibInternalStopTone();
}

// public entrypoints

sl_status_t mfglibEnd(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalEnd_process_ipc_command, &msg);

  return msg.data.mfgEnd.response.result;
}

uint8_t mfglibGetChannel(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalGetChannel_process_ipc_command, &msg);

  return msg.data.mfgGetChannel.response.result;
}

uint16_t mfglibGetCtune(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalGetCtune_process_ipc_command, &msg);

  return msg.data.mfgGetCtune.response.result;
}

uint8_t mfglibGetOptions(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalGetOptions_process_ipc_command, &msg);

  return msg.data.mfgGetOptions.response.result;
}

int8_t mfglibGetPower(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalGetPower_process_ipc_command, &msg);

  return msg.data.mfgGetPower.response.result;
}

int8_t mfglibGetSynOffset(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalGetSynOffset_process_ipc_command, &msg);

  return msg.data.mfgGetSynOffset.response.result;
}

sl_status_t mfglibSendPacket(uint8_t *packet,
                             uint16_t repeat)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if ((packet[0] + 1) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector packet length exceeds expected maximum
  }

  memmove(msg.data.mfgSendPacket.request.packet, packet, sizeof(uint8_t) * (packet[0] + 1));
  msg.data.mfgSendPacket.request.repeat = repeat;
  sli_zigbee_send_ipc_cmd(mfglibInternalSendPacket_process_ipc_command, &msg);

  if ((packet[0] + 1) > MAX_IPC_VEC_ARG_CAPACITY) {
    assert(false); // "vector packet length exceeds expected maximum
  }

  memmove(packet, msg.data.mfgSendPacket.request.packet, sizeof(uint8_t) * (packet[0] + 1));
  return msg.data.mfgSendPacket.response.result;
}

sl_status_t mfglibSetChannel(uint8_t chan)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.mfgSetChannel.request.chan = chan;
  sli_zigbee_send_ipc_cmd(mfglibInternalSetChannel_process_ipc_command, &msg);

  return msg.data.mfgSetChannel.response.result;
}

sl_status_t mfglibSetCtune(uint16_t ctune)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.mfgSetCtune.request.ctune = ctune;
  sli_zigbee_send_ipc_cmd(mfglibInternalSetCtune_process_ipc_command, &msg);

  return msg.data.mfgSetCtune.response.result;
}

sl_status_t mfglibSetOptions(uint8_t options)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.mfgSetOptions.request.options = options;
  sli_zigbee_send_ipc_cmd(mfglibInternalSetOptions_process_ipc_command, &msg);

  return msg.data.mfgSetOptions.response.result;
}

sl_status_t mfglibSetPower(uint16_t txPowerMode,
                           int8_t power)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.mfgSetPower.request.txPowerMode = txPowerMode;
  msg.data.mfgSetPower.request.power = power;
  sli_zigbee_send_ipc_cmd(mfglibInternalSetPower_process_ipc_command, &msg);

  return msg.data.mfgSetPower.response.result;
}

void mfglibSetSynOffset(int8_t synOffset)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.mfgSetSynOffset.request.synOffset = synOffset;
  sli_zigbee_send_ipc_cmd(mfglibInternalSetSynOffset_process_ipc_command, &msg);
}

sl_status_t mfglibStart(MfglibRxCallback mfglibRxCallback)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };
  msg.data.mfgStart.request.mfglibRxCallback = mfglibRxCallback;
  sli_zigbee_send_ipc_cmd(mfglibInternalStart_process_ipc_command, &msg);

  return msg.data.mfgStart.response.result;
}

sl_status_t mfglibStartStream(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalStartStream_process_ipc_command, &msg);

  return msg.data.mfgStartStream.response.result;
}

sl_status_t mfglibStartTone(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalStartTone_process_ipc_command, &msg);

  return msg.data.mfgStartTone.response.result;
}

sl_status_t mfglibStopStream(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalStopStream_process_ipc_command, &msg);

  return msg.data.mfgStopStream.response.result;
}

sl_status_t mfglibStopTone(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(mfglibInternalStopTone_process_ipc_command, &msg);

  return msg.data.mfgStopTone.response.result;
}
