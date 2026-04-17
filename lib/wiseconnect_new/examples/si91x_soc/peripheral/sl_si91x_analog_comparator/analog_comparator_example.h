/***************************************************************************/ /**
* @file  analog_comparator_example.h
* @brief analog comparator examples functions
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

#ifndef ANALOG_COMPARATOR_EXAMPLE_H_
#define ANALOG_COMPARATOR_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
* Analog comparator example initialization function
* Initializes clock, sets reference voltage and configures comparator inputs
* If non-inverting input voltage is greater than inverting input voltage, comparator
* output becomes high & interrupt occurs and toggles the GPIO.
*
* @param none
* @return none
******************************************************************************/
void analog_comparator_example_init(void);

/***************************************************************************/ /**
* Function will run continuously and will wait for interrupt
*
* @param none
* @return none
******************************************************************************/
void analog_comparator_example_process_action(void);

#endif /* ANALOG_COMPARATOR_EXAMPLE_H_ */
