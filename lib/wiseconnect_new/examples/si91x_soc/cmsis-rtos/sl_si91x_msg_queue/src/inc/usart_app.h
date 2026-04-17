/***************************************************************************/ /**
 * @file usart_example.h
 * @brief usart examples functions
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

#ifndef USART_EXAMPLE_H_
#define USART_EXAMPLE_H_

/*******************************************************************************
 *************************** Macros   *******************************
 ******************************************************************************/
// IF send and receive should happen continuously then both macros should be enabled
#define USE_SEND            DISABLE ///< Used to enable USART send
#define USE_RECEIVE         ENABLE  ///< Used to enable USART receive
#define SL_USART_SYNCH_MODE DISABLE ///< To validate the synchronous mode, this macro needs to be enabled

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
//Enum for different transmission scenarios
typedef enum {
  SL_SEND_DATA,
  SL_RECEIVE_DATA,
  SL_TRANSFER_DATA,
  SL_TRANSMISSION_COMPLETED,
} usart_mode_enum_t;

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * USART example initialization function
 * Calendar clock is configured.
 * A deafult structure of datetime is builded and set as the current value of calendar
 * The value of calendar is fetched and displayed on serial console.
 * As per the macros are enabled, the example will run alarm, millisecond trigger
 * one second trigger, time conversion and clock calibration.
 * 
 * @param none
 * @return none
 ******************************************************************************/
void usart_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously and will wait for trigger
 * 
 * @param none
 * @return none
 ******************************************************************************/
void usart_example_process_action(void);

#endif /* USART_EXAMPLE_H_ */
