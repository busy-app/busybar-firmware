/***************************************************************************//**
 * @file
 * @brief Circular buffer implementation for host communication.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 ******************************************************************************/

#include "host_comm_ringbuf.h"
#include <stddef.h>
#include <string.h>

void host_comm_ringbuf_init(host_comm_ringbuf_t *rb)
{
  rb->head = 0;
  rb->tail = 0;
  rb->size = 0;
}

bool host_comm_ringbuf_is_empty(host_comm_ringbuf_t *rb)
{
  return rb->size == 0;
}

bool host_comm_ringbuf_is_full(host_comm_ringbuf_t *rb)
{
  return rb->size == (size_t)sizeof(rb->buffer);
}

size_t host_comm_ringbuf_write(host_comm_ringbuf_t *rb, const uint8_t *data, size_t len)
{
  size_t bytes_written = 0;

  while (bytes_written < len && !host_comm_ringbuf_is_full(rb)) {
    rb->buffer[rb->head] = data[bytes_written++];
    rb->head = (rb->head + 1) % (size_t)sizeof(rb->buffer);
    rb->size++;
  }

  return bytes_written;
}

size_t host_comm_ringbuf_read(host_comm_ringbuf_t *rb, uint8_t *data, size_t len)
{
  size_t bytes_read = 0;

  while (bytes_read < len && !host_comm_ringbuf_is_empty(rb)) {
    data[bytes_read++] = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % (size_t)sizeof(rb->buffer);
    rb->size--;
  }

  return bytes_read;
}

size_t host_comm_ringbuf_get_max_size(host_comm_ringbuf_t *rb)
{
  return (size_t)sizeof(rb->buffer);
}

size_t host_comm_ringbuf_data_size(host_comm_ringbuf_t *rb)
{
  return rb->size;
}
