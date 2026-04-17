/******************************************************************************
* @file sl_si91x_peripheral_sdio_secondary.c
* @brief SDIO SLAVE API implementation
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_peripheral_sdio_secondary.h"
#include "rsi_egpio.h"

/*******************************************************************************
 * This API initializes the SDIO secondary, by default SDIO pin access with NWP
 * this API gives the pin access to M4, selects the SDIO mode and mash the sdio
 * primary interrupts
 ******************************************************************************/
void sl_si91x_sdio_secondary_peripheral_init(void)
{
  // Clearing bit to give SDIO pin access to M4
  (*(volatile uint32_t *)(NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR)) = (0x1 << 5);

  // Select SDIO mode
  MISC_CFG_HOST_CTRL |= SDIO_MODE_SELECT;

  while ((MISC_CFG_HOST_CTRL & SDIO_MODE_SELECT) != SDIO_MODE_SELECT)
    ;

  // Mask the sdio primary interrupts
  M4_HOST_INTR_MASK_REG = MASK_HOST_INTERRUPT;
}
