/***************************************************************************//**
 * @file nvm3_default.h
 * @brief NVM3 definition of the default data structures.
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

#ifndef NVM3_DEFAULT_H
#define NVM3_DEFAULT_H

#include "nvm3.h"

/// Default handle
extern nvm3_Handle_t *nvm3_defaultHandle;
/// Default initialization data
extern nvm3_Init_t   *nvm3_defaultInit;

/***************************************************************************//**
 * Initialize the default NVM3 instance.
 *
 * @return
 *   @ref ECODE_NVM3_OK on success and an NVM3 @ref Ecode_t on failure.
 *
 * Once initialized the instance can be accessed through the NVM3 API using
 * nvm3_defaultHandle as the nvm3_Handle_t handle.
 ******************************************************************************/
Ecode_t nvm3_initDefault(void);

/***************************************************************************//**
 * Deinit the default NVM3 instance.
 *
 * @return
 *   @ref ECODE_NVM3_OK on success and an NVM3 @ref Ecode_t on failure.
 ******************************************************************************/
Ecode_t nvm3_deinitDefault(void);

#endif /* NVM3_DEFAULT_H */
