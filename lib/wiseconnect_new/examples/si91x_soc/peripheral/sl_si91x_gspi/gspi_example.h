/***************************************************************************/ /**
 * @file gspi_example.h
 * @brief GSPI examples functions
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

#ifndef GSPI_EXAMPLE_H_
#define GSPI_EXAMPLE_H_

#define SL_USE_TRANSFER ENABLE
#define SL_USE_SEND     DISABLE
#define SL_USE_RECEIVE  DISABLE

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * GSPI example initialization function
 * Clock is configured, followed by power mode, and GSPI configuration
 * 
 * @param none
 * @return none
 ******************************************************************************/
void gspi_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously 
 * State machine flow is set for transfer, send and receive operations.
 * Loopback test is initiated through the master, and it will compare the 
 * data received.
 * After loopback, send and receive are executed.
 * 
 * @note For send and receive, master/slave connection is required.
 * For transfer, loopback can be used.
 * 
 * @param none
 * @return none
 ******************************************************************************/
void gspi_example_process_action(void);

#endif /* CALENDAR_EXAMPLE_H_ */
