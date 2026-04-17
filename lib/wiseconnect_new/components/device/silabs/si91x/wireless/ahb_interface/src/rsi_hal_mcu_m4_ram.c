/***************************************************************************/ /**
 * @file  rsi_hal_mcu_m4_ram.c
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

#if defined(SLI_SI917)
//! This file should be in RAM
#include "sl_device.h"

/*==================================================*/
/**
 * @fn          void sli_mv_m4_app_from_flash_to_ram(int option)
 * @brief       Raise interrupt to NWP and poll for task done 
 * @param[in]   option
 * @param[out]  none  
 */

void sli_mv_m4_app_from_flash_to_ram(int option)
{

  //! Disable all interrupts
  __disable_irq();

  if (option == UPGRADE_M4_IMAGE_OTA) {
    //! Raise interrupt to NWP
    raise_m4_to_ta_interrupt(UPGRADE_M4_IMAGE);

    //! Poll for bit to clear
    while (M4SS_P2P_INTR_CLR_REG & UPGRADE_M4_IMAGE)
      ;
  } else if (option == TA_WRITES_ON_COMM_FLASH) {
    //! Raise interrupt to NWP
    raise_m4_to_ta_interrupt(M4_WAITING_FOR_TA_TO_WR_ON_FLASH);

    //! Poll for bit to clear
    while (M4SS_P2P_INTR_CLR_REG & M4_WAITING_FOR_TA_TO_WR_ON_FLASH)
      ;
  } else if (option == M4_WAIT_FOR_NWP_DEINIT) {
    //! Raise interrupt to NWP
    raise_m4_to_ta_interrupt(M4_WAITING_FOR_TA_DEINIT);

    //! Poll for bit to clear
    while (M4SS_P2P_INTR_CLR_REG & M4_WAITING_FOR_TA_DEINIT)
      ;
  }

  //! Enable all interrupts
  __enable_irq();
}

#endif
