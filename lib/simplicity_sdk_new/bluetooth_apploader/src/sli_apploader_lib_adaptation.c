/***************************************************************************//**
 * @brief System adaptation layer used internally by Bluetooth AppLoader
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

#include "sli_apploader_lib_adaptation.h"
#include <em_device.h>
#include <em_cmu.h>

void sli_apploader_set_interrupt_priorities()
{
  NVIC_SetPriority(FRC_PRI_IRQn, 1);
  NVIC_SetPriority(FRC_IRQn, 1);
  NVIC_SetPriority(RAC_SEQ_IRQn, 1);
  NVIC_SetPriority(RAC_RSM_IRQn, 1);
  NVIC_SetPriority(MODEM_IRQn, 1);
  NVIC_SetPriority(BUFC_IRQn, 1);
  NVIC_SetPriority(PROTIMER_IRQn, 1);
}
