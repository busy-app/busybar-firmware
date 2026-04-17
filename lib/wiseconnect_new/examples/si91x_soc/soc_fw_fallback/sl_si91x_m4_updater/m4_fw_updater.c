/*******************************************************************************
 * @file  sl_si91x_m4_fw_updater.c
 * @brief
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

// Include Files
#include "rsi_ccp_user_config.h"
#include "rsi_debug.h"
#include "si91x_device.h"
#include "core_cm4.h"
#include <stdio.h>
#include <string.h>
#include "sl_si91x_fw_fallback.h"

/***************************************************************************/ /**
 *  @fn          void sl_si91x_m4_updater_app(void)
 *  @pre         None
 *  @brief       Retrieves the M4 application address and jumps to the M4 application.
 *               This function fetches the application address from the active firmware slot.
 *               If a valid address is obtained, it jumps to the M4 application; otherwise,
 *               it enters an infinite loop in case of failure.
 *  @param[in]   None
 *  @return      None
 ******************************************************************************/
void sl_si91x_m4_updater_app(void)
{
  sl_status_t status = SL_STATUS_FAIL;
  uint32_t app_addr  = 0;
  DEBUGOUT("\r\n M4 Updater Start ...\n");
  status = sl_si91x_get_m4_app_addr(&app_addr);
  if (status != SL_STATUS_OK) {
    while (1)
      ;
  }
  sl_si91x_jump_to_m4_application(app_addr);
}
