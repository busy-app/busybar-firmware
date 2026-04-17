/***************************************************************************/ /**
 * @file joystick_example.h
 * @brief Joystick example functions
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

#ifndef SL_JOYSTICK_EXAMPLE_H_
#define SL_JOYSTICK_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * Joystick example initialization function
 * This API initialize the Joystick and start/enable the joystick to data acquisition
 *
 * @param none
 * @return none
 ******************************************************************************/
void joystick_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously
 * This API will get the position/direction of Joystick and print the position on console.
 * @param none
 * @return none
 ******************************************************************************/
void joystick_example_process_action(void);

#endif /* SL_JOYSTICK_EXAMPLE_H_ */
