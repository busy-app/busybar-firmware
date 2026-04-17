/***************************************************************************//**
 * @file
 * @brief Shared logic for host communication (common).
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
 ******************************************************************************/

#include "host_comm_config.h"
#include "host_comm_ringbuf.h"
#include "app_log.h"
#include "app_sleep.h"
#include <pthread.h>
#include <stdio.h>
#include <stddef.h>

// Shared RX and TX buffer initialization
void host_comm_buffers_init(host_comm_ringbuf_t *rx_buffer,
                            host_comm_ringbuf_t *tx_buffer)
{
  host_comm_ringbuf_init(rx_buffer);
  host_comm_ringbuf_init(tx_buffer);
}

// Shared thread creation logic
sl_status_t host_comm_threads_create(pthread_t *thread_rx,
                                     pthread_t *thread_tx,
                                     void *(*msg_recv_func)(void *),
                                     void *(*msg_send_func)(void *))
{
  int iret;

  iret = pthread_create(thread_rx, NULL, msg_recv_func, NULL);
  if (iret) {
    app_log_error("pthread_create() for RX thread return code: %d" APP_LOG_NL,
                  iret);
    return SL_STATUS_FAIL;
  }

  iret = pthread_create(thread_tx, NULL, msg_send_func, NULL);
  if (iret) {
    app_log_error("pthread_create() for TX thread return code: %d" APP_LOG_NL,
                  iret);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

// Shared RX and TX buffer logic
void *msg_recv_func_shared(host_comm_ringbuf_t *rx_buffer,
                           pthread_mutex_t *rx_mutex,
                           int (*host_comm_pk)(void *),
                           int (*host_comm_input)(void *, uint32_t, uint8_t *),
                           void *handle_ptr)
{
  int32_t ret;
  // Reduced temporary buffer size for memory efficiency, but may need
  // to be increased if the warning below for the input buffer size is triggered
  // in the particular application context.
  uint8_t temp_buf[DEFAULT_HOST_BUFLEN >> 3];

  while (1) {
    int32_t len = host_comm_pk(handle_ptr);

    if (len < 0) {
      // Try to read at least one byte anyway, because a negative value may
      // indicate that peek is unsupported on the system
      len = 1;
    } else if ((size_t)len > sizeof(temp_buf)) {
      size_t free_ringbuf_size = host_comm_ringbuf_get_max_size(rx_buffer)
                                 - host_comm_ringbuf_data_size(rx_buffer);
      // Set proper length limit to avoid buffer overflow
      len = free_ringbuf_size > sizeof(temp_buf)
            ? sizeof(temp_buf) : free_ringbuf_size;
      app_log_warning("Input buffer size may be low, please consider increasing it."
                      APP_LOG_NL);
    }

    ret = host_comm_input(handle_ptr, len, temp_buf);
    if (ret > 0) {
      pthread_mutex_lock(rx_mutex);
      size_t written = host_comm_ringbuf_write(rx_buffer, temp_buf, (size_t)ret);
      pthread_mutex_unlock(rx_mutex);

      if (written < (size_t)ret) {
        app_log_error("RX buffer overflow, data lost." APP_LOG_NL);
      }
    } else {
      app_sleep_us(RECV_FUNC_US_SLEEP);
    }
  }

  return NULL;
}

void *msg_send_func_shared(host_comm_ringbuf_t *tx_buffer,
                           pthread_mutex_t *tx_mutex,
                           int (*host_comm_output)(void *, uint32_t, uint8_t *),
                           void *handle_ptr)
{
  uint8_t temp_buf[DEFAULT_HOST_BUFLEN >> 3]; // Reduced size for memory efficiency

  while (1) {
    pthread_mutex_lock(tx_mutex);
    size_t len = host_comm_ringbuf_read(tx_buffer, temp_buf, sizeof(temp_buf));
    pthread_mutex_unlock(tx_mutex);

    if (len > 0) {
      int32_t ret = host_comm_output(handle_ptr, (uint32_t)len, temp_buf);
      if (ret < 0) {
        app_log_error("TX failed with return value: %d" APP_LOG_NL, ret);
      }
    } else {
      app_sleep_us(RECV_FUNC_US_SLEEP);
    }
  }

  return NULL;
}
