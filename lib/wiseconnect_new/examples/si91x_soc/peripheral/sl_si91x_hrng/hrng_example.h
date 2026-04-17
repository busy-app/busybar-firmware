/***************************************************************************/ /**
 * @file hrng_example.h
 * @brief HRNG example
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

#ifndef SL_SI91X_HRNG_EXAMPLE_H_
#define SL_SI91X_HRNG_EXAMPLE_H_

#include "sl_status.h"
// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * @brief
 *    HRNG example initialization function.
 *
 * @details
 *    This function initializes the HRNG peripheral and triggers the HRNG transfer process.
 *
 * @pre
 *    The system must be initialized before calling this function.
 ******************************************************************************/
void hrng_example_init(void);

/***************************************************************************/
/**
 * @brief
 *    HRNG process action function.
 *
 * @details
 *    This function handles continuous HRNG actions, such as starting the HRNG, generating random bytes,
 *    and stopping it. It will also wait for interrupts to proceed with further operations.
 *
 * @pre
 *    HRNG should be initialized using @ref hrng_example_init.
 *
 * @return
 *    None.
 ******************************************************************************/
void hrng_example_process_action(void);

#endif /* SL_SI91X_HRNG_EXAMPLE_H_ */
