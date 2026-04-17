/***************************************************************************//**
 * @file
 * @brief EZSP host queue functions
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

////////////////////////////////////////////////////////////////////////////////
//                                  CAUTION                                   //
//        Before these functions are used in a multi-threaded application,    //
//        they must be modified to create critical regions to prevent         //
//        data corruption that could result from overlapping accesses.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include PLATFORM_HEADER
#include <string.h>
#include "stack/include/sl_zigbee_types.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp-host-configuration-defaults.h"
#include "app/ezsp-host/ezsp-host-common.h"
#include "app/ezsp-host/ezsp-host-ui.h"
#include "app/ezsp-host/ezsp-host-io.h"
#include "app/ezsp-host/ezsp-host-queues.h"

//------------------------------------------------------------------------------
// Preprocessor definitions

//------------------------------------------------------------------------------
// Global Variables

EzspQueue txQueue;
EzspQueue reTxQueue;
EzspQueue rxQueue;
EzspFreeList txFree;
EzspFreeList rxFree;

//------------------------------------------------------------------------------
// Local Variables

static EzspBuffer ezspTxPool[TX_POOL_BUFFERS];
static EzspBuffer ezspRxPool[EZSP_HOST_RX_POOL_SIZE];

//------------------------------------------------------------------------------
// Forward Declarations

//#define EZSP_QUEUE_TEST
#ifdef EZSP_QUEUE_TEST
static void ezspQueueTest(void);
static uint32_t ezspInternalQueueTest(void);
#endif

//------------------------------------------------------------------------------
// Queue functions

// Initialize all queues to empty, and link all buffers into the free lists.
void sl_zigbee_ezsp_init_queues(void)
{
  EzspBuffer *buffer;

  txQueue.tail = NULL;
  reTxQueue.tail = NULL;
  txFree.link = NULL;
  for (buffer = ezspTxPool; buffer < &ezspTxPool[TX_POOL_BUFFERS]; buffer++) {
    sl_zigbee_ezsp_free_buffer(&txFree, buffer);
  }

  rxQueue.tail = NULL;
  rxFree.link = NULL;
  for (buffer = ezspRxPool;
       buffer < &ezspRxPool[EZSP_HOST_RX_POOL_SIZE];
       buffer++) {
    sl_zigbee_ezsp_free_buffer(&rxFree, buffer);
  }

#ifdef EZSP_QUEUE_TEST
  ezspQueueTest();
#endif
}

// Add a buffer to a free list
void sl_zigbee_ezsp_free_buffer(EzspFreeList *list, EzspBuffer *buffer)
{
  if (buffer && list) {
    buffer->link = list->link;
    list->link = buffer;
  } else {
    sl_zigbee_ezsp_trace_event("Called sl_zigbee_ezsp_free_buffer with NULL list or NULL buffer\r\n");
    assert(false);
  }
}

// Get a buffer from the free list
EzspBuffer *ezspAllocBuffer(EzspFreeList *list)
{
  EzspBuffer *buffer;

  buffer = list->link;
  if (buffer != NULL) {
    list->link = buffer->link;
    buffer->len = 0;
    (void) memset(buffer->data, 0, EZSP_MAX_FRAME_LENGTH);
  }
  return buffer;
}

// Remove the buffer at the head of a queue
EzspBuffer *ezspRemoveQueueHead(EzspQueue *queue)
{
  EzspBuffer *head, *prev;

  head = queue->tail;
  if (head == NULL) {
    sl_zigbee_ezsp_trace_event("Tried to remove head from an empty queue\r\n");
    assert(false);
    return NULL;
  }
  if (head->link == NULL) {
    queue->tail = NULL;
  } else {
    do {
      prev = head;
      head = head->link;
    } while (head->link != NULL);
    prev->link = NULL;
  }
  return head;
}

// Get a pointer to the buffer at the head of a queue
EzspBuffer *ezspQueueHead(EzspQueue *queue)
{
  EzspBuffer *head;

  head = queue->tail;
  if (head == NULL) {
    sl_zigbee_ezsp_trace_event("Tried to access head in an empty queue\r\n");
    assert(false);
  }
  if (head != NULL) {
    while (head->link != NULL)
      head = head->link;
  }
  return head;
}

// Get a pointer to the Nth entry in a queue (the tail corresponds to N = 1)
EzspBuffer *ezspQueueNthEntry(EzspQueue *queue, uint8_t n)
{
  EzspBuffer *buffer;

  if (n == 0) {
    sl_zigbee_ezsp_trace_event("Asked for 0th element in queue\r\n");
    assert(false);
  }
  buffer = queue->tail;
  while (--n) {
    if (buffer == NULL) {
      sl_zigbee_ezsp_trace_event("Less than N entries in queue\r\n");
      assert(false);
      return NULL;
    }
    buffer = buffer->link;
  }
  return buffer;
}

// Get a pointer to the entry before the specified entry (closer to the tail).
// If the buffer specified is NULL, the head entry is returned.
// If the buffer specified is the tail, NULL is returned;
EzspBuffer *ezspQueuePrecedingEntry(EzspQueue *queue, EzspBuffer *buffer)
{
  EzspBuffer *ptr;

  ptr = queue->tail;
  if (ptr == buffer) {
    return NULL;
  } else {
    do {
      if (ptr->link == buffer) {
        return ptr;
      }
      ptr = ptr->link;
    } while (ptr != NULL);
    sl_zigbee_ezsp_trace_event("Buffer not in queue\r\n");
    assert(false);
    return NULL;
  }
}

// Remove the specified entry from a queue, return a pointer to the preceding
// entry (if any).
EzspBuffer *ezspRemoveQueueEntry(EzspQueue *queue, EzspBuffer *buffer)
{
  EzspBuffer *ptr;

  ptr = ezspQueuePrecedingEntry(queue, buffer);
  if (ptr != NULL) {
    ptr->link = buffer->link;
  } else {
    queue->tail = buffer->link;
  }
  return ptr;
}

// Get the number of buffers in a queue
uint8_t sl_zigbee_ezsp_queue_length(EzspQueue *queue)
{
  EzspBuffer *head;
  uint8_t count;

  head = queue->tail;
  for (count = 0; head != NULL; count++) {
    head = head->link;
  }
  return count;
}

// Get the number of buffers in a free list
uint8_t sl_zigbee_ezsp_free_list_length(EzspFreeList *list)
{
  EzspBuffer *next;
  uint8_t count;

  next = list->link;
  for (count = 0; next != NULL; count++) {
    next = next->link;
  }
  return count;
}

// Add a buffer to the tail of a queue
void sl_zigbee_ezsp_add_queue_tail(EzspQueue *queue, EzspBuffer *buffer)
{
  if (queue && buffer) {
    buffer->link = queue->tail;
    queue->tail = buffer;
  } else {
    sl_zigbee_ezsp_trace_event("Called sl_zigbee_ezsp_add_queue_tail with NULL queue or NULL buffer"
                               "\r\n");
    assert(false);
  }
}

// Return whether or not the queue is empty
bool sl_zigbee_ezsp_queue_is_empty(EzspQueue *queue)
{
  return (queue->tail == NULL);
}

//------------------------------------------------------------------------------
// Test functions

#ifdef EZSP_QUEUE_TEST

static void ezspQueueTest(void)
{
  static bool alreadyRan = false;
  uint32_t status;

  if (!alreadyRan) {
    alreadyRan = true;
    status = ezspInternalQueueTest();
    if (status) {
      printf("EZSP queue test failed - error = %d\r\n", status);
    } else {
      printf("EZSP queue test passed\r\n");
    }
    sl_zigbee_ezsp_init_queues();
  }
}

// Returns 0 if all tests pass, otherwise the number of the first test to fail.
static uint32_t ezspInternalQueueTest(void)
{
  uint8_t i;
  EzspBuffer *buf, *bufx;

  sl_zigbee_ezsp_init_queues();
  if (!sl_zigbee_ezsp_queue_is_empty(&txQueue)) {
    return 10;
  }
  if (!sl_zigbee_ezsp_queue_is_empty(&reTxQueue)) {
    return 20;
  }
  if (!sl_zigbee_ezsp_queue_is_empty(&rxQueue)) {
    return 30;
  }
  if (txFree.link == NULL) {
    return 40;
  }
  if (rxFree.link == NULL) {
    return 50;
  }
  if (sl_zigbee_ezsp_free_list_length(&txFree) != TX_POOL_BUFFERS) {
    return 60;
  }

  for (i = 1;; i++) {
    buf = ezspAllocBuffer(&txFree);
    if (buf == NULL) {
      if (i != TX_POOL_BUFFERS + 1) {
        return 70;
      }
      break;
    }
    if (i > TX_POOL_BUFFERS) {
      return 80;
    }
    buf->len = i;
    sl_zigbee_ezsp_add_queue_tail(&txQueue, buf);
  }
  if (sl_zigbee_ezsp_queue_length(&txQueue) != TX_POOL_BUFFERS) {
    return 90;
  }
  if (sl_zigbee_ezsp_free_list_length(&txFree) != 0) {
    return 100;
  }

  for (i = 1; i <= TX_POOL_BUFFERS; i++) {
    buf = ezspQueueNthEntry(&txQueue, i);
    if (buf->len != TX_POOL_BUFFERS - i + 1) {
      return 110;
    }
  }

  for (i = 1; i <= TX_POOL_BUFFERS; i++) {
    buf = ezspQueueHead(&txQueue);
    if (buf == NULL) {
      return 120;
    }
    if (buf->len != i) {
      return 130;
    }
    buf = ezspRemoveQueueHead(&txQueue);
    if (buf == NULL) {
      return 140;
    }
    if (buf->len != i) {
      return 150;
    }

    sl_zigbee_ezsp_free_buffer(&txFree, buf);
  }
  if (!sl_zigbee_ezsp_queue_is_empty(&txQueue)) {
    return 170;
  }

  buf = ezspQueuePrecedingEntry(&txQueue, NULL);
  if (buf != NULL) {
    return 180;
  }

  for (i = 1;; i++) {
    buf = ezspAllocBuffer(&txFree);
    if (buf == NULL) {
      break;
    }
    buf->len = i;
    sl_zigbee_ezsp_add_queue_tail(&txQueue, buf);
  }

  bufx = NULL;
  for (i = 1; i <= TX_POOL_BUFFERS; i++) {
    buf = ezspQueuePrecedingEntry(&txQueue, bufx);
    bufx = buf;
    if (buf->len != i) {
      return 190;
    }
  }
  buf = ezspQueuePrecedingEntry(&txQueue, bufx);
  if (buf != NULL) {
    return 200;
  }

  bufx = ezspQueuePrecedingEntry(&txQueue, NULL);
  buf = ezspRemoveQueueEntry(&txQueue, bufx);
  if (bufx->len != 1) {
    return 210;
  }
  if (buf->len != 2) {
    return 220;
  }
  if (sl_zigbee_ezsp_queue_length(&txQueue) != (TX_POOL_BUFFERS - 1)) {
    return 230;
  }
  sl_zigbee_ezsp_free_buffer(&txFree, buf);
  bufx = buf;

  buf = ezspQueuePrecedingEntry(&txQueue, bufx);
  if (buf->len != 3) {
    return 240;
  }
  bufx = buf;

  buf = ezspRemoveQueueEntry(&txQueue, bufx);
  if (buf->len != 4) {
    return 230;
  }
  sl_zigbee_ezsp_free_buffer(&txFree, buf);
  bufx = buf;

  txQueue.tail = NULL;
  txFree.link = NULL;
  for (buf = ezspTxPool; buf < &ezspTxPool[TX_POOL_BUFFERS]; buf++) {
    sl_zigbee_ezsp_free_buffer(&txFree, buf);
  }
  for (i = 1;; i++) {
    buf = ezspAllocBuffer(&txFree);
    if (buf == NULL) {
      break;
    }
    buf->len = i;
    sl_zigbee_ezsp_add_queue_tail(&txQueue, buf);
  }

  buf = ezspRemoveQueueEntry(&txQueue, txQueue.tail);
  if (buf != NULL) {
    return 240;
  }
  if (sl_zigbee_ezsp_queue_length(&txQueue) != (TX_POOL_BUFFERS - 1)) {
    return 250;
  }

  return 0;
}
#endif // ifdef EZSP_QUEUE_TEST
