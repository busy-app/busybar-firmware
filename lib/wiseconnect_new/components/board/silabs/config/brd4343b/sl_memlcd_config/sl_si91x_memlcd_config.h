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

// <<< sl:start pin_tool >>>
// <ulp_ssi signal=MOSI_,SCK_,CS1_> SL_MEMLCD
// $[ULP_SSI_SL_MEMLCD]
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

// ULP_SSI CS1_ on ULP_GPIO_4/GPIO_68
#ifndef SL_MEMLCD_CS1__PORT
#define SL_MEMLCD_CS1__PORT ULP
#endif
#ifndef SL_MEMLCD_CS1__PIN
#define SL_MEMLCD_CS1__PIN 4
#endif
#ifndef SL_MEMLCD_CS1_LOC
#define SL_MEMLCD_CS1_LOC 10
#endif
// [ULP_SSI_SL_MEMLCD]$
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
#define SL_MEMLCD_DISP_EXT_COMIN_PORT ULP
#endif
#ifndef SL_MEMLCD_DISP_EXT_COMIN_PIN
#define SL_MEMLCD_DISP_EXT_COMIN_PIN 5
#endif
// [GPIO_SL_MEMLCD_DISP_EXT_COMIN]$
// <<< sl:end pin_tool >>>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_MEMLCD_CONFIG_H