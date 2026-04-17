/***************************************************************************/ /**
 * @file power_manager_tickless_idle_example.h
 * @brief Power Manager example functions
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

#ifndef POWER_MANAGER_EXAMPLE_H_
#define POWER_MANAGER_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * Initializes the thread.
 * Button is initialized here.
 * New thread is created with the application_start function.
 * - Application start function is called in the thread.
 * - All the activities are performed in this function.
 * - Wireless interface is initialized first and then power manager service is initialized.
 * - Events are subscribed to get the callbacks.
 * - This function is called in the thread.
 * - This function transits between different power states and sleep
 *******************************************************************************/
void power_manager_example_init(void);

/***************************************************************************/ /**
 * This Function is called inside the thread in while loop.
 ******************************************************************************/
void power_manager_example_process_action(void);

#endif /* POWER_MANAGER_EXAMPLE_H_ */
