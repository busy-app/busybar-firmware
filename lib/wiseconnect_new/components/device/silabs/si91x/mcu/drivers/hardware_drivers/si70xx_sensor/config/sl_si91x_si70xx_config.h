/***************************************************************************/ /**
 * @file sl_si91x_si70xx_config.h
 * @brief SI70XX Driver User Config
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

#ifndef SL_SI91X_SI70XX_CONFIG_H
#define SL_SI91X_SI70XX_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>

#define SI7006_ADDR 0x40 //I2C device address for Si7006
#define SI7013_ADDR 0x41 //I2C device address for Si7013
#define SI7020_ADDR 0X40 //I2C device address for Si7020
#define SI7021_ADDR 0x40 //I2C device address for Si7021

// <h>Si70xx Slave Address
// <o SL_SI70XX_CONFIG_SLAVE_ADDR> Si70xx Slave
//   <SI7006_ADDR=>  SI7006 SLAVE ADDRESS   (0x40)
//   <SI7013_ADDR=>  SI7013 SLAVE ADDRESS   (0x41)
//   <SI7020_ADDR=>  SI7020 SLAVE ADDRESS   (0X40)
//   <SI7021_ADDR=>  SI7021 SLAVE ADDRESS   (0x40)
#define SL_SI70XX_CONFIG_SLAVE_ADDR SI7021_ADDR
// </h>

// <h>I2C Instance for Si70xx Sensor
// <o SL_SI70XX_CONFIG_I2C_INSTANCE> I2C Instance (make sure to install selected I2C instance)
//   <SL_I2C0=>  I2C0
//   <SL_I2C1=>  I2C1
//   <SL_ULP_I2C=>  I2C2 (ULP_I2C)
#define SL_SI70XX_CONFIG_I2C_INSTANCE SL_ULP_I2C
// </h>

// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_SI70XX_CONFIG_H
