/***************************************************************************//**
 * @file
 * @brief Memory LCD interface
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
#include "sl_memlcd.h"
#include "sl_memlcd_display.h"
#include "sl_sleeptimer.h"
#include "sl_udelay.h"
#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "sl_code_classification.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && defined(SL_MEMLCD_USE_EUSART)
#include "sl_power_manager.h"
#endif

#include <string.h>

#define CMD_UPDATE        0x01
#define CMD_ALL_CLEAR     0x04

/* Concatenate preprocessor tokens A and B. */
#define SL_CONCAT(A, B) A ## B

/* Generate the cmu clock symbol based on instance. */
#if defined(SL_MEMLCD_USE_USART)
#define SL_MEMLCD_SPI_CLOCK(N) SL_CONCAT(SL_BUS_CLOCK_USART, N)
#elif defined(SL_MEMLCD_USE_EUSART)
#define SL_MEMLCD_SPI_CLOCK(N) SL_CONCAT(SL_BUS_CLOCK_EUSART, N)
#endif

#if defined(SL_MEMLCD_EXTCOMIN_PORT)
/** Timer used for periodic maintenance of the display. */
static sl_sleeptimer_timer_handle_t extcomin_timer;

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMLCD, SL_CODE_CLASS_TIME_CRITICAL)
static void extcomin_toggle(sl_sleeptimer_timer_handle_t *handle, void *data);
#endif

/** Memory lcd instance. This variable will be initialized in the
 *  @ref sl_memlcd_configure() function. */
static sl_memlcd_t memlcd_instance;

/** Flag to monitor is this driver has been initialized. The memlcd_instance
 *  is only valid after initialized=true. */
static bool initialized = false;

/** SPI handle to use for SPI communication to the memory lcd. */
#if defined(SL_MEMLCD_USE_EUSART)
static sli_memlcd_spi_handle_t spi_handle = {
  .eusart     = SL_MEMLCD_SPI_PERIPHERAL,
  .clock      = SL_MEMLCD_SPI_CLOCK(SL_MEMLCD_SPI_PERIPHERAL_NO),
  .mosi_port  = SL_MEMLCD_SPI_TX_PORT,
  .mosi_pin   = SL_MEMLCD_SPI_TX_PIN,
  .miso_port  = SL_MEMLCD_SPI_VALUE_NONE,
  .miso_pin   = SL_MEMLCD_SPI_VALUE_NONE,
  .sclk_port  = SL_MEMLCD_SPI_SCLK_PORT,
  .sclk_pin   = SL_MEMLCD_SPI_SCLK_PIN,
};
#elif defined(SL_MEMLCD_USE_USART)
static sli_memlcd_spi_handle_t spi_handle = {
  .usart     = SL_MEMLCD_SPI_PERIPHERAL,
  .clock     = SL_MEMLCD_SPI_CLOCK(SL_MEMLCD_SPI_PERIPHERAL_NO),
  .mosi_port = SL_MEMLCD_SPI_TX_PORT,
  .mosi_pin  = SL_MEMLCD_SPI_TX_PIN,
  .miso_port = SL_MEMLCD_SPI_VALUE_NONE,
  .miso_pin  = SL_MEMLCD_SPI_VALUE_NONE,
  .clk_port  = SL_MEMLCD_SPI_CLK_PORT,
  .clk_pin   = SL_MEMLCD_SPI_CLK_PIN,
};
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && defined(SL_MEMLCD_USE_EUSART)
static sl_power_manager_em_transition_event_handle_t on_power_manager_event_handle;

static void on_power_manager_event(sl_power_manager_em_t from,
                                   sl_power_manager_em_t to);

static sl_power_manager_em_transition_event_info_t on_power_manager_event_info =
{
  .event_mask = (SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM2 | SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM2),
  .on_event = on_power_manager_event,
};
#endif

sl_status_t sl_memlcd_configure(struct sl_memlcd_t *device)
{
  sl_gpio_t memlcd_spi_cs_gpio = {
    .port = SL_MEMLCD_SPI_CS_PORT,
    .pin = SL_MEMLCD_SPI_CS_PIN,
  };

  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  /* Initialize the SPI bus. */
#if defined(SL_MEMLCD_USE_EUSART)
  sli_memlcd_spi_init(&spi_handle, device->spi_freq, SL_HAL_EUSART_CLOCK_MODE_0);
#elif defined(SL_MEMLCD_USE_USART)
  sli_memlcd_spi_init(&spi_handle, device->spi_freq, SL_HAL_USART_CLOCK_MODE_0);
#endif

  /* Setup GPIOs */
  sl_gpio_set_pin_mode(&memlcd_spi_cs_gpio, SL_GPIO_MODE_PUSH_PULL, 0);

#if defined(SL_MEMLCD_EXTCOMIN_PORT)
  sl_gpio_t memlcd_extcomin_gpio = {
    .port = SL_MEMLCD_EXTCOMIN_PORT,
    .pin = SL_MEMLCD_EXTCOMIN_PIN,
  };
  sl_gpio_set_pin_mode(&memlcd_extcomin_gpio, SL_GPIO_MODE_PUSH_PULL, 0);
#endif

  memlcd_instance = *device;
  initialized = true;
  sl_memlcd_power_on(device, true);
  sl_memlcd_clear(device);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && defined(SL_MEMLCD_USE_EUSART)
  // Subscribe to notification to prepare eusart before/after deepsleep.
  sl_power_manager_subscribe_em_transition_event(&on_power_manager_event_handle, &on_power_manager_event_info);
#endif

  return SL_STATUS_OK;
}

sl_status_t sl_memlcd_refresh(const struct sl_memlcd_t *device)
{
  #if defined(SL_MEMLCD_USE_EUSART)
  sli_memlcd_spi_init(&spi_handle, device->spi_freq, SL_HAL_EUSART_CLOCK_MODE_0);
  #elif defined(SL_MEMLCD_USE_USART)
  sli_memlcd_spi_init(&spi_handle, device->spi_freq, SL_HAL_USART_CLOCK_MODE_0);
  #endif

  return SL_STATUS_OK;
}

sl_status_t sl_memlcd_power_on(const struct sl_memlcd_t *device, bool on)
{
  (void) device;
  (void) on;
  sl_status_t status = SL_STATUS_OK;

#if defined(SL_MEMLCD_EXTCOMIN_PORT)
  if (on) {
    uint32_t freq = sl_sleeptimer_get_timer_frequency();
    status = sl_sleeptimer_restart_periodic_timer(&extcomin_timer,
                                                  freq / (device->extcomin_freq * 2),
                                                  extcomin_toggle,
                                                  NULL,
                                                  0,
                                                  SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  } else {
    status = sl_sleeptimer_stop_timer(&extcomin_timer);
  }
#endif

  return status;
}

sl_status_t sl_memlcd_clear(const struct sl_memlcd_t *device)
{
  uint16_t cmd;
  sl_gpio_t memlcd_spi_cs_gpio = {
    .port = SL_MEMLCD_SPI_CS_PORT,
    .pin = SL_MEMLCD_SPI_CS_PIN,
  };

  /* Set SCS */
  sl_gpio_set_pin(&memlcd_spi_cs_gpio);

  /* SCS setup time */
  sl_udelay_wait(device->setup_us);

  /* Send command */
  cmd = CMD_ALL_CLEAR;
  sli_memlcd_spi_tx(&spi_handle, &cmd, 2);
  sli_memlcd_spi_wait(&spi_handle);

  /* SCS hold time */
  sl_udelay_wait(device->hold_us);

  /* Clear SCS */
  sl_gpio_clear_pin(&memlcd_spi_cs_gpio);

  return SL_STATUS_OK;
}

sl_status_t sl_memlcd_draw(const struct sl_memlcd_t *device, const void *data, unsigned int row_start, unsigned int row_count)
{
  unsigned int i;
  const uint8_t *p = data;
  uint16_t cmd;
  int row_len;
#if defined(SL_MEMLCD_LPM013M126A)
  uint16_t reversed_row;
#endif

  sl_gpio_t memlcd_spi_cs_gpio = {
    .port = SL_MEMLCD_SPI_CS_PORT,
    .pin = SL_MEMLCD_SPI_CS_PIN,
  };

  row_len = (device->width * device->bpp) / 8;
  row_start++;

  /* Assert SCS */
  sl_gpio_set_pin(&memlcd_spi_cs_gpio);

  /* SCS setup time */
  sl_udelay_wait(device->setup_us);

#if defined(SL_MEMLCD_LPM013M126A)
  /* LPM013M126A uses MSB first for the row address */
  reversed_row = SL_RBIT16(row_start);

  /* Send update command and first line address */
  /* CMD_UPDATE is only 6 bits and the address line is 10 bits */
  cmd = CMD_UPDATE | reversed_row;
#else
  /* Send update command and first line address */
  cmd = CMD_UPDATE | (row_start << 8);
#endif

  sli_memlcd_spi_tx(&spi_handle, &cmd, 2);

  /* Get start address to draw from */
  for ( i = 0; i < row_count; i++ ) {
    /* Send pixels for this line */
    sli_memlcd_spi_tx(&spi_handle, p, row_len);
    p += row_len;

    if (i == (row_count - 1)) {
      /* Dummy Data transfer at the end of update mode by multiple lines */
      cmd = 0xffff;
    } else {
#if defined(SL_MEMLCD_LPM013M126A)
      /* LPM013M126A uses MSB first for the row address*/
      reversed_row = SL_RBIT16(row_start + i + 1);

      /* Send multiple line address */
      /* Dummy data is only 6 bits and the address line is 10 bits */
      cmd = 0x3f | reversed_row;
#else
      /* Dummy data is 8 bits and the address line is 8 bit */
      cmd = 0xff | ((row_start + i + 1) << 8);
#endif
    }
    sli_memlcd_spi_tx(&spi_handle, &cmd, 2);
  }

  sli_memlcd_spi_wait(&spi_handle);

  /* SCS hold time */
  sl_udelay_wait(device->hold_us);

  /* De-assert SCS */
  sl_gpio_clear_pin(&memlcd_spi_cs_gpio);

  /* Clean up garbage RX data */
  /* This is important when paired with others slaves */
  sli_memlcd_spi_rx_flush(&spi_handle);

  return SL_STATUS_OK;
}

const sl_memlcd_t *sl_memlcd_get(void)
{
  if (initialized) {
    return &memlcd_instance;
  } else {
    return NULL;
  }
}

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && defined(SL_MEMLCD_USE_EUSART)
/***************************************************************************//**
 * Power Manager callback notification for EUSART.
 * It is used to prepare EUSART module before/after deepsleeping.
 ******************************************************************************/
static void on_power_manager_event(sl_power_manager_em_t from,
                                   sl_power_manager_em_t to)
{
  (void)from;

  if (to == SL_POWER_MANAGER_EM1
      || to == SL_POWER_MANAGER_EM0) {
    sli_memlcd_spi_exit_em23(&spi_handle);
  }

  if (to == SL_POWER_MANAGER_EM2
      || to == SL_POWER_MANAGER_EM3) {
    sli_memlcd_spi_enter_em23(&spi_handle);
  }
}
#endif

#if defined (SL_MEMLCD_EXTCOMIN_PORT)
/**************************************************************************//**
 * @brief
 *   Inverse polarity across the Liquid Crystal cells in the display.
 *
 * @detail
 *   This function inverses the polarity across the Liquid Crystal cells
 *   in the LCD. Must be called at least every second.
 *****************************************************************************/
static void extcomin_toggle(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void) handle;
  (void) data;

  sl_gpio_t memlcd_extcomin_gpio = {
    .port = SL_MEMLCD_EXTCOMIN_PORT,
    .pin = SL_MEMLCD_EXTCOMIN_PIN,
  };

  sl_gpio_toggle_pin(&memlcd_extcomin_gpio);
}
#endif
