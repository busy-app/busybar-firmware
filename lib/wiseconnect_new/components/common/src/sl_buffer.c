/*
 * EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND
 * CONDITIONS OF THE CONTROLLING LICENSE AGREEMENT FOUND AT LICENSE.md
 * IN THIS SDK. IF YOU DO NOT AGREE TO THE LICENSE TERMS AND CONDITIONS,
 * PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.
 * (c) Copyright 2018, Silicon Laboratories Inc.  All rights reserved.
 */
#include "sl_buffer.h"
#include "sl_memory.h"

static sl_buffer_t *sli_buffer_last(sl_buffer_t *buffer);

/*
 * Sets data to point to the start of the buffer at the given offset and sets the data_length to the number of contiguous bytes of data available
 * There may be more data available at a larger offset
 */
sl_status_t sl_buffer_get_data(sl_buffer_t *buffer, uint16_t offset, uint8_t **data, uint16_t *data_length)
{
  while (buffer != NULL) {
    if (offset < buffer->data_length) {
      *data        = buffer->data + offset;
      *data_length = buffer->data_length - offset;
      return SL_SUCCESS;
    } else {
      offset -= buffer->data_length;
      buffer = buffer->next;
    }
  }

  return SL_ERROR;
}

/*
 * Free data will attempt to free entire blocks of data up to the given offset
 */
sl_status_t sl_buffer_free_data(sl_buffer_t *buffer, uint16_t up_to_offset)
{
  UNUSED_PARAMETER(buffer);
  UNUSED_PARAMETER(up_to_offset);
  return SL_UNSUPPORTED;
}

sl_status_t sl_buffer_expand(sl_buffer_t *buffer, uint16_t size_increase)
{
  sl_buffer_t *new_segment = sl_memory_allocate_buffer(buffer->type, size_increase);
  if (new_segment == NULL) {
    return SL_ERROR_OUT_OF_BUFFERS;
  }
  sli_buffer_last(buffer)->next = new_segment;
  return SL_SUCCESS;
}

sl_status_t sl_buffer_write(sl_buffer_t *buffer, uint16_t offset, void *data, uint16_t data_length)
{
  UNUSED_PARAMETER(buffer);
  UNUSED_PARAMETER(offset);
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(data_length);
  return SL_UNSUPPORTED;
}

/*
 * sl_buffer_read will copy data into the user buffer and then free it
 */
sl_status_t sl_buffer_read(sl_buffer_t *buffer, void *user_buffer, uint16_t user_buffer_length)
{
  UNUSED_PARAMETER(buffer);
  UNUSED_PARAMETER(user_buffer);
  UNUSED_PARAMETER(user_buffer_length);
  return SL_UNSUPPORTED;
}

static sl_buffer_t *sli_buffer_last(sl_buffer_t *buffer)
{
  while (buffer->next != NULL) {
    buffer = buffer->next;
  }
  return buffer;
}
