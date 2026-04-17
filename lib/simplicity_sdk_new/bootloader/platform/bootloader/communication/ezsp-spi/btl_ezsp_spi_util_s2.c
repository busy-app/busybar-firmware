/***************************************************************************//**
 * @file
 * @brief Implements the utility functionality for the EZSP SPI protocol.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "btl_ezsp_spi.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "driver/btl_driver_delay.h"
// -‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Functions

void bootloader_ezsp_init(void)
{
  // Setup EZSP-specific GPIO (nHOST_INT, nWAKE)
#if defined(CMU_CTRL_HFPERCLKEN)
  CMU->CTRL |= CMU_CTRL_HFPERCLKEN;
  CMU_ClockEnable(cmuClock_GPIO, true);
#elif defined(_CMU_CLKEN0_MASK)
  // Enable GPIO clock
  CMU->CLKEN0_SET = CMU_CLKEN0_GPIO;
#endif

  GPIO_PinModeSet(SL_EZSPSPI_HOST_INT_PORT,
                  SL_EZSPSPI_HOST_INT_PIN,
                  gpioModePushPull,
                  1);
  GPIO_PinModeSet(SL_EZSPSPI_WAKE_INT_PORT,
                  SL_EZSPSPI_WAKE_INT_PIN,
                  gpioModeInputPull,
                  1);
}

// Checks the WAKE line and performs handshake if needed
void bootloader_ezsp_wakeHandshake(void)
{
  unsigned int hostIntState = GPIO_PinOutGet(SL_EZSPSPI_HOST_INT_PORT,
                                             SL_EZSPSPI_HOST_INT_PIN);

  if (hostIntState == 0) {
    // AN711 10: A wake handshake cannot be performed if nHOST_INT is already
    // asserted.
    return;
  }

  // check for wake up sequence
  if (GPIO_PinInGet(SL_EZSPSPI_WAKE_INT_PORT, SL_EZSPSPI_WAKE_INT_PIN) == 0) {
    // This is a handshake. Assert nHOST_INT until WAKE deasserts
    nHOST_ASSERT();
    while (GPIO_PinInGet(SL_EZSPSPI_WAKE_INT_PORT, SL_EZSPSPI_WAKE_INT_PIN)
           == 0) ;
    delay_microseconds(20);
    nHOST_DEASSERT();
    delay_microseconds(50);
  }
}

void bootloader_ezsp_shutdown(void)
{
  // Reset specific GPIO to Hi-Z
  GPIO_PinModeSet(SL_EZSPSPI_HOST_INT_PORT,
                  SL_EZSPSPI_HOST_INT_PIN,
                  gpioModeDisabled,
                  0);
  GPIO_PinModeSet(SL_EZSPSPI_WAKE_INT_PORT,
                  SL_EZSPSPI_WAKE_INT_PIN,
                  gpioModeDisabled,
                  0);
}
