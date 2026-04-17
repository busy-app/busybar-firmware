/***************************************************************************//**
 * @file
 * @brief SLEEPTIMER Hardware abstraction implementation for BURTC.
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

#include "em_device.h"
#if defined(_SILICON_LABS_32B_SERIES_2) || defined(_SILICON_LABS_32B_SERIES_3)

// Define module name for Power Manager debug feature
#define CURRENT_MODULE_NAME    "SLEEPTIMER_BURTC"

#include "sl_sleeptimer.h"
#include "sli_sleeptimer_hal.h"

#include "sl_core.h"

#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_burtc.h"

#define sleeptimer_hal_burtc_get_counter() BURTC_CounterGet()
#define sleeptimer_hal_burtc_get_compare() BURTC_CompareGet(0U)
#define sleeptimer_hal_burtc_set_compare(compare) BURTC_CompareSet(0, compare)
#define sleeptimer_hal_burtc_get_interrupts() BURTC_IntGet()
#define sleeptimer_hal_burtc_set_interrupts(flags) BURTC_IntSet(flags)
#define sleeptimer_hal_burtc_enable_interrupts(interrupts) BURTC_IntEnable(interrupts)
#define sleeptimer_hal_burtc_disable_interrupts(interrupts) BURTC_IntDisable(interrupts)
#define sleeptimer_hal_burtc_clear_interrupts(flags) BURTC_IntClear(flags)

#elif defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_hal_burtc.h"

#define sleeptimer_hal_burtc_get_counter() sl_hal_burtc_get_counter()
#define sleeptimer_hal_burtc_get_compare() sl_hal_burtc_get_compare()
#define sleeptimer_hal_burtc_set_compare(compare) sl_hal_burtc_set_compare(compare)
#define sleeptimer_hal_burtc_get_interrupts() sl_hal_burtc_get_pending_interrupts()
#define sleeptimer_hal_burtc_set_interrupts(flags) sl_hal_burtc_set_interrupts(flags)
#define sleeptimer_hal_burtc_enable_interrupts(interrupts) sl_hal_burtc_enable_interrupts(interrupts)
#define sleeptimer_hal_burtc_disable_interrupts(interrupts) sl_hal_burtc_disable_interrupts(interrupts)
#define sleeptimer_hal_burtc_clear_interrupts(flags) sl_hal_burtc_clear_interrupts(flags)

#endif

#include "sl_clock_manager.h"
#include "sl_device_peripheral.h"
#include "sl_interrupt_manager.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#if SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_BURTC

#if defined(_SILICON_LABS_32B_SERIES_0)
#error BURTC implementation of the sleeptimer not available on Series 0 chips
#endif

// Minimum difference between current count value and what the comparator of the timer can be set to.
// 1 tick is added to the minimum diff for the algorithm of compensation for the IRQ handler that
// triggers when CNT == compare_value + 1. For more details refer to sleeptimer_hal_set_compare() function's header.
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8)
#define SLEEPTIMER_COMPARE_MIN_DIFF  (5 + 1)
#else
#define SLEEPTIMER_COMPARE_MIN_DIFF  (4 + 1)
#endif

#define SLEEPTIMER_TMR_WIDTH (_BURTC_CNT_MASK)

static uint32_t get_time_diff(uint32_t a, uint32_t b);

/******************************************************************************
 * Convert HAL interrupt flag BURTC-interrupt-enable bitmask
 *****************************************************************************/
static uint32_t irqien_hal2burtc(uint8_t hal_flag)
{
  uint32_t burtc_if = 0u;

  if (hal_flag & SLEEPTIMER_EVENT_OF) {
    burtc_if |= BURTC_IEN_OF;
  }

  if (hal_flag & SLEEPTIMER_EVENT_COMP) {
    burtc_if |= BURTC_IEN_COMP;
  }

  return burtc_if;
}

/******************************************************************************
 * Convert BURTC interrupt flags to HAL events
 *****************************************************************************/
static uint8_t irqflags_burtc2hal(uint32_t burtc_flag)
{
  uint8_t hal_if = 0u;

  if (burtc_flag & BURTC_IF_OF) {
    hal_if |= SLEEPTIMER_EVENT_OF;
  }

  if (burtc_flag & BURTC_IF_COMP) {
    hal_if |= SLEEPTIMER_EVENT_COMP;
  }

  return hal_if;
}

/******************************************************************************
 * Initializes BURTC sleep timer.
 *****************************************************************************/
void sleeptimer_hal_init_timer()
{
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_BURTC);

#if defined(_SILICON_LABS_32B_SERIES_2)
  BURTC_Init_TypeDef burtc_init = BURTC_INIT_DEFAULT;

  burtc_init.start  = false;
  burtc_init.clkDiv = SL_SLEEPTIMER_FREQ_DIVIDER;
#if (SL_SLEEPTIMER_DEBUGRUN == 1)
  burtc_init.debugRun = true;
#endif

  BURTC_Init(&burtc_init);
  BURTC_IntDisable(_BURTC_IEN_MASK);
  BURTC_IntClear(_BURTC_IF_MASK);
  BURTC_CounterReset();

  BURTC_Start();
  BURTC_SyncWait();
#elif defined(_SILICON_LABS_32B_SERIES_3)
  sl_hal_burtc_init_config_t burtc_init = SL_HAL_BURTC_INIT_DEFAULT;

  burtc_init.clock_divider = SL_SLEEPTIMER_FREQ_DIVIDER;
#if (SL_SLEEPTIMER_DEBUGRUN == 1)
  burtc_init.debug_run = true;
#endif
  sl_hal_burtc_init(&burtc_init);
  sl_hal_burtc_enable();
  sl_hal_burtc_disable_interrupts(_BURTC_IEN_MASK);
  sl_hal_burtc_clear_interrupts(_BURTC_IF_MASK);
  sl_hal_burtc_reset_counter();

  sl_hal_burtc_start();
  sl_hal_burtc_wait_sync();
#endif

  // Setup BURTC interrupt
  sl_interrupt_manager_clear_irq_pending(BURTC_IRQn);
  sl_interrupt_manager_enable_irq(BURTC_IRQn);
}

/******************************************************************************
 * Gets BURTC counter.
 *****************************************************************************/
uint32_t sleeptimer_hal_get_counter(void)
{
  return sleeptimer_hal_burtc_get_counter();
}

/******************************************************************************
 * Gets BURTC compare value
 *****************************************************************************/
uint32_t sleeptimer_hal_get_compare(void)
{
  return sleeptimer_hal_burtc_get_compare();
}

/******************************************************************************
 * Sets BURTC compare value
 *
 * @note Compare match value is set to the requested value - 1. This is done
 * to compensate for the fact that the BURTC compare match interrupt always
 * triggers at the end of the requested ticks and that the IRQ handler is
 * executed when current tick count == compare_value + 1.
 *****************************************************************************/
void sleeptimer_hal_set_compare(uint32_t value)
{
  CORE_DECLARE_IRQ_STATE;
  uint32_t counter;
  uint32_t compare_current;
  uint32_t compare_new = value;

  CORE_ENTER_CRITICAL();
  counter = sleeptimer_hal_get_counter();
  compare_current = sleeptimer_hal_get_compare();

  if ((((sleeptimer_hal_burtc_get_interrupts()) & _BURTC_IF_COMP_MASK) != 0)
      || get_time_diff(compare_current, counter) > SLEEPTIMER_COMPARE_MIN_DIFF
      || compare_current == counter) {
    // Add margin if necessary
    if (get_time_diff(compare_new, counter) < SLEEPTIMER_COMPARE_MIN_DIFF) {
      compare_new = counter + SLEEPTIMER_COMPARE_MIN_DIFF;
    }

    // wrap around if necessary
    compare_new %= SLEEPTIMER_TMR_WIDTH;
    sleeptimer_hal_burtc_set_compare(compare_new - 1);
    sleeptimer_hal_enable_int(SLEEPTIMER_EVENT_COMP);
  }
  CORE_EXIT_CRITICAL();
}

/******************************************************************************
 * Enables BURTC interrupts.
 *****************************************************************************/
void sleeptimer_hal_enable_int(uint8_t local_flag)
{
  sleeptimer_hal_burtc_enable_interrupts(irqien_hal2burtc(local_flag));
}

/******************************************************************************
 * Disables BURTC interrupts.
 *****************************************************************************/
void sleeptimer_hal_disable_int(uint8_t local_flag)
{
  sleeptimer_hal_burtc_disable_interrupts(irqien_hal2burtc(local_flag));
}

/*******************************************************************************
 * Hardware Abstraction Layer to set timer interrupts.
 ******************************************************************************/
void sleeptimer_hal_set_int(uint8_t local_flag)
{
  sleeptimer_hal_burtc_set_interrupts(irqien_hal2burtc(local_flag));
}

/******************************************************************************
 * Gets status of specified interrupt.
 *
 * Note: This function must be called with interrupts disabled.
 *****************************************************************************/
bool sli_sleeptimer_hal_is_int_status_set(uint8_t local_flag)
{
  bool int_is_set = false;

  uint32_t irq_flag = sleeptimer_hal_burtc_get_interrupts();

  switch (local_flag) {
    case SLEEPTIMER_EVENT_COMP:
      int_is_set = (irq_flag & BURTC_IF_COMP);
      break;

    case SLEEPTIMER_EVENT_OF:
      int_is_set = (irq_flag & BURTC_IF_OF);
      break;

    default:
      break;
  }

  return int_is_set;
}

/*******************************************************************************
 * Gets BURTC timer frequency.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_timer_frequency(void)
{
  uint32_t frequency;
  sl_clock_branch_t clock_branch;

  clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_BURTC);
  sl_clock_manager_get_clock_branch_frequency(clock_branch, &frequency);
  return (frequency >> (sleeptimer_hal_presc_to_log2(SL_SLEEPTIMER_FREQ_DIVIDER - 1)));
}

/*******************************************************************************
 * BURTC interrupt handler.
 ******************************************************************************/
void BURTC_IRQHandler(void)
{
  CORE_DECLARE_IRQ_STATE;
  uint8_t local_flag = 0;
  uint32_t irq_flag;

  CORE_ENTER_ATOMIC();

  irq_flag = sleeptimer_hal_burtc_get_interrupts();

  local_flag = irqflags_burtc2hal(irq_flag);

  sleeptimer_hal_burtc_clear_interrupts(irq_flag & (BURTC_IF_OF | BURTC_IF_COMP));
  process_timer_irq(local_flag);

  CORE_EXIT_ATOMIC();
}

/*******************************************************************************
 * Computes difference between two times taking into account timer wrap-around.
 *
 * @param a Time.
 * @param b Time to substract from a.
 *
 * @return Time difference.
 ******************************************************************************/
static uint32_t get_time_diff(uint32_t a, uint32_t b)
{
  return (a - b);
}

/*******************************************************************************
 * @brief
 *   Gets the precision (in PPM) of the sleeptimer's clock.
 *
 * @return
 *   Clock accuracy, in PPM.
 ******************************************************************************/
uint16_t sleeptimer_hal_get_clock_accuracy(void)
{
  uint16_t precision;
  sl_clock_manager_get_clock_branch_precision(SL_CLOCK_BRANCH_EM4GRPACLK, &precision);
  return precision;
}

/*******************************************************************************
 * Hardware Abstraction Layer to get the capture channel value.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_capture(void)
{
  // Invalid for BURTC peripheral
  EFM_ASSERT(0);
  return 0;
}

/*******************************************************************************
 * Hardware Abstraction Layer to reset PRS signal triggered by the associated
 * peripheral.
 ******************************************************************************/
void sleeptimer_hal_reset_prs_signal(void)
{
  // Invalid for BURTC peripheral
  EFM_ASSERT(0);
}

/***************************************************************************//**
 * Set lowest energy mode based on a project's configurations and clock source
 *
 * @note If power_manager_no_deepsleep component is included in a project, the
 *       lowest possible energy mode is EM1, else lowest energy mode is
 *       determined by peripheral used.
 ******************************************************************************/
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
void sli_sleeptimer_set_pm_em_requirement(void)
{
  // No EM requirement to add for LF peripherals that continue to work in
  // Deep Sleep.
}
#endif
#endif

#endif
