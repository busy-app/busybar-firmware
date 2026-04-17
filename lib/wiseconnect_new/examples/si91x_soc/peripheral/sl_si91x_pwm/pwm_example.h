/***************************************************************************/ /**
 * @file pwm_example.h
 * @brief pwm example
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef PWM_EXAMPLE_H_
#define PWM_EXAMPLE_H_

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * PWM example initialization function ,this function initializes the PWM pins,
 * clock etc.
 * @param none
 * @return none
 ******************************************************************************/
void pwm_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously in while loop.
 * @param none
 * @return none
 ******************************************************************************/
void pwm_example_process_action(void);

#endif /* PWM_EXAMPLE_H_ */
