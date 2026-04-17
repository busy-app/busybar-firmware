/**
 * Indicator LED support.
 *
 * @copyright 2018 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include "board_indicator.h"
#include "zpal_log.h"
#include <assert.h>
#include "em_letimer.h"
#include "em_cmu.h"
#include "sl_simple_led_instances.h"
#include "sl_sleeptimer.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
#include "zw_shutdown_manager.h"
#endif
/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/
typedef struct _timer_settings{
  uint32_t on_time_ms;
  uint32_t off_time_ms;
  uint32_t cycles;
} timer_settings;

#define INDEFINITE_NUMBER_OF_BLINKING_CYCLES 0xFFFFFFFF
/****************************************************************************/
/*                              PRIVATE DATA                                */
/****************************************************************************/

static bool m_indicator_initialized    = false;
static bool m_indicator_active_from_cc = false;
#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
static bool m_shutdown_lock_taken      = false;
#endif

static timer_settings indicator_settings;

static sl_sleeptimer_timer_handle_t my_sleeptimer_handle;
/****************************************************************************/
/*                      PRIVATE FUNCTIONS                                   */
/****************************************************************************/
static void sleeptimer_off_cb(sl_sleeptimer_timer_handle_t *handle, void *data);
static void sleeptimer_on_cb(sl_sleeptimer_timer_handle_t *handle, void *data);

static void sleeptimer_on_cb(__attribute__((unused)) sl_sleeptimer_timer_handle_t *handle, __attribute__((unused)) void *data)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_HW, "%s on=%u, off=%u, num=%u\n", __func__, indicator_settings.on_time_ms, indicator_settings.off_time_ms, indicator_settings.cycles);
  sl_simple_led_turn_on(sl_led_led0.context);
  sl_sleeptimer_start_timer_ms(&my_sleeptimer_handle, indicator_settings.on_time_ms, sleeptimer_off_cb, NULL, 0, 0);
}

static void sleeptimer_off_cb(__attribute__((unused)) sl_sleeptimer_timer_handle_t *handle, __attribute__((unused)) void *data)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_HW, "%s on=%u, off=%u, num=%u\n", __func__, indicator_settings.on_time_ms, indicator_settings.off_time_ms, indicator_settings.cycles);

  sl_simple_led_turn_off(sl_led_led0.context);

  if (indicator_settings.cycles > 1) {
    if (indicator_settings.cycles != INDEFINITE_NUMBER_OF_BLINKING_CYCLES) {
      indicator_settings.cycles--;
    }
    sl_sleeptimer_start_timer_ms(&my_sleeptimer_handle, indicator_settings.off_time_ms, sleeptimer_on_cb, NULL, 0, 0);
  } else {
    m_indicator_active_from_cc = false;
#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
    if (m_shutdown_lock_taken) {
      m_shutdown_lock_taken = false;
      zw_shutdown_manager_release_lock();
    }
#endif
  }
}

/****************************************************************************/
/*                      PUBLIC FUNCTIONS                                    */
/****************************************************************************/
void Board_IndicateStatus(board_status_t status)
{
  switch (status) {
    case BOARD_STATUS_LEARNMODE_ACTIVE:
      /* Blink indicator at 1 Hz with LED on for 100 ms */
      Board_IndicatorControl(100, 900, 0, false);
      break;
    default:
      /* Turn off the indicator LED */
      Board_IndicatorControl(0, 0, 0, false);
      break;
  }
}

void Board_IndicatorInit(void)
{
  if (!m_indicator_initialized) {
    m_indicator_initialized = true;
  }
}

bool Board_IndicatorControl(uint32_t on_time_ms,
                            uint32_t off_time_ms,
                            uint32_t num_cycles,
                            bool called_from_indicator_cc)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_HW, "Board_IndicatorControl() on=%u, off=%u, num=%u\n", on_time_ms, off_time_ms, num_cycles);

  if (!m_indicator_initialized) {
    assert(false);
    return false;
  }

  indicator_settings.cycles = num_cycles;
  if ((on_time_ms == 0) && (off_time_ms == 0)) {
    sl_sleeptimer_stop_timer(&my_sleeptimer_handle);
    sl_simple_led_turn_off(sl_led_led0.context);
    indicator_settings.cycles = 0;
    indicator_settings.off_time_ms = 0;
    indicator_settings.on_time_ms  = 0;
    m_indicator_active_from_cc = false;
#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
    if (m_shutdown_lock_taken) {
      m_shutdown_lock_taken = false;
      zw_shutdown_manager_release_lock();
    }
#endif
  } else {
    if (indicator_settings.cycles == 0) {
      // Indefinite long blinking
      indicator_settings.cycles = INDEFINITE_NUMBER_OF_BLINKING_CYCLES;
    }

    indicator_settings.off_time_ms = off_time_ms;
    indicator_settings.on_time_ms  = on_time_ms;

    sl_simple_led_turn_on(sl_led_led0.context);
    sl_sleeptimer_start_timer_ms(&my_sleeptimer_handle, indicator_settings.on_time_ms, sleeptimer_off_cb, NULL, 0, 0);
#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
    if (!m_shutdown_lock_taken) {
      zw_shutdown_manager_add_lock();
      m_shutdown_lock_taken = true;
    }
#endif
    m_indicator_active_from_cc = called_from_indicator_cc;
  }

  return true;
}

bool Board_IsIndicatorActive(void)
{
  return m_indicator_active_from_cc;
}

void cc_indicator_handler(uint32_t on_time_ms, uint32_t off_time_ms, uint32_t num_cycles)
{
  Board_IndicatorControl(on_time_ms, off_time_ms, num_cycles, true);
}
