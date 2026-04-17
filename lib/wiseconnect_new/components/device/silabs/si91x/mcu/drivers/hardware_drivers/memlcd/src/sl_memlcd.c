/***************************************************************************/ /**
 * @file
 * @brief Memory LCD interface
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

#include "sl_memlcd.h"
#include "sl_sleeptimer.h"
#include "em_device.h"
#include "sl_ulp_timer_instances.h"
#include "sl_si91x_ulp_timer_common_config.h"
#include "rsi_power_save.h"
#include "sl_si91x_peripheral_gpio.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS ********************************
 ******************************************************************************/

#define SL_MEMLCD_SPI_CS_PORT RTE_MEMLCD_CS_PORT
#define SL_MEMLCD_SPI_CS_PIN  RTE_MEMLCD_CS_PIN

#define SL_MEMLCD_EXTCOMIN_PIN  RTE_MEMLCD_EXTCOMIN_PIN
#define SL_MEMLCD_EXTCOMIN_PORT RTE_MEMLCD_EXTCOMIN_PORT

#define SL_BOARD_ENABLE_DISPLAY_PIN  RTE_MEMLCD_ENABLE_DISPLAY_PIN
#define SL_BOARD_ENABLE_DISPLAY_PORT RTE_MEMLCD_ENABLE_DISPLAY_PORT

#define CMD_UPDATE    0x01
#define CMD_ALL_CLEAR 0x04

#define ONE_USEC 32 // Ticks required for every one microsecond

#define EGPIO_ULP_PORT 4

/*******************************************************************************
 ************************ Global function Prototypes ***************************
 ******************************************************************************/

#if defined(SL_MEMLCD_EXTCOMIN_PORT)
typedef struct sl_sleeptimer_timer_handle sl_sleeptimer_timer_handle_t;
/** Timer used for periodic maintenance of the display. */
static sl_sleeptimer_timer_handle_t extcomin_timer;
static void extcomin_toggle(sl_sleeptimer_timer_handle_t *handle, void *data);
#endif

static void delay_us(uint16_t time_us);

/*******************************************************************************
 **************************** Global Variables *********************************
 ******************************************************************************/

/** Memory lcd instance. This variable will be initialized in the
 *  @ref sl_memlcd_configure() function. */
static sl_memlcd_t memlcd_instance;
struct sl_memlcd_t *memlcd_post_wakeup_handle;

/** Flag to monitor is this driver has been initialized. The memlcd_instance
 *  is only valid after initialized=true. */
static bool initialized = false;

/*******************************************************************************
 * Function to generate delay
 *
 * @param time_us (uint16_t) time in second
 * @return none
 ******************************************************************************/
static void delay_us(uint16_t time_us)
{
  uint32_t start_time;
  uint32_t temp;
  sl_si91x_ulp_timer_restart(TIMER_0);
  sl_si91x_ulp_timer_get_count(TIMER_0, &start_time);
  while (true) {
    sl_si91x_ulp_timer_get_count(TIMER_0, &temp);
    if ((temp - start_time) > (time_us * ONE_USEC)) {
      break;
    }
  }
}

sl_status_t sl_memlcd_configure(struct sl_memlcd_t *device)
{
  /* Initialize the SPI bus. */
  sli_memlcd_spi_init(device->spi_freq);

  /* Setup GPIOs */
  sl_gpio_set_pin_mode(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN, SL_GPIO_MODE_0, 0);
  // Set output direction
  sl_si91x_gpio_set_pin_direction(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN, GPIO_OUTPUT);
  //clearing the GPIO pin
  sl_gpio_clear_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN);

#if defined(SL_MEMLCD_EXTCOMIN_PORT)
#if (SL_SI91X_ACX_MODULE == 1)
  /* Setup GPIOs */
  sl_gpio_set_pin_mode(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN, SL_GPIO_MODE_0, 0);
  // Set output direction
  sl_si91x_gpio_set_pin_direction(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN, GPIO_OUTPUT);
  //clearing the GPIO pin
  sl_gpio_clear_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN);
#else
  // NPSS GPIO
  sl_si91x_gpio_select_uulp_npss_receiver(SL_MEMLCD_EXTCOMIN_PIN, GPIO_RECEIVER_EN);
  sl_si91x_gpio_set_uulp_npss_pin_mux(SL_MEMLCD_EXTCOMIN_PIN, NPSS_GPIO_PIN_MUX_MODE0);
  sl_si91x_gpio_set_uulp_npss_direction(SL_MEMLCD_EXTCOMIN_PIN, GPIO_OUTPUT);
  sl_si91x_gpio_set_uulp_npss_pin_value(SL_MEMLCD_EXTCOMIN_PIN, GPIO_PIN_CLEAR);
#endif
#endif

  memlcd_instance = *device;
  initialized     = true;
  sl_memlcd_power_on(device, true);
  sl_memlcd_clear(device);

  return SL_STATUS_OK;
}

sl_status_t sl_memlcd_refresh(const struct sl_memlcd_t *device)
{
  sli_memlcd_spi_init(device->spi_freq);
  return SL_STATUS_OK;
}

void sl_memlcd_display_enable(void)
{
  // Enabling LCD display
  //enable the display pin in reciever mode
  sl_si91x_gpio_select_uulp_npss_receiver(SL_BOARD_ENABLE_DISPLAY_PIN, GPIO_RECEIVER_EN);

  //set the pin mux mode
  sl_si91x_gpio_set_uulp_npss_pin_mux(SL_BOARD_ENABLE_DISPLAY_PIN, NPSS_GPIO_PIN_MUX_MODE0);

  // set the direction of the display pin in UULP GPIO instance
  sl_si91x_gpio_set_uulp_npss_direction(SL_BOARD_ENABLE_DISPLAY_PIN, GPIO_OUTPUT);

  //set the display pin in UULP GPIO instance
  sl_si91x_gpio_set_uulp_npss_pin_value(SL_BOARD_ENABLE_DISPLAY_PIN, GPIO_PIN_SET);
}

void sl_memlcd_display_disable(void)
{
  // Disabling LCD display
  //enable the display pin in reciever mode
  sl_si91x_gpio_select_uulp_npss_receiver(SL_BOARD_ENABLE_DISPLAY_PIN, GPIO_RECEIVER_EN);

  //set the pin mux mode
  sl_si91x_gpio_set_uulp_npss_pin_mux(SL_BOARD_ENABLE_DISPLAY_PIN, NPSS_GPIO_PIN_MUX_MODE0);

  // set the direction of the display pin in UULP GPIO instance
  sl_si91x_gpio_set_uulp_npss_direction(SL_BOARD_ENABLE_DISPLAY_PIN, GPIO_OUTPUT);

  //clear the display pin in UULP GPIO instance
  sl_si91x_gpio_set_uulp_npss_pin_value(SL_BOARD_ENABLE_DISPLAY_PIN, GPIO_PIN_CLEAR);
}

sl_status_t sl_memlcd_power_on(const struct sl_memlcd_t *device, bool on)
{
  (void)device;
  (void)on;

  sl_status_t status = SL_STATUS_OK;

#if defined(SL_MEMLCD_EXTCOMIN_PORT)
  if (on) {
    status        = sl_sleeptimer_init();
    uint32_t freq = sl_sleeptimer_get_timer_frequency();
    status        = sl_sleeptimer_restart_periodic_timer(&extcomin_timer,
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
  cmd = CMD_ALL_CLEAR;

  /* Set SCS */
  sl_gpio_set_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN);
  /* SCS setup time */
  delay_us(device->setup_us);
  /* Send command */
  sli_memlcd_spi_tx(&cmd, 2);
  sli_memlcd_spi_wait();

  /* SCS hold time */
  delay_us(device->hold_us);

  /* Clear SCS */
  sl_gpio_clear_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN);

  return SL_STATUS_OK;
}

sl_status_t sl_memlcd_draw(const struct sl_memlcd_t *device,
                           const void *data,
                           unsigned int row_start,
                           unsigned int row_count)
{
  unsigned int i;

  const uint8_t *p = data;
  uint16_t cmd;
  int row_len;
#if defined(SL_MEMLCD_LPM013M126A)
  uint16_t reversed_row;
#endif

  row_len = (device->width * device->bpp) / 8;
  row_start++;

  sl_gpio_set_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN);

  delay_us(device->setup_us);

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

  sli_memlcd_spi_tx(&cmd, 2);

  /* Get start address to draw from */
  for (i = 0; i < row_count; i++) {
    /* Send pixels for this line */

    sli_memlcd_spi_tx(p, row_len);

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

    sli_memlcd_spi_tx(&cmd, 2);
  }

  sli_memlcd_spi_wait();
  delay_us(device->hold_us);

  sl_gpio_clear_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN);
  /* Clean up garbage RX data */
  /* This is important when paired with others slaves */
  sli_memlcd_spi_rx_flush();

  return SL_STATUS_OK;
}

/*******************************************************************************
 * Function to re-initialize MEMLCD interface after sleep wakeup
 *
 * @param none
 * @return sl_status_t
 ******************************************************************************/
sl_status_t sl_memlcd_post_wakeup_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  RSI_PS_NpssPeriPowerUp(SLPSS_PWRGATE_ULP_MCURTC);
  RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_SSI | ULPSS_PWRGATE_ULP_MISC);

  sl_si91x_ulp_timer_configure_clock(&sl_timer_clk_handle);
  sl_si91x_ulp_timer_set_configuration(&sl_timer_handle_timer0);
  sl_si91x_ulp_timer_set_count(TIMER_0, ONE_USEC * 7000);
  sl_si91x_ulp_timer_start(TIMER_0);

  sl_memlcd_display_enable();

  memlcd_post_wakeup_handle = (struct sl_memlcd_t *)sl_memlcd_get();

  /* Initialize the SPI bus. */
  status = sli_memlcd_spi_init(memlcd_post_wakeup_handle->spi_freq);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  sl_gpio_set_pin_mode(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN, SL_GPIO_MODE_0, 0);
  // Set output direction
  sl_si91x_gpio_set_pin_direction(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN, GPIO_OUTPUT);
  //clearing the GPIO pin
  sl_gpio_clear_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_SPI_CS_PIN);

#if defined(SL_MEMLCD_EXTCOMIN_PORT)
#if (SL_SI91X_ACX_MODULE == 1)
  /* Setup GPIOs */
  sl_gpio_set_pin_mode(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN, SL_GPIO_MODE_0, 0);
  // Set output direction
  sl_si91x_gpio_set_pin_direction(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN, GPIO_OUTPUT);
  //clearing the GPIO pin
  sl_gpio_clear_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN);
#else
  // NPSS GPIO
  sl_si91x_gpio_select_uulp_npss_receiver(SL_MEMLCD_EXTCOMIN_PIN, GPIO_RECEIVER_EN);
  sl_si91x_gpio_set_uulp_npss_pin_mux(SL_MEMLCD_EXTCOMIN_PIN, NPSS_GPIO_PIN_MUX_MODE0);
  sl_si91x_gpio_set_uulp_npss_direction(SL_MEMLCD_EXTCOMIN_PIN, GPIO_OUTPUT);
  sl_si91x_gpio_set_uulp_npss_pin_value(SL_MEMLCD_EXTCOMIN_PIN, GPIO_PIN_CLEAR);
#endif
#endif

  status = sl_memlcd_power_on(memlcd_post_wakeup_handle, true);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
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

/**************************************************************************/ /**
 * @brief
 *   Inverse polarity across the Liquid Crystal cells in the display.
 *
 * @detail
 *   This function inverses the polarity across the Liquid Crystal cells
 *   in the LCD. Must be called at least every second.
 *****************************************************************************/
#if defined(SL_MEMLCD_EXTCOMIN_PORT)
static void extcomin_toggle(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;

#if (SL_SI91X_ACX_MODULE == 1)
  /* Setup GPIOs */
  sl_gpio_set_pin_mode(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN, SL_GPIO_MODE_0, 0);
  // Set output direction
  sl_si91x_gpio_set_pin_direction(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN, GPIO_OUTPUT);
  if (sl_gpio_get_pin_input(SL_GPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN) == true) {
    //clearing the GPIO pin
    sl_gpio_clear_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN);
  } else {
    //clearing the GPIO pin
    sl_gpio_set_pin_output(EGPIO_ULP_PORT, SL_MEMLCD_EXTCOMIN_PIN);
  }
#else
  if (sl_si91x_gpio_get_uulp_npss_pin(SL_MEMLCD_EXTCOMIN_PIN) == true) {
    sl_si91x_gpio_set_uulp_npss_pin_value(SL_MEMLCD_EXTCOMIN_PIN, GPIO_PIN_CLEAR);
  } else {
    sl_si91x_gpio_set_uulp_npss_pin_value(SL_MEMLCD_EXTCOMIN_PIN, GPIO_PIN_SET);
  }
#endif
}
#endif