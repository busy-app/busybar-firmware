/***************************************************************************/ /**
 * @file  siwx917_soc_ncp_host.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_wifi_constants.h"
#include "sl_si91x_host_interface.h"
#include "sl_board_configuration.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "cmsis_os2.h" // CMSIS RTOS2
#include "sl_si91x_types.h"
#include <stdbool.h>
#include <string.h>
#include "rsi_m4.h"
#include "sl_common.h"
#include "si91x_device.h"
#include "core_cm4.h"

extern int16_t sli_mem_rd(uint32_t addr, uint16_t len, uint8_t *dBuf);
void sli_si91x_ta_events_init(void);
sl_status_t sli_si91x_bus_set_interrupt_mask(uint32_t mask);
sl_status_t sli_si91x_bus_enable_high_speed(void);
sl_status_t sl_si91x_bus_write_memory(uint32_t addr, uint16_t length, const uint8_t *buffer);
sl_status_t sl_si91x_bus_read_memory(uint32_t addr, uint16_t length, const uint8_t *buffer);

void sl_si91x_host_enable_high_speed_bus()
{
  //! Sets specific control register bits
  //! Sets the baud rate
}

sl_status_t sli_si91x_bus_rx_irq_handler(void)
{
  return SL_STATUS_OK;
}

void sli_si91x_bus_rx_done_handler(void)
{
  return;
}

sl_status_t sli_si91x_bus_set_interrupt_mask(uint32_t mask)
{
  UNUSED_PARAMETER(mask);
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_bus_enable_high_speed(void)
{
  return SL_STATUS_OK;
}

void sl_si91x_host_set_sleep_indicator(void)
{
  // sets a sleep indicator by activating a specific GPIO pin
}

uint32_t sl_si91x_host_get_wake_indicator(void)
{
  return 1;
}

sl_status_t sl_si91x_host_init(const sl_si91x_host_init_configuration_t *config)
{
  UNUSED_PARAMETER(config);
  // Initialize SI91X NWP events
  sli_si91x_ta_events_init();
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_host_deinit(void)
{
  return SL_STATUS_OK;
}

void sl_si91x_host_hold_in_reset(void)
{
  // initializes and activates a GPIO pin to forcefully reset or disable a connected device.
}

void sl_si91x_host_release_from_reset(void)
{
  // Release the reset state of a connected device
}

void sl_si91x_host_enable_bus_interrupt(void)
{
  // allowing the system to respond to specific events
}

void sl_si91x_host_disable_bus_interrupt(void)
{
  // Preventing the system from responding to events
}

sl_status_t sl_si91x_bus_write_memory(uint32_t addr, uint16_t length, const uint8_t *buffer)
{
  UNUSED_PARAMETER(length);
  *(uint32_t *)addr = *(const uint32_t *)buffer;
  return 0;
}

sl_status_t sl_si91x_bus_read_memory(uint32_t addr, uint16_t length, const uint8_t *buffer)
{
  UNUSED_PARAMETER(addr);
  UNUSED_PARAMETER(length);
  UNUSED_PARAMETER(buffer);
  // Read memory
  return 0;
}
bool sl_si91x_host_is_in_irq_context(void)
{
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U;
}
