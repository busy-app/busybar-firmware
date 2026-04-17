/***************************************************************************/ /**
 * @file gpio_uulp_example.h
 * @brief GPIO UULP example 
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

#ifndef GPIO_UULP_EXAMPLE_H_
#define GPIO_UULP_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * GPIO example initialization function. It initializes pin mode,
 * direction and configures UULP pin interrupt
 * @param none
 * @return none
 ******************************************************************************/
void gpio_uulp_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously in while loop and led toggles for every 1sec 
 * @param none
 * @return none
 ******************************************************************************/
void gpio_uulp_example_process_action(void);

#endif /* GPIO_UULP_EXAMPLE_H_ */
