/*******************************************************************************
 * @file  sl_si91x_lf_fsm_clock_sel.h
 * @brief
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
#ifndef __SL_SI91X_LF_FSM_CLOCK_SEL__
#define __SL_SI91X_LF_FSM_CLOCK_SEL__

// <<< Use Configuration Wizard in Context Menu >>>
// <h>LF-FSM Clock Configurations(32kHz)

// Define clock selection options numerically
// 1: RC 32kHz Clock, 2: XTAL 32kHz Clock
#define RC_KHZ_CLK_SEL   1 // RC 32kHz clock selection
#define XTAL_KHZ_CLK_SEL 2 // XTAL 32kHz clock selection

// <o LF_FSM_CLOCK_SELECTION> LF-FSM CLOCK SELECTION
//   <XTAL_KHZ_CLK_SEL=> 32KHZ XTAL CLK
//   <RC_KHZ_CLK_SEL=> 32KHZ RC CLK
// <i> Default: XTAL_KHZ_CLK_SEL

// User's clock source selection
#define LF_FSM_CLOCK_SELECTION XTAL_KHZ_CLK_SEL // Default is XTAL_KHZ_CLK_SEL

// Clock selection logic
#if (LF_FSM_CLOCK_SELECTION == XTAL_KHZ_CLK_SEL)
// If XTAL 32kHz clock is selected
#define _32KHZ_XTAL_CLOCK 1 // Define XTAL 32kHz clock
#elif (LF_FSM_CLOCK_SELECTION == RC_KHZ_CLK_SEL)
// If RC 32kHz clock is selected
#define _32KHZ_RC_CLOCK 1 // Define RC 32kHz clock
#endif

// </h>
// <<< end of configuration section >>>

#endif //__SL_SI91X_LF_FSM_CLOCK_SEL__
