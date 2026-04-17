/***************************************************************************/ /**
* @file ulp_timer_example.h
* @brief ULP-timers example
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licenser of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

#ifndef SL_SI91X_ULP_TIMER_EXAMPLE_H_
#define SL_SI91X_ULP_TIMER_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
* Ulp timer example initialization function
* Clock and Timer Configurations
* At the configured timeout of timer, on board LED will toggle
*
* @param none
* @return none
******************************************************************************/
void ulp_timer_example_init(void);

/***************************************************************************/ /**
* Function will run continuously and will wait for interrupt
*
* @param none
* @return none
******************************************************************************/
void ulp_timer_example_process_action(void);

#endif /* SL_SI91X_ULP_TIMER_EXAMPLE_H_ */