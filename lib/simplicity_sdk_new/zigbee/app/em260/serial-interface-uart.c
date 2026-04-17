/***************************************************************************//**
 * @file
 * @brief Implementation of the interface described in serial-interface.h
 * using the ASH UART protocol.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "hal/hal.h"
#if !defined(ZIGBEE_STACK_ON_HOST) && !defined(SL_ZIGBEE_TEST)
#include "stack/framework/zigbee_debug_channel.h"
#elif !defined(ZIGBEE_STACK_ON_HOST)
#include "stack/framework/debug.h"
#endif
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"
#include "em260-callbacks.h"
#include "serial/serial.h"
#include "serial-interface.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "ash-protocol.h"
#include "ash-ncp.h"
extern const uint8_t ashPort;

#ifndef UNUSED_VAR
#define UNUSED_VAR(x) (void)(x)
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#if defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)
#define IOSTREAM_INSTANCE_HEADER "sl_iostream_init_eusart_instances.h"
#elif defined(SL_CATALOG_IOSTREAM_USART_PRESENT)
#define IOSTREAM_INSTANCE_HEADER "sl_iostream_init_usart_instances.h"
#endif // SL_CATALOG_IOSTREAM_EUSART_PRESENT

#include IOSTREAM_INSTANCE_HEADER
#include "sli_iostream_uart.h"
#endif // SL_CATALOG_KERNEL_PRESENT

//------------------------------------------------------------------------------
// Global Variables

static sli_buffer_manager_buffer_t ezspBuffer = SL_ZIGBEE_NULL_MESSAGE_BUFFER;

// This points to contents of ezspBuffer.
//
// When receiving a command or sending a response, ezspBuffer is the buffer
// statically allocated by ASH.  These are allocated first thing and are
// guaranteed to be contiguous.  When sending a callback, ezspBuffer is
// a freshly allocated buffer, but for callbacks we only write within the
// first 32 bytes, so the lack of contiguity is not a concern.
uint8_t *ezspFrameContents;

static uint16_t delayTestStartTime;
static uint16_t delayTestDelayLength = 0;

//------------------------------------------------------------------------------
// Serial Interface Downwards

void serialInit(sli_zigbee_event_t* event)
{
  (void)event;

  ashStart();
}

void serialAllocateBuffers(void)
{
  ezspBuffer = ashAllocateStaticBuffers();
  sl_legacy_buffer_manager_hold_message_buffer(ezspBuffer);
  ezspFrameContents = sl_legacy_buffer_manager_message_buffer_contents(ezspBuffer);
}

sli_buffer_manager_buffer_t serialGetEzspBuffer(void)
{
  return ezspBuffer;
}

bool serialCommandReceived(void)
{
  ashSendExec();
  if (ezspBuffer != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    return true;
  }
  if (delayTestDelayLength == 0
      || (elapsedTimeInt16u(delayTestStartTime,
                            halCommonGetInt16uMillisecondTick())
          > delayTestDelayLength)) {
    delayTestDelayLength = 0;
#ifdef SL_CATALOG_KERNEL_PRESENT
    // set it to unblock while waiting for data
    sl_iostream_uart_set_read_block(sl_iostream_uart_vcom_handle, false);
#endif // SL_CATALOG_KERNEL_PRESENT
    ashReceive(&ezspBuffer);
  }
  if (ezspBuffer != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    ezspFrameContents = sl_legacy_buffer_manager_message_buffer_contents(ezspBuffer);
    return true;
  } else {
    return false;
  }
}

void serialResponseReady(bool highPriority)
{
  serialDebugTrace(); // Trace responses before randomization!
  ashSend(ezspBuffer, highPriority);
  ezspBuffer = SL_ZIGBEE_NULL_MESSAGE_BUFFER;
  ashSendExec();
}

void serialDebugTrace(void)
{
  if (ezspBuffer != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
#ifndef ZIGBEE_STACK_ON_HOST
    sli_zigbee_debug_binary_format(EM_DEBUG_EZSP, "b", ezspBuffer);
#endif
  }
}

sli_buffer_manager_buffer_t serialFillBufferFromCommand(uint8_t startIndex,
                                                        uint8_t length)
{
  sli_buffer_manager_buffer_t buffer;
  buffer = sl_legacy_buffer_manager_fill_linked_buffers(NULL, length);
  if (buffer != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    sl_legacy_buffer_manager_copy_buffer_bytes(buffer, 0, ezspBuffer, startIndex, length);
  }
  return buffer;
}

uint8_t serialGetCommandLength(void)
{
  return sl_legacy_buffer_manager_message_buffer_length(ezspBuffer);
}

void serialSetResponseLength(uint8_t data)
{
  assert(sl_legacy_buffer_manager_set_message_buffer_length(ezspBuffer, data) == SL_STATUS_OK);

  // sl_legacy_buffer_manager_set_message_buffer_length has the potential effect of setting ezspBuffer
  // to a brand new buffer. This can happen if ezspBuffer is being extended.
  // As a result of that, we'll need to refresh ezspFrameContents, which is
  // supposed to track the buffer contents of ezspBuffer
  ezspFrameContents = sl_legacy_buffer_manager_message_buffer_contents(ezspBuffer);
}

bool serialCallbackResponse(void)
{
  sli_buffer_manager_buffer_t callback = sli_legacy_buffer_manager_buffer_queue_remove_head(&callbackQueue);
  uint8_t callbackLength = sl_legacy_buffer_manager_message_buffer_length(callback);

  ezspBuffer = callback;

  ezspFrameContents = sl_legacy_buffer_manager_message_buffer_contents(ezspBuffer);
  ezspWritePointer = ezspFrameContents + callbackLength;
  return true;
}

void serialMfglibFillPacket(uint8_t *packet)
{
  uint8_t length = sl_legacy_buffer_manager_get_linked_buffers_byte(ezspBuffer, EZSP_PARAMETERS_INDEX);
  sl_legacy_buffer_manager_copy_from_linked_buffers(ezspBuffer,
                                                    EZSP_PARAMETERS_INDEX,
                                                    packet,
                                                    length + 1);
}

void serialDelayTest(uint16_t delay)
{
  delayTestStartTime = halCommonGetInt16uMillisecondTick();
  delayTestDelayLength = delay;
}

void serialPendingCallbacks(bool pending)
{
  UNUSED_VAR(pending);
}
