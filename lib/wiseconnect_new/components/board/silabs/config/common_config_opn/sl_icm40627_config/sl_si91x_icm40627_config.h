/***************************************************************************/ /**
* @file sl_si91x_icm40627_config.h
* * @brief SSI Master/Slave API configuration
* *******************************************************************************
* * # License
* * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
* *******************************************************************************
* *
* * SPDX-License-Identifier: Zlib
* *
* * The licensor of this software is Silicon Laboratories Inc.
* *
* * This software is provided 'as-is', without any express or implied
* * warranty. In no event will the authors be held liable for any damages
* * arising from the use of this software.
* *
* * Permission is granted to anyone to use this software for any purpose,
* * including commercial applications, and to alter it and redistribute it
* * freely, subject to the following restrictions:
* *
* * 1. The origin of this software must not be misrepresented; you must not
* *    claim that you wrote the original software. If you use this software
* *    in a product, an acknowledgment in the product documentation would be
* *    appreciated but is not required.
* * 2. Altered source versions must be plainly marked as such, and must not be
* *    misrepresented as being the original software.
* * 3. This notice may not be removed or altered from any source distribution.
* *
* ******************************************************************************/

#ifndef SL_SI91X_ICM40627_CONFIG_H
#define SL_SI91X_ICM40627_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
#if USER_CONFIGURATION_ENABLE
// <<< sl:start pin_tool >>>
// <ulp_ssi signal=MOSI_,MISO_,SCK_,CS0_> SL_SI91X_ICM40627
// $[ULP_SSI_SL_SI91X_ICM40627]
#ifndef SL_SI91X_ICM40627_PERIPHERAL
#define SL_SI91X_ICM40627_PERIPHERAL ULP_SSI
#endif

// ULP_SSI MOSI_ on ULP_GPIO_1/GPIO_65
#ifndef SL_SI91X_ICM40627_MOSI__PORT
#define SL_SI91X_ICM40627_MOSI__PORT ULP
#endif
#ifndef SL_SI91X_ICM40627_MOSI__PIN
#define SL_SI91X_ICM40627_MOSI__PIN 1
#endif
#ifndef SL_SI91X_ICM40627_MOSI_LOC
#define SL_SI91X_ICM40627_MOSI_LOC 0
#endif

// ULP_SSI MISO_ on ULP_GPIO_2/GPIO_66
#ifndef SL_SI91X_ICM40627_MISO__PORT
#define SL_SI91X_ICM40627_MISO__PORT ULP
#endif
#ifndef SL_SI91X_ICM40627_MISO__PIN
#define SL_SI91X_ICM40627_MISO__PIN 2
#endif
#ifndef SL_SI91X_ICM40627_MISO_LOC
#define SL_SI91X_ICM40627_MISO_LOC 12
#endif

// ULP_SSI SCK_ on ULP_GPIO_8/GPIO_72
#ifndef SL_SI91X_ICM40627_SCK__PORT
#define SL_SI91X_ICM40627_SCK__PORT ULP
#endif
#ifndef SL_SI91X_ICM40627_SCK__PIN
#define SL_SI91X_ICM40627_SCK__PIN 8
#endif
#ifndef SL_SI91X_ICM40627_SCK_LOC
#define SL_SI91X_ICM40627_SCK_LOC 7
#endif

// ULP_SSI CS0_ on ULP_GPIO_10/GPIO_74
#ifndef SL_SI91X_ICM40627_CS0__PORT
#define SL_SI91X_ICM40627_CS0__PORT ULP
#endif
#ifndef SL_SI91X_ICM40627_CS0__PIN
#define SL_SI91X_ICM40627_CS0__PIN 10
#endif
#ifndef SL_SI91X_ICM40627_CS0_LOC
#define SL_SI91X_ICM40627_CS0_LOC 9
#endif
// [ULP_SSI_SL_SI91X_ICM40627]$

// <gpio> SL_SI91X_IMU_ALS_INT
// $[GPIO_SL_SI91X_IMU_ALS_INT]
#ifndef SL_SI91X_IMU_ALS_INT_PORT
#define SL_SI91X_IMU_ALS_INT_PORT UULP_VBAT
#endif
#ifndef SL_SI91X_IMU_ALS_INT_PIN
#define SL_SI91X_IMU_ALS_INT_PIN 3
#endif
// [GPIO_SL_SI91X_IMU_ALS_INT]$

// <<< sl:end pin_tool >>>
#else

#warning \
  "ICM40627 pins are not configured. To configure, either install [ENABLE USER CONFIGURATION] component or define USER_CONFIGURATION_ENABLE macro to 1, then configure the pins as per the Custom board."

#endif // USER_CONFIGURATION_ENABLE
#ifdef __cplusplus
}
#endif
#endif /* SL_SI91X_ICM40627_CONFIG_H */
