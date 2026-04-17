/*******************************************************************************
* @file  sl_pwm_board.h
* @brief sl pwm board specific configuration
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

#ifndef __SL_PWM_BOARD_H__
#define __SL_PWM_BOARD_H__
/**
 * @defgroup  Board_Specific_Driver   Board specific drivers and support functions
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup  Board_Drivers BOARD: Common board components used with board drivers
 * @{
 */
#include "em_device.h"

#define SL_SI91X_PWM_0L_PORT RTE_PWM_1L_PORT
#define SL_SI91X_PWM_0L_PIN  RTE_PWM_1L_PIN
#define SL_SI91X_PWM_0L_MUX  RTE_PWM_1L_MUX
#define SL_SI91X_PWM_0L_PAD  RTE_PWM_1L_PAD

#define SL_SI91X_PWM_0H_PORT RTE_PWM_1H_PORT
#define SL_SI91X_PWM_0H_PIN  RTE_PWM_1H_PIN
#define SL_SI91X_PWM_0H_MUX  RTE_PWM_1H_MUX
#define SL_SI91X_PWM_0H_PAD  RTE_PWM_1H_PAD

#define SL_SI91X_PWM_1L_PORT RTE_PWM_2L_PORT
#define SL_SI91X_PWM_1L_PIN  RTE_PWM_2L_PIN
#define SL_SI91X_PWM_1L_MUX  RTE_PWM_2L_MUX
#define SL_SI91X_PWM_1L_PAD  RTE_PWM_2L_PAD

#define SL_SI91X_PWM_1H_PORT RTE_PWM_2H_PORT
#define SL_SI91X_PWM_1H_PIN  RTE_PWM_2H_PIN
#define SL_SI91X_PWM_1H_MUX  RTE_PWM_2H_MUX
#define SL_SI91X_PWM_1H_PAD  RTE_PWM_2H_PAD

#define SL_SI91X_PWM_2L_PORT RTE_PWM_3L_PORT
#define SL_SI91X_PWM_2L_PIN  RTE_PWM_3L_PIN
#define SL_SI91X_PWM_2L_MUX  RTE_PWM_3L_MUX
#define SL_SI91X_PWM_2L_PAD  RTE_PWM_3L_PAD

#define SL_SI91X_PWM_2H_PORT RTE_PWM_3H_PORT
#define SL_SI91X_PWM_2H_PIN  RTE_PWM_3H_PIN
#define SL_SI91X_PWM_2H_MUX  RTE_PWM_3H_MUX
#define SL_SI91X_PWM_2H_PAD  RTE_PWM_3H_PAD

#define SL_SI91X_PWM_3L_PORT RTE_PWM_4L_PORT
#define SL_SI91X_PWM_3L_PIN  RTE_PWM_4L_PIN
#define SL_SI91X_PWM_3L_MUX  RTE_PWM_4L_MUX
#define SL_SI91X_PWM_3L_PAD  RTE_PWM_4L_PAD

#define SL_SI91X_PWM_3H_PORT RTE_PWM_4H_PORT
#define SL_SI91X_PWM_3H_PIN  RTE_PWM_4H_PIN
#define SL_SI91X_PWM_3H_MUX  RTE_PWM_4H_MUX
#define SL_SI91X_PWM_3H_PAD  RTE_PWM_4H_PAD

#define SL_SI91X_PWM_FAULTA_PORT RTE_PWM_FAULTA_PORT
#define SL_SI91X_PWM_FAULTA_PIN  RTE_PWM_FAULTA_PIN
#define SL_SI91X_PWM_FAULTA_MUX  RTE_PWM_FAULTA_MUX
#define SL_SI91X_PWM_FAULTA_PAD  RTE_PWM_FAULTA_PAD

#define SL_SI91X_PWM_FAULTB_PORT RTE_PWM_FAULTB_PORT
#define SL_SI91X_PWM_FAULTB_PIN  RTE_PWM_FAULTB_PIN
#define SL_SI91X_PWM_FAULTB_MUX  RTE_PWM_FAULTB_MUX
#define SL_SI91X_PWM_FAULTB_PAD  RTE_PWM_FAULTB_PAD

#define SL_SI91X_PWM_SLP_EVENT_TRIG_PORT RTE_PWM_SLP_EVENT_TRIG_PORT
#define SL_SI91X_PWM_SLP_EVENT_TRIG_PIN  RTE_PWM_SLP_EVENT_TRIG_PIN
#define SL_SI91X_PWM_SLP_EVENT_TRIG_MUX  RTE_PWM_SLP_EVENT_TRIG_MUX
#define SL_SI91X_PWM_SLP_EVENT_TRIG_PAD  RTE_PWM_SLP_EVENT_TRIG_PAD

#define SL_SI91X_PWM_TMR_EXT_TRIG_1_PORT RTE_PWM_TMR_EXT_TRIG_1_PORT
#define SL_SI91X_PWM_TMR_EXT_TRIG_1_PIN  RTE_PWM_TMR_EXT_TRIG_1_PIN
#define SL_SI91X_PWM_TMR_EXT_TRIG_1_MUX  RTE_PWM_TMR_EXT_TRIG_1_MUX
#define SL_SI91X_PWM_TMR_EXT_TRIG_1_PAD  RTE_PWM_TMR_EXT_TRIG_1_PAD

#define SL_SI91X_PWM_TMR_EXT_TRIG_2_PORT RTE_PWM_TMR_EXT_TRIG_2_PORT
#define SL_SI91X_PWM_TMR_EXT_TRIG_2_PIN  RTE_PWM_TMR_EXT_TRIG_2_PIN
#define SL_SI91X_PWM_TMR_EXT_TRIG_2_MUX  RTE_PWM_TMR_EXT_TRIG_2_MUX
#define SL_SI91X_PWM_TMR_EXT_TRIG_2_PAD  RTE_PWM_TMR_EXT_TRIG_2_PAD

#ifdef __cplusplus
}
#endif

#endif /*__SL_PWM_BOARD_H__*/

/* @} end of Board_Drivers */
/* @} end of Board_Specific_Driver */
