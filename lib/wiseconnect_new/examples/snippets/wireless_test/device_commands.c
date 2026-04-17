/*******************************************************************************
* @file  device_commands.c
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

#include "em_device.h"
#include "sl_status.h"
#include "sl_wifi.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_hal_soc_soft_reset.h"
#endif // SLI_SI91X_MCU_INTERFACE

sl_status_t wifi_reset_command_handler()
{
#ifdef SLI_SI91X_MCU_INTERFACE
  // In MCU mode, call sl_si91x_soc_nvic_reset to reset the NWP firmware
  sl_si91x_soc_nvic_reset();
#else
  // In NCP mode sl_wifi_deinit to be called to soft reset NWP firmware
  sl_wifi_deinit();
  // In NCP Mode call NVIC_SystemReset to reset the system
  NVIC_SystemReset();
#endif
  while (1)
    ;

  return SL_STATUS_OK;
}
