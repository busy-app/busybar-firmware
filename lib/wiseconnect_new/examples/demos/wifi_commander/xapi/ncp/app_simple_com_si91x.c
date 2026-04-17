/***************************************************************************/ /**
 * @file
 * @brief Simple Communication Interface (UART)
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

#include "string.h"
#include "sl_status.h"
#include "sl_common.h"
#include "app_simple_com.h"
#include "app_simple_com_config.h"
#include "sl_si91x_usart.h"
#include "FreeRTOS.h"
#include "timers.h"

static usart_peripheral_t ncp_usart = ULPUART;
static sl_usart_handle_t ncp_handle = 0;

static uint8_t rx_buf[APP_SIMPLE_COM_RX_BUF_SIZE] = { 0 };
static uint8_t *rx                                = rx_buf;
static uint8_t rx_byte                            = 0;
static volatile uint32_t rx_complete              = 0;
static volatile uint32_t tx_complete              = 0;
volatile uint32_t rx_cntr                         = 0;

TimerHandle_t rx_timer;

static sl_si91x_usart_control_config_t config = { .baudrate      = 115200,
                                                  .mode          = SL_USART_MODE_ASYNCHRONOUS,
                                                  .parity        = SL_USART_NO_PARITY,
                                                  .stopbits      = SL_USART_STOP_BITS_1,
                                                  .hwflowcontrol = SL_USART_FLOW_CONTROL_NONE,
                                                  .databits      = SL_USART_DATA_BITS_8,
                                                  .usart_module  = 0 };

static sl_status_t status2status(sl_usart_status_t *status)
{
  return (*((sl_status_t *)status)) & ~0x3;
}

static void ncp_callback(uint32_t event)
{
  switch (event) {
    case SL_USART_EVENT_SEND_COMPLETE:
      tx_complete = 1;
      break;
    case SL_USART_EVENT_RECEIVE_COMPLETE:
      rx_complete = 1;
      if (rx - rx_buf < APP_SIMPLE_COM_RX_BUF_SIZE) {
        *rx++ = rx_byte;
      }
      app_simple_com_receive();
      rx_cntr++;
      break;
    default:
      break;
  }
}

static void reset_rx(void)
{
  sl_usart_status_t s = sl_si91x_usart_get_status(ncp_handle);
  sl_status_t status  = status2status(&s);
  uint32_t len        = rx - rx_buf;
  app_simple_com_receive_cb(status, len, rx_buf);
  rx = rx_buf;
  memset(rx_buf, 0, sizeof(rx_buf));
  rx_cntr = 0;
}

static void timer_cb(TimerHandle_t timer)
{
  reset_rx();
  (void)timer;
}

void app_simple_com_init(void)
{
  sl_status_t status;

  status = sl_si91x_usart_init(ncp_usart, &ncp_handle);
  if (status != SL_STATUS_OK) {
    return;
  }

  status = sl_si91x_usart_set_configuration(ncp_handle, &config);
  if (status != SL_STATUS_OK) {
    return;
  }

  status = sl_si91x_usart_multiple_instance_register_event_callback(ncp_usart, ncp_callback);
  if (status != SL_STATUS_OK) {
    return;
  }

  NVIC_SetPriority(UDMA0_IRQn, 4); // hotfix, to be removed

  rx_timer = xTimerCreate("simple com rx timer",
                          pdMS_TO_TICKS(5), // 5ms max delay between bytes
                          pdFALSE,          // no auto-reload
                          (void *)0,
                          timer_cb);
}

void app_simple_com_step(void)
{
  if (tx_complete) {
    tx_complete         = 0;
    sl_usart_status_t s = sl_si91x_usart_get_status(ncp_handle);
    sl_status_t status  = status2status(&s);
    app_simple_com_transmit_cb(status);
  }

  if (rx_complete) {
    rx_complete = 0;
    xTimerStart(rx_timer, 0);
  }

  if (rx - rx_buf == APP_SIMPLE_COM_RX_BUF_SIZE) { // buffer full
    reset_rx();
  }

  app_simple_com_os_task_proceed();
}

void app_simple_com_transmit(uint32_t len, const uint8_t *data)
{
  sl_status_t status = sl_si91x_usart_send_data(ncp_handle, data, len);
  (void)status;
}

void app_simple_com_receive(void)
{
  // receive byte by byte
  sl_status_t status = sl_si91x_usart_receive_data(ncp_handle, &rx_byte, 1);
  (void)status;
}

SL_WEAK void app_simple_com_transmit_cb(sl_status_t status)
{
  (void)status;
}

SL_WEAK void app_simple_com_receive_cb(sl_status_t status, uint32_t len, uint8_t *data)
{
  (void)status;
  (void)data;
  (void)len;
}

SL_WEAK void app_simple_com_os_task_proceed(void)
{
  // No action on bare metal implementation
}
