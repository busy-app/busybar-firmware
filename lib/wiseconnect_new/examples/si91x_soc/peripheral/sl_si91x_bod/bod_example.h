/***************************************************************************/
/**
 * @file bod_example.h
 * @brief BOD example
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

#ifndef BOD_EXAMPLE_H_
#define BOD_EXAMPLE_H_

#define SL_BOD_DEFAULT_SLOT_VALUE  2      ///< Default BOD Slot value
#define SL_BOD_DEFAULT_THRESHOLD   2.7f   ///< Default BOD threshold value
#define SL_BOD_DELAY_VALUE         100000 ///< Delay value for process action
#define SL_BOD_VBAT_THRESHOD_VALUE 15.0f  ///< Vbat threshold value
#define SL_BOD_500MS_DELAY         500    ///< 500ms application Delay
// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * BOD example initialization function
 * Initializes clock, sets reference voltage and configures Voltage Threshold,
 * configures the slot value and registers the callback for BOD.
 *
 * @param none
 * @return none
 ******************************************************************************/
void bod_example_init(void);

/***************************************************************************/
/**
 * Function will run continuously and will wait for interrupt
 *
 * @param none
 * @return none
 ******************************************************************************/
void bod_example_process_action(void);

#endif /* BOD_EXAMPLE_H_ */
