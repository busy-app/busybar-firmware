/***************************************************************************//**
 * @file sli_token_manager_backend_nvm.c
 * @brief Flash backend operation. Invokes respective flash read, write request
 * to internal or external flash HAL driver.
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "sli_token_manager_hal_flash.h"
#include "sli_token_manager_backend_nvm.h"

sl_status_t sli_token_manager_flash_open(uint32_t flash_adr, uint32_t flash_size)
{
  // Init static secure token region
  return sli_token_manager_hal_flash_handle.open(flash_adr, flash_size);
}

void sli_token_manager_flash_close(void)
{
  sli_token_manager_hal_flash_handle.close();
}

// Odd size is not supported by this function
sl_status_t sli_token_manager_flash_write(uint32_t flash_adr, void const *data, uint32_t size)
{
  return sli_token_manager_hal_flash_handle.write(flash_adr, data, size);
}

// Odd size is not supported by this function
sl_status_t sli_token_manager_flash_read(uint32_t flash_adr, void *data, uint32_t size)
{
  return sli_token_manager_hal_flash_handle.read(flash_adr, data, size);
}

sl_status_t sli_token_manager_flash_erase(uint32_t flash_adr, uint32_t size)
{
  return sli_token_manager_hal_flash_handle.erase(flash_adr, size);
}
