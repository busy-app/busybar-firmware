/***************************************************************************//**
 * @file
 * @brief Implementation of the interface described in serial-interface.h
 * using the ASH UART protocol.
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

#include "stack/include/sl_zigbee_types.h"

#include "hal/hal.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#include "app/util/ezsp/serial-interface.h"
#include "ash-protocol.h"
#include "ash-common.h"
#include "app/ezsp-host/ash/ash-host.h"
#include "app/ezsp-host/ezsp-host-io.h"
#include "app/ezsp-host/ezsp-host-priv.h"
#include "app/ezsp-host/ezsp-host-queues.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"

#define elapsedTimeInt16u(oldTime, newTime) \
  ((uint16_t) ((uint16_t)(newTime) - (uint16_t)(oldTime)))

//------------------------------------------------------------------------------
// Global Variables

static bool waitingForResponse = false;
static uint16_t waitStartTime;
#define WAIT_FOR_RESPONSE_TIMEOUT (ASH_MAX_TIMEOUTS * sli_ash_host_get_config(ackTimeMax))

static uint8_t ezspFrameLength;
uint8_t *ezspFrameLengthLocation = &ezspFrameLength;
static uint8_t ezspFrameContentsStorage[EZSP_MAX_FRAME_LENGTH];
uint8_t *ezspFrameContents = ezspFrameContentsStorage;

//------------------------------------------------------------------------------
// Serial Interface Downwards

sl_zigbee_ezsp_status_t sl_zigbee_ezsp_init(void)
{
  sl_zigbee_ezsp_status_t status;
  uint8_t i;
  for (i = 0; i < 5; i++) {
    status = ashResetNcp();
    if (status != SL_ZIGBEE_EZSP_SUCCESS) {
      return status;
    }
    status = ashStart();
    if (status == SL_ZIGBEE_EZSP_SUCCESS) {
      return status;
    }
  }
  return status;
}

bool sl_zigbee_ezsp_callback_pending(void)
{
  return (ncpSleepEnabled && ncpHasCallbacks);
}

void sl_zigbee_ezsp_close(void)
{
  sl_zigbee_ezsp_serial_close();
}

static bool checkConnection(void)
{
  bool connected = ashIsConnected();
  if (!connected) {
    // Attempt to restore the connection. This will reset the EM260.
    sl_zigbee_ezsp_close();
    sl_zigbee_ezsp_init();
  }

  return connected;
}

uint8_t serialPendingResponseCount(void)
{
  return sl_zigbee_ezsp_queue_length(&rxQueue);
}

WEAK_TEST sl_zigbee_ezsp_status_t serialResponseReceived(void)
{
  sl_zigbee_ezsp_status_t status;
  EzspBuffer *buffer;
  EzspBuffer *dropBuffer = NULL;
  if (!checkConnection()) {
    sl_zigbee_ezsp_trace_ezsp_verbose("serialResponseReceived(): SL_ZIGBEE_EZSP_NOT_CONNECTED");
    return SL_ZIGBEE_EZSP_NOT_CONNECTED;
  }
  ashSendExec();
  status = ashReceiveExec();
  if (status != SL_ZIGBEE_EZSP_SUCCESS
      && status != SL_ZIGBEE_EZSP_ASH_IN_PROGRESS
      && status != SL_ZIGBEE_EZSP_NO_RX_DATA) {
    sl_zigbee_ezsp_trace_ezsp_verbose("serialResponseReceived(): ashReceiveExec(): 0x%02X",
                                      status);
    return status;
  }
  if (waitingForResponse
      && elapsedTimeInt16u(waitStartTime, halCommonGetInt16uMillisecondTick())
      > WAIT_FOR_RESPONSE_TIMEOUT) {
    waitingForResponse = false;
    sl_zigbee_ezsp_trace_ezsp_frame_id("no response", ezspFrameContents);
    sl_zigbee_ezsp_trace_ezsp_verbose("serialResponseReceived(): SL_ZIGBEE_EZSP_ERROR_NO_RESPONSE");
    return SL_ZIGBEE_EZSP_ERROR_NO_RESPONSE;
  }
  status = SL_ZIGBEE_EZSP_NO_RX_DATA;
  buffer = ezspQueuePrecedingEntry(&rxQueue, NULL);
  while (buffer != NULL) {
    // While we are waiting for a response to a command, we use the asynch
    // callback flag to ignore asynchronous callbacks. This allows our caller
    // to assume that no callbacks will appear between sending a command and
    // receiving its response.
    if (waitingForResponse
        && (buffer->data[EZSP_FRAME_CONTROL_INDEX]
            & EZSP_FRAME_CONTROL_ASYNCH_CB)
        ) {
      if (sl_zigbee_ezsp_free_list_length(&rxFree) == 0) {
        dropBuffer = buffer;
      }
      buffer = ezspQueuePrecedingEntry(&rxQueue, buffer);
    } else {
      sl_zigbee_ezsp_trace_ezsp_verbose("serialResponseReceived(): ID=0x%02X Seq=0x%02X Buffer=%u",
                                        buffer->data[EZSP_FRAME_ID_INDEX],
                                        buffer->data[EZSP_SEQUENCE_INDEX],
                                        buffer);
      ezspRemoveQueueEntry(&rxQueue, buffer);
      (void) memcpy(ezspFrameContents, buffer->data, buffer->len);
      sl_zigbee_ezsp_trace_ezsp_frame_id("got response", buffer->data);
      ezspFrameLength = buffer->len;
      sl_zigbee_ezsp_free_buffer(&rxFree, buffer);
      sl_zigbee_ezsp_trace_ezsp_verbose("serialResponseReceived(): sl_zigbee_ezsp_free_buffer(): %u", buffer);
      buffer = NULL;
      status = SL_ZIGBEE_EZSP_SUCCESS;
      waitingForResponse = false;
    }
  }
  if (dropBuffer != NULL) {
    ezspRemoveQueueEntry(&rxQueue, dropBuffer);
    sl_zigbee_ezsp_free_buffer(&rxFree, dropBuffer);
    sl_zigbee_ezsp_trace_ezsp_frame_id("dropping", dropBuffer->data);
    sl_zigbee_ezsp_trace_ezsp_verbose("serialResponseReceived(): sl_zigbee_ezsp_free_buffer(): drop %u", dropBuffer);
    sl_zigbee_ezsp_trace_ezsp_verbose("serialResponseReceived(): sl_zigbee_ezsp_error_handler(): SL_ZIGBEE_EZSP_ERROR_QUEUE_FULL");
    sl_zigbee_ezsp_error_handler(SL_ZIGBEE_EZSP_ERROR_QUEUE_FULL);
  }
  return status;
}

WEAK_TEST sl_zigbee_ezsp_status_t serialSendCommand(void)
{
  sl_zigbee_ezsp_status_t status;
  if (!checkConnection()) {
    sl_zigbee_ezsp_trace_ezsp_verbose("serialSendCommand(): SL_ZIGBEE_EZSP_NOT_CONNECTED");
    return SL_ZIGBEE_EZSP_NOT_CONNECTED;
  }

  sl_zigbee_ezsp_trace_ezsp_frame_id("send command", ezspFrameContents);
  status = ashSend(ezspFrameLength, ezspFrameContents);
  if (status != SL_ZIGBEE_EZSP_SUCCESS) {
    sl_zigbee_ezsp_trace_ezsp_verbose("serialSendCommand(): ashSend(): 0x%02X", status);
    return status;
  }
  waitingForResponse = true;
  sl_zigbee_ezsp_trace_ezsp_verbose("serialSendCommand(): ID=0x%02X Seq=0x%02X",
                                    ezspFrameContents[EZSP_FRAME_ID_INDEX],
                                    ezspFrameContents[EZSP_SEQUENCE_INDEX]);
  waitStartTime = halCommonGetInt16uMillisecondTick();
  return status;
}

uint8_t serialGetCommandLength(void)
{
  return *ezspFrameLengthLocation;
}

bool sl_zigbee_ezsp_ok_to_sleep(void)
{
  return
    (ncpSleepEnabled
     && (ezspSleepMode != EZSP_FRAME_CONTROL_IDLE)
     && ashOkToSleep() );
}

void sl_zigbee_ezsp_enable_ncp_sleep(bool enable)
{
  uint8_t enableval = (uint8_t) enable;
  sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_UART_SYNCH_CALLBACKS, 1, &enableval);
  ncpSleepEnabled = enable;
}

void sl_zigbee_ezsp_wake_up(void)
{
  sl_zigbee_ezsp_status_t status = ashWakeUpNcp(true);
  while (status == SL_ZIGBEE_EZSP_ASH_IN_PROGRESS) {
    simulatedTimePasses();
    status = ashWakeUpNcp(false);
  }
}

// Offset from start of sl_zigbee_ezsp_echo frame to data payload - skip over length byte
#define ECHO_DATA_OFFSET      (EZSP_EXTENDED_PARAMETERS_INDEX + 1)
// Maximum length of sl_zigbee_ezsp_echo data
#define ECHO_MAX_DATA_LENGTH  (EZSP_MAX_FRAME_LENGTH - ECHO_DATA_OFFSET)

sl_zigbee_ezsp_status_t serialTestFlowControl(void)
{
  uint8_t buffer[ECHO_MAX_DATA_LENGTH];
  uint8_t rnum;

  // Construct the largest possible echo command frame, in which every data byte
  // is preceded by an escape byte, by setting all data to the ASH_FLAG value.
  // If data randomization is enabled, the data array must be XOR'ed with the
  // ASH pseudo-random sequence. Note that the random sequence must be cycled
  // enough times to skip over the preceding bytes in the EZSP frame.
  rnum = ashRandomizeArray(0, buffer, ECHO_DATA_OFFSET);
  (void) memset(buffer, ASH_FLAG, ECHO_MAX_DATA_LENGTH);
  if (sli_ash_host_get_config(randomize)) {
    (void)ashRandomizeArray(rnum, buffer, ECHO_MAX_DATA_LENGTH);
  }

  // Use delay command to have the NCP pause before reading the next command,
  // then see if the frame had to be retransmitted due to buffer overflow.
  sl_zigbee_ezsp_delay_test(100);   // wait 100 milliseconds before reading next command
  ashCount.txReDataFrames = 0;
  (void)sl_zigbee_ezsp_echo(ECHO_MAX_DATA_LENGTH, buffer, buffer);
  sl_zigbee_ezsp_tick();
  return (ashCount.txReDataFrames == 0) ? SL_ZIGBEE_EZSP_SUCCESS
         : SL_ZIGBEE_EZSP_HOST_FATAL_ERROR;
}
