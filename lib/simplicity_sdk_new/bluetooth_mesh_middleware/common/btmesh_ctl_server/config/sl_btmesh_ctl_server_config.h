/***************************************************************************//**
 * @file
 * @brief
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
#ifndef SL_BTMESH_CTL_SERVER_CONFIG_H
#define SL_BTMESH_CTL_SERVER_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> CTL Server configuration

// <o SL_BTMESH_CTL_SERVER_NVM_SAVE_TIME_CFG_VAL> Timeout [ms] for saving States of the model to NVM.
// <i> Default: 5000
// <i> Timeout [ms] for saving States of the model to NVM.
#define SL_BTMESH_CTL_SERVER_NVM_SAVE_TIME_CFG_VAL   (5000)

// <o SL_BTMESH_CTL_SERVER_PS_KEY_CFG_VAL> PS Key for NVM Page where the States of the Lighting Model are saved.
// <i> Default: 0x4005
// <i> PS Key for NVM Page where the States of the Lighting Model are saved.
#define SL_BTMESH_CTL_SERVER_PS_KEY_CFG_VAL   (0x4005)

// <o SL_BTMESH_CTL_SERVER_PWM_UPDATE_PERIOD_CFG_VAL> Periodicity [ms] for updating the PWM duty cycle during a transition.
// <i> Default: 10
// <i> Periodicity [ms] for updating the PWM duty cycle during a transition.
#define SL_BTMESH_CTL_SERVER_PWM_UPDATE_PERIOD_CFG_VAL   (10)

// <o SL_BTMESH_CTL_SERVER_UI_UPDATE_PERIOD_CFG_VAL> Periodicity [ms] for updating the UI with temperature & delta UV during a transition.
// <i> Default: 100
// <i> Periodicity [ms] for updating the temperature & delta UV values on the UI.
#define SL_BTMESH_CTL_SERVER_UI_UPDATE_PERIOD_CFG_VAL   (100)

// <o SL_BTMESH_CTL_SERVER_DEFAULT_TEMPERATURE_CFG_VAL> Default Color Temperature
// <i> Default: 6500
// <i> Default Color Temperature value.
#define SL_BTMESH_CTL_SERVER_DEFAULT_TEMPERATURE_CFG_VAL   (6500)

// <o SL_BTMESH_CTL_SERVER_DEFAULT_DELTAUV_CFG_VAL> Default Delta UV
// <i> Default: 0
// <i> Default Delta UV.
#define SL_BTMESH_CTL_SERVER_DEFAULT_DELTAUV_CFG_VAL   (0)

// <o SL_BTMESH_CTL_SERVER_MINIMUM_TEMPERATURE_CFG_VAL> Minimum Color Temperature
// <i> Default: 800
// <i> Minimum Color Temperature.
#define SL_BTMESH_CTL_SERVER_MINIMUM_TEMPERATURE_CFG_VAL   (800)

// <o SL_BTMESH_CTL_SERVER_MAXIMUM_TEMPERATURE_CFG_VAL> Maximum Color Temperature
// <i> Default: 20000
// <i> Maximum Color Temperature.
#define SL_BTMESH_CTL_SERVER_MAXIMUM_TEMPERATURE_CFG_VAL   (20000)

// <e SL_BTMESH_CTL_SERVER_LOGGING_CFG_VAL> Enable Logging
// <i> Default: 1
// <i> Enable / disable Logging for Lighting Server model specific messages for this component.
#define SL_BTMESH_CTL_SERVER_LOGGING_CFG_VAL   (1)

// </e>

// </h>

// <h> Light CTL Server Metadata Settings

// <o SL_BTMESH_METADATA_LIGHT_CTL_TEMP_RANGE_1> Light CTL Temperature Range metadata - Minimum Temperature value <0x0001-0xFFFF>
// <i> Determines the minimum non-zero light temperature the physical device is capable to emit.
// <i> This metadata is based on the physical capabilities of the device and
// <i> restricts the Light CTL Setup Server Temperature Range minimum value.
// <i> Default: 800
#define SL_BTMESH_METADATA_LIGHT_CTL_TEMP_RANGE_1   800

// <o SL_BTMESH_METADATA_LIGHT_CTL_TEMP_RANGE_2> Light CTL Temperature Range metadata - Maximum Temperature value <0x0001-0xFFFF>
// <i> Determines the maximum non-zero light temperature the physical device is capable to emit.
// <i> This metadata is based on the physical capabilities of the device and
// <i> restricts the Light CTL Setup Server Temperature Range maximum value.
// <i> Default: 20000
#define SL_BTMESH_METADATA_LIGHT_CTL_TEMP_RANGE_2   20000

// </h>

// <<< end of configuration section >>>

// Models Metadata additional info
#define SL_BTMESH_METADATA_LIGHT_CTL_TEMP_RANGE_ID          0x0004

// The PWM update period shall not be greater than the UI update period
#if (SL_BTMESH_CTL_SERVER_UI_UPDATE_PERIOD_CFG_VAL) < (SL_BTMESH_CTL_SERVER_PWM_UPDATE_PERIOD_CFG_VAL)
#error "The SL_BTMESH_CTL_SERVER_PWM_UPDATE_PERIOD_CFG_VAL shall be less than SL_BTMESH_CTL_SERVER_UI_UPDATE_PERIOD_CFG_VAL."
#endif

#endif // SL_BTMESH_CTL_SERVER_CONFIG_H
