/***************************************************************************/ /**
 * @file sl_si91x_pwm_init_channel_0_config.h
 * @brief PWM configuration file.
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

#ifndef SL_SI91X_PWM_CHANNEL_0_CONFIG_H
#define SL_SI91X_PWM_CHANNEL_0_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************* PWM Configuration **************************/
// <h> PWM CHANNEL_0 Configuration
// <o SL_PWM_CHANNEL_0_FREQUENCY> Frequency <500-200000>
// <i> Default: 25000
#define SL_PWM_CHANNEL_0_FREQUENCY 25000

// <o SL_PWM_CHANNEL_0_POLARITY_LOW> Output Polarity Low
//   <SL_POLARITYL_LOW=> POLARITY_LOW
//   <SL_POLARITYL_HIGH=> POLARITY_HIGH
// <i> Default: SL_POLARITYL_HIGH
#define SL_PWM_CHANNEL_0_POLARITY_LOW SL_POLARITYL_HIGH

// <o SL_PWM_CHANNEL_0_POLARITY_HIGH> Output Polarity High
//   <SL_POLARITYH_LOW=> POLARITY_LOW
//   <SL_POLARITYH_HIGH=> POLARITY_HIGH
// <i> Default: SL_POLARITYH_HIGH
#define SL_PWM_CHANNEL_0_POLARITY_HIGH SL_POLARITYH_HIGH

// <o SL_PWM_CHANNEL_0_MODE> PWM Mode
//   <SL_MODE_INDEPENDENT=> Independent
//   <SL_MODE_COMPLEMENTARY=> Complementary
// <i> Default: SL_MODE_INDEPENDENT
#define SL_PWM_CHANNEL_0_MODE SL_MODE_INDEPENDENT

// <o SL_PWM_CHANNEL_0_TIMER_COUNTER> Timer Counter <0-65535>
// <i> Default: 0
#define SL_PWM_CHANNEL_0_TIMER_COUNTER 0

// <o SL_PWM_CHANNEL_0_DUTY_CYCLE> Duty Cycle <0-100>
// <i> Default: 50
#define SL_PWM_CHANNEL_0_DUTY_CYCLE 50

// <o SL_PWM_CHANNEL_0_TIMER_MODE> Base Timer Mode
//   <SL_FREE_RUN_MODE=> Free Run
//   <SL_SINGLE_EVENT_MODE=> Single Event
//   <SL_DOWN_COUNT_MODE=> Down Count
//   <SL_UP_DOWN_MODE=> Up_Down
//   <SL_UP_DOWN_DOUBLE_UPDATE=> Up_Down Double
// <i> Default: SL_FREE_RUN_MODE
#define SL_PWM_CHANNEL_0_TIMER_MODE SL_FREE_RUN_MODE

// <o SL_PWM_CHANNEL_0_EVENT> Ext Trigger
//   <0=> None
//   <1=> Fault A
//   <2=> Fault B
//   <3=> Ext Trigger1
//   <4=> Ext Trigger2
// <i> Default: 0
#define SL_PWM_CHANNEL_0_EVENT 0

// </h> End PWM CHANNEL_0 Configuration
/******************************************************************************/
// <<< end of configuration section >>>
#if USER_CONFIGURATION_ENABLE
// <<< sl:start pin_tool >>>
// <pwm_ch0 signal=0H,0L,> SL_PWM_OUT_CHANNEL0
// $[PWM_CH0_SL_PWM_OUT_CHANNEL0]
#ifndef SL_PWM_OUT_CHANNEL0_PERIPHERAL
#define SL_PWM_OUT_CHANNEL0_PERIPHERAL PWM_CH0
#endif
#ifndef SL_PWM_OUT_CHANNEL0_PERIPHERAL_NO
#define SL_PWM_OUT_CHANNEL0_PERIPHERAL_NO 0
#endif

// PWM_CH0 0H on GPIO_7
#ifndef SL_PWM_OUT_CHANNEL0_0H_PORT
#define SL_PWM_OUT_CHANNEL0_0H_PORT HP
#endif
#ifndef SL_PWM_OUT_CHANNEL0_0H_PIN
#define SL_PWM_OUT_CHANNEL0_0H_PIN 7
#endif
#ifndef SL_PWM_OUT_CHANNEL0_0H_LOC
#define SL_PWM_OUT_CHANNEL0_0H_LOC 0
#endif

// PWM_CH0 0L on GPIO_6
#ifndef SL_PWM_OUT_CHANNEL0_0L_PORT
#define SL_PWM_OUT_CHANNEL0_0L_PORT HP
#endif
#ifndef SL_PWM_OUT_CHANNEL0_0L_PIN
#define SL_PWM_OUT_CHANNEL0_0L_PIN 6
#endif
#ifndef SL_PWM_OUT_CHANNEL0_0L_LOC
#define SL_PWM_OUT_CHANNEL0_0L_LOC 2
#endif
// [PWM_CH0_SL_PWM_OUT_CHANNEL0]$
// <<< sl:end pin_tool >>>

// <<< sl:start pin_tool >>>
// <pwm signal=(FAULTA),(FAULTB),(TMR_EXT_TRIG_1),(TMR_EXT_TRIG_2)> SL_PWM_CHANNEL0
// $[PWM_SL_PWM_CHANNEL0]
#ifndef SL_PWM_CHANNEL0_PERIPHERAL
#define SL_PWM_CHANNEL0_PERIPHERAL PWM
#endif

// PWM FAULTA on GPIO_25
#ifndef SL_PWM_CHANNEL0_FAULTA_PORT
#define SL_PWM_CHANNEL0_FAULTA_PORT HP
#endif
#ifndef SL_PWM_CHANNEL0_FAULTA_PIN
#define SL_PWM_CHANNEL0_FAULTA_PIN 25
#endif
#ifndef SL_PWM_CHANNEL0_FAULTA_LOC
#define SL_PWM_CHANNEL0_FAULTA_LOC 16
#endif

// PWM FAULTB on GPIO_26
#ifndef SL_PWM_CHANNEL0_FAULTB_PORT
#define SL_PWM_CHANNEL0_FAULTB_PORT HP
#endif
#ifndef SL_PWM_CHANNEL0_FAULTB_PIN
#define SL_PWM_CHANNEL0_FAULTB_PIN 26
#endif
#ifndef SL_PWM_CHANNEL0_FAULTB_LOC
#define SL_PWM_CHANNEL0_FAULTB_LOC 19
#endif

// PWM TMR_EXT_TRIG_1 on GPIO_27
#ifndef SL_PWM_CHANNEL0_TMR_EXT_TRIG_1_PORT
#define SL_PWM_CHANNEL0_TMR_EXT_TRIG_1_PORT HP
#endif
#ifndef SL_PWM_CHANNEL0_TMR_EXT_TRIG_1_PIN
#define SL_PWM_CHANNEL0_TMR_EXT_TRIG_1_PIN 27
#endif
#ifndef SL_PWM_CHANNEL0_EXTTRIG1_LOC
#define SL_PWM_CHANNEL0_EXTTRIG1_LOC 22
#endif

// PWM TMR_EXT_TRIG_2 on GPIO_28
#ifndef SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_PORT
#define SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_PORT HP
#endif
#ifndef SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_PIN
#define SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_PIN 28
#endif
#ifndef SL_PWM_CHANNEL0_EXTTRIG2_LOC
#define SL_PWM_CHANNEL0_EXTTRIG2_LOC 26
#endif
// [PWM_SL_PWM_CHANNEL0]$
// <<< sl:end pin_tool >>>

// PWM channel number for CHANNEL_0 instance
#define SL_PWM_CHANNEL_0_OUTPUT_CHANNEL 0

#ifdef SL_PWM_OUT_CHANNEL0_0L_LOC
// Pin set for CHANNEL_0 PWM channel
#if (SL_PWM_OUT_CHANNEL0_0L_LOC == 2)
#define SL_PWM_CHANNEL_0_PIN_L SL_PWM_OUT_CHANNEL0_0L_PIN
#else
#define SL_PWM_CHANNEL_0_PIN_L (SL_PWM_OUT_CHANNEL0_0L_PIN + 64)
#endif
#define SL_PWM_CHANNEL_0_PORT_L 0
#else
#define SL_PWM_CHANNEL_0_PIN_L  SL_SI91X_PWM_0L_PIN
#define SL_PWM_CHANNEL_0_PORT_L 0
#endif // SL_PWM_OUT_CHANNEL0_0L_LOC
#ifdef SL_PWM_OUT_CHANNEL0_0H_LOC
#if (SL_PWM_OUT_CHANNEL0_0H_LOC == 0)
#define SL_PWM_CHANNEL_0_PIN_H SL_PWM_OUT_CHANNEL0_0H_PIN
#else
#define SL_PWM_CHANNEL_0_PIN_H (SL_PWM_OUT_CHANNEL0_0H_PIN + 64)
#endif
#define SL_PWM_CHANNEL_0_PORT_H 0
#else
#define SL_PWM_CHANNEL_0_PIN_H  SL_SI91X_PWM_0H_PIN
#define SL_PWM_CHANNEL_0_PORT_H 0
#endif // SL_PWM_OUT_CHANNEL0_0H_LOC

#define SL_PWM_CHANNEL_0_MUX_L SL_SI91X_PWM_0L_MUX
#define SL_PWM_CHANNEL_0_MUX_H SL_SI91X_PWM_0H_MUX

#define SL_PWM_CHANNEL_0_PAD_L SL_SI91X_PWM_0L_PAD
#define SL_PWM_CHANNEL_0_PAD_H SL_SI91X_PWM_0H_PAD
// PWM Fault Pin set resolution
#if (SL_PWM_CHANNEL_0_EVENT == 0)
#ifdef SL_PWM_CHANNEL0_FAULTA_LOC
#define SL_PWM_CHANNEL_0_PORT 0
#define SL_PWM_CHANNEL_0_PIN  SL_PWM_CHANNEL0_FAULTA_PIN
#else
#define SL_PWM_CHANNEL_0_PORT 0
#define SL_PWM_CHANNEL_0_PIN  SL_SI91X_PWM_FAULTA_PIN
#endif // SL_PWM_CHANNEL0_FAULTA_LOC
#define SL_PWM_CHANNEL_0_MUX 0
#define SL_PWM_CHANNEL_0_PAD SL_SI91X_PWM_FAULTA_PAD
#endif

#if (SL_PWM_CHANNEL_0_EVENT == 1)
#ifdef SL_PWM_CHANNEL0_FAULTA_LOC
#define SL_PWM_CHANNEL_0_PORT 0
#define SL_PWM_CHANNEL_0_PIN  SL_PWM_CHANNEL0_FAULTA_PIN
#else
#define SL_PWM_CHANNEL_0_PORT 0
#define SL_PWM_CHANNEL_0_PIN  SL_SI91X_PWM_FAULTA_PIN
#endif // SL_PWM_CHANNEL0_FAULTA_LOC
#define SL_PWM_CHANNEL_0_MUX SL_SI91X_PWM_FAULTA_MUX
#define SL_PWM_CHANNEL_0_PAD SL_SI91X_PWM_FAULTA_PAD
#endif

#if (SL_PWM_CHANNEL_0_EVENT == 2)
#ifdef SL_PWM_CHANNEL0_FAULTB_LOC
#define SL_PWM_CHANNEL_0_PORT 0
#if (SL_PWM_CHANNEL0_FAULTB_LOC == 19)
#define SL_PWM_CHANNEL_0_PIN SL_PWM_CHANNEL0_FAULTB_PIN
#else
#define SL_PWM_CHANNEL_0_PIN (SL_PWM_CHANNEL0_FAULTB_PIN + 64)
#endif
#else
#define SL_PWM_CHANNEL_0_PORT 0
#define SL_PWM_CHANNEL_0_PIN  SL_SI91X_PWM_FAULTB_PIN
#endif // SL_PWM_CHANNEL0_FAULTB_LOC
#define SL_PWM_CHANNEL_0_MUX SL_SI91X_PWM_FAULTB_MUX
#define SL_PWM_CHANNEL_0_PAD SL_SI91X_PWM_FAULTB_PAD
#endif

#if (SL_PWM_CHANNEL_0_EVENT == 3)
#ifdef SL_PWM_CHANNEL0_EXTTRIG1_LOC
#define SL_PWM_CHANNEL_0_PORT 0
#if ((SL_PWM_CHANNEL0_TMR_EXT_TRIG_1_LOC == 24) || (SL_PWM_CHANNEL0_TMR_EXT_TRIG_1_LOC == 25))
#define SL_PWM_CHANNEL_0_PIN (SL_PWM_CHANNEL0_TMR_EXT_TRIG_1_PIN + 64)
#else
#define SL_PWM_CHANNEL_0_PIN SL_PWM_CHANNEL0_TMR_EXT_TRIG_1_PIN
#endif
#else
#define SL_PWM_CHANNEL_0_PORT 0
#define SL_PWM_CHANNEL_0_PIN  SL_SI91X_PWM_TMR_EXT_TRIG_1_PIN
#endif // SL_PWM_CHANNEL0_EXTTRIG1_LOC
#define SL_PWM_CHANNEL_0_MUX SL_SI91X_PWM_TMR_EXT_TRIG_1_MUX
#define SL_PWM_CHANNEL_0_PAD SL_SI91X_PWM_TMR_EXT_TRIG_1_PAD
#endif

#if (SL_PWM_CHANNEL_0_EVENT == 4)
#ifdef SL_PWM_CHANNEL0_EXTTRIG2_LOC
#define SL_PWM_CHANNEL_0_PORT 0
#if ((SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_LOC == 28) || (SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_LOC == 29))
#define SL_PWM_CHANNEL_0_PIN (SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_PIN + 64)
#else
#define SL_PWM_CHANNEL_0_PIN SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_PIN
#endif
#else
#define SL_PWM_CHANNEL_0_PORT 0
#define SL_PWM_CHANNEL_0_PIN  SL_SI91X_PWM_TMR_EXT_TRIG_2_PIN
#endif // SL_PWM_CHANNEL0_EXTTRIG2_LOC
#define SL_PWM_CHANNEL_0_MUX SL_SI91X_PWM_TMR_EXT_TRIG_2_MUX
#define SL_PWM_CHANNEL_0_PAD SL_SI91X_PWM_TMR_EXT_TRIG_2_PAD
#endif

#else

#warning \
  "PWM Channel 0 pins are not configured. To configure, either install [ENABLE USER CONFIGURATION] component or define USER_CONFIGURATION_ENABLE macro to 1, then configure the pins as per the Custom board."

#endif // USER_CONFIGURATION_ENABLE
#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_PWM_CHANNEL_0_CONFIG_H */
