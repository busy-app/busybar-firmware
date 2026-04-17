/***************************************************************************/
/**
 * @file bod_button_example.h
 * @brief BOD Button example
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

#ifndef BOD_BUTTON_EXAMPLE_H_
#define BOD_BUTTON_EXAMPLE_H_

#define SL_BOD_BUTTON1_MIN_VOLTAGE_PERCENTAGE 56 ///< Minimum voltage percentage for Button 1
#define SL_BOD_BUTTON1_MAX_VOLTAGE_PERCENTAGE 65 ///< Maximum voltage percentage for Button 1

#define SL_BOD_BUTTON2_MIN_VOLTAGE_PERCENTAGE 48 ///< Minimum voltage percentage for Button 2
#define SL_BOD_BUTTON2_MAX_VOLTAGE_PERCENTAGE 55 ///< Maximum voltage percentage for Button 2

#define SL_BOD_BUTTON3_MIN_VOLTAGE_PERCENTAGE 42 ///< Minimum voltage percentage for Button 3
#define SL_BOD_BUTTON3_MAX_VOLTAGE_PERCENTAGE 47 ///< Maximum voltage percentage for Button 3

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
  * BOD Button example initialization function
  * Initializes clock, sets reference voltage and configures Voltage Threshold,
  * configures the slot value and registers the callback for BOD Button.
  *
  * @param none
  * @return none
  ******************************************************************************/
void bod_button_example_init(void);

/***************************************************************************/
/**
  * BOD Button example process action function
  * Reads the battery status, configures the voltage thresholds for buttons,
  * enables button wakeup and interrupts.
  *
  * @param none
  * @return none
  ******************************************************************************/
void bod_button_process_action(void);

#endif /* BOD_BUTTON_EXAMPLE_H_ */
