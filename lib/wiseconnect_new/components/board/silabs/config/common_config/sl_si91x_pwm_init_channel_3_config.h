/***************************************************************************/ /**
 * @file sl_si91x_pwm_init_channel_3_config.h
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

#ifndef SL_SI91X_PWM_CHANNEL_3_CONFIG_H
#define SL_SI91X_PWM_CHANNEL_3_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************* PWM Configuration **************************/
// <h> PWM CHANNEL_3 Configuration
// <o SL_PWM_CHANNEL_3_FREQUENCY> Frequency <500-200000>
// <i> Default: 25000
#define SL_PWM_CHANNEL_3_FREQUENCY 25000

// <o SL_PWM_CHANNEL_3_POLARITY_LOW> Output Polarity Low
//   <SL_POLARITYL_LOW=> POLARITY_LOW
//   <SL_POLARITYL_HIGH=> POLARITY_HIGH
// <i> Default: SL_POLARITYL_HIGH
#define SL_PWM_CHANNEL_3_POLARITY_LOW SL_POLARITYL_HIGH

// <o SL_PWM_CHANNEL_3_POLARITY_HIGH> Output Polarity High
//   <SL_POLARITYH_LOW=> POLARITY_LOW
//   <SL_POLARITYH_HIGH=> POLARITY_HIGH
// <i> Default: SL_POLARITYH_HIGH
#define SL_PWM_CHANNEL_3_POLARITY_HIGH SL_POLARITYH_HIGH

// <o SL_PWM_CHANNEL_3_MODE> PWM Mode
//   <SL_MODE_INDEPENDENT=> Independent
//   <SL_MODE_COMPLEMENTARY=> Complementary
// <i> Default: SL_MODE_INDEPENDENT
#define SL_PWM_CHANNEL_3_MODE SL_MODE_INDEPENDENT

// <o SL_PWM_CHANNEL_3_TIMER_COUNTER> Timer Counter <0-65535>
// <i> Default: 0
#define SL_PWM_CHANNEL_3_TIMER_COUNTER 0

// <o SL_PWM_CHANNEL_3_DUTY_CYCLE> Duty Cycle <0-100>
// <i> Default: 50
#define SL_PWM_CHANNEL_3_DUTY_CYCLE 50

// <o SL_PWM_CHANNEL_3_TIMER_MODE> Base Timer Mode
//   <SL_FREE_RUN_MODE=> Free Run
//   <SL_SINGLE_EVENT_MODE=> Single Event
//   <SL_DOWN_COUNT_MODE=> Down Count
//   <SL_UP_DOWN_MODE=> Up_Down
//   <SL_UP_DOWN_DOUBLE_UPDATE=> Up_Down Double
// <i> Default: SL_FREE_RUN_MODE
#define SL_PWM_CHANNEL_3_TIMER_MODE SL_FREE_RUN_MODE

// <o SL_PWM_CHANNEL_3_EVENT> Ext Trigger
//   <0=> None
//   <1=> Fault A
//   <2=> Fault B
//   <3=> Ext Trigger1
//   <4=> Ext Trigger2
// <i> Default: 0
#define SL_PWM_CHANNEL_3_EVENT 0

// </h> End PWM CHANNEL_3 Configuration
/******************************************************************************/
// <<< end of configuration section >>>
// <<< sl:start pin_tool >>>
// <pwm_ch3 signal=3H,3L> SL_PWM_OUT_CHANNEL3
// $[PWM_CH3_SL_PWM_OUT_CHANNEL3]
#ifndef SL_PWM_OUT_CHANNEL3_PERIPHERAL
#define SL_PWM_OUT_CHANNEL3_PERIPHERAL PWM_CH3
#endif
#ifndef SL_PWM_OUT_CHANNEL3_PERIPHERAL_NO
#define SL_PWM_OUT_CHANNEL3_PERIPHERAL_NO 3
#endif

// PWM_CH3 3H on ULP_GPIO_7/GPIO_71
#ifndef SL_PWM_OUT_CHANNEL3_3H_PORT
#define SL_PWM_OUT_CHANNEL3_3H_PORT ULP
#endif
#ifndef SL_PWM_OUT_CHANNEL3_3H_PIN
#define SL_PWM_OUT_CHANNEL3_3H_PIN 7
#endif
#ifndef SL_PWM_OUT_CHANNEL3_3H_LOC
#define SL_PWM_OUT_CHANNEL3_3H_LOC 13
#endif

// PWM_CH3 3L on ULP_GPIO_6/GPIO_70
#ifndef SL_PWM_OUT_CHANNEL3_3L_PORT
#define SL_PWM_OUT_CHANNEL3_3L_PORT ULP
#endif
#ifndef SL_PWM_OUT_CHANNEL3_3L_PIN
#define SL_PWM_OUT_CHANNEL3_3L_PIN 6
#endif
#ifndef SL_PWM_OUT_CHANNEL3_3L_LOC
#define SL_PWM_OUT_CHANNEL3_3L_LOC 15
#endif
// [PWM_CH3_SL_PWM_OUT_CHANNEL3]$
// <<< sl:end pin_tool >>>

// <<< sl:start pin_tool >>>
// <pwm signal=(FAULTA),(FAULTB),(TMR_EXT_TRIG_1),(TMR_EXT_TRIG_2)> SL_PWM_CHANNEL3
// $[PWM_SL_PWM_CHANNEL3]
#ifndef SL_PWM_CHANNEL3_PERIPHERAL
#define SL_PWM_CHANNEL3_PERIPHERAL PWM
#endif

// PWM FAULTA on GPIO_25
#ifndef SL_PWM_CHANNEL3_FAULTA_PORT
#define SL_PWM_CHANNEL3_FAULTA_PORT HP
#endif
#ifndef SL_PWM_CHANNEL3_FAULTA_PIN
#define SL_PWM_CHANNEL3_FAULTA_PIN 25
#endif
#ifndef SL_PWM_CHANNEL3_FAULTA_LOC
#define SL_PWM_CHANNEL3_FAULTA_LOC 16
#endif

// PWM FAULTB on GPIO_26
#ifndef SL_PWM_CHANNEL3_FAULTB_PORT
#define SL_PWM_CHANNEL3_FAULTB_PORT HP
#endif
#ifndef SL_PWM_CHANNEL3_FAULTB_PIN
#define SL_PWM_CHANNEL3_FAULTB_PIN 26
#endif
#ifndef SL_PWM_CHANNEL3_FAULTB_LOC
#define SL_PWM_CHANNEL3_FAULTB_LOC 19
#endif

// PWM TMR_EXT_TRIG_1 on GPIO_27
#ifndef SL_PWM_CHANNEL3_TMR_EXT_TRIG_1_PORT
#define SL_PWM_CHANNEL3_TMR_EXT_TRIG_1_PORT HP
#endif
#ifndef SL_PWM_CHANNEL3_TMR_EXT_TRIG_1_PIN
#define SL_PWM_CHANNEL3_TMR_EXT_TRIG_1_PIN 27
#endif
#ifndef SL_PWM_CHANNEL3_EXTTRIG1_LOC
#define SL_PWM_CHANNEL3_EXTTRIG1_LOC 22
#endif

// PWM TMR_EXT_TRIG_2 on GPIO_28
#ifndef SL_PWM_CHANNEL3_TMR_EXT_TRIG_2_PORT
#define SL_PWM_CHANNEL3_TMR_EXT_TRIG_2_PORT HP
#endif
#ifndef SL_PWM_CHANNEL3_TMR_EXT_TRIG_2_PIN
#define SL_PWM_CHANNEL3_TMR_EXT_TRIG_2_PIN 28
#endif
#ifndef SL_PWM_CHANNEL3_EXTTRIG2_LOC
#define SL_PWM_CHANNEL3_EXTTRIG2_LOC 26
#endif
// [PWM_SL_PWM_CHANNEL3]$
// <<< sl:end pin_tool >>>

// PWM channel number for CHANNEL_3 instance
#define SL_PWM_CHANNEL_3_OUTPUT_CHANNEL 3

// Pin set for CHANNEL_3 PWM channel
#ifdef SL_PWM_OUT_CHANNEL3_3L_LOC
#if (SL_PWM_OUT_CHANNEL3_3L_LOC == 15)
#define SL_PWM_CHANNEL_3_PIN_L (SL_PWM_OUT_CHANNEL3_3L_PIN + 64)
#else
#define SL_PWM_CHANNEL_3_PIN_L SL_PWM_OUT_CHANNEL3_3L_PIN
#endif
#define SL_PWM_CHANNEL_3_PORT_L 0
#else
#define SL_PWM_CHANNEL_3_PIN_L  SL_SI91X_PWM_3L_PIN
#define SL_PWM_CHANNEL_3_PORT_L 0
#endif //SL_PWM_OUT_CHANNEL3_3L_LOC

#ifdef SL_PWM_OUT_CHANNEL3_3L_LOC
#if (SL_PWM_OUT_CHANNEL3_3H_LOC == 13)
#define SL_PWM_CHANNEL_3_PIN_H (SL_PWM_OUT_CHANNEL3_3H_PIN + 64)
#else
#define SL_PWM_CHANNEL_3_PIN_H SL_PWM_OUT_CHANNEL3_3H_PIN
#endif
#define SL_PWM_CHANNEL_3_PORT_H 0
#else
#define SL_PWM_CHANNEL_3_PIN_H  SL_SI91X_PWM_3H_PIN
#define SL_PWM_CHANNEL_3_PORT_H 0
#endif //SL_PWM_OUT_CHANNEL3_3L_LOC

#define SL_PWM_CHANNEL_3_MUX_L SL_SI91X_PWM_3L_MUX
#define SL_PWM_CHANNEL_3_MUX_H SL_SI91X_PWM_3H_MUX

#define SL_PWM_CHANNEL_3_PAD_L SL_SI91X_PWM_3L_PAD
#define SL_PWM_CHANNEL_3_PAD_H SL_SI91X_PWM_3H_PAD

// PWM Fault Pin set resolution
#if (SL_PWM_CHANNEL_3_EVENT == 0)
#ifdef SL_PWM_CHANNEL3_FAULTA_LOC
#define SL_PWM_CHANNEL_3_PORT 0
#define SL_PWM_CHANNEL_3_PIN  SL_PWM_CHANNEL3_FAULTA_PIN
#else
#define SL_PWM_CHANNEL_3_PORT 0
#define SL_PWM_CHANNEL_3_PIN  SL_SI91X_PWM_FAULTA_PIN
#endif //SL_PWM_CHANNEL3_FAULTA_LOC
#define SL_PWM_CHANNEL_3_MUX 0
#define SL_PWM_CHANNEL_3_PAD SL_SI91X_PWM_FAULTA_PAD
#endif

#if (SL_PWM_CHANNEL_3_EVENT == 1)
#ifdef SL_PWM_CHANNEL3_FAULTA_LOC
#define SL_PWM_CHANNEL_3_PORT 0
#if (SL_PWM_CHANNEL3_FAULTA_LOC == 16)
#define SL_PWM_CHANNEL_3_PIN SL_PWM_CHANNEL3_FAULTA_PIN
#else
#define SL_PWM_CHANNEL_3_PIN (SL_PWM_CHANNEL3_FAULTA_PIN + 64)
#endif
#else
#define SL_PWM_CHANNEL_3_PORT 0
#define SL_PWM_CHANNEL_3_PIN  SL_SI91X_PWM_FAULTA_PIN
#endif //SL_PWM_CHANNEL3_FAULTA_LOC
#define SL_PWM_CHANNEL_3_MUX SL_SI91X_PWM_FAULTA_MUX
#define SL_PWM_CHANNEL_3_PAD SL_SI91X_PWM_FAULTA_PAD
#endif

#if (SL_PWM_CHANNEL_3_EVENT == 2)
#ifdef SL_PWM_CHANNEL3_FAULTB_LOC
#define SL_PWM_CHANNEL_3_PORT 0
#if (SL_PWM_CHANNEL3_FAULTB_LOC == 19)
#define SL_PWM_CHANNEL_3_PIN SL_PWM_CHANNEL3_FAULTB_PIN
#else
#define SL_PWM_CHANNEL_3_PIN (SL_PWM_CHANNEL3_FAULTB_PIN + 64)
#endif
#else
#define SL_PWM_CHANNEL_3_PORT 0
#define SL_PWM_CHANNEL_3_PIN  SL_SI91X_PWM_FAULTB_PIN
#endif //SL_PWM_CHANNEL3_FAULTB_LOC
#define SL_PWM_CHANNEL_3_MUX SL_SI91X_PWM_FAULTB_MUX
#define SL_PWM_CHANNEL_3_PAD SL_SI91X_PWM_FAULTB_PAD
#endif

#if (SL_PWM_CHANNEL_3_EVENT == 3)
#ifdef SL_PWM_CHANNEL3_EXTTRIG1_LOC
#define SL_PWM_CHANNEL_3_PORT 0
#if ((SL_PWM_CHANNEL3_TMR_EXT_TRIG_1_LOC == 24) || (SL_PWM_CHANNEL3_TMR_EXT_TRIG_1_LOC == 25))
#define SL_PWM_CHANNEL_3_PIN (SL_PWM_CHANNEL3_TMR_EXT_TRIG_1_PIN + 64)
#else
#define SL_PWM_CHANNEL_3_PIN SL_PWM_CHANNEL3_TMR_EXT_TRIG_1_PIN
#endif
#else
#define SL_PWM_CHANNEL_3_PORT 0
#define SL_PWM_CHANNEL_3_PIN  SL_SI91X_PWM_TMR_EXT_TRIG_1_PIN
#endif //SL_PWM_CHANNEL3_EXTTRIG1_LOC
#define SL_PWM_CHANNEL_3_MUX SL_SI91X_PWM_TMR_EXT_TRIG_1_MUX
#define SL_PWM_CHANNEL_3_PAD SL_SI91X_PWM_TMR_EXT_TRIG_1_PAD
#endif

#if (SL_PWM_CHANNEL_3_EVENT == 4)
#ifdef SL_PWM_CHANNEL3_EXTTRIG2_LOC
#define SL_PWM_CHANNEL_3_PORT 0
#if ((SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_LOC == 28) || (SL_PWM_CHANNEL0_TMR_EXT_TRIG_2_LOC == 29))
#define SL_PWM_CHANNEL_3_PIN (SL_PWM_CHANNEL3_TMR_EXT_TRIG_2_PIN + 64)
#else
#define SL_PWM_CHANNEL_3_PIN SL_PWM_CHANNEL3_TMR_EXT_TRIG_2_PIN
#endif
#else
#define SL_PWM_CHANNEL_3_PORT 0
#define SL_PWM_CHANNEL_3_PIN  SL_SI91X_PWM_TMR_EXT_TRIG_2_PIN
#endif //SL_PWM_CHANNEL3_EXTTRIG2_LOC
#define SL_PWM_CHANNEL_3_MUX SL_SI91X_PWM_TMR_EXT_TRIG_2_MUX
#define SL_PWM_CHANNEL_3_PAD SL_SI91X_PWM_TMR_EXT_TRIG_2_PAD
#endif

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_PWM_CHANNEL_3_CONFIG_H */
