/***************************************************************************//**
 * @file
 * @brief Core functionality for Silicon Labs bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "btl_core.h"
#include "api/btl_errorcode.h"
// --------------------------------
// Global functions
int32_t btl_init(void)
{
  return BOOTLOADER_OK;
}

int32_t btl_deinit(void)
{
  return BOOTLOADER_OK;
}
