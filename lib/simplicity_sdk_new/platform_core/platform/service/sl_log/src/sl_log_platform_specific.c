/***************************************************************************//**
 * @file
 * @brief Platform specific helpers for the logging core
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
#include "sl_clock_manager.h"
#include "sl_interrupt_manager.h"
#include "sl_log_platform_specific.h"
#include "sl_log_common_config.h"
#include "sl_log_helper.h"
#include "em_device.h"
#include "sl_hal_timer.h"

/*******************************************************************************
*******************************   DEFINES   ***********************************
*******************************************************************************/

/**
 * @brief Host timestamp timer frequency
 *
 * @note
 *  The underlying timestamp timer operates at a hardware frequency of 10MHz.
 *  However, the count values returned by the timer are always divided by 10
 *  before being provided to the logger service. This effectively reduces the
 *  timer's observable frequency for logging purposes to 1MHz.
 */
#define TIMER_FREQUENCY     1000000
#define TIMER_TOP_VALUE     0xFFFFFFFF

#define _CONCAT_TWO_TOKENS(token_1, token_2)                     token_1 ## token_2
#define _CONCAT_THREE_TOKENS(token_1, token_2, token_3)          token_1 ## token_2 ## token_3
#define CONCAT_TWO_TOKENS(token_1, token_2)                      _CONCAT_TWO_TOKENS(token_1, token_2)
#define CONCAT_THREE_TOKENS(token_1, token_2, token_3)           _CONCAT_THREE_TOKENS(token_1, token_2, token_3)

#define TIMER_INSTANCE      TIMER(SL_LOG_CONFIG_TIMER_INSTANCE)
#define TIMER_BUS_CLOCK     CONCAT_TWO_TOKENS(SL_BUS_CLOCK_TIMER, SL_LOG_CONFIG_TIMER_INSTANCE)
#define LOGGER_TIMER_IRQ         CONCAT_THREE_TOKENS(TIMER, SL_LOG_CONFIG_TIMER_INSTANCE, _IRQn)
#define LOGGER_TIMER_IRQHandler  CONCAT_THREE_TOKENS(TIMER, SL_LOG_CONFIG_TIMER_INSTANCE, _IRQHandler)

/*******************************************************************************
**************************   LOCAL FUNCTIONS   ********************************
*******************************************************************************/

/**
 * @brief Start the platform timestamp counter used by the logging core.
 *
 * Initializes and starts the TIMERn used to provide microsecond
 * resolution timestamps for log events.
 *
 * @return SL_STATUS_OK on success or an sl_status_t error code.
 */
sl_status_t sl_log_hal_platform_core_init(void)
{
  sl_clock_manager_enable_bus_clock(TIMER_BUS_CLOCK);

  sl_hal_timer_config_t init_config = SL_HAL_TIMER_CONFIG_DEFAULT;
  init_config.prescaler = SL_HAL_TIMER_PRESCALER_DIV8;

  sl_hal_timer_init(TIMER_INSTANCE, &init_config);
  sl_hal_timer_enable(TIMER_INSTANCE);
  sl_hal_timer_set_top(TIMER_INSTANCE, TIMER_TOP_VALUE);
  sl_hal_timer_start(TIMER_INSTANCE);
  sl_hal_timer_clear_interrupts(TIMER_INSTANCE, TIMER_IEN_OF);
  sl_hal_timer_enable_interrupts(TIMER_INSTANCE, TIMER_IEN_OF);

  sl_interrupt_manager_clear_irq_pending(LOGGER_TIMER_IRQ);
  sl_interrupt_manager_enable_irq(LOGGER_TIMER_IRQ);

  return SL_STATUS_OK;
}

/**
 * @brief Stop the platform timestamp counter.
 *
 * Stops the TIMERn peripheral.
 *
 * @return SL_STATUS_OK on success or an sl_status_t error code.
 */
sl_status_t sl_log_hal_core_deinit(void)
{
  sl_hal_timer_stop(TIMER_INSTANCE);
  return SL_STATUS_OK;
}

/**
 * @brief Get the current timestamp count for the specified core.
 *
 * For the host (core_id == 0) this reads the local timer and applies the
 * synchronization delta.
 *
 * @param[in] core_id Core identifier (0 = host)
 * @return Current timestamp in microseconds
 */
uint32_t sl_log_hal_get_timestamp_count(uint8_t core_id)
{
  (void)core_id;

  /* Each tick represents 0.1 microseconds.
   * This function returns the count value converted to microseconds.
   */
  return sl_hal_timer_get_counter(TIMER_INSTANCE) / 10;
}

/**
 * @brief Get the timestamp timer frequency (Hz) for a core.
 *
 * @param[in] core_id Core identifier (unused)
 * @return Timer frequency in Hz (typically 1,000,000)
 */
uint32_t sl_log_hal_get_timestamp_timer_frequency(uint8_t core_id)
{
  (void)core_id;

  return TIMER_FREQUENCY;
}

/**
 * @brief Prepare logging subsystem before entering sleep.
 *
 * Typical actions include stopping timers and deinitializing backend
 * transports to reduce power consumption.
 *
 * @param[in] config Pointer to logging configuration
 * @return SL_STATUS_OK on success or an sl_status_t error code
 */
sl_status_t sl_log_hal_pre_sleep_process(void *args)
{
  (void)args;

  /* TODO: Implement this feature in the next development phase. */
  return SL_STATUS_OK;
}

/**
 * @brief Restore logging subsystem after wake-up from sleep.
 *
 * Reinitialize backend transports and restart the timestamp counter.
 *
 * @param[in] config Pointer to logging configuration
 * @return SL_STATUS_OK on success or an sl_status_t error code
 */
sl_status_t sl_log_hal_post_sleep_process(void *args)
{
  (void)args;

  /* TODO: Implement this feature in the next development phase. */
  return SL_STATUS_OK;
}

/**
 * @brief Set the platform-specific logging configuration.
 *
 * @param[in] args Pointer to platform config structure
 * @param[in] core_id Core identifier
 * @return SL_STATUS_OK currently always returned
 */
sl_status_t sl_log_hal_set_configuration(void *args, uint8_t core_id)
{
  (void)args;
  (void)core_id;

  return SL_STATUS_OK;
}

/**
 * @brief Get the platform-specific logging configuration.
 *
 * @param[out] args Pointer to a platform config structure to fill
 * @param[in]  core_id Core identifier
 * @return SL_STATUS_OK currently always returned
 */
sl_status_t sl_log_hal_get_configuration(void *args, uint8_t core_id)
{
  (void)args;
  (void)core_id;

  return SL_STATUS_OK;
}

/**
 * @brief   Core API structure.
 *
 */
sl_log_api_core_t sl_log_api_core = { .platform_core_init       = sl_log_hal_platform_core_init,
                                      .platform_core_deinit        = sl_log_hal_core_deinit,
                                      .get_timestamp                 = sl_log_hal_get_timestamp_count,
                                      .get_timestamp_timer_frequency = sl_log_hal_get_timestamp_timer_frequency,
                                      .post_sleep_process            = sl_log_hal_post_sleep_process,
                                      .pre_sleep_process             = sl_log_hal_pre_sleep_process,
                                      .set_configuration             = sl_log_hal_set_configuration,
                                      .get_configuration             = sl_log_hal_get_configuration };

/**
 * @brief Return pointer to the core API structure.
 *
 * Provides the generic logging core APIs
 *
 * @return Pointer to the populated sl_log_api_backend_t structure.
 */
sl_log_api_core_t *sl_log_get_api_core(void)
{
  return &sl_log_api_core;
}

/*******************************************************************************
 * TIMER interrupt handler.
 ******************************************************************************/
void LOGGER_TIMER_IRQHandler(void)
{
  uint32_t irq_flag = sl_hal_timer_get_pending_interrupts(TIMER_INSTANCE);

  if (irq_flag & TIMER_IEN_OF) {
      sl_hal_timer_clear_interrupts(TIMER_INSTANCE, irq_flag & TIMER_IEN_OF);
      /* TODO: Implement logic to store the overflow count in the ring buffer
       * without triggering a log_flush operation.
       */
  }
}