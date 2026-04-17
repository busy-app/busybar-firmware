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
 *    freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 ******************************************************************************/

#ifndef HOST_COMM_COMMON_H
#define HOST_COMM_COMMON_H

#include "host_comm_ringbuf.h"
#include <pthread.h>

/**************************************************************************//**
 * @brief Initialize shared RX and TX ring buffers.
 *
 * @param[in] rx_buffer Pointer to the RX ring buffer.
 * @param[in] tx_buffer Pointer to the TX ring buffer.
 *****************************************************************************/
void host_comm_buffers_init(host_comm_ringbuf_t *rx_buffer,
                            host_comm_ringbuf_t *tx_buffer);

/**************************************************************************//**
 * @brief Shared RX thread function for receiving data.
 *
 * @param[in] rx_buffer Pointer to the RX ring buffer.
 * @param[in] rx_mutex Pointer to the RX mutex for thread safety.
 * @param[in] host_comm_pk Function pointer for peeking data availability.
 * @param[in] host_comm_input Function pointer for reading data.
 * @param[in] handle_ptr Pointer to the communication handle.
 *
 * @return NULL.
 *****************************************************************************/
void *msg_recv_func_shared(host_comm_ringbuf_t *rx_buffer,
                           pthread_mutex_t *rx_mutex,
                           int (*host_comm_pk)(void *),
                           int (*host_comm_input)(void *, uint32_t, uint8_t *),
                           void *handle_ptr);

/**************************************************************************//**
 * @brief Shared TX thread function for sending data.
 *
 * @param[in] tx_buffer Pointer to the TX ring buffer.
 * @param[in] tx_mutex Pointer to the TX mutex for thread safety.
 * @param[in] host_comm_output Function pointer for writing data.
 * @param[in] handle_ptr Pointer to the communication handle.
 *
 * @return NULL.
 *****************************************************************************/
void *msg_send_func_shared(host_comm_ringbuf_t *tx_buffer,
                           pthread_mutex_t *tx_mutex,
                           int (*host_comm_output)(void *, uint32_t, uint8_t *),
                           void *handle_ptr);

/**************************************************************************//**
 * @brief Create RX and TX threads for host communication.
 *
 * @param[in] thread_rx Pointer to the RX thread.
 * @param[in] thread_tx Pointer to the TX thread.
 * @param[in] msg_recv_func Function pointer for the RX thread function.
 * @param[in] msg_send_func Function pointer for the TX thread function.
 *
 * @return SL_STATUS_OK if successful, otherwise an error code.
 *****************************************************************************/
sl_status_t host_comm_threads_create(pthread_t *thread_rx,
                                     pthread_t *thread_tx,
                                     void *(*msg_recv_func)(void *),
                                     void *(*msg_send_func)(void *));

#endif // HOST_COMM_COMMON_H
