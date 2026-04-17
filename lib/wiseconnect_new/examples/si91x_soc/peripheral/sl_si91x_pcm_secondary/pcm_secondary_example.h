/***************************************************************************/ /**
 * @file pcm_secondary_example.h
 * @brief pcm secondary example functions
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

#ifndef PCM_SECONDARY_EXAMPLE_H_
#define PCM_SECONDARY_EXAMPLE_H_

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * PCM example initialization function
 * Clock is configured, followed by power mode, and PCM configuration
 * 
 * @param none
 * @return none
 ******************************************************************************/
void pcm_secondary_example_init(void);

/***************************************************************************/ /**
 * Function will run continuously 
 * State machine flow is set for transfer, send and receive operations.
 * 
 * 
 * @param none
 * @return none
 ******************************************************************************/
void pcm_secondary_example_process_action(void);

#endif /* PCM_SECONDARY_EXAMPLE_H_ */
