/***************************************************************************/ /**
* @file sl_si91x_i2c_12c0_config.h
* @brief I2c driver configuration file.
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_I2C_I2C0_CONFIG_H
#define SL_SI91X_I2C_I2C0_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_i2c.h"
/******************************************************************************/
/******************************* I2C Configuration **************************/
// <h> I2C0 Configuration

// <o SL_I2C_I2C0_MODE> Mode
//   <SL_I2C_LEADER_MODE=> Leader mode
//   <SL_I2C_FOLLOWER_MODE=> Follower mode
// <i> Selection of the I2C Mode.
#define SL_I2C_I2C0_MODE SL_I2C_LEADER_MODE

// <o SL_I2C_I2C0_OPERATING_MODE> Operating Mode
//   <SL_I2C_STANDARD_MODE=> Standard mode
//   <SL_I2C_FAST_MODE=> Fast mode
//   <SL_I2C_FAST_PLUS_MODE=> Fast plus mode
//   <SL_I2C_HIGH_SPEED_MODE=> High speed mode
// <i> Selection of the I2C Mode.
#define SL_I2C_I2C0_OPERATING_MODE SL_I2C_STANDARD_MODE

// <q SL_I2C_I2C0_INIT_DMA> DMA
// <i> Default: 0
#define SL_I2C_I2C0_INIT_DMA 0

// </h> End I2C0 Configuration
/******************************************************************************/
// <<< end of configuration section >>>

// Previously, SL_I2C_I2C0_TRANSFER_TYPE was updated directly through UC. Now, it is updated indirectly through SL_I2C_I2C0_INIT_DMA.
#define SL_I2C_I2C0_TRANSFER_TYPE SL_I2C_I2C0_INIT_DMA

// <<< sl:start pin_tool >>>
// <i2c0 signal=SCL,SDA> SL_I2C0
// $[I2C0_SL_I2C0]
#ifndef SL_I2C0_PERIPHERAL
#define SL_I2C0_PERIPHERAL I2C0
#endif
#ifndef SL_I2C0_PERIPHERAL_NO
#define SL_I2C0_PERIPHERAL_NO 0
#endif

// I2C0 SCL on GPIO_7
#ifndef SL_I2C0_SCL_PORT
#define SL_I2C0_SCL_PORT HP
#endif
#ifndef SL_I2C0_SCL_PIN
#define SL_I2C0_SCL_PIN 7
#endif
#ifndef SL_I2C0_SCL_LOC
#define SL_I2C0_SCL_LOC 0
#endif

// I2C0 SDA on GPIO_6
#ifndef SL_I2C0_SDA_PORT
#define SL_I2C0_SDA_PORT HP
#endif
#ifndef SL_I2C0_SDA_PIN
#define SL_I2C0_SDA_PIN 6
#endif
#ifndef SL_I2C0_SDA_LOC
#define SL_I2C0_SDA_LOC 3
#endif
// [I2C0_SL_I2C0]$
// <<< sl:end pin_tool >>>

#define SL_I2C_I2C0_SCL_PORT SL_I2C0_SCL_PORT
#define SL_I2C_I2C0_SCL_PIN  SL_I2C0_SCL_PIN
#define SL_I2C_I2C0_SCL_MUX  SL_SI91X_I2C0_SCL_MUX
#define SL_I2C_I2C0_SCL_PAD  SL_SI91X_I2C0_SCL_PAD
#define SL_I2C_I2C0_SCL_REN  SL_SI91X_I2C0_SCL_REN

#define SL_I2C_I2C0_SDA_PORT SL_I2C0_SDA_PORT
#define SL_I2C_I2C0_SDA_PIN  SL_I2C0_SDA_PIN
#define SL_I2C_I2C0_SDA_MUX  SL_SI91X_I2C0_SDA_MUX
#define SL_I2C_I2C0_SDA_PAD  SL_SI91X_I2C0_SDA_PAD
#define SL_I2C_I2C0_SDA_REN  SL_SI91X_I2C0_SDA_REN

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_I2C_I2C0_CONFIG_H