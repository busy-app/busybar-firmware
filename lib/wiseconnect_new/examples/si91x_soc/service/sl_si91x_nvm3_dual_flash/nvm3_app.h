/***************************************************************************/ /**
 * @file
 * @brief NVM3 examples functions
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

#ifndef NVM3_APP_H
#define NVM3_APP_H

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#define KEY_SIZE  10
#define DATA_SIZE 254

/***************************************************************************/ /**
 * Function to initialize NVM3 example
 * @brief
 *  This will call nvm3_open() with default parameters for
 *  memory base address and size, cache size, etc.
 *  NVM-memory.
 * @param  none
 * @return none
 ******************************************************************************/
void nvm3_app_init(void);

/***************************************************************************/ /**
 * Function will run continuously and wait for input
 *
 * @param  none
 * @return none
 ******************************************************************************/
void nvm3_app_process_action(void);

#endif // NVM3_APP_H
