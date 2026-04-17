/***************************************************************************//**
 * @file
 * @brief Host communication application module (posix).
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 *
 ******************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <stddef.h>
#include "uart.h"
#include "tcp.h"
#include "app_assert.h"
#include "app_log.h"
#include "host_comm.h"
#include "app_sleep.h"
#include "host_comm_config.h"
#include "named_socket.h"
#include "host_comm_ringbuf.h"
#include "host_comm_common.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_HOST_NCP_CPC_PRESENT
#include "cpc_bt.h"
#endif // SL_CATALOG_HOST_NCP_CPC_PRESENT

// Default parameter values.
#define DEFAULT_UART_PORT             ""
#define DEFAULT_UART_BAUD_RATE        115200
#define DEFAULT_UART_FLOW_CONTROL     1
#define DEFAULT_UART_TIMEOUT          100
#define DEFAULT_TCP_ADDRESS           ""
#define DEFAULT_TCP_PORT              "4901"
#define MAX_OPT_LEN                   255
#define DEFAULT_CPC_INST_NAME         "cpcd_0"

#define IS_EMPTY_STRING(s)            ((s)[0] == '\0')
#define HANDLE_VALUE_MIN              0

// Define global HOST_COMM_API_DEFINE library.
HOST_COMM_API_DEFINE();

static host_comm_ringbuf_t rx_buffer;
static host_comm_ringbuf_t tx_buffer;

// end the receiving loop if signal is received.
static volatile bool run = true;

static bool comm_channel_selected = false;

// UART serial port options.
static char uart_port[MAX_OPT_LEN] = DEFAULT_UART_PORT;
static uint32_t uart_baud_rate = DEFAULT_UART_BAUD_RATE;
static uint32_t uart_flow_control = DEFAULT_UART_FLOW_CONTROL;

// TCP/IP address.
static char tcp_address[MAX_OPT_LEN] = DEFAULT_TCP_ADDRESS;

// AF socket descriptor path
static char named_socket_target_address[MAX_OPT_LEN];
#ifdef SL_CATALOG_HOST_NCP_CPC_PRESENT
// CPCd instance name.
static char cpc_instance_name[MAX_OPT_LEN] = DEFAULT_CPC_INST_NAME;

// CPC connection
static bool cpc_conn = false;
#endif // SL_CATALOG_HOST_NCP_CPC_PRESENT

int32_t handle = -1;
void *handle_ptr;

// Static receive function
void *msg_recv_func(void *ptr);
void *msg_send_func(void *ptr);

pthread_t thread_rx;
pthread_t thread_tx;

static pthread_mutex_t rx_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t tx_mutex = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************
 * Initialize low level connection.
 *****************************************************************************/
sl_status_t host_comm_init(void)
{
  int32_t status;

  if (!comm_channel_selected) {
    app_log_error("No communication channel provided, "
                  "but exactly one is expected!" APP_LOG_NL);
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!IS_EMPTY_STRING(uart_port)) {
    // Initialise UART serial connection.
    handle_ptr = &handle;
    HOST_COMM_API_INITIALIZE_NONBLOCK(uartTx, uartRx, uartRxPeek);
    status = uartOpen(handle_ptr, (int8_t *)uart_port, uart_baud_rate,
                      uart_flow_control, DEFAULT_UART_TIMEOUT);
    if (status < HANDLE_VALUE_MIN) {
      app_log_error("Failed to open serial connection (%d)"
                    APP_LOG_NL, status);
      exit(EXIT_FAILURE);
    }
    uartFlush(handle_ptr);
  } else if (!IS_EMPTY_STRING(tcp_address)) {
    // Initialise TCP/IP connection.
    handle_ptr = &handle;
    HOST_COMM_API_INITIALIZE_NONBLOCK(tcp_tx, tcp_rx, tcp_rx_peek);
    status = tcp_open(handle_ptr, tcp_address, DEFAULT_TCP_PORT);
    app_assert(status == HANDLE_VALUE_MIN,
               "[E: %d] Failed to open TCP/IP connection" APP_LOG_NL,
               status);
  } else if (!IS_EMPTY_STRING(named_socket_target_address)) {
    handle_ptr = &handle;
    // Initialise serial communication as non-blocking.
    HOST_COMM_API_INITIALIZE_NONBLOCK(tcp_tx, tcp_rx, tcp_rx_peek);
    if (connect_domain_socket_server(handle_ptr, named_socket_target_address,
                                     CLIENT_PATH, 0)) {
      app_log_critical("Connection to domain socket unsuccessful. Exiting.." APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
#ifdef SL_CATALOG_HOST_NCP_CPC_PRESENT
  } else if (cpc_conn) {
    handle_ptr = &handle;
    HOST_COMM_API_INITIALIZE_NONBLOCK(cpc_bt_tx, cpc_bt_rx, cpc_bt_rx_peek);
    if (cpc_bt_open(handle_ptr, cpc_instance_name)) {
      app_log_critical("Connection to CPCd unsuccessful. Exiting.." APP_LOG_NL);
      exit(EXIT_FAILURE);
    }
#endif // SL_CATALOG_HOST_NCP_CPC_PRESENT
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  host_comm_buffers_init(&rx_buffer, &tx_buffer);

  return host_comm_threads_create(&thread_rx, &thread_tx, msg_recv_func, msg_send_func);
}

/******************************************************************************
 * Set low level host communication connection options.
 *****************************************************************************/
sl_status_t host_comm_set_option(char option, char *value)
{
  sl_status_t sc = SL_STATUS_OK;

  switch (option) {
    // TCP/IP address.
    case 't':
      if (!comm_channel_selected) {
        if (strlen(value) >= MAX_OPT_LEN - 1) {
          app_log_error("Value for option '%c' is too long, truncating to %d characters." APP_LOG_NL, option, MAX_OPT_LEN - 1);
        }
        strncpy(tcp_address, value, MAX_OPT_LEN - 1);
        tcp_address[MAX_OPT_LEN - 1] = '\0'; // Ensure null-termination
        comm_channel_selected = true;
      } else {
        app_log_error("More than one communication channel "
                      "provided, but exactly one is expected!" APP_LOG_NL);
        sc = SL_STATUS_INVALID_PARAMETER;
      }
      break;
    // UART serial port.
    case 'u':
      if (!comm_channel_selected) {
        if (strlen(value) >= MAX_OPT_LEN - 1) {
          app_log_error("Value for option '%c' is too long, truncating to %d characters." APP_LOG_NL, option, MAX_OPT_LEN - 1);
        }
        strncpy(uart_port, value, MAX_OPT_LEN - 1);
        uart_port[MAX_OPT_LEN - 1] = '\0'; // Ensure null-termination
        comm_channel_selected = true;
      } else {
        app_log_error("More than one communication channel "
                      "provided, but exactly one is expected!" APP_LOG_NL);
        sc = SL_STATUS_INVALID_PARAMETER;
      }
      break;
    // UART baud rate.
    case 'b':
      uart_baud_rate = atol(value);
      break;
    // UART flow control disable.
    case 'f':
      uart_flow_control = 0;
      break;
    // AF socket descriptor
    case 'n':
      if (!comm_channel_selected) {
        if (strlen(value) >= MAX_OPT_LEN - 1) {
          app_log_error("Value for option '%c' is too long, truncating to %d characters." APP_LOG_NL, option, MAX_OPT_LEN - 1);
        }
        strncpy(named_socket_target_address, value, MAX_OPT_LEN - 1);
        named_socket_target_address[MAX_OPT_LEN - 1] = '\0'; // Ensure null-termination
        comm_channel_selected = true;
      } else {
        app_log_error("More than one communication channel "
                      "provided, but exactly one is expected!" APP_LOG_NL);
        sc = SL_STATUS_INVALID_PARAMETER;
      }
      break;
#ifdef SL_CATALOG_HOST_NCP_CPC_PRESENT
    // CPC connection
    case 'C':
      if (!comm_channel_selected) {
        if (strlen(value) >= MAX_OPT_LEN - 1) {
          app_log_error("Value for option '%c' is too long, truncating to %d characters." APP_LOG_NL, option, MAX_OPT_LEN - 1);
        }
        strncpy(cpc_instance_name, value, MAX_OPT_LEN - 1);
        cpc_instance_name[MAX_OPT_LEN - 1] = '\0'; // Ensure null-termination
        cpc_conn = true;
        comm_channel_selected = true;
      } else {
        app_log_error("More than one communication channel "
                      "provided, but exactly one is expected!" APP_LOG_NL);
        sc = SL_STATUS_INVALID_PARAMETER;
      }

      break;
#endif // SL_CATALOG_HOST_NCP_CPC_PRESENT
    // Unknown option.
    default:
      sc = SL_STATUS_NOT_FOUND;
      break;
  }
  return sc;
}

/******************************************************************************
 * Deinitialize low level connection.
 *****************************************************************************/
void host_comm_deinit(void)
{
  run = false;
  pthread_cancel(thread_rx);
  pthread_cancel(thread_tx);

  if (!IS_EMPTY_STRING(uart_port)) {
    uartClose(handle_ptr);
  } else if (!IS_EMPTY_STRING(tcp_address)) {
    tcp_close(handle_ptr);
  }
}

/******************************************************************************
 * Write data to NCP through low level drivers.
 *****************************************************************************/
int32_t host_comm_tx(uint32_t len, uint8_t* data)
{
  pthread_mutex_lock(&tx_mutex);
  size_t written = host_comm_ringbuf_write(&tx_buffer, data, (size_t)len);
  pthread_mutex_unlock(&tx_mutex);
  if (written < (size_t)len) {
    app_log_error("TX buffer overflow, data lost." APP_LOG_NL);
  }
  return (int32_t)written;
}

/******************************************************************************
 * Read data from NCP.
 *****************************************************************************/
int32_t host_comm_rx(uint32_t len, uint8_t* data)
{
  int32_t ret = -1;
  pthread_mutex_lock(&rx_mutex);
  ret = (int32_t)host_comm_ringbuf_read(&rx_buffer, data, (size_t)len);
  pthread_mutex_unlock(&rx_mutex);
  return ret;
}

/******************************************************************************
 * Peek if readable data exists.
 *****************************************************************************/
int32_t host_comm_peek(void)
{
  int32_t len = 0;
  pthread_mutex_lock(&rx_mutex);
  len = (int32_t)host_comm_ringbuf_data_size(&rx_buffer);
  pthread_mutex_unlock(&rx_mutex);
  return len;
}

/******************************************************************************
 * Read data from low level drivers.
 *****************************************************************************/
void *msg_recv_func(void *ptr)
{
  // unused variable
  (void)ptr;

  return msg_recv_func_shared(&rx_buffer, &rx_mutex, host_comm_pk, host_comm_input, handle_ptr);
}

/******************************************************************************
 * TX thread function
 *****************************************************************************/
void *msg_send_func(void *ptr)
{
  // unused variable
  (void)ptr;

  return msg_send_func_shared(&tx_buffer, &tx_mutex, host_comm_output, handle_ptr);
}
