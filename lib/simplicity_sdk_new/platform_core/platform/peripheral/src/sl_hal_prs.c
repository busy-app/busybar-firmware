/***************************************************************************//**
 * @file
 * @brief Peripheral Reflex System (PRS) Peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_hal_prs.h"
#if defined(PRS_COUNT) && (PRS_COUNT > 0)

#include "sl_assert.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static sl_hal_prs_sync_producer_signal_t prs_get_sync_channel_signal(uint8_t channel);
static sl_hal_prs_async_producer_signal_t prs_get_async_channel_signal(uint8_t channel);

static bool prs_is_async_channel_free(uint8_t channel);
static bool prs_find_free_channel_in_range(uint8_t start_channel,
                                           uint8_t end_channel,
                                           uint8_t *channel);
static bool prs_find_free_channel_reverse(uint8_t start_channel,
                                          uint8_t end_channel,
                                          uint8_t *channel);
/** @endcond */

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE void sl_hal_prs_async_set_channel_swlevel(uint8_t channel,
                                                          bool level);
extern __INLINE bool sl_hal_prs_async_get_channel_swlevel(uint8_t channel);
extern __INLINE bool sl_hal_prs_get_channel_value(uint8_t channel,
                                                  sl_hal_prs_channel_type_t channel_type);
extern __INLINE void sl_hal_prs_async_set_channel_swpulse(uint8_t channel);
extern __INLINE void sl_hal_prs_async_connect_channel_producer(uint8_t channel,
                                                               sl_hal_prs_async_producer_signal_t producer_signal);
extern __INLINE void sl_hal_prs_sync_connect_channel_producer(uint8_t channel,
                                                              sl_hal_prs_sync_producer_signal_t producer_signal);
extern __INLINE void sl_hal_prs_async_combine_signals(uint8_t channel_a,
                                                      uint8_t channel_b,
                                                      sl_hal_prs_logic_t logic);

/***************************************************************************//**
 * Configure Async channel.
 ******************************************************************************/
void sl_hal_prs_async_init_channel(const sl_hal_prs_async_channel_init_t *init)
{
  EFM_ASSERT(init->channel < SL_HAL_PRS_ASYNC_CHAN_COUNT);

#if defined(_PRS_ASYNC_CH_CTRL_AUXSEL_MASK)
  sl_hal_prs_async_combine_signals(init->channel, init->aux_prs, init->logic);
#else
  sl_hal_prs_async_combine_signals(init->channel, init->channel - 1, init->logic);
#endif
  sl_hal_prs_async_connect_channel_producer(init->channel, init->producer_signal);
  sl_hal_prs_connect_channel_consumer(init->channel, SL_HAL_PRS_TYPE_ASYNC, init->consumer_event);
}

/***************************************************************************//**
 * Configure Sync channel.
 ******************************************************************************/
void sl_hal_prs_sync_init_channel(const sl_hal_prs_sync_channel_init_t *init)
{
  EFM_ASSERT(init->channel < SL_HAL_PRS_SYNC_CHAN_COUNT);

  sl_hal_prs_sync_connect_channel_producer(init->channel, init->producer_signal);
  sl_hal_prs_connect_channel_consumer(init->channel, SL_HAL_PRS_TYPE_SYNC, init->consumer_event);
}

/***************************************************************************//**
 * Search for the first free PRS channel.
 ******************************************************************************/
sl_status_t sl_hal_prs_get_free_channel(uint8_t *channel,
                                        sl_hal_prs_channel_type_t channel_type)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (channel_type == SL_HAL_PRS_TYPE_ASYNC) {
    sl_hal_prs_async_producer_signal_t producer_signal;

    for (uint8_t i = 0; i < SL_HAL_PRS_ASYNC_CHAN_COUNT; i++) {
      producer_signal = prs_get_async_channel_signal(i);
      if (producer_signal == SL_HAL_PRS_ASYNC_NONE) {
        *channel = i;
        status = SL_STATUS_OK;
        break;
      }
    }
  } else {
    sl_hal_prs_sync_producer_signal_t producer_signal;

    for (uint8_t i = 0; i < SL_HAL_PRS_SYNC_CHAN_COUNT; i++) {
      producer_signal = prs_get_sync_channel_signal(i);
      if (producer_signal == SL_HAL_PRS_SYNC_NONE) {
        *channel = i;
        status = SL_STATUS_OK;
        break;
      }
    }
  }

  return status;
}

/***************************************************************************//**
 * Reset all PRS channels.
 ******************************************************************************/
void sl_hal_prs_reset(void)
{
  PRS->ASYNC_SWLEVEL = 0;
  for (uint8_t i = 0; i < SL_HAL_PRS_ASYNC_CHAN_COUNT; i++) {
    PRS->ASYNC_CH[i].CTRL = _PRS_ASYNC_CH_CTRL_RESETVALUE;
  }
  for (uint8_t i = 0; i < SL_HAL_PRS_SYNC_CHAN_COUNT; i++) {
    PRS->SYNC_CH[i].CTRL = _PRS_SYNC_CH_CTRL_RESETVALUE;
  }
}

/***************************************************************************//**
 * Connect a peripheral consumer to a PRS channel.
 ******************************************************************************/
void sl_hal_prs_connect_channel_consumer(uint8_t channel,
                                         sl_hal_prs_channel_type_t channel_type,
                                         sl_hal_prs_consumer_event_t consumer_event)
{
  EFM_ASSERT((uint32_t)consumer_event < PER_REG_BLOCK_SET_OFFSET);

  volatile uint32_t * addr = (volatile uint32_t *) PRS;
  uint32_t offset = consumer_event;
  addr = addr + offset / 4;

  if (consumer_event != SL_HAL_PRS_CONSUMER_NONE) {
    if (channel_type == SL_HAL_PRS_TYPE_ASYNC) {
      EFM_ASSERT(channel < SL_HAL_PRS_ASYNC_CHAN_COUNT);

      *addr = channel << _PRS_CONSUMER_TIMER0_CC0_PRSSEL_SHIFT;
    } else {
      EFM_ASSERT(channel < SL_HAL_PRS_SYNC_CHAN_COUNT);

      *addr = channel << _PRS_CONSUMER_TIMER0_CC0_SPRSSEL_SHIFT;
    }
  }
}

/***************************************************************************//**
 * Send the output of a PRS channel to a GPIO pin.
 ******************************************************************************/
void sl_hal_prs_pin_output(uint8_t channel,
                           sl_hal_prs_channel_type_t channel_type,
                           sl_gpio_port_t port,
                           uint8_t pin)
{
  volatile uint32_t *addr;

  if (channel_type == SL_HAL_PRS_TYPE_ASYNC) {
    addr = &GPIO->PRSROUTE[0].ASYNCH0ROUTE;
  } else {
    addr = &GPIO->PRSROUTE[0].SYNCH0ROUTE;
  }
  addr += channel;
  *addr = ((uint32_t)port << _GPIO_PRS_ASYNCH0ROUTE_PORT_SHIFT)
          | ((uint32_t)pin << _GPIO_PRS_ASYNCH0ROUTE_PIN_SHIFT);

  if (channel_type == SL_HAL_PRS_TYPE_ASYNC) {
    GPIO->PRSROUTE[0].ROUTEEN |= 0x1 << (channel + _GPIO_PRS_ROUTEEN_ASYNCH0PEN_SHIFT);
  } else {
    GPIO->PRSROUTE[0].ROUTEEN |= 0x1 << (channel + _GPIO_PRS_ROUTEEN_SYNCH0PEN_SHIFT);
  }
}

/***************************************************************************//**
 * Get PRS source signal for an sync channel.
 ******************************************************************************/
static sl_hal_prs_sync_producer_signal_t prs_get_sync_channel_signal(uint8_t channel)
{
  sl_hal_prs_sync_producer_signal_t producer_signal;

  producer_signal = (PRS->SYNC_CH[channel].CTRL
                     & (_PRS_SYNC_CH_CTRL_SOURCESEL_MASK | _PRS_SYNC_CH_CTRL_SIGSEL_MASK));

  return producer_signal;
}

/***************************************************************************//**
 * Get PRS source signal for an async channel.
 ******************************************************************************/
static sl_hal_prs_async_producer_signal_t prs_get_async_channel_signal(uint8_t channel)
{
  sl_hal_prs_async_producer_signal_t producer_signal;

  producer_signal = (PRS->ASYNC_CH[channel].CTRL
                     & (_PRS_ASYNC_CH_CTRL_SOURCESEL_MASK | _PRS_ASYNC_CH_CTRL_SIGSEL_MASK));

  return producer_signal;
}

/***************************************************************************//**
 * Check if an async PRS channel is free.
 ******************************************************************************/
static bool prs_is_async_channel_free(uint8_t channel)
{
  return ((PRS->ASYNC_CH[channel].CTRL
           & (_PRS_ASYNC_CH_CTRL_SOURCESEL_MASK | _PRS_ASYNC_CH_CTRL_SIGSEL_MASK))
          == (PRS_ASYNC_CH_CTRL_SOURCESEL_DEFAULT | PRS_ASYNC_CH_CTRL_SIGSEL_DEFAULT));
}

/***************************************************************************//**
 * Search for free channel in specific range.
 ******************************************************************************/
static bool prs_find_free_channel_in_range(uint8_t start,
                                           uint8_t end,
                                           uint8_t *channel)
{
  for (uint8_t ch = start; ch <= end; ch++) {
    if (prs_is_async_channel_free(ch)) {
      *channel = ch;
      return true;
    }
  }
  return false;
}

/***************************************************************************//**
 * Search for free channel from highest to lowest in specific range.
 ******************************************************************************/
static bool prs_find_free_channel_reverse(uint8_t start,
                                          uint8_t end,
                                          uint8_t *channel)
{
  for (uint8_t ch = end; ch >= start; ch--) {
    if (prs_is_async_channel_free(ch)) {
      *channel = ch;
      return true;
    }
  }
  return false;
}

/***************************************************************************//**
 * Search for a free async PRS channel with optional GPIO port filtering.
 ******************************************************************************/
sl_status_t sl_hal_prs_get_free_async_channel_for_gpio(uint8_t *channel,
                                                       const sl_gpio_t *gpio_port_pin)
{
  if (channel == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  *channel = 0xFFU; // Initialize to invalid channel
  bool found = false;

  if (gpio_port_pin != NULL) {
    // GPIO port is specified, use GPIO-aware allocation
    if (gpio_port_pin->port < SL_GPIO_PORT_C) {
      // Get free PRS signal for GPIO Ports A and B.
      found = prs_find_free_channel_in_range(SL_HAL_PRS_FIRST_ASYNC_CHANNEL_GPIO_PAB,
                                             SL_HAL_PRS_FIRST_ASYNC_CHANNEL_GPIO_PCD - 1,
                                             channel);
    } else {
      // Get free PRS signal for GPIO Ports C and D.
      found = prs_find_free_channel_in_range(SL_HAL_PRS_FIRST_ASYNC_CHANNEL_GPIO_PCD,
                                             SL_HAL_PRS_LAST_ASYNC_CHANNEL_GPIO_PCD,
                                             channel);
    }
  } else {
    // No GPIO port specified, search all channels starting from highest for better resource management
    found = prs_find_free_channel_reverse(SL_HAL_PRS_FIRST_ASYNC_CHANNEL_GPIO_PAB,
                                          SL_HAL_PRS_LAST_ASYNC_CHANNEL_GPIO_PCD,
                                          channel);
  }

  return found ? SL_STATUS_OK : SL_STATUS_NO_MORE_RESOURCE;
}

#endif /* defined(PRS_COUNT) && (PRS_COUNT > 0) */
