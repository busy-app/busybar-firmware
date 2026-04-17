/***************************************************************************/ /**
 * @file sl_si91x_pwm_init_led0_config.h
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

#ifndef SL_SI91X_PWM_LED0_CONFIG_H
#define SL_SI91X_PWM_LED0_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************* PWM Configuration **************************/
// <h> PWM LED0 Configuration

// <o SL_PWM_LED0_FREQUENCY> Frequency <500-200000>
// <i> Default: 25000
#define SL_PWM_LED0_FREQUENCY 25000

// <o SL_PWM_LED0_POLARITY_LOW> Output Polarity Low
//   <SL_POLARITYL_LOW=> POLARITY_LOW
//   <SL_POLARITYL_HIGH=> POLARITY_HIGH
// <i> Default: SL_POLARITYL_HIGH
#define SL_PWM_LED0_POLARITY_LOW SL_POLARITYL_HIGH

// <o SL_PWM_LED0_POLARITY_HIGH> Output Polarity High
//   <SL_POLARITYH_LOW=> POLARITY_LOW
//   <SL_POLARITYH_HIGH=> POLARITY_HIGH
// <i> Default: SL_POLARITYH_HIGH
#define SL_PWM_LED0_POLARITY_HIGH SL_POLARITYH_HIGH

// <o SL_PWM_LED0_MODE> PWM Mode
//   <SL_MODE_INDEPENDENT=> Independent
//   <SL_MODE_COMPLEMENTARY=> Complementary
// <i> Default: SL_INDEPENDENT
#define SL_PWM_LED0_MODE SL_MODE_INDEPENDENT

// <o SL_PWM_LED0_TIMER_COUNTER> Timer Counter <0-655365>
// <i> Default: 0
#define SL_PWM_LED0_TIMER_COUNTER 0

// <o SL_PWM_LED0_DUTY_CYCLE> Duty Cycle <0-100>
// <i> Default: 50
#define SL_PWM_LED0_DUTY_CYCLE 50

// <o SL_PWM_LED0_TIMER_MODE> Base Timer Mode
//   <SL_FREE_RUN_MODE=> Free Run
//   <SL_SINGLE_EVENT_MODE=> Single Event
//   <SL_DOWN_COUNT_MODE=> Down Count
//   <SL_UP_DOWN_MODE=> Up_Down
//   <SL_UP_DOWN_DOUBLE_UPDATE=> Up_Down Double
// <i> Default: SL_FREE_RUN_MODE
#define SL_PWM_LED0_TIMER_MODE SL_FREE_RUN_MODE

// <o SL_PWM_LED0_TIMER_SELECTION> Base Timer Selection
//   <SL_BASE_TIMER_EACH_CHANNEL=> Timer(Each Channel)
//   <SL_BASE_TIMER_ALL_CHANNEL=> Timer(All Channels)
// <i> Default: SL_BASE_TIMER_EACH_CHANNEL
#define SL_PWM_LED0_TIMER_SELECTION SL_BASE_TIMER_EACH_CHANNEL

// <o SL_PWM_LED0_EVENT> Ext Trigger
//   <0=> None
//   <1=> Fault A
//   <2=> Fault B
//   <3=> Ext Trigger1
//   <4=> Ext Trigger2
// <i> Default: 0
#define SL_PWM_LED0_EVENT 0

// </h>
/******************************************************************************/
// <<< end of configuration section >>>

// PWM channel number for LED0 instance
#define SL_PWM_LED0_OUTPUT_CHANNEL 1

// Pin set for LED0 PWM channel
#define SL_PWM_LED0_PIN_L 66
#define SL_PWM_LED0_PIN_H 67

#define SL_PWM_LED0_PORT_L 0
#define SL_PWM_LED0_PORT_H 0

#define SL_PWM_LED0_MUX_L 8
#define SL_PWM_LED0_MUX_H 8

#define SL_PWM_LED0_PAD_L 24
#define SL_PWM_LED0_PAD_H 25

// PWM Fault Pin set resolution
#if (SL_PWM_LED0_EVENT == 0)
#define SL_PWM_LED0_PORT SL_SI91X_PWM_FAULTA_PORT
#define SL_PWM_LED0_PIN  SL_SI91X_PWM_FAULTA_PIN
#define SL_PWM_LED0_MUX  0
#define SL_PWM_LED0_PAD  SL_SI91X_PWM_FAULTA_PAD
#endif

#if (SL_PWM_LED0_EVENT == 1)
#define SL_PWM_LED0_PORT SL_SI91X_PWM_FAULTA_PORT
#define SL_PWM_LED0_PIN  SL_SI91X_PWM_FAULTA_PIN
#define SL_PWM_LED0_MUX  SL_SI91X_PWM_FAULTA_MUX
#define SL_PWM_LED0_PAD  SL_SI91X_PWM_FAULTA_PAD
#endif

#if (SL_PWM_LED0_EVENT == 2)
#define SL_PWM_LED0_PORT SL_SI91X_PWM_FAULTB_PORT
#define SL_PWM_LED0_PIN  SL_SI91X_PWM_FAULTB_PIN
#define SL_PWM_LED0_MUX  SL_SI91X_PWM_FAULTB_MUX
#define SL_PWM_LED0_PAD  SL_SI91X_PWM_FAULTB_PAD
#endif

#if (SL_PWM_LED0_EVENT == 3)
#define SL_PWM_LED0_PORT SL_SI91X_PWM_TMR_EXT_TRIG_1_PORT
#define SL_PWM_LED0_PIN  SL_SI91X_PWM_TMR_EXT_TRIG_1_PIN
#define SL_PWM_LED0_MUX  SL_SI91X_PWM_TMR_EXT_TRIG_1_MUX
#define SL_PWM_LED0_PAD  SL_SI91X_PWM_TMR_EXT_TRIG_1_PAD
#endif

#if (SL_PWM_LED0_EVENT == 4)
#define SL_PWM_LED0_PORT SL_SI91X_PWM_TMR_EXT_TRIG_2_PORT
#define SL_PWM_LED0_PIN  SL_SI91X_PWM_TMR_EXT_TRIG_2_PIN
#define SL_PWM_LED0_MUX  SL_SI91X_PWM_TMR_EXT_TRIG_2_MUX
#define SL_PWM_LED0_PAD  SL_SI91X_PWM_TMR_EXT_TRIG_2_PAD
#endif

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_PWM_LED0_CONFIG_H */