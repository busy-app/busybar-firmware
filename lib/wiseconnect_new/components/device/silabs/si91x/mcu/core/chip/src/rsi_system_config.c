/*******************************************************************************
* @file  rsi_system_config.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "rsi_ccp_user_config.h"
#ifndef SLI_SI91X_MCU_ENABLE_IPMU_APIS
#define SLI_SI91X_MCU_ENABLE_IPMU_APIS
#endif

#ifdef CHIP_9118
#include "iPMU_prog/iPMU_dotc/rsi_system_config.c"
#endif

#ifndef SL_SI91X_DISABLE_LOWPWR_RET_LDO
#define V1P20_IPMU_CHANGES
#endif

#ifdef SLI_SI917
#include "iPMU_prog/iPMU_dotc/rsi_system_config_917.c"
#endif
#include "iPMU_prog/iPMU_dotc/ipmu_apis.c"
//EOF
