/***************************************************************************/ /**
 * @file file_operations_example.h
 * @brief file system example functions
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef FILE_OPERATIONS_EXAMPLE_H_
#define FILE_OPERATIONS_EXAMPLE_H_

/*******************************************************************************
 *************************** Macros   *******************************
 ******************************************************************************/

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * FILE System example initialization function
 * @param none
 * @return none
 ******************************************************************************/
void file_operations_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously and will wait for trigger
 * 
 * @param none
 * @return none
 ******************************************************************************/
void file_operations_example_process_action(void);

#endif /* FILE_OPERATIONS_EXAMPLE_H_ */
