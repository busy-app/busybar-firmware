/***************************************************************************//**
 * @file
 * @brief SPI abstraction used by memory lcd display
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_common.h"
#include "sl_clock_manager.h"
#include "sl_memlcd_spi.h"
#include "sl_gpio.h"

sl_status_t sli_memlcd_spi_init(sli_memlcd_spi_handle_t *handle, int baudrate, sl_hal_usart_clock_mode_t mode)
{
  sl_hal_usart_sync_init_t init = SL_HAL_USART_INIT_SYNC_DEFAULT;
  USART_TypeDef *usart = handle->usart;
  uint32_t ref_freq;

#if USART_COUNT > 1
  int usart_index = USART_NUM(usart);
#else
  int usart_index = 0;
#endif
  // take note if another driver has enable RX
  uint32_t rxpen = GPIO->USARTROUTE[usart_index].ROUTEEN & _GPIO_USART_ROUTEEN_RXPEN_MASK;

  sl_gpio_t clk_gpio = {
    .port = (sl_gpio_port_t)handle->clk_port,
    .pin = handle->clk_pin,
  };
  sl_gpio_t mosi_gpio = {
    .port = (sl_gpio_port_t)handle->mosi_port,
    .pin = handle->mosi_pin,
  };

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  sl_clock_manager_enable_bus_clock(handle->clock);

  sl_gpio_set_pin_mode(&clk_gpio, SL_GPIO_MODE_PUSH_PULL, 0);
  sl_gpio_set_pin_mode(&mosi_gpio, SL_GPIO_MODE_PUSH_PULL, 0);

  sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_PCLK, &ref_freq);

  init.clock_mode = mode;
  init.msb_first = true;
  init.clock_div = sl_hal_usart_sync_calculate_clock_div(ref_freq, baudrate);
  sl_hal_usart_enable(usart);
  sl_hal_usart_init_sync(usart, &init);
  sl_hal_usart_enable_rx(usart);
  sl_hal_usart_enable_tx(usart);

  GPIO->USARTROUTE[usart_index].TXROUTE = (handle->mosi_port << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                          | (handle->mosi_pin << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[usart_index].CLKROUTE = (handle->clk_port << _GPIO_USART_CLKROUTE_PORT_SHIFT)
                                           | (handle->clk_pin << _GPIO_USART_CLKROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[usart_index].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN | GPIO_USART_ROUTEEN_CLKPEN | rxpen;

  return SL_STATUS_OK;
}

sl_status_t sli_memlcd_spi_shutdown(sli_memlcd_spi_handle_t *handle)
{
  sl_hal_usart_disable(handle->usart);
  sl_clock_manager_disable_bus_clock(handle->clock);
  return SL_STATUS_OK;
}

sl_status_t sli_memlcd_spi_tx(sli_memlcd_spi_handle_t *handle, const void *data, unsigned len)
{
  const char *buffer = data;
  USART_TypeDef *usart = handle->usart;

  for (unsigned i = 0; i < len; i++) {
    sl_hal_usart_tx(usart, SL_RBIT8(buffer[i]));
  }

  /* Note that at this point all the data is loaded into the fifo, this does
   * not necessarily mean that the transfer is complete. */
  return SL_STATUS_OK;
}

void sli_memlcd_spi_wait(sli_memlcd_spi_handle_t *handle)
{
  USART_TypeDef *usart = handle->usart;

  /* Wait for all transfers to finish */
  while (!(usart->STATUS & USART_STATUS_TXC))
    ;
}

void sli_memlcd_spi_rx_flush(sli_memlcd_spi_handle_t *handle)
{
  USART_TypeDef *usart = handle->usart;

  /* Read data until RXFIFO empty */
  while (usart->STATUS & USART_STATUS_RXDATAV) {
    sl_hal_usart_rx(usart);
  }
}
