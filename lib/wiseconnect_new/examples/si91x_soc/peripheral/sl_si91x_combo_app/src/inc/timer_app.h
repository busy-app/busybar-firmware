/*
 * timer_app.h
 *
 *  Created on: Dec 8, 2023
 *      Author: vebommak
 */

#ifndef SRC_INC_TIMER_APP_H_
#define SRC_INC_TIMER_APP_H_
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define ONE_SEC                       1000000 // 1 second in microseconds
#define TIME_IN_MICROSECONDS          ONE_SEC // timer timeout in micro seconds
#define TIME_ONE_MS                   1000    // timer timeout in milli seconds
#define CLOCKS_PER_MICROSECONDS_20MHZ 20      // clock cycles per microsecond for 20 mhz clock frequency
#define CLOCKS_PER_MICROSECONDS_32MHZ 32
#define MS_5000                       5000 // 5 Sec delay in milliseconds
/*****************************************************************************
 * ULP Timer-match value calculation :
 * For one microsecond type, match-value = number of microseconds
 * For 256 microsecond type, match-value = (time in microseconds)/256
 * For normal up/down counter type, timer =  (number of clock cycles per microseconds * time in microseconds)
 ******************************************************************************/
#define SL_TIMER_MATCH_VALUE \
  ((CLOCKS_PER_MICROSECONDS_20MHZ) * (TIME_IN_MICROSECONDS)) // Timer match value for down-counter type with 20mhz clock

#define LED0                           0    // For On-board LED-0
#define ONE_SEC_MATCH_VALUE_256US_TYPE 3906 // Timer match value for 1-sec, in 256us type
#define FIFTH_INTERRUPT_COUNT          5    // Count for fifth timeout interrupt
#define TENTH_INTERRUPT_COUNT          10   // Count for tenth timeout interrupt

// Macros used to construct ulp-timer instance
#define ULP_TIMER_INSTANCE \
  SL_ULP_TIMER_TIMER0 // ulp-timer instance to be used, user can pass selected timer-number in place of '0'
#define _CONCAT_TWO_TOKENS(token_1, token_2) token_1##token_2
#define CONCAT_TWO_TOKENS(token_1, token_2)  _CONCAT_TWO_TOKENS(token_1, token_2)
#define SL_ULP_TIMER_HANDLE                  CONCAT_TWO_TOKENS(sl_timer_handle_timer, ULP_TIMER_INSTANCE)
#define SL_ULP_TIMER_CALLBACK                CONCAT_TWO_TOKENS(on_timeout_callback, ULP_TIMER_INSTANCE)
/*******************************************************************************
 **********************  Global Function prototypes   ***************************
 ******************************************************************************/
void timer_init(void);

#endif /* SRC_INC_TIMER_APP_H_ */
