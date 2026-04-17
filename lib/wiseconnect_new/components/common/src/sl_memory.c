/*
 * EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND
 * CONDITIONS OF THE CONTROLLING LICENSE AGREEMENT FOUND AT LICENSE.md
 * IN THIS SDK. IF YOU DO NOT AGREE TO THE LICENSE TERMS AND CONDITIONS,
 * PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.
 * (c) Copyright 2018, Silicon Laboratories Inc.  All rights reserved.
 */
#include "sl_memory.h"
#include <malloc.h>

#if defined(SLI_SI91X_LWIP_HOSTED_NETWORK_STACK)
#include "lwip/pbuf.h"
#endif

// Allocates a new SL buffer object and the associated data storage space
sl_buffer_t *sl_memory_allocate_buffer(sl_buffer_type_t type, uint16_t size)
{
  sl_buffer_t *buffer;
#if defined(SLI_SI91X_LWIP_HOSTED_NETWORK_STACK)
  UNUSED_PARAMETER(type);
  buffer = (sl_buffer_t *)pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
#else
  buffer = malloc(size + sizeof(sl_buffer_t));
  if (buffer != NULL) {
    buffer->type        = type | SL_BUFFER_OBJECT_ALLOCATED | SL_BUFFER_DATA_ALLOCATED;
    buffer->data        = &buffer[1];
    buffer->data_length = size;
    buffer->next_length = 0;
  }
#endif
  return buffer;
}

// Allocates a new SL buffer object for an existing block of memory
sl_buffer_t *sl_memory_make_into_buffer(sl_buffer_type_t type, void *data, uint16_t data_size)
{
  sl_buffer_t *buffer;
#if defined(SLI_SI91X_LWIP_HOSTED_NETWORK_STACK)
  UNUSED_PARAMETER(type);
  buffer = (sl_buffer_t *)pbuf_alloc_reference(data, data_size, PBUF_ROM);
#else
  buffer = malloc(sizeof(sl_buffer_t));
  if (buffer != NULL) {
    buffer->type        = type | SL_BUFFER_OBJECT_ALLOCATED;
    buffer->data        = data;
    buffer->data_length = data_size;
    buffer->next_length = 0;
  }
#endif
  return buffer;
}

sl_buffer_t *sl_memory_free_buffer(sl_buffer_t *buffer, sl_memory_free_action_t action)
{
#if defined(SLI_SI91X_LWIP_HOSTED_NETWORK_STACK)
  if (action == SL_MEMORY_FREE_BUFFER_CHAIN) {
    pbuf_free((struct pbuf *)buffer);
    return NULL;
  } else {
    sl_buffer_t *temp = buffer->next;
    buffer->next      = NULL;
    pbuf_free((struct pbuf *)buffer);
    return temp;
  }
#else
  while (buffer != NULL) {
    if (buffer->type & SL_BUFFER_DATA_ALLOCATED) {
      free(buffer->data);
      buffer->data        = SL_INVALID_POINTER;
      buffer->data_length = 0;
      buffer->next_length = 0;
    }
    sl_buffer_t *next = buffer->next;
    if (buffer->type & SL_BUFFER_OBJECT_ALLOCATED) {
      buffer->type = SL_INVALID_BUFFER;
      free(buffer);
    }

    // Check if we continue looping through the chain
    if (action == SL_MEMORY_FREE_BUFFER_CHAIN) {
      buffer = next;
    } else {
      return next;
    }
  }
#endif
  return NULL;
}

void *sl_memory_allocate(sl_memory_allocation_type_t type, uint16_t size)
{
  return malloc(size);
}

void sl_memory_free(void *start_of_allocation)
{
  free(start_of_allocation);
}
