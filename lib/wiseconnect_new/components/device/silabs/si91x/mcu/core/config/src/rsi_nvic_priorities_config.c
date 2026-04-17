/*******************************************************************************
* @file  rsi_nvic_priorities_config.c
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
/*
  Sets the default priority for the NVIC interrupts equal to Max SYSCall Priority to prevent config assert */

#include "rsi_nvic_priorities_config.h"

#ifdef SLI_SI91X_ENABLE_OS
void sl_si91x_device_init_nvic()
{
  for (IRQn_Type i = TIMER0_IRQn; i < SI91X_EXT_IRQ_COUNT; i++) {
    NVIC_SetPriority(i, DEFAULT_PRIORITY);
  }
}
#endif
