/***************************************************************************/ /**
 * @file
 * @brief IO Stream USART Component.
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

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sl_iostream.h"
#include "sl_iostream_uart_si91x.h"
#include "sli_iostream_uart_si91x.h"
#include "sl_iostream_usart_si91x.h"
#include "sl_atomic.h"
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "sl_si91x_usart.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
sl_usart_handle_t usart_handle;
volatile boolean_t send_complete = false, transfer_complete = false, receive_complete = false;
/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
void callback_event(uint32_t event);
static sl_status_t uart_deinit(void *stream);

static sl_status_t uart_write(void *context, const void *buffer, size_t buffer_length);

static sl_status_t uart_read(void *context, void *buffer, size_t buffer_length, size_t *bytes_read);

static void set_auto_cr_lf(void *context, bool on);

static bool get_auto_cr_lf(void *context);

static size_t read_rx_buffer(sl_iostream_uart_context_t *uart_context, uint8_t *buffer, size_t buffer_len);
static sl_status_t usart_tx(void *context, char c);

static sl_status_t usart_deinit(void *context);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/******************************************************************************
 * Set LF to CRLF conversion
 *****************************************************************************/
static void set_auto_cr_lf(void *context, bool on)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  sl_atomic_store(uart_context->lf_to_crlf, on);
}

/******************************************************************************
 * Get LF to CRLF conversion
 *****************************************************************************/
static bool get_auto_cr_lf(void *context)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool conversion;

  sl_atomic_load(conversion, uart_context->lf_to_crlf);

  return conversion;
}

/*******************************************************************************
 * USART Stream init
 ******************************************************************************/
sl_status_t sli_iostream_uart_context_init(sl_iostream_uart_t *uart,
                                           sl_iostream_uart_context_t *context,
                                           sl_iostream_uart_config_t *config,
                                           sl_status_t (*tx)(void *context, char c),
                                           void (*tx_completed)(void *context, bool enable),
                                           void (*set_next_byte_detect)(void *context),
                                           sl_status_t (*deinit)(void *context),
                                           uint8_t rx_em_req,
                                           uint8_t tx_em_req)
{
  (void)rx_em_req;
  (void)tx_em_req;
  (void)tx_completed;
  (void)set_next_byte_detect;

  // Configure iostream struct and context
  memset(context, 0, sizeof(*context));

  context->rx_buffer     = config->rx_buffer;
  context->rx_buffer_len = config->rx_buffer_length;
  context->lf_to_crlf    = config->lf_to_crlf;
  context->tx            = tx;
  context->deinit        = deinit;

  uart->stream.context = context;
  uart->stream.write   = uart_write;
  uart->stream.read    = uart_read;
  uart->set_auto_cr_lf = set_auto_cr_lf;
  uart->get_auto_cr_lf = get_auto_cr_lf;
  uart->deinit         = uart_deinit;

  sl_iostream_set_system_default(&uart->stream);

  return SL_STATUS_OK;
}

/*******************************************************************************
 * IOSTREAM USART init
 ******************************************************************************/
sl_status_t sl_iostream_usart_init(sl_iostream_uart_t *iostream_uart,
                                   sl_iostream_uart_config_t *uart_config,
                                   USART_InitAsync_TypeDef *init,
                                   sl_iostream_usart_config_t *config,
                                   sl_iostream_usart_context_t *usart_context)
{
  sl_status_t status;

  status = sli_iostream_uart_context_init(iostream_uart,
                                          &usart_context->context,
                                          uart_config,
                                          usart_tx,
                                          NULL,
                                          NULL,
                                          usart_deinit,
                                          1,
                                          1);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Save useful config info to usart context
  usart_context->tx_pin = config->tx_pin;
  usart_context->rx_pin = config->rx_pin;

  // 1.Configure USART for basic async operation
  // 2.Peripheral clock enable
  // 3. GPIO Configuration
  // 4.Enable the usart
  // Initialize the USART and enable the clock
  status = sl_si91x_usart_init(init->usart_module, &usart_handle);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Configure Tx and Rx lines for data transfer and configure with all the usart configuration i.e parity bit ,stop bit
  status = sli_si91x_usart_set_non_uc_configuration(usart_handle, init);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Register user callback function
  status = sl_si91x_usart_multiple_instance_register_event_callback(init->usart_module, callback_event);
  if (status != SL_STATUS_OK) {
    return status;
  }

  return SL_STATUS_OK;
}

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/*******************************************************************************
 * Internal stream write implementation
 ******************************************************************************/
static sl_status_t usart_tx(void *context, char c)
{
  sl_status_t status;
  (void)context;

  // Transfer the char
  status = sl_si91x_usart_send_data(usart_handle, &c, 1);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Wait till transfer comple
  while (!send_complete)
    ;
  // Clear the variable to send next byte
  send_complete = false;

  return SL_STATUS_OK;
}

/*******************************************************************************
 * Internal stream write implementation
 ******************************************************************************/
static sl_status_t uart_write(void *context, const void *buffer, size_t buffer_length)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  char *c                                  = (char *)buffer;
  bool lf_to_crlf                          = false;
  uint32_t i                               = 0;
  sl_status_t status                       = SL_STATUS_FAIL;

  sl_atomic_load(lf_to_crlf, uart_context->lf_to_crlf);

  while (i < buffer_length) {
    if (lf_to_crlf == true) {
      if (*c == '\n') {
        status = uart_context->tx(uart_context, '\r');
      }
    }
    status = uart_context->tx(uart_context, *c);
    if (status != SL_STATUS_OK) {
      return status;
    }
    c++;
    i++;
  }

  return status;
}

/*******************************************************************************
 * Internal stream read implementation
 ******************************************************************************/
static sl_status_t uart_read(void *context, void *buffer, size_t buffer_length, size_t *bytes_read)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  // Read the data received
  *bytes_read = read_rx_buffer(uart_context, (uint8_t *)buffer, (size_t)buffer_length);
  if (*bytes_read == 0) {
    return SL_STATUS_EMPTY;
  } else {
    return SL_STATUS_OK;
  }
}

/*******************************************************************************
 * Tries to read the requested amount of data.
 * Returns the number of bytes read.
 ******************************************************************************/
static size_t read_rx_buffer(sl_iostream_uart_context_t *uart_context, uint8_t *buffer, size_t buffer_len)
{
  size_t ret_val;
  uint8_t ch;
  // read the smallest amount between the data available and the size of the user buffer
  size_t read_size = buffer_len < uart_context->rx_buffer_len ? buffer_len : uart_context->rx_buffer_len;

  for (ret_val = 0; ret_val < read_size; ret_val++) {
    // Read the byte received
    sl_si91x_usart_receive_data(usart_handle, &ch, 1);
    // wait till data byte receive
    while (!receive_complete)
      ;
    // Clear the variable to receive next byte
    receive_complete = false;
    //Copying the char received from USART to buffer
    uart_context->rx_buffer[ret_val] = ch;
  }
  // Copy the data received in buffer
  memcpy(buffer, uart_context->rx_buffer, read_size);

  return ret_val;
}

/*******************************************************************************
 * UART Stream De-init
 ******************************************************************************/
static sl_status_t uart_deinit(void *stream)
{
  sl_iostream_uart_t *uart                 = (sl_iostream_uart_t *)stream;
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)uart->stream.context;
  sl_iostream_t *default_stream;
  sl_status_t status = SL_STATUS_OK;

  default_stream = sl_iostream_get_default();

  // Check if uart stream is the default and if it's the case,
  // remove it's reference as the default
  if ((sl_iostream_uart_t *)default_stream == uart) {
    sl_iostream_set_system_default(NULL);
  }
  // Uninitialize the uart and set the power mode to off
  status = sl_si91x_usart_deinit(usart_handle);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Clear iostream struct and context
  uart->stream.context = NULL;
  uart->stream.write   = NULL;
  uart->stream.read    = NULL;
  uart->set_auto_cr_lf = NULL;
  uart->get_auto_cr_lf = NULL;

  status = uart_context->deinit(uart_context);

  return status;
}

/*******************************************************************************
 * USART Stream De-init.
 ******************************************************************************/
static sl_status_t usart_deinit(void *context)
{
  sl_status_t status = SL_STATUS_OK;
  (void)context;

  // Deinit the USART
  status = sl_si91x_usart_deinit(usart_handle);

  return status;
}

/*******************************************************************************
 * Callback function triggered on data Transfer and reception
 ******************************************************************************/
void callback_event(uint32_t event)
{
  switch (event) {
    case SL_USART_EVENT_SEND_COMPLETE:
      send_complete = true;
      break;
    case SL_USART_EVENT_RECEIVE_COMPLETE:
      receive_complete = true;
      break;
    case SL_USART_EVENT_TRANSFER_COMPLETE:
      send_complete = true;
      break;
  }
}