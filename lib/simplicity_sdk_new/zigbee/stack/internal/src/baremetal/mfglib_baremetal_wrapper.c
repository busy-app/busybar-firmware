/***************************************************************************//**
 * @file mfglib_baremetal_wrapper.c
 * @brief internal implementations for 'mfglib' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

sl_status_t mfglibEnd(void)
{
  return mfglibInternalEnd();
}

uint8_t mfglibGetChannel(void)
{
  return mfglibInternalGetChannel();
}

uint16_t mfglibGetCtune(void)
{
  return mfglibInternalGetCtune();
}

uint8_t mfglibGetOptions(void)
{
  return mfglibInternalGetOptions();
}

int8_t mfglibGetPower(void)
{
  return mfglibInternalGetPower();
}

int8_t mfglibGetSynOffset(void)
{
  return mfglibInternalGetSynOffset();
}

sl_status_t mfglibSendPacket(uint8_t *packet,
                             uint16_t repeat)
{
  return mfglibInternalSendPacket(packet,
                                  repeat);
}

sl_status_t mfglibSetChannel(uint8_t chan)
{
  return mfglibInternalSetChannel(chan);
}

sl_status_t mfglibSetCtune(uint16_t ctune)
{
  return mfglibInternalSetCtune(ctune);
}

sl_status_t mfglibSetOptions(uint8_t options)
{
  return mfglibInternalSetOptions(options);
}

sl_status_t mfglibSetPower(uint16_t txPowerMode,
                           int8_t power)
{
  return mfglibInternalSetPower(txPowerMode,
                                power);
}

void mfglibSetSynOffset(int8_t synOffset)
{
  mfglibInternalSetSynOffset(synOffset);
}

sl_status_t mfglibStart(MfglibRxCallback mfglibRxCallback)
{
  return mfglibInternalStart(mfglibRxCallback);
}

sl_status_t mfglibStartStream(void)
{
  return mfglibInternalStartStream();
}

sl_status_t mfglibStartTone(void)
{
  return mfglibInternalStartTone();
}

sl_status_t mfglibStopStream(void)
{
  return mfglibInternalStopStream();
}

sl_status_t mfglibStopTone(void)
{
  return mfglibInternalStopTone();
}
