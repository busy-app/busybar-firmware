/*******************************************************************************
 * @file  sl_sleeptimer_hal_rs_ulptimer.c
 * @brief Sleeptimer hardware abstraction implementation for RS911x ULP timer
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_sleeptimer.h"
#include "sli_sleeptimer_hal.h"
#include "rsi_timers.h"
#include "rsi_ulpss_clk.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_rom_timer.h"
#include "si91x_device.h"
#include "em_core.h"
#include "clock_update.h"

#if SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_RS_ULPTIMER

/*******************************************************************************
 ***************************  Defines/Macros   ********************************
 ******************************************************************************/
#define MATCH_REG_MAX_VALUE           0xFFFFFFFE
#define SLEEPTIMER_COUNTER_IRQHandler IRQ002_Handler //Timer 0 IRQ handler

#define SL_SLEEPTIMER_TIMER_INSTANCE TIMER_1

#if SL_SLEEPTIMER_TIMER_INSTANCE == TIMER_0
#error "Timer instance locked"
#elif SL_SLEEPTIMER_TIMER_INSTANCE == TIMER_1
#define SLEEPTIMER_COMPARE_IRQHandler IRQ003_Handler //Timer 1 IRQ handler
#elif SL_SLEEPTIMER_TIMER_INSTANCE == TIMER_2
#define SLEEPTIMER_COMPARE_IRQHandler IRQ004_Handler //Timer 2 IRQ handler
#elif SL_SLEEPTIMER_TIMER_INSTANCE == TIMER_3
#define SLEEPTIMER_COMPARE_IRQHandler IRQ005_Handler //Timer 3 IRQ handler
#else
#error "Timer instance not available"
#endif

// Macros used to constructing TIMER instance
#define _CONCAT_TWO_TOKENS(token_1, token_2)            token_1##token_2
#define _CONCAT_THREE_TOKENS(token_1, token_2, token_3) token_1##token_2##token_3
#define CONCAT_TWO_TOKENS(token_1, token_2)             _CONCAT_TWO_TOKENS(token_1, token_2)
#define CONCAT_THREE_TOKENS(token_1, token_2, token_3)  _CONCAT_THREE_TOKENS(token_1, token_2, token_3)

// Minimum difference between current count value and what the comparator of the timer can be set to.
// 1 tick is added to the minimum diff for the algorithm of compensation for the IRQ handler that
// triggers when CNT == compare_value + 1. For more details refer to sleeptimer_hal_set_compare() function's header.
#define SLEEPTIMER_COMPARE_MIN_DIFF (2 + 1)

#define SLEEPTIMER_TMR_WIDTH  (0xFFFFFFFFU) //Width of sleeptimer count register
#define SLEEPTIMER_PERIPHERAL CONCAT_TWO_TOKENS(TIMER_, SL_SLEEPTIMER_TIMER_INSTANCE) //Sleeptimer peripheral instance
#define SLEEPTIMER_PERIPHERAL_IRQ \
  CONCAT_THREE_TOKENS(TIMER, SL_SLEEPTIMER_TIMER_INSTANCE, _IRQn) //Sleeptimer peripheral instance IRQ number
//Number of counts expired from previous sleeptimer_hal_get_counter() call to current call in sleeptimer_hal_set_compare() function
#define COUNTS_COMPENSATATION 2170

/*******************************************************************************
 ***************************  Static Variables   ********************************
 ******************************************************************************/
static uint32_t current_match_value = 0; //This variable serves as sleeptimer CNT register
static uint32_t low_period_timeout  = 1;

/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  Local Function   Prototypes********* *************
 ******************************************************************************/

/*******************************************************************************
 ***************************  Local Function Definition*************************
 ******************************************************************************/

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
 ***************************  Global function Definitions **********************
 ******************************************************************************/
uint32_t sleeptimer_hal_get_capture(void);
void sleeptimer_hal_reset_prs_signal(void);
/*******************************************************************************
 * Initialize ULP timer peripheral.
 *
 * @param none
 *
 * @return none
 ******************************************************************************/
void sleeptimer_hal_init_timer(void)
{
  //Enable RC clock
  //Todo: Ideally this frequency should be configured for 32kHz. But due to ULP timer
  //register read time constrain (Read time for ULP timer registers with 32kHz timer
  //frequency is significantly high (i.e) around 100usec), this is temporarily configured
  //as RCMHz for ble stack application test purpose.
  RSI_ULPSS_TimerClkConfig(ULPCLK, ENABLE_STATIC_CLK, 0, ULP_TIMER_MHZ_RC_CLK, 0);

  //Configure periodic timer with count down mode, this timer will be always running
  //and taken as a reference for sleeptimer
  RSI_TIMERS_SetTimerMode(TIMERS, PERIODIC_TIMER, TIMER_0);
  RSI_TIMERS_SetTimerType(TIMERS, COUNTER_DOWN_MODE, TIMER_0);
  TIMERS->MATCH_CTRL[TIMER_0].MCUULP_TMR_CNTRL_b.COUNTER_UP = 1;
  RSI_TIMERS_SetMatch(TIMERS, TIMER_0, MATCH_REG_MAX_VALUE);
  RSI_TIMERS_InterruptEnable(TIMERS, TIMER_0);
  NVIC_EnableIRQ(TIMER0_IRQn);
  RSI_TIMERS_TimerStart(TIMERS, TIMER_0);

  //Configure another one-shot timer. This timer is used to generate timeouts with
  //respect to reference timer configured above
  RSI_TIMERS_SetTimerMode(TIMERS, ONESHOT_TIMER, SLEEPTIMER_PERIPHERAL);
  RSI_TIMERS_SetTimerType(TIMERS, COUNTER_DOWN_MODE, SLEEPTIMER_PERIPHERAL);
  TIMERS->MATCH_CTRL[SLEEPTIMER_PERIPHERAL].MCUULP_TMR_CNTRL_b.COUNTER_UP = 1;
  RSI_TIMERS_InterruptEnable(TIMERS, SLEEPTIMER_PERIPHERAL);
  NVIC_EnableIRQ(SLEEPTIMER_PERIPHERAL_IRQ);
}

/*******************************************************************************
 * This function converts match register value of ULP timer to expected CNT register value (0 to 0xFFFFFFFF)
 * and return the value.
 *
 * @param none
 *
 * @return uint32_t expected CNT register value
 ******************************************************************************/
uint32_t sleeptimer_hal_get_counter(void)
{
  return (TIMERS->MATCH_CTRL[TIMER_0].MCUULP_TMR_MATCH);
}

/*******************************************************************************
 * This function returns the latest compare value.
 *
 * @param none
 *
 * @return uint32_t compare value
 ******************************************************************************/
uint32_t sleeptimer_hal_get_compare(void)
{
  return current_match_value;
}

/*******************************************************************************
 * Sets ULP timer compare value. Since there is no compare register in ULP timer
 * we are maintaining the latest compare value in current_match_value variable and
 * every time a new value needs to be programmed, difference of recent current_match_value
 * and new value is programmed in match register and a one-shot timer is started with the
 * this difference value to generate the required timeout.
 *
 * @param uint32_t: value - compare value
 *
 * @return none
 ******************************************************************************/
void sleeptimer_hal_set_compare(uint32_t value)
{
  CORE_DECLARE_IRQ_STATE;
  uint32_t counter;
  uint32_t compare;
  uint32_t compare_value = value;

  CORE_ENTER_CRITICAL();
  //get current counter value
  counter = sleeptimer_hal_get_counter();
  //get latest compare value set
  compare = sleeptimer_hal_get_compare();

  if (((RSI_TIMERS_InterruptStatus(TIMERS, SLEEPTIMER_PERIPHERAL)) != 0)
      || (get_time_diff(compare, counter) > SLEEPTIMER_COMPARE_MIN_DIFF) || (counter == compare)) {
    // Add margin if necessary
    if (get_time_diff(compare_value, counter) < SLEEPTIMER_COMPARE_MIN_DIFF) {
      compare_value = counter + SLEEPTIMER_COMPARE_MIN_DIFF;
    }
    compare_value %= SLEEPTIMER_TMR_WIDTH;
    if ((current_match_value == 0) && (compare_value > 0)) {
      //Timeout value is loaded for first time or after an overflow
      RSI_TIMERS_SetMatch(TIMERS, SLEEPTIMER_PERIPHERAL, (compare_value - counter) + COUNTS_COMPENSATATION);
      current_match_value = compare_value;
      sleeptimer_hal_enable_int(SLEEPTIMER_EVENT_COMP);
      RSI_TIMERS_TimerStart(TIMERS, SLEEPTIMER_PERIPHERAL);
    } else if (current_match_value > compare_value) {
      //Lower timeout value loaded before timeout of previously loaded value
      current_match_value = value;
      compare_value       = (compare_value - counter) + COUNTS_COMPENSATATION;
      RSI_TIMERS_SetMatch(TIMERS, SLEEPTIMER_PERIPHERAL, compare_value);
      sleeptimer_hal_enable_int(SLEEPTIMER_EVENT_COMP);
      RSI_TIMERS_TimerStart(TIMERS, SLEEPTIMER_PERIPHERAL);
      low_period_timeout = 0;
    } else if ((compare_value > current_match_value) && (low_period_timeout == 1)) {
      //Previously loaded timeout is successfully generated and next timeout value is loaded
      compare_value       = (compare_value - counter) + COUNTS_COMPENSATATION;
      current_match_value = value;
      RSI_TIMERS_SetMatch(TIMERS, SLEEPTIMER_PERIPHERAL, compare_value);
      sleeptimer_hal_enable_int(SLEEPTIMER_EVENT_COMP);
      RSI_TIMERS_TimerStart(TIMERS, SLEEPTIMER_PERIPHERAL);
    }
  }
  CORE_EXIT_CRITICAL();
}

/*******************************************************************************
 * Enable ULP timer interrupts.
 *
 * @param uint8_t: local_flag - Type of interrupt (Compare/Overflow)
 *
 * @return none
 ******************************************************************************/
void sleeptimer_hal_enable_int(uint8_t local_flag)
{
  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    //Enable Compare interrupt
    RSI_TIMERS_InterruptEnable(TIMERS, SLEEPTIMER_PERIPHERAL);
  }
}

/*******************************************************************************
 * Disable ULP timer interrupt for sleeptimer peripheral
 *
 * @param uint8_t: local_flag - Type of interrupt (Compare/Overflow)
 *
 * @return none
 ******************************************************************************/
void sleeptimer_hal_disable_int(uint8_t local_flag)
{
  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    //Disable Compare interrupt
    RSI_TIMERS_InterruptDisable(TIMERS, SLEEPTIMER_PERIPHERAL);
  }
}

/*******************************************************************************
 * Set ULP timer interrupt for sleeptimer peripheral
 *
 * @param uint8_t: local_flag - Type of interrupt (Compare/Overflow)
 *
 * @return none
 ******************************************************************************/
void sleeptimer_hal_set_int(uint8_t local_flag)
{
  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    //Enable Compare interrupt
    RSI_TIMERS_InterruptEnable(TIMERS, SLEEPTIMER_PERIPHERAL);
  }
}

/*******************************************************************************
 * Gets status of specified interrupt for sleeptimer peripheral
 * Note: This function must be called with interrupts disabled.
 *
 * @param uint8_t: local_flag - Type of interrupt (Compare/Overflow)
 *
 * @return bool: true - interrupt enabled, false - interrupt disabled
 ******************************************************************************/
bool sli_sleeptimer_hal_is_int_status_set(uint8_t local_flag)
{
  bool int_is_set = false;
  //Read the interrupt status of ULP timer
  uint32_t irq_flag = RSI_TIMERS_InterruptStatus(TIMERS, SLEEPTIMER_PERIPHERAL);

  switch (local_flag) {
    case SLEEPTIMER_EVENT_COMP:
      //Update the compare interrupt status
      int_is_set = irq_flag;
      break;

    case SLEEPTIMER_EVENT_OF:
      //Always return false because ULP timer doesn't have overflow interrupt
      int_is_set = false;
      break;

    default:
      break;
  }

  return int_is_set;
}

/*******************************************************************************
 * ULP Timer IRQ Handler for sleeptimer peripheral. This IRQ handler reads the interrupt
 * status and calls the process_timer_irq. ULP timer doesn't have any Overflow interrupt,
 * so overflow is detected by sleeptimer_hal_set_compare() function.
 *
 * @param none
 *
 * @return none
 ******************************************************************************/
void SLEEPTIMER_COMPARE_IRQHandler(void)
{
  CORE_DECLARE_IRQ_STATE;
  uint8_t local_flag = 0;
  uint32_t irq_flag  = 0;
  irq_flag           = RSI_TIMERS_InterruptStatus(TIMERS, SLEEPTIMER_PERIPHERAL);
  CORE_ENTER_ATOMIC();
  if (irq_flag) {
    //Compare interrupt triggered
    local_flag |= SLEEPTIMER_EVENT_COMP;
  }
  //Clear compare interrupt
  RSI_TIMERS_InterruptClear(TIMERS, SLEEPTIMER_PERIPHERAL);
  low_period_timeout = 1;
  //Process the timer irq
  process_timer_irq(local_flag);
  CORE_EXIT_ATOMIC();
}

void SLEEPTIMER_COUNTER_IRQHandler(void)
{
  CORE_DECLARE_IRQ_STATE;
  uint8_t local_flag = 0;
  uint32_t irq_flag  = 0;
  //Read the Overflow interrupt status
  irq_flag = RSI_TIMERS_InterruptStatus(TIMERS, TIMER_0);
  CORE_ENTER_ATOMIC();
  if (irq_flag) {
    //overflow interrupt triggered
    local_flag |= SLEEPTIMER_EVENT_OF;
  }
  //Clear overflow interrupt
  RSI_TIMERS_InterruptClear(TIMERS, TIMER_0);
  //Process the timer irq
  process_timer_irq(local_flag);
  CORE_EXIT_ATOMIC();
}
/*******************************************************************************
 * Gets ULP timer frequency
 *
 * @param none
 *
 * @return uint32_t: timer frequency
 ******************************************************************************/
uint32_t sleeptimer_hal_get_timer_frequency(void)
{
  //return the ULP timer frequency
  return RSI_CLK_GetBaseClock(ULPSS_TIMER);
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
#endif /* SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_91x_ULPTIMER*/
