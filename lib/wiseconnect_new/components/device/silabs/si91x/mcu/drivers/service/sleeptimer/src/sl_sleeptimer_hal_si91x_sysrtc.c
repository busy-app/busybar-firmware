/*******************************************************************************
 * @file
 * @brief SLEEPTIMER hardware abstraction implementation for Si91x SYSRTC.
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
#include "sl_sleeptimer.h"
#include "sli_sleeptimer_hal.h"
#include "sl_core.h"
#include "rsi_sysrtc.h"
#include "rsi_sysrtc_headers.h"
#include "si91x_device.h"

#if (SL_SLEEPTIMER_PERIPHERAL != SL_SLEEPTIMER_PERIPHERAL_DEFAULT)
#error "Peripheral not supported for Si91x"
#endif
#if (SL_SLEEPTIMER_WALLCLOCK_CONFIG == 1)
#error "Wall clock feature not present"
#endif

// Default clock selection for the SLEEP TIMER.
// This is used if no specific configuration is provided for the clock source.
#define SLEEP_TIMER_CLK_SEL         RSI_SYSRTC_CLK_32kHz_Xtal // Default to 32kHz XTAL clock
#define SLEEP_TIMER_CLOCK_FREQUENCY DEFAULT_32KHZ_XTAL_CLOCK  // Default frequency for 32kHz XTAL clock

// Check if the macro SLEEP_TIMER_CLOCK_UC is defined, indicating the configuration will be customized
#ifdef SLEEP_TIMER_CLOCK_UC
#include "sl_si91x_sleep_timer_clock_sel.h" // Include the custom sleep timer clock configuration header

// Ensure that both clocks (XTAL and RC) are not enabled simultaneously for the sleep timer
#if ((_32KHZ_XTAL_ST_CLOCK == 1) && (_32KHZ_RC_ST_CLOCK == 1))
#error "Both 32kHz XTAL and RC clocks cannot be enabled simultaneously!"  // Error if both clocks are enabled

// If the 32kHz XTAL clock is enabled, override the default selection and set the clock source to XTAL
#elif (_32KHZ_XTAL_ST_CLOCK == 1)
#undef SLEEP_TIMER_CLK_SEL                                    // Remove the previous clock source definition
#undef SLEEP_TIMER_CLOCK_FREQUENCY                            // Remove the previous frequency setting
#define SLEEP_TIMER_CLK_SEL         RSI_SYSRTC_CLK_32kHz_Xtal // Set the clock to 32kHz XTAL
#define SLEEP_TIMER_CLOCK_FREQUENCY DEFAULT_32KHZ_XTAL_CLOCK  // Set the frequency to 32kHz XTAL

// If the 32kHz RC clock is enabled, override the default selection and set the clock source to RC
#elif (_32KHZ_RC_ST_CLOCK == 1)
#undef SLEEP_TIMER_CLK_SEL                                  // Remove the previous clock source definition
#undef SLEEP_TIMER_CLOCK_FREQUENCY                          // Remove the previous frequency setting
#define SLEEP_TIMER_CLK_SEL         RSI_SYSRTC_CLK_32kHz_RC // Set the clock to 32kHz RC
#define SLEEP_TIMER_CLOCK_FREQUENCY DEFAULT_32KHZ_RC_CLOCK  // Set the frequency to 32kHz RC

// If no valid clock source is defined, raise an error indicating the issue
#else
#error "No valid clock source defined for SLEEP_TIMER_CLK_SEL!"  // Error if no valid clock is configured
#endif

#endif // End of SLEEP_TIMER_CLOCK_UC configuration

// Minimum difference between current count value and what the comparator of the timer can be set to.
// 1 tick is added to the minimum diff for the algorithm of compensation for the IRQ handler that
// triggers when CNT == compare_value + 1. For more details refer to sleeptimer_hal_set_compare() function's header.
#define SLEEPTIMER_COMPARE_MIN_DIFF (2 + 2)

#define SLEEPTIMER_TMR_WIDTH (_SYSRTC_CNT_MASK)

// Minimum difference between compare value and counter value, which requires compensation.
#define MINIMUM_DIFF_FOR_COMPENSATION 4

#define SLEEPTIMER_SI91X_INTERRUPT_HANDLER IRQ022_Handler

#define SYSRTC_GROUP0 0u

#define SYSRTC_CHANNEL0     0u
#define SYSRTC_IRQ_PRIORITY 6

static bool cc_disabled = true;
uint32_t sleeptimer_hal_get_capture(void);
void sleeptimer_hal_reset_prs_signal(void);
/*******************************************************************************
 * Computes difference between two times taking into account timer wrap-around.
 *
 * @param a Time.
 * @param b Time to substract from a.
 *
 * @return Time difference.
 ******************************************************************************/
__STATIC_INLINE uint32_t get_time_diff(uint32_t a, uint32_t b)
{
  return (a - b);
}

/*******************************************************************************
 * Initializes Si91x SYSRTC sleep timer.
 ******************************************************************************/
void sleeptimer_hal_init_timer(void)
{
  rsi_sysrtc_config_t sysrtc_config      = SYSRTC_CONFIG_DEFAULT;
  rsi_sysrtc_group_config_t group_config = SYSRTC_GROUP_CONFIG_DEFAULT;
  const rsi_sysrtc_group_channel_compare_config_t group_compare_channel_config =
    SYSRTC_GROUP_CHANNEL_COMPARE_CONFIG_DEFAULT_REGMODE;

  // Enable 32kHz clock to SYSRTC peripheral
  rsi_sysrtc_clk_set(SLEEP_TIMER_CLK_SEL, 0u);

  // Initialize SYSRTC module
  rsi_sysrtc_init(&sysrtc_config);

  group_config.compare_channel0_enable   = false;
  group_config.p_compare_channel0_config = &group_compare_channel_config;
  // Initialize SYSRTC group 0 for sleeptimer service
  rsi_sysrtc_init_group(SYSRTC_GROUP0, &group_config);

  // Disable & clear group 0 interrupts
  rsi_sysrtc_disable_group_interrupts(SYSRTC_GROUP0, _SYSRTC_GRP0_IEN_MASK);
  rsi_sysrtc_clear_group_interrupts(SYSRTC_GROUP0, _SYSRTC_GRP0_IF_MASK);
  // Enable SYSRTC module
  rsi_sysrtc_enable();
  // Initialize counter to 0
  rsi_sysrtc_set_counter(0u);

  // Enable interrupts from SYSRTC module
  RSI_NpssIntrUnMask(NPSS_TO_MCU_SYSRTC_INTR);
  NVIC_EnableIRQ(NPSS_TO_MCU_SYRTC_INTR_IRQn);
#ifdef SLI_SI91X_ENABLE_OS
  //Giving higher priority to SYSRTC to service wakeup
  NVIC_SetPriority(NPSS_TO_MCU_SYRTC_INTR_IRQn, SYSRTC_IRQ_PRIORITY);
#endif
  // Start SYSRTC module
  rsi_sysrtc_start();
}

/*******************************************************************************
 * Gets SYSRTC counter value.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_counter(void)
{
  return rsi_sysrtc_get_counter();
}

/*******************************************************************************
 * Gets SYSRTC compare value.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_compare(void)
{
  return rsi_sysrtc_get_group_compare_channel_value(SYSRTC_GROUP0, SYSRTC_CHANNEL0);
}

/*******************************************************************************
 * Sets SYSRTC group 0 compare0 value.
 *
 * @note Compare match value is set to the requested value - 1. This is done
 * to compensate for the fact that the SYSRTC compare match interrupt always
 * triggers at the end of the requested ticks and that the IRQ handler is
 * executed when current tick count == compare_value + 1.
 ******************************************************************************/
void sleeptimer_hal_set_compare(uint32_t value)
{
  CORE_DECLARE_IRQ_STATE;
  uint32_t counter;
  uint32_t compare;
  uint32_t compare_value = value;
  int32_t diff;

  CORE_ENTER_CRITICAL();

  // Read the current compare and counter register value
  counter = sleeptimer_hal_get_counter();
  compare = sleeptimer_hal_get_compare();

  if (((rsi_sysrtc_get_group_interrupts(SYSRTC_GROUP0) & SYSRTC_GRP0_IEN_CMP0) != 0)
      || (get_time_diff(compare, counter) > SLEEPTIMER_COMPARE_MIN_DIFF) || compare == counter) {
    compare_value %= SLEEPTIMER_TMR_WIDTH;
    // Sometimes for smaller tick values compare value is not set properly because by the time
    // compare value is set, current counter value is exceeding the expected compare value. To compensate
    // this, extra ticks are added just to generate the interrupt.
    diff = (int32_t)(compare_value - sleeptimer_hal_get_counter());
    if ((diff <= MINIMUM_DIFF_FOR_COMPENSATION) && (diff > -MINIMUM_DIFF_FOR_COMPENSATION)) {
      compare_value += (uint32_t)((MINIMUM_DIFF_FOR_COMPENSATION - 1) - diff);
    }
    // Set the compare value for next timeout
    rsi_sysrtc_set_compare_value(SYSRTC_GROUP0, SYSRTC_CHANNEL0, compare_value - 1);
    sleeptimer_hal_enable_int(SLEEPTIMER_EVENT_COMP);
    rsi_sysrtc_enable_interrupts();
  }
  CORE_EXIT_CRITICAL();
  // If compare interrupt is disabled previously, enable here
  if (cc_disabled) {
    SYSRTC0->GRP0_CTRL |= SYSRTC_GRP0_CTRL_CMP0EN;
    cc_disabled = false;
  }
}

/*******************************************************************************
 * Enables SYSRTC interrupts.
 ******************************************************************************/
void sleeptimer_hal_enable_int(uint8_t local_flag)
{
  uint32_t sysrtc_ien = 0u;

  if (local_flag & SLEEPTIMER_EVENT_OF) {
    // Set overflow interrupt
    sysrtc_ien |= SYSRTC_GRP0_IEN_OVF;
  }

  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    // Set compare interrupt
    sysrtc_ien |= SYSRTC_GRP0_IEN_CMP0;
  }

  rsi_sysrtc_enable_group_interrupts(SYSRTC_GROUP0, sysrtc_ien);
}

/*******************************************************************************
 * Disables SYSRTC interrupts.
 ******************************************************************************/
void sleeptimer_hal_disable_int(uint8_t local_flag)
{
  uint32_t sysrtc_int_dis = 0u;

  if (local_flag & SLEEPTIMER_EVENT_OF) {
    // Disable overflow interrupt
    sysrtc_int_dis |= SYSRTC_GRP0_IEN_OVF;
  }

  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    // Disable compare interrupt
    sysrtc_int_dis |= SYSRTC_GRP0_IEN_CMP0;

    cc_disabled = true;
    SYSRTC0->GRP0_CTRL &= ~_SYSRTC_GRP0_CTRL_CMP0EN_MASK;
  }

  rsi_sysrtc_disable_group_interrupts(SYSRTC_GROUP0, sysrtc_int_dis);
}

/*******************************************************************************
 * Hardware Abstraction Layer to set timer interrupts.
 ******************************************************************************/
void sleeptimer_hal_set_int(uint8_t local_flag)
{
  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    SYSRTC0->GRP0_IF_SET = SYSRTC_GRP0_IF_CMP0;
  }
}

/*******************************************************************************
 * Gets status of specified interrupt.
 *
 * Note: This function must be called with interrupts disabled.
 ******************************************************************************/
bool sli_sleeptimer_hal_is_int_status_set(uint8_t local_flag)
{
  bool int_is_set = false;
  // As per initial testing of SYSRTC peripheral in Si91x B0, interrupt needs to be
  // disabled to get the status of group interrupts.
  rsi_sysrtc_disable_interrupts();
  uint32_t irq_flag = rsi_sysrtc_get_group_interrupts(SYSRTC_GROUP0);

  switch (local_flag) {
    case SLEEPTIMER_EVENT_COMP:
      // Update compare interrupt status
      int_is_set = ((irq_flag & SYSRTC_GRP0_IF_CMP0) == SYSRTC_GRP0_IF_CMP0);
      break;

    case SLEEPTIMER_EVENT_OF:
      // update overflow interrupt status
      int_is_set = ((irq_flag & SYSRTC_GRP0_IF_OVF) == SYSRTC_GRP0_IF_OVF);
      break;

    default:
      break;
  }
  // Enable back interrupts
  rsi_sysrtc_enable_interrupts();

  return int_is_set;
}

/*******************************************************************************
 * Si91x SYSRTC interrupt handler for sleeptimer service.
 ******************************************************************************/
void SLEEPTIMER_SI91X_INTERRUPT_HANDLER(void)
{
  CORE_DECLARE_IRQ_STATE;
  uint8_t local_flag = 0;
  uint32_t irq_flag;

  // As per initial testing of SYSRTC peripheral in Si91x B0, interrupt needs to be
  // disabled to get the status of group interrupts.
  rsi_sysrtc_disable_interrupts();
  CORE_ENTER_ATOMIC();
  // Get the type of interrupt
  irq_flag = rsi_sysrtc_get_group_interrupts(SYSRTC_GROUP0);

  if (irq_flag & SYSRTC_GRP0_IF_OVF) {
    // Set overflow event
    local_flag |= SLEEPTIMER_EVENT_OF;
  }

  if (irq_flag & SYSRTC_GRP0_IF_CMP0) {
    // Set compare event
    local_flag |= SLEEPTIMER_EVENT_COMP;
  }
  // clear interrupt
  rsi_sysrtc_clear_group_interrupts(SYSRTC_GROUP0, irq_flag & (SYSRTC_GRP0_IF_OVF | SYSRTC_GRP0_IF_CMP0));
  // Process the interrupt and executes callback functions
  process_timer_irq(local_flag);
  CORE_EXIT_ATOMIC();
  rsi_sysrtc_enable_interrupts();
}

/*******************************************************************************
 * Gets SYSRTC timer frequency.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_timer_frequency(void)
{
  // There is currently no call for in Si91x library to obtain peripheral frequency of SYSRTC.
  return SLEEP_TIMER_CLOCK_FREQUENCY;
}

/*******************************************************************************
 * This feature is not used in Si91x.
 ******************************************************************************/
uint16_t sleeptimer_hal_get_clock_accuracy(void)
{
  return 0;
}

/*******************************************************************************
 * This feature is not used in Si91x.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_capture(void)
{
  return 0;
}

/*******************************************************************************
 * This feature is not used in Si91x.
 ******************************************************************************/
void sleeptimer_hal_reset_prs_signal(void)
{
}
