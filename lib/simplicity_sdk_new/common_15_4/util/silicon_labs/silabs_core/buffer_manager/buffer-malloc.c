/***************************************************************************//**
 * @file
 * @brief malloc() and free() implemented on top of buffers.
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

// An implementation of malloc() and free() that uses buffers.  malloc()
// returns a pointer to the contents of a buffer.  Freed buffers are kept
// on a list and reused; a new buffer is allocated only if no buffer on
// the freelist is large enough to be used.
//
// The buffers are not marked by this code.  sli_legacy_buffer_manager_malloc_free_list must be
// set to NULL_BUFFER before calling sli_legacy_buffer_manager_reclaim_unused_buffers().

#include "core/sl_zigbee_stack.h"
#include "buffer-malloc.h"

// Keep statistics.
uint32_t sli_legacy_buffer_manager_malloc_count = 0;
uint32_t sli_legacy_buffer_manager_malloc_size = 0;

#define getFreeLink(buffer)      (sli_legacy_buffer_manager_get_buffer_link((buffer), 0))
#define setFreeLink(buffer, new) (sli_legacy_buffer_manager_set_buffer_link((buffer), 0, (new)))

#ifdef SL_ZIGBEE_TEST
static UNUSED void printFreeList(char *tag)
{
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_malloc_free_list;
  fprintf(stderr, "[%s:", tag);
  for (; buffer != NULL_BUFFER; buffer = getFreeLink(buffer)) {
    fprintf(stderr, " %04X", buffer);
  }
  fprintf(stderr, "]\n");
}

static UNUSED void printFreeSizes(char *tag)
{
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_malloc_free_list;
  fprintf(stderr, "[%s:", tag);
  for (; buffer != NULL_BUFFER; buffer = getFreeLink(buffer)) {
    fprintf(stderr, " %d", sli_legacy_buffer_manager_get_buffer_length(buffer));
  }
  fprintf(stderr, "]\n");
}
#endif

// Make 'front' precede 'back', merging them if they are adjacent.
// Either or both may be NULL_BUFFER.

static void connect(sli_buffer_manager_buffer_t front, sli_buffer_manager_buffer_t back)
{
  if (front == NULL_BUFFER) {
    sli_legacy_buffer_manager_malloc_free_list = back;
  } else if (sli_legacy_buffer_manager_following_buffer(front) == back) {
    setFreeLink(front, getFreeLink(back));
    sli_legacy_buffer_manager_merge_buffers(front, back);
  } else {
    setFreeLink(front, back);
  }
}

void sli_legacy_buffer_manager_buffer_free(void *pointer)
{
  sli_buffer_manager_buffer_t buffer = sli_legacy_buffer_manager_buffer_pointer_to_buffer(pointer);
  if (buffer != NULL_BUFFER) {
    sli_buffer_manager_buffer_t next = sli_legacy_buffer_manager_malloc_free_list;
    sli_buffer_manager_buffer_t previous = NULL_BUFFER;

    for (;
         next != NULL_BUFFER && next < buffer;
         previous = next, next = getFreeLink(next)) {
      ;
    }

    connect(buffer, next);
    connect(previous, buffer);
  }
}

// This uses 'best fit': the smallest free buffer that is 'size' or
// larger is returned.  Any extra is split off into a separate buffer
// that is left on the free list.

void *sli_legacy_buffer_manager_buffer_malloc(uint16_t size)
{
  sli_buffer_manager_buffer_t next = sli_legacy_buffer_manager_malloc_free_list;
  sli_buffer_manager_buffer_t previous = NULL_BUFFER;
  sli_buffer_manager_buffer_t result = NULL_BUFFER;
  sli_buffer_manager_buffer_t resultPrevious = NULL_BUFFER;

  size += size & 1;             // round up to a word boundary

  for (;
       next != NULL_BUFFER;
       previous = next, next = getFreeLink(next)) {
    uint16_t nextSize = sli_legacy_buffer_manager_get_buffer_length(next);

    if (size <= nextSize
        && (result == NULL_BUFFER
            || nextSize < sli_legacy_buffer_manager_get_buffer_length(result))) {
      result = next;
      resultPrevious = previous;
      if (size == nextSize) {
        break;
      }
    }
  }

  if (result == NULL_BUFFER) {
    result = sli_legacy_buffer_manager_allocate_buffer(size);
    if (result == NULL_BUFFER) {
      return NULL;
    }
    sli_legacy_buffer_manager_malloc_count += 1;
    sli_legacy_buffer_manager_malloc_size += size;
  } else {
    sli_buffer_manager_buffer_t leftover = sli_legacy_buffer_manager_split_buffer(result, size);
    sli_buffer_manager_buffer_t resultNext = getFreeLink(result);
    if (leftover == NULL_BUFFER) {
      connect(resultPrevious, resultNext);
    } else {
      connect(leftover, resultNext);
      connect(resultPrevious, leftover);
    }
    connect(result, NULL_BUFFER);
  }

  return sli_legacy_buffer_manager_get_buffer_pointer(result);
}
