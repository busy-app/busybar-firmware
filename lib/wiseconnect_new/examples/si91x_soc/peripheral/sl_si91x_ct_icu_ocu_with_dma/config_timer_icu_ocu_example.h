/***************************************************************************/ /**
* @file  config_timer_icu_ocu_example.h
* @brief Config Timer ICU OCU DMA examples functions
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_CONFIG_TIMER_ICU_OCU_EXAMPLE_H_
#define SL_SI91X_CONFIG_TIMER_ICU_OCU_EXAMPLE_H_

#define CT_COUNTER_DMA_MODE_USECASE    CLEAR //< To run DMA counter mode
#define CT_COUNTER_INPUT_EVENT_USECASE SET   //< To run input event mode
// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
* Config-timer-ICU-OCU_DMA example initialization function
* Clock and Timer Configurations, timeout callback registration
* First use-case: Here it will generate 2 PWM outputs, Counter-0 and Counter-1 will generate
* PWM output with varied duty cycle.
* Second use-case: It will capture the input event on GPIO pin and store the captured value in
* capture_value variable.
*
* @param none
* @return none
******************************************************************************/
void config_timer_icu_ocu_example_init(void);

/***************************************************************************/ /**
* Function will run continuously and will wait for interrupt
*
* @param none
* @return none
******************************************************************************/
void config_timer_icu_ocu_example_process_action(void);

#endif /* SL_SI91X_CONFIG_TIMER_ICU_OCU_EXAMPLE_H_ */