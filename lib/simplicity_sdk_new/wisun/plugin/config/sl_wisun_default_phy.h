/**************************************************************************//**
* @file sl_wisun_default_phy.h
* @brief Default PHY selection per radio board
*******************************************************************************
* # License
* <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WISUN_DEFAULT_PHY_H
#define SL_WISUN_DEFAULT_PHY_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HARDWARE_BOARD_DEFAULT_RF_BAND_915)
#define SL_WISUN_DEFAULT_PHY_CONFIG_TYPE  SL_WISUN_PHY_CONFIG_FAN11
#define SL_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define SL_WISUN_DEFAULT_OPERATING_CLASS 1
#define SL_WISUN_DEFAULT_OPERATING_MODE 0x1b
#define SL_WISUN_DEFAULT_CHAN_PLAN_ID 1
#define SL_WISUN_DEFAULT_PHY_MODE_ID 2
#elif defined(HARDWARE_BOARD_DEFAULT_RF_BAND_868)
#define SL_WISUN_DEFAULT_PHY_CONFIG_TYPE  SL_WISUN_PHY_CONFIG_FAN11
#define SL_WISUN_DEFAULT_REGULATORY_DOMAIN 3 // EU
#define SL_WISUN_DEFAULT_OPERATING_CLASS 1
#define SL_WISUN_DEFAULT_OPERATING_MODE 0x1a
#define SL_WISUN_DEFAULT_CHAN_PLAN_ID 32
#define SL_WISUN_DEFAULT_PHY_MODE_ID 1
#elif defined(HARDWARE_BOARD_DEFAULT_RF_BAND_470)
#define SL_WISUN_DEFAULT_PHY_CONFIG_TYPE  SL_WISUN_PHY_CONFIG_FAN11
#define SL_WISUN_DEFAULT_REGULATORY_DOMAIN 4 // CN
#define SL_WISUN_DEFAULT_OPERATING_CLASS 1
#define SL_WISUN_DEFAULT_OPERATING_MODE 0x1b
#define SL_WISUN_DEFAULT_CHAN_PLAN_ID 160
#define SL_WISUN_DEFAULT_PHY_MODE_ID 2
#else
#error Unsupported radio board
#endif

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_DEFAULT_PHY_H
