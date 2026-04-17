/***************************************************************************//**
 * @file
 * @brief Radio coexistence directional priority priority(DP)
 * @details Use the coexistence priority GPIO to communicate to the coexistence
 *   master device whether the request is for an RX or TX. In the case of an
 *   RX, the priority GPIO will be pulsed for a configurable period of time.
 *   In the case of a TX, the priority GPIO will be held through out the
 *   duration of the request.
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
#include "coexistence-hal.h"
#include "rail.h"

#include "sl_hal_prs.h"
#include "sl_hal_timer.h"
#include "sl_device_peripheral.h"
#include "sl_clock_manager.h"

#if SL_RAIL_UTIL_COEX_DP_ENABLED
#ifdef PLATFORM_HEADER
#include PLATFORM_HEADER
#endif //PLATFORM_HEADER
#ifdef SL_RAIL_UTIL_COEX_PWM_REQ_PORT
#define SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PORT (SL_RAIL_UTIL_COEX_PWM_REQ_PORT)
#define SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PIN (SL_RAIL_UTIL_COEX_PWM_REQ_PIN)
#else //!SL_RAIL_UTIL_COEX_PWM_REQ_PORT
#define SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PORT (SL_RAIL_UTIL_COEX_REQ_PORT)
#define SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PIN (SL_RAIL_UTIL_COEX_REQ_PIN)
#endif //SL_RAIL_UTIL_COEX_PWM_REQ_PORT
#if SL_RAIL_UTIL_COEX_PRI_ASSERT_LEVEL == 0
#error "Directional priority does not support active low priority(SL_RAIL_UTIL_COEX_PRI_ASSERT_LEVEL == 0)"
#endif //SL_RAIL_UTIL_COEX_PRI_ASSERT_LEVEL == 0
#if SL_RAIL_UTIL_COEX_REQ_ASSERT_LEVEL == 0
#error "Directional priority does not support active low request(SL_RAIL_UTIL_COEX_REQ_ASSERT_LEVEL == 0)"
#endif //SL_RAIL_UTIL_COEX_REQ_ASSERT_LEVEL == 0

#define TIMER_FREQUENCY (get_timer_frequency() / TIMER_DIVISOR)
#define SL_BUS_CLOCK_TIMER_DP GET_TIMER_REG(SL_BUS_CLOCK, _SL_RAIL_UTIL_COEX_DP_TIMER)
#define _GPIO_TIMER GET_TIMER_REG(_GPIO, _SL_RAIL_UTIL_COEX_DP_TIMER)
#ifdef _SILICON_LABS_32B_SERIES_3
#define _GPIO_TIMER_CC0ROUTE_PIN_SHIFT GET_TIMER_REG(_GPIO_TIMER, _CC0ROUTE_PIN_SHIFT)
#define _GPIO_TIMER_CC0ROUTE_PORT_SHIFT GET_TIMER_REG(_GPIO_TIMER, _CC0ROUTE_PORT_SHIFT)
#define TIMERROUTE TIMER_DP_ROUTE
#define TIMER_DEVICE_ID(timer) (0U)
#else
/** Map TIMER reference to index of device. */
#define TIMER_DEVICE_ID(timer) ( \
    (timer) == TIMER0   ? 0      \
    : (timer) == TIMER1 ? 1      \
    : (timer) == TIMER2 ? 2      \
    : (timer) == TIMER3 ? 3      \
    : -1)
#endif
#define SL_PERIPHERAL_TIMER_DP GET_TIMER_REG(SL_PERIPHERAL, _SL_RAIL_UTIL_COEX_DP_TIMER)
#define SL_RAIL_UTIL_COEX_DP_TIMER_ROUTE GET_TIMER_REG(SL_RAIL_UTIL_COEX_DP_TIMER, ROUTE)
#define TIMER_DIVISOR 2
#define TIMER_PRESC_DIV_PREFIX TIMER_CFG_PRESC_DIV
#define PRS_CH_CTRL_SIGSEL_TIMER_DP GET_TIMER_REG(PRS_ASYNC, _SL_RAIL_UTIL_COEX_DP_TIMER)
#define PRS_CH_CTRL_SIGSEL_TIMERCC0_DP GET_TIMER_REG(PRS_CH_CTRL_SIGSEL_TIMER_DP, _CC0)
#define GET_TIMER_REG(reg, timer) GET_TIMER_REG_(reg, timer)
#define GET_TIMER_REG_(reg, timer) reg ## timer

#if SL_RAIL_UTIL_COEX_PRI_SHARED
#define SL_RAIL_UTIL_COEX_DP_MODE SL_GPIO_MODE_WIRED_OR
#else //!SL_RAIL_UTIL_COEX_PRI_SHARED
#define SL_RAIL_UTIL_COEX_DP_MODE SL_GPIO_MODE_PUSH_PULL
#endif //SL_RAIL_UTIL_COEX_PRI_SHARED

#define __STRIP_TYPECAST(x)
#define _STRIP_TYPECAST(x)              __STRIP_TYPECAST x
#define STRIP_TYPECAST(x)               _STRIP_TYPECAST x
#define SL_RAIL_UTIL_COEX_DP_TIMER_ADDR STRIP_TYPECAST(SL_RAIL_UTIL_COEX_DP_TIMER_PERIPHERAL)

#if defined(TIMER0_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == TIMER0_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _TIMER0
#define TIMER_DP_ROUTE TIMER0ROUTE
#elif defined(TIMER1_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == TIMER1_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _TIMER1
#define TIMER_DP_ROUTE TIMER1ROUTE
#elif defined(TIMER2_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == TIMER2_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _TIMER2
#define TIMER_DP_ROUTE TIMER2ROUTE
#elif defined(TIMER3_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == TIMER3_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _TIMER3
#define TIMER_DP_ROUTE TIMER3ROUTE
#elif defined(TIMER4_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == TIMER4_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _TIMER4
#define TIMER_DP_ROUTE TIMER4ROUTE
#elif defined(TIMER5_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == TIMER5_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _TIMER5
#define TIMER_DP_ROUTE TIMER5ROUTE
#elif defined(TIMER6_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == TIMER6_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _TIMER6
#define TIMER_DP_ROUTE TIMER6ROUTE
#elif defined(WTIMER0_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == WTIMER0_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _WTIMER0
#define TIMER_DP_ROUTE WTIMER0ROUTE
#elif defined(WTIMER1_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == WTIMER1_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _WTIMER1
#define TIMER_DP_ROUTE WTIMER1ROUTE
#elif defined(WTIMER2_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == WTIMER2_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _WTIMER2
#define TIMER_DP_ROUTE WTIMER2ROUTE
#elif defined(WTIMER3_BASE) && SL_RAIL_UTIL_COEX_DP_TIMER_ADDR == WTIMER3_BASE
#define _SL_RAIL_UTIL_COEX_DP_TIMER _WTIMER3
#define TIMER_DP_ROUTE WTIMER3ROUTE
#else
#error "Unrecognized timer selection!"
#endif
#define SL_RAIL_UTIL_COEX_DP_TIMER SL_RAIL_UTIL_COEX_DP_TIMER_PERIPHERAL

static uint8_t directionalPriorityPulseWidthUs;
static bool directionalPriorityInitialized = false;

typedef struct PRS_ChannelConfig {
  union {
    uint32_t source;
    uint32_t *sourcePtr;
  };
  union {
    uint32_t signal;
    uint32_t *signalPtr;
  };
  uint32_t ctrl;
  uint32_t channel;
  bool needToDereference;
} PRS_ChannelConfig_t;

#define TIMER_PRESC_DIV GET_TIMER_REG(TIMER_PRESC_DIV_PREFIX, TIMER_DIVISOR)
#define MICROSECONDS_PER_SECOND (1000000UL)

static uint32_t get_timer_frequency(void)
{
  uint32_t clkFreq;
  sl_clock_branch_t clock_branch;
  clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_TIMER_DP);
  sl_clock_manager_get_clock_branch_frequency(clock_branch, &clkFreq);
  return clkFreq;
}

static bool configDpTimer(uint8_t pulseWidthUs)
{
  uint32_t ticks = (pulseWidthUs * TIMER_FREQUENCY) / MICROSECONDS_PER_SECOND;
  // Setup TIMER for ONE-SHOT Triggers by REQUEST rising edge-----------------
  // Reset compare output at start, run off HFPERCLK / 16, run in debug, count up,
  // Reload on rising edge, use one-shot mode
  // set PRS to track CC out level, set on start, clear on compare, PWM
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_TIMER_DP);
  sl_hal_timer_wait_sync(SL_RAIL_UTIL_COEX_DP_TIMER);
  SL_RAIL_UTIL_COEX_DP_TIMER->EN_CLR = TIMER_EN_EN;
#if defined(_TIMER_EN_DISABLING_MASK)
  while ((SL_RAIL_UTIL_COEX_DP_TIMER->EN & TIMER_EN_DISABLING) != 0U) {
  }
#endif
  SL_RAIL_UTIL_COEX_DP_TIMER->CFG = TIMER_CFG_OSMEN
                                    | TIMER_CFG_DEBUGRUN
                                    | TIMER_PRESC_DIV
                                    | TIMER_CFG_RSSCOIST;
  SL_RAIL_UTIL_COEX_DP_TIMER->CC[0].CFG = TIMER_CC_CFG_MODE_PWM
                                          | TIMER_CC_CFG_PRSCONF;
  SL_RAIL_UTIL_COEX_DP_TIMER->EN_SET = TIMER_EN_EN;

  SL_RAIL_UTIL_COEX_DP_TIMER->CTRL = TIMER_CTRL_RISEA_RELOADSTART;
  SL_RAIL_UTIL_COEX_DP_TIMER->CC[0].CTRL = TIMER_CC_CTRL_OUTINV
                                           | TIMER_CC_CTRL_CMOA_SET;
  // pulse => CEIL(1200kHz*PULSE-1)
  GPIO->TIMERROUTE[TIMER_DEVICE_ID(SL_RAIL_UTIL_COEX_DP_TIMER)].CC0ROUTE = (SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PORT << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
                                                                           | (SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PIN
                                                                              << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  sl_hal_timer_set_top_buffer(SL_RAIL_UTIL_COEX_DP_TIMER, ticks);
  sl_hal_timer_set_top(SL_RAIL_UTIL_COEX_DP_TIMER, ticks);
  sl_hal_timer_set_counter(SL_RAIL_UTIL_COEX_DP_TIMER, 0);
  sl_hal_timer_channel_set_compare_buffer(SL_RAIL_UTIL_COEX_DP_TIMER, 0, ticks);
  sl_hal_timer_channel_set_compare(SL_RAIL_UTIL_COEX_DP_TIMER, 0, ticks);
  sl_hal_timer_enable(SL_RAIL_UTIL_COEX_DP_TIMER);
  return true;
}

__STATIC_INLINE void configPrsChain(PRS_ChannelConfig_t *prsConfig,
                                    unsigned int channelCount)
{
#ifdef SL_RAIL_UTIL_COEX_PRI_PORT
  // enable ptaPriCfg interrupt if not already enabled
  if (sli_coex_ptaPriCfg.intNo == INVALID_INTERRUPT) {
    sli_coex_enableGpioInt(&(sli_coex_ptaPriCfg), true);
  }
#endif //SL_RAIL_UTIL_COEX_PRI_PORT

#ifdef SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  // enable ptaPwmReqCfg interrupt if not already enabled
  if (sli_coex_ptaPwmReqCfg.intNo == INVALID_INTERRUPT) {
    sli_coex_enableGpioInt(&(sli_coex_ptaPwmReqCfg), true);
  }
#else //!SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  // enable ptaReqCfg interrupt if not already enabled
  if (sli_coex_ptaReqCfg.intNo == INVALID_INTERRUPT) {
    sli_coex_enableGpioInt(&(sli_coex_ptaReqCfg), true);
  }
#endif //SL_RAIL_UTIL_COEX_PWM_REQ_PORT

  for (unsigned int ch = 0; ch < channelCount; ++ch) {
    if (prsConfig[ch].needToDereference) {
      sl_hal_prs_async_connect_channel_producer(prsConfig[ch].channel,
                                                *(prsConfig[ch].sourcePtr)
                                                | *(prsConfig[ch].signalPtr));
    } else {
      sl_hal_prs_async_connect_channel_producer(prsConfig[ch].channel,
                                                prsConfig[ch].source
                                                | prsConfig[ch].signal);
    }
    sl_hal_prs_async_combine_signals(prsConfig[ch].channel,
                                     WRAP_PRS_ASYNC(prsConfig[ch].channel - 1),
                                     (sl_hal_prs_logic_t)prsConfig[ch].ctrl);
  }
}

#define  CONFIG_PRS_CHAIN(prsChain) (configPrsChain(prsChain, \
                                                    sizeof(prsChain) / sizeof(prsChain[0])))

PRS_ChannelConfig_t prsChainOff[] = {
#ifdef SL_RAIL_UTIL_COEX_PRI_PORT
  {
    .sourcePtr = &(sli_coex_ptaPriCfg.source),
    .signalPtr = &(sli_coex_ptaPriCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A,
    .channel = SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL,
    .needToDereference = true
  }
#else //!defined(SL_RAIL_UTIL_COEX_PRI_PORT)
#ifdef SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  {
    .sourcePtr = &(sli_coex_ptaPwmReqCfg.source),
    .signalPtr = &(sli_coex_ptaPwmReqCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 2),
    .needToDereference = true
  },
  {
    .sourcePtr = &(sli_coex_ptaPwmReqCfg.source),
    .signalPtr = &(sli_coex_ptaPwmReqCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 1),
    .needToDereference = true
  },
  {
    .sourcePtr = &(sli_coex_ptaPwmReqCfg.source),
    .signalPtr = &(sli_coex_ptaPwmReqCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A_AND_NOT_B,
    .channel = SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL,
    .needToDereference = true
  }
#else //!SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  {
    .sourcePtr = &(sli_coex_ptaReqCfg.source),
    .signalPtr = &(sli_coex_ptaReqCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 2),
    .needToDereference = true
  },
  {
    .sourcePtr = &(sli_coex_ptaReqCfg.source),
    .signalPtr = &(sli_coex_ptaReqCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 1),
    .needToDereference = true
  },
  {
    .sourcePtr = &(sli_coex_ptaReqCfg.source),
    .signalPtr = &(sli_coex_ptaReqCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A_AND_NOT_B,
    .channel = SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL,
    .needToDereference = true
  }
#endif //SL_RAIL_UTIL_COEX_PWM_REQ_PORT
#endif //SL_RAIL_UTIL_COEX_PRI_PORT
};

#ifdef PRS_RACL_PAEN
#define PRS_RAC_PAEN PRS_RACL_PAEN
#endif

PRS_ChannelConfig_t prsChainOn[] = {
#ifdef SL_RAIL_UTIL_COEX_PRI_PORT
  {
    .sourcePtr = &(sli_coex_ptaPriCfg.source),
    .signalPtr = &(sli_coex_ptaPriCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 3),
    .needToDereference = true
  },
  {
    .signal = PRS_CH_CTRL_SIGSEL_TIMERCC0_DP,
    .ctrl = SL_HAL_PRS_LOGIC_NOT_A_AND_B,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 2),
    .needToDereference = false
  },
  {
    .signal = PRS_RAC_PAEN,
    .ctrl = SL_HAL_PRS_LOGIC_A_NOR_B,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 1),
    .needToDereference = false
  },
#else //!SL_RAIL_UTIL_COEX_PRI_PORT
  {
    .signal = PRS_CH_CTRL_SIGSEL_TIMERCC0_DP,
    .ctrl = SL_HAL_PRS_LOGIC_NOT_A,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 2),
    .needToDereference = false
  },
  {
    .signal = PRS_RAC_PAEN,
    .ctrl = SL_HAL_PRS_LOGIC_NOT_A,
    .channel = WRAP_PRS_ASYNC(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL - 1),
    .needToDereference = false
  },
#endif //SL_RAIL_UTIL_COEX_PRI_PORT
#ifdef SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  {
    .sourcePtr = &(sli_coex_ptaPwmReqCfg.source),
    .signalPtr = &(sli_coex_ptaPwmReqCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A_AND_NOT_B,
    .channel = SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL,
    .needToDereference = true
  }
#else //!SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  {
    .sourcePtr = &(sli_coex_ptaReqCfg.source),
    .signalPtr = &(sli_coex_ptaReqCfg.signal),
    .ctrl = SL_HAL_PRS_LOGIC_A_AND_NOT_B,
    .channel = SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL,
    .needToDereference = true
  }
#endif //SL_RAIL_UTIL_COEX_PWM_REQ_PORT
};

bool COEX_HAL_ConfigDp(uint8_t pulseWidthUs)
{
  sl_status_t status = SL_STATUS_OK;
  // Common PRS setup (clock enable, REQUEST and PRIORITY GPIO INT PRS sources)
  // enable clock to PRS
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);
#ifdef SL_RAIL_UTIL_COEX_PRI_PORT
  if (sli_coex_ptaPriCfg.intNo == INVALID_INTERRUPT) {
    sli_coex_enableGpioInt(&(sli_coex_ptaPriCfg), true);
  }
  // Disable priority and request interrupts
  status = sl_gpio_configure_external_interrupt(
    &(sl_gpio_t){SL_RAIL_UTIL_COEX_PRI_PORT,
                 SL_RAIL_UTIL_COEX_PRI_PIN },
    &sli_coex_ptaPriCfg.intNo,
    SL_GPIO_INTERRUPT_NO_EDGE,
    NULL,
    (void *)NULL);
  if (status != SL_STATUS_OK) {
    return false;
  }
#endif //SL_RAIL_UTIL_COEX_PRI_PORT
#ifdef SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  if (sli_coex_ptaPwmReqCfg.intNo == INVALID_INTERRUPT) {
    sli_coex_enableGpioInt(&(sli_coex_ptaPwmReqCfg), true);
  }
  status = sl_gpio_configure_external_interrupt(
    &(sl_gpio_t){SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PORT,
                 SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PIN },
    &sli_coex_ptaPwmReqCfg.intNo,
    SL_GPIO_INTERRUPT_NO_EDGE,
    NULL,
    (void *)NULL);
#else //!SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  if (sli_coex_ptaReqCfg.intNo == INVALID_INTERRUPT) {
    sli_coex_enableGpioInt(&(sli_coex_ptaReqCfg), true);
  }
  status = sl_gpio_configure_external_interrupt(
    &(sl_gpio_t){SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PORT,
                 SL_RAIL_UTIL_COEX_DP_TIMER_CC0_PIN },
    &sli_coex_ptaReqCfg.intNo,
    SL_GPIO_INTERRUPT_NO_EDGE,
    NULL,
    (void *)NULL);
#endif //SL_RAIL_UTIL_COEX_PWM_REQ_PORT
  if (status != SL_STATUS_OK) {
    return false;
  }
  // Common PRS wrap-up (enable PRIORITY GPIO, route PRS output to GPIO)
  // enable PRIORITY output pin with initial value of 0
  sl_gpio_t gpio_pin = { .port = SL_RAIL_UTIL_COEX_DP_OUT_PORT, .pin = SL_RAIL_UTIL_COEX_DP_OUT_PIN };
  sl_gpio_set_pin_mode(&gpio_pin, SL_RAIL_UTIL_COEX_DP_MODE, false);
  sl_hal_prs_pin_output(SL_RAIL_UTIL_COEX_DP_OUT_CHANNEL, SL_HAL_PRS_TYPE_ASYNC, SL_RAIL_UTIL_COEX_DP_OUT_PORT, SL_RAIL_UTIL_COEX_DP_OUT_PIN);
  return COEX_HAL_SetDpPulseWidth(pulseWidthUs);
}

bool COEX_HAL_SetDpPulseWidth(uint8_t pulseWidthUs)
{
  if (directionalPriorityInitialized
      && directionalPriorityPulseWidthUs == pulseWidthUs) {
    return true;
  }
  directionalPriorityInitialized = true;
  directionalPriorityPulseWidthUs = pulseWidthUs;
  if (pulseWidthUs == 0) {
    SL_RAIL_UTIL_COEX_DP_TIMER->EN_SET = TIMER_EN_EN;
    sl_hal_timer_disable(SL_RAIL_UTIL_COEX_DP_TIMER);
    CONFIG_PRS_CHAIN(prsChainOff);
    return true;
  }
  if (!configDpTimer(directionalPriorityPulseWidthUs)) {
    return false;
  }
  CONFIG_PRS_CHAIN(prsChainOn);
  return true;
}

uint8_t COEX_HAL_GetDpPulseWidth(void)
{
  return directionalPriorityPulseWidthUs;
}

#else //!SL_RAIL_UTIL_COEX_DP_ENABLED

bool COEX_HAL_ConfigDp(uint8_t pulseWidthUs)
{
  (void)pulseWidthUs;
  return false;
}

uint8_t COEX_HAL_GetDpPulseWidth(void)
{
  return 0;
}

bool COEX_HAL_SetDpPulseWidth(uint8_t pulseWidthUs)
{
  (void)pulseWidthUs;
  return false;
}
#endif //SL_RAIL_UTIL_COEX_DP_ENABLED
