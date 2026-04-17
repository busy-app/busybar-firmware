/***************************************************************************//**
 * @file
 * @brief MX25 flash shutdown
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "sl_udelay.h"
#include "sl_mx25_flash_shutdown.h"
#include "sl_hal_eusart.h"
#include "sl_device_peripheral.h"
#include "stddef.h"

// Fallback to baudrate of 7.5 MHz if not defined for backwards compatibility
#ifndef SL_MX25_FLASH_SHUTDOWN_BAUDRATE
#define SL_MX25_FLASH_SHUTDOWN_BAUDRATE   7500000
#endif

// Define usart clock
#ifndef SL_MX25_FLASH_SHUTDOWN_SCLK
#define MERGE(x, y)   x##y
#define EUSART_CLOCK(n) MERGE(SL_BUS_CLOCK_EUSART, n)
#define SL_MX25_FLASH_SHUTDOWN_SCLK EUSART_CLOCK(SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO)
#endif

#ifndef SL_MX25_FLASH_SHUTDOWN_SPERIPHERAL
#define EUSART_PERIPHERAL_MERGE(x, y) x##y
#define EUSART_PERIPHERAL(n) EUSART_PERIPHERAL_MERGE(SL_PERIPHERAL_EUSART, n)
#define SL_MX25_FLASH_SHUTDOWN_SPERIPHERAL EUSART_PERIPHERAL(SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO)
#endif

#ifdef SL_MX25_FLASH_SHUTDOWN_PERIPHERAL
static void cs_low(void)
{
  sl_gpio_t mx25_flash_shutdown_cs_gpio = {
    .port = SL_MX25_FLASH_SHUTDOWN_CS_PORT,
    .pin = SL_MX25_FLASH_SHUTDOWN_CS_PIN,
  };
  sl_gpio_clear_pin(&mx25_flash_shutdown_cs_gpio);
}

static void cs_high(void)
{
  sl_gpio_t mx25_flash_shutdown_cs_gpio = {
    .port = SL_MX25_FLASH_SHUTDOWN_CS_PORT,
    .pin = SL_MX25_FLASH_SHUTDOWN_CS_PIN,
  };
  sl_gpio_set_pin(&mx25_flash_shutdown_cs_gpio);
}
#endif

/***************************************************************************//**
 *    Puts the MX25 into deep power down mode.
 ******************************************************************************/
void sl_mx25_flash_shutdown(void)
{
#ifdef SL_MX25_FLASH_SHUTDOWN_PERIPHERAL
  // Init flash
  sl_hal_eusart_spi_config_t init = SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF;
  sl_hal_eusart_spi_advanced_config_t advancedInit = SL_HAL_EUSART_SPI_ADVANCED_INIT_DEFAULT;
  uint32_t ref_freq;
  sl_clock_branch_t eusart_clk_branch;
  uint32_t baudrate = SL_MX25_FLASH_SHUTDOWN_BAUDRATE;

  eusart_clk_branch = sl_device_peripheral_get_clock_branch(SL_MX25_FLASH_SHUTDOWN_SPERIPHERAL);
  sl_clock_manager_get_clock_branch_frequency(eusart_clk_branch, &ref_freq);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  sl_clock_manager_enable_bus_clock(SL_MX25_FLASH_SHUTDOWN_SCLK);

  advancedInit.msb_first      = true;
  advancedInit.auto_cs_enable = false;

  init.advanced_config = &advancedInit;

  // Set desired baudrate
  if (baudrate > ref_freq) {
    baudrate = ref_freq;
  }
  init.clock_div = sl_hal_eusart_spi_calculate_clock_div(ref_freq, baudrate);
  
  sl_hal_eusart_init_spi(SL_MX25_FLASH_SHUTDOWN_PERIPHERAL, &init);
  sl_hal_eusart_enable(SL_MX25_FLASH_SHUTDOWN_PERIPHERAL);
  sl_hal_eusart_enable_tx(SL_MX25_FLASH_SHUTDOWN_PERIPHERAL);
  sl_hal_eusart_enable_rx(SL_MX25_FLASH_SHUTDOWN_PERIPHERAL);

  // IO config
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_MX25_FLASH_SHUTDOWN_TX_PORT, SL_MX25_FLASH_SHUTDOWN_TX_PIN }, SL_GPIO_MODE_PUSH_PULL, 1);
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_MX25_FLASH_SHUTDOWN_RX_PORT, SL_MX25_FLASH_SHUTDOWN_RX_PIN }, SL_GPIO_MODE_INPUT, 0);
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_MX25_FLASH_SHUTDOWN_SCLK_PORT, SL_MX25_FLASH_SHUTDOWN_SCLK_PIN }, SL_GPIO_MODE_PUSH_PULL, 1);
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_MX25_FLASH_SHUTDOWN_CS_PORT, SL_MX25_FLASH_SHUTDOWN_CS_PIN }, SL_GPIO_MODE_PUSH_PULL, 1);

  GPIO->EUSARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].SCLKROUTE  = ((SL_MX25_FLASH_SHUTDOWN_SCLK_PORT << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT)
                                                                        | (SL_MX25_FLASH_SHUTDOWN_SCLK_PIN  << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT));
  GPIO->EUSARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].RXROUTE   = ((SL_MX25_FLASH_SHUTDOWN_RX_PORT << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
                                                                       | (SL_MX25_FLASH_SHUTDOWN_RX_PIN  << _GPIO_EUSART_RXROUTE_PIN_SHIFT));
  GPIO->EUSARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].TXROUTE   = ((SL_MX25_FLASH_SHUTDOWN_TX_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                                                       | (SL_MX25_FLASH_SHUTDOWN_TX_PIN  << _GPIO_EUSART_TXROUTE_PIN_SHIFT));
  GPIO->EUSARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].ROUTEEN   = (GPIO_EUSART_ROUTEEN_RXPEN
                                                                       | GPIO_EUSART_ROUTEEN_TXPEN
                                                                       | GPIO_EUSART_ROUTEEN_SCLKPEN);

  // Wait for flash warm-up
  sl_udelay_wait(800);               // wait for tVSL=800us

  // Wake up flash in case the device is in deep power down mode already.
  cs_low();
  sl_udelay_wait(20);                  // wait for tCRDP=20us
  cs_high();
  sl_udelay_wait(35);                  // wait for tRDP=35us

  // Chip select go low to start a flash command
  cs_low();

  // Deep Power Down Mode command (0xB9)
  sl_hal_eusart_spi_tx_rx(SL_MX25_FLASH_SHUTDOWN_PERIPHERAL, 0xB9);

  // Chip select go high to end a flash command
  cs_high();

  // Deinit flash
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_MX25_FLASH_SHUTDOWN_TX_PORT, SL_MX25_FLASH_SHUTDOWN_TX_PIN }, SL_GPIO_MODE_DISABLED, 0);
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_MX25_FLASH_SHUTDOWN_RX_PORT, SL_MX25_FLASH_SHUTDOWN_RX_PIN }, SL_GPIO_MODE_DISABLED, 0);
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_MX25_FLASH_SHUTDOWN_SCLK_PORT, SL_MX25_FLASH_SHUTDOWN_SCLK_PIN }, SL_GPIO_MODE_DISABLED, 1);
  sl_gpio_set_pin_mode(&(sl_gpio_t) {SL_MX25_FLASH_SHUTDOWN_CS_PORT, SL_MX25_FLASH_SHUTDOWN_CS_PIN }, SL_GPIO_MODE_DISABLED, 1);

  sl_hal_eusart_reset(SL_MX25_FLASH_SHUTDOWN_PERIPHERAL);

  GPIO->EUSARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].ROUTEEN  = _GPIO_EUSART_ROUTEEN_RESETVALUE;
  GPIO->EUSARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].SCLKROUTE = _GPIO_EUSART_SCLKROUTE_RESETVALUE;
  GPIO->EUSARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].RXROUTE  = _GPIO_EUSART_RXROUTE_RESETVALUE;
  GPIO->EUSARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].TXROUTE  = _GPIO_EUSART_TXROUTE_RESETVALUE;

  sl_clock_manager_disable_bus_clock(SL_MX25_FLASH_SHUTDOWN_SCLK);
#endif
}
