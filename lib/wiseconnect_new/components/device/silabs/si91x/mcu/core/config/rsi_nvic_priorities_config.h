/*******************************************************************************
* @file  rsi_nvic_priorities_config.h
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

#ifndef NVIC_PRIORITIES_CONFIG_H_
#define NVIC_PRIORITIES_CONFIG_H_

#include "FreeRTOSConfig.h"

#define DEFAULT_PRIORITY configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - configPRIO_BITS)

void sl_si91x_device_init_nvic();

#endif /* NVIC_PRIORITIES_CONFIG_H_ */
