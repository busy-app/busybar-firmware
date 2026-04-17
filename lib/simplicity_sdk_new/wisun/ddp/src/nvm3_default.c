/***************************************************************************//**
 * @file nvm3_default.c
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

#include "nvm3_default.h"
#include "nvm3_hal_flash.h"

nvm3_Handle_t  nvm3_defaultHandleData;
nvm3_Handle_t *nvm3_defaultHandle = &nvm3_defaultHandleData;

nvm3_Init_t nvm3_defaultInitData =
{
  NULL,                   ///< NVM memory area base address
  0,                      ///< NVM memory area size in bytes
  NULL,                   ///< A pointer to cache
  0,                      ///< The size of the cache in number of elements
  NVM3_MAX_OBJECT_SIZE,   ///< The maximum object size in bytes
  0,                      ///< The size difference between the user and forced repacks
  &nvm3_halFlashHandle    ///< HAL handle
};

nvm3_Init_t *nvm3_defaultInit = &nvm3_defaultInitData;

Ecode_t nvm3_initDefault(void)
{
  return nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
}

Ecode_t nvm3_deinitDefault(void)
{
  return nvm3_close(nvm3_defaultHandle);
}
