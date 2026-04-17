/***************************************************************************/ /**
* @file sl_si91x_memlcd_config.h
* @brief I2c driver instance configuration file.
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_MEMLCD_CONFIG_H
#define SL_SI91X_MEMLCD_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#if USER_CONFIGURATION_ENABLE
// <<< sl:start pin_tool >>>
// <ssi signal=MOSI_,SCK_,CS0_> SL_MEMLCD
// $[SSI_SL_MEMLCD]
#ifndef SL_MEMLCD_PERIPHERAL
#define SL_MEMLCD_PERIPHERAL ULP_SSI
#endif

// ULP_SSI MOSI_ on ULP_GPIO_1/GPIO_65
#ifndef SL_MEMLCD_MOSI__PORT
#define SL_MEMLCD_MOSI__PORT ULP
#endif
#ifndef SL_MEMLCD_MOSI__PIN
#define SL_MEMLCD_MOSI__PIN 1
#endif
#ifndef SL_MEMLCD_MOSI_LOC
#define SL_MEMLCD_MOSI_LOC 0
#endif

// ULP_SSI SCK_ on ULP_GPIO_8/GPIO_72
#ifndef SL_MEMLCD_SCK__PORT
#define SL_MEMLCD_SCK__PORT ULP
#endif
#ifndef SL_MEMLCD_SCK__PIN
#define SL_MEMLCD_SCK__PIN 8
#endif
#ifndef SL_MEMLCD_SCK_LOC
#define SL_MEMLCD_SCK_LOC 7
#endif

// ULP_SSI CS0_ on ULP_GPIO_10/GPIO_74
#ifndef SL_MEMLCD_CS0__PORT
#define SL_MEMLCD_CS0__PORT ULP
#endif
#ifndef SL_MEMLCD_CS0__PIN
#define SL_MEMLCD_CS0__PIN 10
#endif
#ifndef SL_MEMLCD_CS0_LOC
#define SL_MEMLCD_CS0_LOC 9
#endif
// [SSI_SL_MEMLCD]$
// <<< sl:end pin_tool >>>

// <<< sl:start pin_tool >>>
// <gpio> SL_MEMLCD_DISP_ENABLE
// $[GPIO_SL_MEMLCD_DISP_ENABLE]
#ifndef SL_MEMLCD_DISP_ENABLE_PORT
#define SL_MEMLCD_DISP_ENABLE_PORT UULP_VBAT
#endif
#ifndef SL_MEMLCD_DISP_ENABLE_PIN
#define SL_MEMLCD_DISP_ENABLE_PIN 0
#endif
// [GPIO_SL_MEMLCD_DISP_ENABLE]$
// <<< sl:end pin_tool >>>

// <<< sl:start pin_tool >>>
// <gpio> SL_MEMLCD_DISP_EXT_COMIN
// $[GPIO_SL_MEMLCD_DISP_EXT_COMIN]
#ifndef SL_MEMLCD_DISP_EXT_COMIN_PORT
#define SL_MEMLCD_DISP_EXT_COMIN_PORT UULP_VBAT
#endif
#ifndef SL_MEMLCD_DISP_EXT_COMIN_PIN
#define SL_MEMLCD_DISP_EXT_COMIN_PIN 3
#endif
// [GPIO_SL_MEMLCD_DISP_EXT_COMIN]$
// <<< sl:end pin_tool >>>

#else

#warning \
  "Memory LCD instances not defined for this OPN or SoC project. To configure board connections, either install [ENABLE USER CONFIGURATION] component or set USER_CONFIGURATION_ENABLE MACRO to 1, then define the required macros in this header file according to your board connections."

#endif // USER_CONFIGURATION_ENABLE

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_MEMLCD_CONFIG_H