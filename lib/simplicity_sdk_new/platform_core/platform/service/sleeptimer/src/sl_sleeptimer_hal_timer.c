/***************************************************************************//**
 * @file
 * @brief SLEEPTIMER hardware abstraction implementation for TIMER.
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

// Define module name for Power Manager debug feature
#define CURRENT_MODULE_NAME    "SLEEPTIMER_TIMER"

#include "em_device.h"
#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_timer.h"
#elif defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_hal_timer.h"
#endif
#include "sl_sleeptimer.h"
#include "sli_sleeptimer_hal.h"
#include "sl_core.h"
#include "sl_clock_manager.h"
#include "sl_interrupt_manager.h"
#include "sl_device_peripheral.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#if (SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_TIMER)

// Minimum difference between current count value and what the comparator of the timer can be set to.
// 1 tick is added to the minimum diff for the algorithm of compensation for the IRQ handler that
// triggers when CNT == compare_value + 1. For more details refer to sleeptimer_hal_set_compare() function's header.
#define SLEEPTIMER_COMPARE_MIN_DIFF  (1 + 1)

// Macros used to constructs TIMER instance
#define _CONCAT_TWO_TOKENS(token_1, token_2)                     token_1 ## token_2
#define _CONCAT_THREE_TOKENS(token_1, token_2, token_3)          token_1 ## token_2 ## token_3
#define CONCAT_TWO_TOKENS(token_1, token_2)                      _CONCAT_TWO_TOKENS(token_1, token_2)
#define CONCAT_THREE_TOKENS(token_1, token_2, token_3)           _CONCAT_THREE_TOKENS(token_1, token_2, token_3)

#if defined(TIMER_PRESENT)                        \
  && (SL_SLEEPTIMER_TIMER_INSTANCE < TIMER_COUNT) \
  && (TIMER_CNTWIDTH(SL_SLEEPTIMER_TIMER_INSTANCE) == 0x20)
  #define SLEEPTIMER_TIMER_INSTANCE    TIMER(SL_SLEEPTIMER_TIMER_INSTANCE)
  #define SLEEPTIMER_TIMER_CHANNEL     0
  #define SLEEPTIMER_PERIPHERAL_TIMER  CONCAT_TWO_TOKENS(SL_PERIPHERAL_TIMER, SL_SLEEPTIMER_TIMER_INSTANCE)
  #define SLEEPTIMER_TIMER_IRQ         CONCAT_THREE_TOKENS(TIMER, SL_SLEEPTIMER_TIMER_INSTANCE, _IRQn)
  #define SLEEPTIMER_TIMER_IRQHandler  CONCAT_THREE_TOKENS(TIMER, SL_SLEEPTIMER_TIMER_INSTANCE, _IRQHandler)
  #define SLEEPTIMER_TIMER_IEN_COMPARE TIMER_IEN_CC0
  #define SLEEPTIMER_TIMER_CLK         CONCAT_TWO_TOKENS(SL_BUS_CLOCK_TIMER, SL_SLEEPTIMER_TIMER_INSTANCE)
  #define SLEEPTIMER_TIMER_TOP_MAX     _TIMER_TOP_MASK
  #define SLEEPTIMER_TMR_WIDTH         _TIMER_CNT_MASK
#else
  #define TIMER_UNSUPORTED
#endif

#if defined(TIMER_UNSUPORTED)
#error "The TIMER peripheral instance or channel is not supported. It must be a valid 32-bits size instance."
#endif

#if defined(_SILICON_LABS_32B_SERIES_2)
#define sleeptimer_hal_timer_get_counter(timer_instance)                        TIMER_CounterGet(timer_instance)
#define sleeptimer_hal_timer_get_compare(timer_instance, channel)               TIMER_CaptureGet(timer_instance, channel)
#define sleeptimer_hal_timer_set_compare(timer_instance, channel, compare)      TIMER_CompareSet(timer_instance, channel, compare)
#define sleeptimer_hal_timer_get_interrupt(timer_instance)                      TIMER_IntGet(timer_instance)
#define sleeptimer_hal_timer_set_interrupt(timer_instance, flags)               TIMER_IntSet(timer_instance, flags)
#define sleeptimer_hal_timer_enable_interrupt(timer_instance, flags)            TIMER_IntEnable(timer_instance, flags)
#define sleeptimer_hal_timer_disable_interrupt(timer_instance, flags)           TIMER_IntDisable(timer_instance, flags)
#define sleeptimer_hal_timer_clear_interrupt(timer_instance, flags)             TIMER_IntClear(timer_instance, flags)
#else
#define sleeptimer_hal_timer_get_counter(timer_instance)                        sl_hal_timer_get_counter(timer_instance)
#define sleeptimer_hal_timer_get_compare(timer_instance, channel)               sl_hal_timer_channel_get_compare(timer_instance, channel)
#define sleeptimer_hal_timer_set_compare(timer_instance, channel, compare)      sl_hal_timer_channel_set_compare(timer_instance, channel, compare)
#define sleeptimer_hal_timer_get_interrupt(timer_instance)                      sl_hal_timer_get_pending_interrupts(timer_instance)
#define sleeptimer_hal_timer_set_interrupt(timer_instance, flags)               sl_hal_timer_set_interrupts(timer_instance, flags)
#define sleeptimer_hal_timer_enable_interrupt(timer_instance, flags)            sl_hal_timer_enable_interrupts(timer_instance, flags)
#define sleeptimer_hal_timer_disable_interrupt(timer_instance, flags)           sl_hal_timer_disable_interrupts(timer_instance, flags)
#define sleeptimer_hal_timer_clear_interrupt(timer_instance, flags)             sl_hal_timer_clear_interrupts(timer_instance, flags)
#endif

static bool comp_int_disabled = true;

__STATIC_INLINE uint32_t get_time_diff(uint32_t a,
                                       uint32_t b);

/******************************************************************************
 * Initializes TIMER sleep timer.
 *****************************************************************************/
void sleeptimer_hal_init_timer(void)
{
  sl_clock_manager_enable_bus_clock(SLEEPTIMER_TIMER_CLK);

#if defined(_SILICON_LABS_32B_SERIES_2)
  TIMER_Init_TypeDef init_config = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef init_config_cc = TIMER_INITCC_DEFAULT;
  init_config_cc.mode = timerCCModeCompare;
  init_config.prescale = timerPrescale1024;
#if (SL_SLEEPTIMER_DEBUGRUN == 1)
  init_config.debugRun = true;
#endif

  TIMER_InitCC(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_CHANNEL, &init_config_cc);
  TIMER_TopSet(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_TOP_MAX);

  TIMER_Init(SLEEPTIMER_TIMER_INSTANCE, &init_config);
#if defined(TIMER_STATUS_SYNCBUSY)
  TIMER_SyncWait(SLEEPTIMER_TIMER_INSTANCE);
#endif

  TIMER_IntDisable(SLEEPTIMER_TIMER_INSTANCE, _TIMER_IEN_MASK);
  TIMER_IntClear(SLEEPTIMER_TIMER_INSTANCE, _TIMER_IEN_MASK);

  TIMER_CompareSet(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_CHANNEL, 0UL);

#elif defined(_SILICON_LABS_32B_SERIES_3)
  sl_hal_timer_config_t init_config = SL_HAL_TIMER_CONFIG_DEFAULT;
  sl_hal_timer_channel_config_t init_config_cc = SL_HAL_TIMER_CHANNEL_CONFIG_DEFAULT;
  init_config_cc.channel_mode = SL_HAL_TIMER_CHANNEL_MODE_COMPARE;
  init_config.prescaler = SL_HAL_TIMER_PRESCALER_DIV1024;
#if (SL_SLEEPTIMER_DEBUGRUN == 1)
  init_config.debugRun = true;
#endif

  sl_hal_timer_channel_init(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_CHANNEL, &init_config_cc);
  sl_hal_timer_init(SLEEPTIMER_TIMER_INSTANCE, &init_config);
  sl_hal_timer_disable_interrupts(SLEEPTIMER_TIMER_INSTANCE, _TIMER_IEN_MASK);
  sl_hal_timer_clear_interrupts(SLEEPTIMER_TIMER_INSTANCE, _TIMER_IEN_MASK);
  sl_hal_timer_enable(SLEEPTIMER_TIMER_INSTANCE);
  sl_hal_timer_wait_sync(SLEEPTIMER_TIMER_INSTANCE);
  sl_hal_timer_set_top(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_TOP_MAX);
  sl_hal_timer_channel_set_compare(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_CHANNEL, 0UL);
  sl_hal_timer_start(SLEEPTIMER_TIMER_INSTANCE);
#endif

  sl_interrupt_manager_clear_irq_pending(SLEEPTIMER_TIMER_IRQ);
  sl_interrupt_manager_enable_irq(SLEEPTIMER_TIMER_IRQ);
}

/******************************************************************************
 * Gets TIMER counter value.
 *****************************************************************************/
uint32_t sleeptimer_hal_get_counter(void)
{
  return sleeptimer_hal_timer_get_counter(SLEEPTIMER_TIMER_INSTANCE);
}

/******************************************************************************
 * Gets TIMER compare value.
 *****************************************************************************/
uint32_t sleeptimer_hal_get_compare(void)
{
  return sleeptimer_hal_timer_get_compare(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_CHANNEL);
}

/******************************************************************************
 * Sets TIMER compare value.
 *
 * @note Compare match value is set to the requested value - 1. This is done
 * to compensate for the fact that the TIMER compare match interrupt always
 * triggers at the end of the requested ticks and that the IRQ handler is
 * executed when current tick count == compare_value + 1.
 *****************************************************************************/
void sleeptimer_hal_set_compare(uint32_t value)
{
  CORE_DECLARE_IRQ_STATE;
  uint32_t counter;
  uint32_t compare;
  uint32_t compare_value = value;

  CORE_ENTER_CRITICAL();
  counter = sleeptimer_hal_get_counter();
  compare = sleeptimer_hal_get_compare();

  if (((sleeptimer_hal_timer_get_interrupt(SLEEPTIMER_TIMER_INSTANCE) & SLEEPTIMER_TIMER_IEN_COMPARE) != 0)
      || get_time_diff(compare, counter) > SLEEPTIMER_COMPARE_MIN_DIFF
      || compare == counter) {
    // Add margin if necessary
    if (get_time_diff(compare_value, counter) < SLEEPTIMER_COMPARE_MIN_DIFF) {
      compare_value = counter + SLEEPTIMER_COMPARE_MIN_DIFF;
    }
    compare_value %= SLEEPTIMER_TMR_WIDTH;

    sleeptimer_hal_timer_set_compare(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_CHANNEL, compare_value - 1);
    sleeptimer_hal_enable_int(SLEEPTIMER_EVENT_COMP);
    comp_int_disabled = false;
  }
  CORE_EXIT_CRITICAL();
}

/******************************************************************************
 * Enables TIMER interrupts.
 *****************************************************************************/
void sleeptimer_hal_enable_int(uint8_t local_flag)
{
  uint32_t timer_ien = 0UL;

  if (local_flag & SLEEPTIMER_EVENT_OF) {
    timer_ien |= TIMER_IEN_OF;
  }

  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    if (comp_int_disabled == true) {
      sleeptimer_hal_timer_clear_interrupt(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_IEN_COMPARE);
      comp_int_disabled = false;
    }
    timer_ien |= SLEEPTIMER_TIMER_IEN_COMPARE;
  }

  sleeptimer_hal_timer_enable_interrupt(SLEEPTIMER_TIMER_INSTANCE, timer_ien);
}

/******************************************************************************
 * Disables TIMER interrupts.
 *****************************************************************************/
void sleeptimer_hal_disable_int(uint8_t local_flag)
{
  uint32_t timer_int_dis = 0UL;

  if (local_flag & SLEEPTIMER_EVENT_OF) {
    timer_int_dis |= TIMER_IEN_OF;
  }

  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    timer_int_dis |= SLEEPTIMER_TIMER_IEN_COMPARE;

    comp_int_disabled = true;
  }

  sleeptimer_hal_timer_disable_interrupt(SLEEPTIMER_TIMER_INSTANCE, timer_int_dis);
}

/*******************************************************************************
 * Hardware Abstraction Layer to set timer interrupts.
 ******************************************************************************/
void sleeptimer_hal_set_int(uint8_t local_flag)
{
  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    sleeptimer_hal_timer_set_interrupt(SLEEPTIMER_TIMER_INSTANCE, SLEEPTIMER_TIMER_IEN_COMPARE);
  }
}

/******************************************************************************
 * Gets status of specified interrupt.
 *
 * Note: This function must be called with interrupts disabled.
 *****************************************************************************/
bool sli_sleeptimer_hal_is_int_status_set(uint8_t local_flag)
{
  bool int_is_set = false;
  uint32_t irq_flag = sleeptimer_hal_timer_get_interrupt(SLEEPTIMER_TIMER_INSTANCE);

  switch (local_flag) {
    case SLEEPTIMER_EVENT_COMP:
      int_is_set = ((irq_flag & SLEEPTIMER_TIMER_IEN_COMPARE) == SLEEPTIMER_TIMER_IEN_COMPARE);
      break;

    case SLEEPTIMER_EVENT_OF:
      int_is_set = ((irq_flag & TIMER_IEN_OF) == TIMER_IEN_OF);
      break;

    default:
      break;
  }

  return int_is_set;
}

/*******************************************************************************
 * TIMER interrupt handler.
 ******************************************************************************/
void SLEEPTIMER_TIMER_IRQHandler(void)
{
  CORE_DECLARE_IRQ_STATE;
  uint8_t local_flag = 0;
  uint32_t irq_flag;

  CORE_ENTER_ATOMIC();
  irq_flag = sleeptimer_hal_timer_get_interrupt(SLEEPTIMER_TIMER_INSTANCE);

  if (irq_flag & TIMER_IEN_OF) {
    local_flag |= SLEEPTIMER_EVENT_OF;
  }

  if (irq_flag & SLEEPTIMER_TIMER_IEN_COMPARE) {
    local_flag |= SLEEPTIMER_EVENT_COMP;
  }
  sleeptimer_hal_timer_clear_interrupt(SLEEPTIMER_TIMER_INSTANCE, irq_flag & (TIMER_IEN_OF | SLEEPTIMER_TIMER_IEN_COMPARE));

  process_timer_irq(local_flag);

  CORE_EXIT_ATOMIC();
}

/*******************************************************************************
 * Gets TIMER timer frequency.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_timer_frequency(void)
{
  // Returns source frequency divided by max prescaler value 1024.
  uint32_t freq;
  sl_clock_branch_t clock_branch;

  clock_branch = sl_device_peripheral_get_clock_branch(SLEEPTIMER_PERIPHERAL_TIMER);
  sl_clock_manager_get_clock_branch_frequency(clock_branch, &freq);
  return (freq >> 10UL);
}

/*******************************************************************************
 * Computes difference between two times taking into account timer wrap-around.
 *
 * @param a Time.
 * @param b Time to substract from a.
 *
 * @return Time difference.
 ******************************************************************************/
__STATIC_INLINE uint32_t get_time_diff(uint32_t a,
                                       uint32_t b)
{
  return (a - b);
}

/*******************************************************************************
 * @brief
 *   Gets the precision (in PPM) of the sleeptimer's clock.
 *
 * @return
 *   Clock accuracy, in PPM.
 *
 ******************************************************************************/
uint16_t sleeptimer_hal_get_clock_accuracy(void)
{
  uint16_t precision;
  sl_clock_manager_get_clock_branch_precision(SL_CLOCK_BRANCH_EM01GRPACLK, &precision);
  return precision;
}

/*******************************************************************************
 * Hardware Abstraction Layer to get the capture channel value.
 *
 * @return Capture value.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_capture(void)
{
  // Invalid for TIMER peripheral
  EFM_ASSERT(0);
  return 0;
}

/*******************************************************************************
 * Hardware Abstraction Layer to reset PRS signal triggered by the associated
 * peripheral.
 ******************************************************************************/
void sleeptimer_hal_reset_prs_signal(void)
{
  // Invalid for TIMER peripheral
  EFM_ASSERT(0);
}

/***************************************************************************//**
 * Set lowest energy mode based on a project's configurations and clock source
 *
 * @note Lowest possible energy mode for TIMER peripheral is EM1.
 ******************************************************************************/
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
void sli_sleeptimer_set_pm_em_requirement(void)
{
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
}
#endif
#endif
