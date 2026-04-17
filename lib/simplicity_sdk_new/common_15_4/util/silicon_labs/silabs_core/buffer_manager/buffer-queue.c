/***************************************************************************//**
 * @file
 * @brief queues of buffers
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
#include STACK_CORE_HEADER
#include "sl_code_classification.h"
#include "buffer-management.h"
#include "buffer-queue.h"

// Queues of buffers.

// A queue is an index into the root set, as all queues must
// be handles.  Queues are linked in a circle through link0, with the
// handle pointing to the end of the queue and the tail pointing at
// the head, the head pointing at the next element and so on back
// around to the tail.

#if 0
static void printQueue(sli_buffer_manager_buffer_t queue, int i)
{
  if (queue == NULL_BUFFER) {
    fprintf(stderr, "[]");
  } else {
    sli_buffer_manager_buffer_t head = sli_legacy_buffer_manager_get_buffer_link(queue, i);
    fprintf(stderr, "[%04X", head);
    queue = sli_legacy_buffer_manager_get_buffer_link(head, i);
    while (queue != head) {
      fprintf(stderr, " %04X", queue);
      queue = sli_legacy_buffer_manager_get_buffer_link(queue, i);
    }
    fprintf(stderr, "]");
  }
}
#define QDEBUG(x) x
#else
#define QDEBUG(x)
#endif

bool sli_legacy_buffer_manager_buffer_queue_is_empty(sli_buffer_manager_buffer_t *queue)
{
  return *queue == NULL_BUFFER;
}

uint16_t sli_legacy_buffer_manager_buffer_queue_length(sli_buffer_manager_buffer_t *queue)
{
  uint16_t length = 0;
  if (queue == NULL) {
    return 0;
  }
  sli_buffer_manager_buffer_t finger = sli_legacy_buffer_manager_buffer_queue_head(queue);
  while (finger != NULL_BUFFER) {
    length++;
    finger = sli_legacy_buffer_manager_buffer_queue_next(queue, finger);
  }
  return length;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sli_buffer_manager_buffer_t sli_legacy_buffer_manager_generic_queue_head(sli_buffer_manager_buffer_t *queue, uint16_t i)
{
  sli_buffer_manager_buffer_t tail = *queue;
  if (tail == NULL_BUFFER) {
    return tail;
  } else {
    return sli_legacy_buffer_manager_get_buffer_link(tail, i);
  }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_legacy_buffer_manager_generic_queue_add(sli_buffer_manager_buffer_t *queue, sli_buffer_manager_buffer_t newTail, uint16_t i)
{
  sli_buffer_manager_buffer_t oldTail = *queue;
  sli_buffer_manager_buffer_t head;

  if (oldTail == NULL_BUFFER) {
    head = newTail;
  } else {
    head = sli_legacy_buffer_manager_get_buffer_link(oldTail, i);
    sli_legacy_buffer_manager_set_buffer_link(oldTail, i, newTail);
  }

  sli_legacy_buffer_manager_set_buffer_link(newTail, i, head);
  *queue = newTail;
  QDEBUG(fprintf(stderr, "[qadd(%08lX, %04X) ", (unsigned long) queue, newTail);
         printQueue(*queue, i);
         fprintf(stderr, "]\n");
         );
}

void sli_legacy_buffer_manager_buffer_queue_pre_insert_buffer(sli_buffer_manager_buffer_t *queue, sli_buffer_manager_buffer_t bufferToInsert, sli_buffer_manager_buffer_t bufferToInsertBefore)
{
  sli_buffer_manager_buffer_t head;
  sli_buffer_manager_buffer_t previousBuffer;
  uint8_t buffersChecked = 0;

  // Find the buffer pointing to bufferToInsertBefore
  head = sli_legacy_buffer_manager_get_buffer_link(*queue, 0);

  // Is the head the buffer to insert before?
  if (head == bufferToInsertBefore) {
    // Record the previous buffer to link the chain
    sli_legacy_buffer_manager_buffer_queue_addToHead(queue, bufferToInsert);
    return;
  } else {
    // Else, the buffer to insert before is in the chain somewhere. Find it.
    previousBuffer = head;
    while (true) {
      if (sli_legacy_buffer_manager_get_buffer_link(previousBuffer, 0) == bufferToInsertBefore) {
        break;
      }

      // Just in case this circular queue is corrupted somehow, we don't want to
      // sit forever in a while loop. In test, assert if we checked more buffers
      // than available
      buffersChecked++;
      if (buffersChecked > 50) {
        //testAssert(0);
      }

      previousBuffer = sli_legacy_buffer_manager_get_buffer_link(previousBuffer, 0);
    }
  }

  sli_legacy_buffer_manager_set_buffer_link(previousBuffer, 0, bufferToInsert);
  sli_legacy_buffer_manager_set_buffer_link(bufferToInsert, 0, bufferToInsertBefore);
}

// Add a buffer, but as the head, not the tail.

void sli_legacy_buffer_manager_buffer_queue_addToHead(sli_buffer_manager_buffer_t *queue, sli_buffer_manager_buffer_t newHead)
{
  sli_buffer_manager_buffer_t tail = *queue;
  sli_legacy_buffer_manager_buffer_queue_add(queue, newHead);
  if (tail != NULL_BUFFER) {
    // Switch back to original tail, so newHead ends up as the head.
    *queue = tail;
  }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BUFFER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sli_buffer_manager_buffer_t sli_legacy_buffer_manager_buffer_queue_next(sli_buffer_manager_buffer_t *queue, sli_buffer_manager_buffer_t finger)
{
  sli_buffer_manager_buffer_t tail = *queue;
  if (finger == tail) {
    return NULL_BUFFER;
  } else {
    return sli_legacy_buffer_manager_get_queue_link(finger);
  }
}

sli_buffer_manager_buffer_t sli_legacy_buffer_manager_generic_queue_removeHead(sli_buffer_manager_buffer_t *queue, uint16_t i)
{
  sli_buffer_manager_buffer_t tail = *queue;

  if (tail == NULL_BUFFER) {
    QDEBUG(fprintf(stderr, "[qsub(%08lX) -> zip]\n", (unsigned long) queue); )
    return NULL_BUFFER;
  } else {
    sli_buffer_manager_buffer_t head = sli_legacy_buffer_manager_get_buffer_link(tail, i);
    if (head == tail) {
      *queue = NULL_BUFFER;
    } else {
      sli_legacy_buffer_manager_set_buffer_link(tail, i, sli_legacy_buffer_manager_get_buffer_link(head, i));
    }
    QDEBUG(fprintf(stderr, "[qsub(%08lX) -> %04X ",
                   (unsigned long) queue, head);
           printQueue(*queue, 0);
           fprintf(stderr, "]\n");
           )
    sli_legacy_buffer_manager_set_buffer_link(head, i, NULL_BUFFER);
    return head;
  }
}

// If 'buffer' is NULL_BUFFER this returns the total number of bytes in
// the queue.
uint16_t sli_legacy_buffer_manager_generic_queue_remove(sli_buffer_manager_buffer_t *queue, sli_buffer_manager_buffer_t buffer, uint16_t i)
{
  sli_buffer_manager_buffer_t tail = *queue;

  QDEBUG(fprintf(stderr, "[qremove(%08lX, %04X) ",
                 (unsigned long) queue, buffer);
         printQueue(*queue, i);
         fprintf(stderr, "]\n");
         );

  if (tail == NULL_BUFFER) {
    return 0;
  } else {
    sli_buffer_manager_buffer_t finger = tail;
    uint16_t length = sli_legacy_buffer_manager_get_buffer_length(finger);
    while (true) {
      sli_buffer_manager_buffer_t next = sli_legacy_buffer_manager_get_buffer_link(finger, i);
      if (next == buffer) {
        sli_legacy_buffer_manager_set_buffer_link(finger, i, sli_legacy_buffer_manager_get_buffer_link(next, i));
        if (tail == buffer) {
          *queue = (finger == tail
                    ? NULL_BUFFER
                    : finger);
        }
        sli_legacy_buffer_manager_set_buffer_link(buffer, i, NULL_BUFFER);
        return length;
      } else if (next == tail) {
        return length;
      } else {
        finger = next;
        length += sli_legacy_buffer_manager_get_buffer_length(finger);
      }
    }
  }
}

uint16_t sli_legacy_buffer_manager_remove_bytes_from_generic_queue(sli_buffer_manager_buffer_t *queue, uint16_t count, uint16_t i)
{
  uint16_t todo = count;

  while (0 < todo && *queue != NULL_BUFFER) {
    sli_buffer_manager_buffer_t head = sli_legacy_buffer_manager_generic_queue_head(queue, i);
    uint16_t length = sli_legacy_buffer_manager_get_buffer_length(head);
    if (length <= todo) {
      sli_legacy_buffer_manager_generic_queue_removeHead(queue, i);
      todo -= length;
    } else {
      sli_legacy_buffer_manager_set_buffer_length_from_end(head, length - todo);
      todo = 0;
    }
  }

  return count - todo;
}

void sli_legacy_buffer_manager_copy_from_generic_queue(sli_buffer_manager_buffer_t *queue, uint16_t count, uint8_t *to, uint16_t i)
{
  sli_buffer_manager_buffer_t tail = *queue;
  sli_buffer_manager_buffer_t finger = tail;

  if (count == 0) {
    return;
  }

  assert(tail != NULL_BUFFER);
  while (true) {
    sli_buffer_manager_buffer_t next = sli_legacy_buffer_manager_get_buffer_link(finger, i);
    uint16_t have = sli_legacy_buffer_manager_get_buffer_length(next);
    uint16_t copy = (have < count
                     ? have
                     : count);
    memmove(to, sli_legacy_buffer_manager_get_buffer_pointer(next), copy);
    to += copy;
    count -= copy;
    if (count == 0) {
      break;
    }
    assert(next != tail);
    finger = next;
  }
}

void sli_legacy_buffer_manager_linked_payload_to_payload_queue(sli_buffer_manager_buffer_t *queue)
{
  sli_buffer_manager_buffer_t head = *queue;
  if (head != NULL_BUFFER) {
    sli_buffer_manager_buffer_t tail = head;
    while (sli_legacy_buffer_manager_get_payload_link(tail) != NULL_BUFFER) {
      tail = sli_legacy_buffer_manager_get_payload_link(tail);
    }
    sli_legacy_buffer_manager_set_payload_link(tail, head);
    *queue = tail;
  }
}

void sli_legacy_buffer_manager_payload_queue_to_linked_payload(sli_buffer_manager_buffer_t *queue)
{
  sli_buffer_manager_buffer_t tail = *queue;

  if (tail != NULL_BUFFER) {
    sli_buffer_manager_buffer_t head = sli_legacy_buffer_manager_get_payload_link(tail);
    sli_legacy_buffer_manager_set_payload_link(tail, NULL_BUFFER);
    *queue = head;
  }
}

//----------------------------------------------------------------
// Utilities to use a queue of buffers as an extensible vector (a
// one-dimensional array).  The buffers in the queue may contain
// different numbers of elements.  This allows the queue to be
// amalgamated, should anyone want to do so.

// Returns the first element in the vector for which
// predicate(element, target) returns true.  If indexLoc is non-NULL
// the index of the element is stored there.

void *emVectorSearch(Vector *vector,
                     EqualityPredicate predicate,
                     const void *target,
                     uint16_t *indexLoc)
{
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_buffer_queue_head(&vector->values);
  uint16_t index = 0;

  while (buffer != NULL_BUFFER) {
    uint8_t *finger = sli_legacy_buffer_manager_get_buffer_pointer(buffer);
    uint16_t length = sli_legacy_buffer_manager_get_buffer_length(buffer);
    uint8_t *end = finger + length;
    for (;
         finger < end && index < vector->valueCount;
         finger += vector->valueSize, index++) {
      if (predicate(finger, target)) {
        if (indexLoc != NULL) {
          *indexLoc = index;
        }
        return finger;
      }
    }
    buffer = sli_legacy_buffer_manager_buffer_queue_next(&vector->values, buffer);
  }
  return NULL;
}

uint16_t sli_legacy_buffer_manager_vector_match_count(Vector *vector,
                                                      EqualityPredicate predicate,
                                                      const void *target)
{
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_buffer_queue_head(&vector->values);
  uint16_t index = 0;
  uint16_t result = 0;

  while (buffer != NULL_BUFFER) {
    uint8_t *finger = sli_legacy_buffer_manager_get_buffer_pointer(buffer);
    uint16_t length = sli_legacy_buffer_manager_get_buffer_length(buffer);
    uint8_t *end = finger + length;
    for (;
         finger < end && index < vector->valueCount;
         finger += vector->valueSize, index++) {
      if (predicate(finger, target)) {
        result++;
      }
    }
    buffer = sli_legacy_buffer_manager_buffer_queue_next(&vector->values, buffer);
  }

  return result;
}

// Return the 'index'th element of the vector.

void *emVectorRef(Vector *vector, uint16_t index)
{
  if (vector->valueCount == 0
      || (vector->valueCount - 1) < index) {
    return NULL;
  }

  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_buffer_queue_head(&vector->values);

  index *= vector->valueSize;

  while (buffer != NULL_BUFFER) {
    uint16_t length = sli_legacy_buffer_manager_get_buffer_length(buffer);
    if (index < length) {
      return sli_legacy_buffer_manager_get_buffer_pointer(buffer) + index;
    } else {
      index -= length;
      buffer = sli_legacy_buffer_manager_buffer_queue_next(&vector->values, buffer);
    }
  }
  return NULL;
}

uint16_t sli_legacy_buffer_manager_vector_find_index(Vector *vector, const uint8_t *value)
{
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_buffer_queue_head(&vector->values);
  uint16_t index = 0;

  while (buffer != NULL_BUFFER) {
    uint8_t *contents = sli_legacy_buffer_manager_get_buffer_pointer(buffer);
    uint16_t length = sli_legacy_buffer_manager_get_buffer_length(buffer);
    if (contents <= value && value < (contents + length)) {
      return (index + (value - contents)) / vector->valueSize;
    } else {
      index += length;
      buffer = sli_legacy_buffer_manager_buffer_queue_next(&vector->values, buffer);
    }
  }
  return -1;
}

// Add space for 'quanta' more elements to the vector.

void *emVectorAdd(Vector *vector, uint16_t quanta)
{
  if (0 < vector->emptyCount) {
    vector->valueCount += 1;
    vector->emptyCount -= 1;
    return emVectorRef(vector, vector->valueCount - 1);
  } else {
    sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_allocate_buffer(vector->valueSize * quanta);
    if (buffer == NULL_BUFFER) {
      return NULL;
    }
    sli_legacy_buffer_manager_buffer_queue_add(&vector->values, buffer);
    vector->valueCount += 1;
    vector->emptyCount = quanta - 1;
    return sli_legacy_buffer_manager_get_buffer_pointer(buffer);
  }
}
