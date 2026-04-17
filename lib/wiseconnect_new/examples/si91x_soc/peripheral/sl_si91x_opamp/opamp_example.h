/*******************************************************************************
* @file  opamp_example.h
* @brief OPAMP Example
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

#ifndef OPAMP_EXAMPLE_H_
#define OPAMP_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * OPAMP example initialization function
 * Initializes clock, sets reference voltage and configures Voltage Threshold,
 * configures the slot value and registers the callback for BOD.
 *
 * @param none
 * @return none
 ******************************************************************************/
void opamp_example_init(void);
/***************************************************************************/
/**
 * Function will run continuously and will wait for interrupt
 *
 * @param none
 * @return none
 ******************************************************************************/
void opamp_example_process_action(void);

#endif /* OPAMP_EXAMPLE_H_ */
