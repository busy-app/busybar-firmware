/***************************************************************************/ /**
 * @file sl_si91x_veml6035_config.h
 * @brief VEML6035 Driver User Config
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

#ifndef SL_SI91X_VEML6035_CONFIG_H
#define SL_SI91X_VEML6035_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>
// <h> VEML6035 Slave Address : 0x29
// </h>
#define VEML6035_SLAVE_ADDR 0x29

// <h>I2C Instance for VEML6035 Sensor
// <o SL_VEML6035_CONFIG_I2C_INSTANCE> I2C Instance (make sure to install selected I2C instance)
//   <SL_I2C0=>  I2C0
//   <SL_I2C1=>  I2C1
//   <SL_ULP_I2C=>  I2C2 (ULP_I2C)
#define SL_VEML6035_CONFIG_I2C_INSTANCE SL_ULP_I2C
// </h>
// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_VEML6035_CONFIG_H
