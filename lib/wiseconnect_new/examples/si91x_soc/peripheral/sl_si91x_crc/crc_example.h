/***************************************************************************/ /**
* @file  crc_example.h
* @brief crc examples functions
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
#ifndef EXAMPLE_CRC_H_
#define EXAMPLE_CRC_H_

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define BUFFSIZE        25 // Size of the buffer.
#define NUMDMADESC      1  // Number of DMA descriptors.
#define SL_DMA_CHANNEL  5  // DMA channel number.
#define SL_DMA_INSTANCE 0  // DMA instance number.

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * It is an initialization function, it initializes the clock for CRC.
 *
 * @param none
 * @return none
 ******************************************************************************/
void crc_example_init(void);

/***************************************************************************/ /**
 * The state machine code for send and receive is implemented here.
 * This function is called in while loop.
 *
 * @param none
 * @return none
 ******************************************************************************/
void crc_example_process_action(void);

#endif /* EXAMPLE_CRC_H_ */
