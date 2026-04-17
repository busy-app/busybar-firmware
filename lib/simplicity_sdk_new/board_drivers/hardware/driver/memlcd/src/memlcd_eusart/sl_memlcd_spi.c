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

#include "sl_memlcd_spi.h"
#include "sl_clock_manager.h"
#include "sl_device_clock.h"
#include "sl_device_peripheral.h"
#include "sl_common.h"
#include "sl_memlcd_eusart_config.h"
#include "sl_hal_bus.h"
#include "sl_gpio.h"
#include "stddef.h"

#define SPI_PERIPHERAL(periph_no)           SL_CONCAT_PASTER_2(SL_PERIPHERAL_EUSART, periph_no)

sl_status_t sli_memlcd_spi_init(sli_memlcd_spi_handle_t *handle, int bitrate, eusart_ClockMode mode)
{
  sl_hal_eusart_spi_config_t init = SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF;
  sl_hal_eusart_spi_advanced_config_t advancedInit = SL_HAL_EUSART_SPI_ADVANCED_INIT_DEFAULT;
  EUSART_TypeDef *eusart = handle->eusart;

  advancedInit.msb_first = true;
  init.advanced_config = &advancedInit;

  sl_gpio_t sclk_gpio = {
    .port = (sl_gpio_port_t)handle->sclk_port,
    .pin = handle->sclk_pin,
  };
  sl_gpio_t mosi_gpio = {
    .port = (sl_gpio_port_t)handle->mosi_port,
    .pin = handle->mosi_pin,
  };

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  sl_clock_manager_enable_bus_clock(handle->clock);

  sl_gpio_set_pin_mode(&sclk_gpio, SL_GPIO_MODE_PUSH_PULL, 0);
  sl_gpio_set_pin_mode(&mosi_gpio, SL_GPIO_MODE_PUSH_PULL, 0);

  static uint32_t ref_freq;
  sl_clock_branch_t eusart_clock_branch = sl_device_peripheral_get_clock_branch(SPI_PERIPHERAL(SL_MEMLCD_SPI_PERIPHERAL_NO));
  sl_clock_manager_get_clock_branch_frequency(eusart_clock_branch, &ref_freq);
  init.clock_div = sl_hal_eusart_spi_calculate_clock_div(ref_freq, bitrate);
  init.clock_mode = mode;

  sl_hal_eusart_init_spi(eusart, &init);
  sl_hal_eusart_enable(handle->eusart);
  sl_hal_eusart_enable_tx(handle->eusart);
  sl_hal_eusart_enable_rx(handle->eusart);

#if EUSART_COUNT > 1
  int eusart_index = EUSART_NUM(eusart);
#else
  int eusart_index = 0;
#endif
  GPIO->EUSARTROUTE[eusart_index].TXROUTE = (handle->mosi_port << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                            | (handle->mosi_pin << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[eusart_index].SCLKROUTE = (handle->sclk_port << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
                                              | (handle->sclk_pin << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);
  // note if another driver has enable RX
  uint32_t rxpen = GPIO->EUSARTROUTE[eusart_index].ROUTEEN & _GPIO_EUSART_ROUTEEN_RXPEN_MASK;
  GPIO->EUSARTROUTE[eusart_index].ROUTEEN = GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_SCLKPEN | rxpen;

  return SL_STATUS_OK;
}

sl_status_t sli_memlcd_spi_shutdown(sli_memlcd_spi_handle_t *handle)
{
  sl_hal_eusart_disable(handle->eusart);
  sl_clock_manager_disable_bus_clock(handle->clock);
  return SL_STATUS_OK;
}

sl_status_t sli_memlcd_spi_tx(sli_memlcd_spi_handle_t *handle, const void *data, unsigned len)
{
  const char *buffer = data;
  EUSART_TypeDef *eusart = handle->eusart;

  for (unsigned i = 0; i < len; i++) {
#if defined(SL_MEMLCD_LPM013M126A)
    sl_hal_eusart_tx(eusart, buffer[i]);
#else
    uint16_t reversed_data = (uint16_t)SL_RBIT8(buffer[i]);
    sl_hal_eusart_tx(eusart,reversed_data);
#endif
  }

  /* Note that at this point all the data is loaded into the fifo, this does
   * not necessarily mean that the transfer is complete. */
  return SL_STATUS_OK;
}

void sli_memlcd_spi_wait(sli_memlcd_spi_handle_t *handle)
{
  EUSART_TypeDef *eusart = handle->eusart;

  /* Wait for all transfers to finish */
  while (!(eusart->STATUS & EUSART_STATUS_TXC))
    ;
}

void sli_memlcd_spi_rx_flush(sli_memlcd_spi_handle_t *handle)
{
  EUSART_TypeDef *eusart = handle->eusart;

  /* Read data until RXFIFO empty */
  while (eusart->STATUS & EUSART_STATUS_RXFL) {
    sl_hal_eusart_rx(eusart);
  }
}

sl_status_t sli_memlcd_spi_exit_em23(sli_memlcd_spi_handle_t *handle)
{
  EUSART_TypeDef *eusart = handle->eusart;

  sl_hal_eusart_enable(eusart);
  sl_hal_eusart_enable_tx(eusart);
  sl_hal_eusart_enable_rx(eusart);
  sl_hal_bus_reg_write_mask(&GPIO->EUSARTROUTE[EUSART_NUM(eusart)].ROUTEEN,
                            _GPIO_EUSART_ROUTEEN_TXPEN_MASK | _GPIO_EUSART_ROUTEEN_SCLKPEN_MASK,
                            GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_SCLKPEN);

  return SL_STATUS_OK;
}

sl_status_t sli_memlcd_spi_enter_em23(sli_memlcd_spi_handle_t *handle)
{
  EUSART_TypeDef *eusart = handle->eusart;

  sl_hal_bus_reg_write_mask(&GPIO->EUSARTROUTE[EUSART_NUM(eusart)].ROUTEEN,
                            _GPIO_EUSART_ROUTEEN_TXPEN_MASK | _GPIO_EUSART_ROUTEEN_SCLKPEN_MASK,
                            0);

  return SL_STATUS_OK;
}
