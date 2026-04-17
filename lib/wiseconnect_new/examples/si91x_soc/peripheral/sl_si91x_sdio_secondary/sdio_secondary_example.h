/***************************************************************************/ /**
 * @file sdio_secondary_example.h
 * @brief Top level application functions
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
#ifndef SDIO_SECONDARY_EXAMPLE_H
#define SDIO_SECONDARY_EXAMPLE_H

/***************************************************************************/ /**
 * Initialize application.
 ******************************************************************************/
void sdio_secondary_example_init(void);

/***************************************************************************/ /**
 * The state machine code for send and receive is implemented here.
 * This function is called in while loop.
 ******************************************************************************/
void sdio_secondary_example_process_action(void);

#endif // SDIO_SECONDARY_EXAMPLE_H
