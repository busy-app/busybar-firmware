/***************************************************************************//**
 * @file
 * @brief Circular buffer implementation.
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

#ifndef HOST_COMM_RINGBUF_H
#define HOST_COMM_RINGBUF_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "host_comm_config.h"

/**
 * @brief Circular buffer structure.
 */
typedef struct {
  size_t head;   /**< Head index. */
  size_t tail;   /**< Tail index. */
  size_t size;   /**< Current size of the buffer. */
  uint8_t buffer[DEFAULT_HOST_BUFLEN];   /**< Buffer storage. */
} host_comm_ringbuf_t;

/**************************************************************************//**
 * @brief Initialize the ring buffer.
 *
 * @param[in] rb Pointer to the ring buffer.
 *****************************************************************************/
void host_comm_ringbuf_init(host_comm_ringbuf_t *rb);

/**************************************************************************//**
 * @brief Check if the ring buffer is empty.
 *
 * @param[in] rb Pointer to the ring buffer.
 *
 * @return True if the buffer is empty, false otherwise.
 *****************************************************************************/
bool host_comm_ringbuf_is_empty(host_comm_ringbuf_t *rb);

/**************************************************************************//**
 * @brief Check if the ring buffer is full.
 *
 * @param[in] rb Pointer to the ring buffer.
 *
 * @return True if the buffer is full, false otherwise.
 *****************************************************************************/
bool host_comm_ringbuf_is_full(host_comm_ringbuf_t *rb);

/**************************************************************************//**
 * @brief Write data to the ring buffer.
 *
 * @param[in] rb Pointer to the ring buffer.
 * @param[in] data Pointer to the data to write.
 * @param[in] len Length of the data to write.
 *
 * @return Number of bytes written.
 *****************************************************************************/
size_t host_comm_ringbuf_write(host_comm_ringbuf_t *rb,
                               const uint8_t *data,
                               size_t len);

/**************************************************************************//**
 * @brief Read data from the ring buffer.
 *
 * @param[in] rb Pointer to the ring buffer.
 * @param[out] data Pointer to the buffer to store read data.
 * @param[in] len Length of the data to read.
 *
 * @return Number of bytes read.
 *****************************************************************************/
size_t host_comm_ringbuf_read(host_comm_ringbuf_t *rb,
                              uint8_t *data,
                              size_t len);

/**************************************************************************//**
 * @brief Get the maximum size of the ring buffer.
 *
 * @param[in] rb Pointer to the ring buffer.
 *
 * @return Maximum size of the buffer.
 *****************************************************************************/
size_t host_comm_ringbuf_get_max_size(host_comm_ringbuf_t *rb);

/**************************************************************************//**
 * @brief Get the current data size in the ring buffer.
 *
 * @param[in] rb Pointer to the ring buffer.
 *
 * @return Current data size in the buffer.
 *****************************************************************************/
size_t host_comm_ringbuf_data_size(host_comm_ringbuf_t *rb);

#endif // HOST_COMM_RINGBUF_H
