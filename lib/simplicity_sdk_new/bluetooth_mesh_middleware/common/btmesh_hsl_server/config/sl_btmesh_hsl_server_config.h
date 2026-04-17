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
#ifndef SL_BTMESH_HSL_SERVER_CONFIG_H
#define SL_BTMESH_HSL_SERVER_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> HSL Server configuration

// <o SL_BTMESH_HSL_SERVER_NVM_SAVE_TIME_CFG_VAL> Timeout [ms] for saving States of the model to NVM.
// <i> Default: 5000
// <i> Timeout [ms] for saving States of the model to NVM.
#define SL_BTMESH_HSL_SERVER_NVM_SAVE_TIME_CFG_VAL   (5000)

// <o SL_BTMESH_HSL_SERVER_PS_KEY_CFG_VAL> PS Key for NVM Page where the States of the HSL Models are saved.
// <i> Default: 0x4008
// <i> PS Key for NVM Page where the States of the HSL Models are saved.
#define SL_BTMESH_HSL_SERVER_PS_KEY_CFG_VAL   (0x4008)

// <o SL_BTMESH_HSL_SERVER_HUE_UPDATE_PERIOD_CFG_VAL> Periodicity [ms] for updating the hue during a transition.
// <i> Default: 10
// <i> Periodicity [ms] for updating the hue during a transition.
#define SL_BTMESH_HSL_SERVER_HUE_UPDATE_PERIOD_CFG_VAL   (10)

// <o SL_BTMESH_HSL_SERVER_SATURATION_UPDATE_PERIOD_CFG_VAL> Periodicity [ms] for updating the saturation during a transition.
// <i> Default: 10
// <i> Periodicity [ms] for updating the saturation during a transition.
#define SL_BTMESH_HSL_SERVER_SATURATION_UPDATE_PERIOD_CFG_VAL   (10)

// <o SL_BTMESH_HSL_SERVER_HUE_UI_UPDATE_PERIOD_CFG_VAL> Periodicity [ms] for updating the UI with hue during a transition.
// <i> Default: 100
// <i> Periodicity [ms] for updating the hue values on the UI.
#define SL_BTMESH_HSL_SERVER_HUE_UI_UPDATE_PERIOD_CFG_VAL   (100)

// <o SL_BTMESH_HSL_SERVER_SATURATION_UI_UPDATE_PERIOD_CFG_VAL> Periodicity [ms] for updating the UI with saturation during a transition.
// <i> Default: 100
// <i> Periodicity [ms] for updating the saturation values on the UI.
#define SL_BTMESH_HSL_SERVER_SATURATION_UI_UPDATE_PERIOD_CFG_VAL   (100)

// <o SL_BTMESH_HSL_SERVER_DEFAULT_HUE_CFG_VAL> Default Hue
// <i> Default: 0
// <i> Default Hue value.
#define SL_BTMESH_HSL_SERVER_DEFAULT_HUE_CFG_VAL   (0)

// <o SL_BTMESH_HSL_SERVER_DEFAULT_SATURATION_CFG_VAL> Default Saturation
// <i> Default: 0
// <i> Default Saturation.
#define SL_BTMESH_HSL_SERVER_DEFAULT_SATURATION_CFG_VAL   (0)

// <o SL_BTMESH_HSL_SERVER_MINIMUM_HUE_CFG_VAL> Minimum Hue
// <i> Default: 0
// <i> Minimum Hue.
#define SL_BTMESH_HSL_SERVER_MINIMUM_HUE_CFG_VAL   (0)

// <o SL_BTMESH_HSL_SERVER_MAXIMUM_HUE_CFG_VAL> Maximum Hue
// <i> Default: 65535
// <i> Maximum Hue.
#define SL_BTMESH_HSL_SERVER_MAXIMUM_HUE_CFG_VAL   (65535)

// <o SL_BTMESH_HSL_SERVER_MINIMUM_SATURATION_CFG_VAL> Minimum Saturation
// <i> Default: 0
// <i> Minimum Saturation.
#define SL_BTMESH_HSL_SERVER_MINIMUM_SATURATION_CFG_VAL   (0)

// <o SL_BTMESH_HSL_SERVER_MAXIMUM_SATURATION_CFG_VAL> Maximum Saturation
// <i> Default: 65535
// <i> Maximum Saturation.
#define SL_BTMESH_HSL_SERVER_MAXIMUM_SATURATION_CFG_VAL   (65535)

// <e SL_BTMESH_HSL_SERVER_LOGGING_CFG_VAL> Enable Logging
// <i> Default: 1
// <i> Enable / disable UART Logging for HSL Server models specific messages for this component.
#define SL_BTMESH_HSL_SERVER_LOGGING_CFG_VAL   (1)

// </e>

// </h>

// <h> Light HSL Server Metadata Settings

// <o SL_BTMESH_METADATA_LIGHT_HSL_HUE_RANGE_1> Light HSL Hue Range metadata - Minimum Hue value <0x0000-0xFFFF>
// <i> Determines the minimum non-zero hue value the physical device is capable to emit.
// <i> This metadata is based on the physical capabilities of the device and
// <i> restricts the Light HSL Setup Server Hue Range minimum value.
// <i> Default: 0x0000
#define SL_BTMESH_METADATA_LIGHT_HSL_HUE_RANGE_1   0x0000

// <o SL_BTMESH_METADATA_LIGHT_HSL_HUE_RANGE_2> Light HSL Hue Range metadata - Maximum Hue value <0x0000-0xFFFF>
// <i> Determines the maximum non-zero hue value the physical device is capable to emit.
// <i> This metadata is based on the physical capabilities of the device and
// <i> restricts the Light HSL Setup Server Hue Range maximum value.
// <i> Default: 0xFFFF
#define SL_BTMESH_METADATA_LIGHT_HSL_HUE_RANGE_2   0xFFFF

// <o SL_BTMESH_METADATA_LIGHT_HSL_SATURATION_RANGE_1> Light HSL Saturation Range metadata - Minimum Saturation value <0x0000-0xFFFF>
// <i> Determines the minimum non-zero saturation value the physical device is capable to emit.
// <i> This metadata is based on the physical capabilities of the device and
// <i> restricts the Light HSL Setup Server Saturation Range minimum value.
// <i> Default: 0x0000
#define SL_BTMESH_METADATA_LIGHT_HSL_SATURATION_RANGE_1   0x0000

// <o SL_BTMESH_METADATA_LIGHT_HSL_SATURATION_RANGE_2> Light HSL Saturation Range metadata - Maximum Saturation value <0x0000-0xFFFF>
// <i> Determines the maximum non-zero saturation value the physical device is capable to emit.
// <i> This metadata is based on the physical capabilities of the device and
// <i> restricts the Light HSL Setup Server Saturation Range maximum value.
// <i> Default: 0xFFFF
#define SL_BTMESH_METADATA_LIGHT_HSL_SATURATION_RANGE_2   0xFFFF

// </h>

// <<< end of configuration section >>>

// Models Metadata additional info
#define SL_BTMESH_METADATA_LIGHT_HSL_HUE_RANGE_ID          0x0005
#define SL_BTMESH_METADATA_LIGHT_HSL_HUE_RANGE_LEN         0x02
#define SL_BTMESH_METADATA_LIGHT_HSL_SATURATION_RANGE_ID   0x0006
#define SL_BTMESH_METADATA_LIGHT_HSL_SATURATION_RANGE_LEN  0x02

// The hue update period shall not be greater than the hue UI update period
#if (SL_BTMESH_HSL_SERVER_HUE_UI_UPDATE_PERIOD_CFG_VAL) < (SL_BTMESH_HSL_SERVER_HUE_UPDATE_PERIOD_CFG_VAL)
#error "The SL_BTMESH_HSL_SERVER_HUE_UPDATE_PERIOD_CFG_VAL shall be less than SL_BTMESH_HSL_SERVER_HUE_UI_UPDATE_PERIOD_CFG_VAL."
#endif

// The saturation update period shall not be greater than the saturation UI update period
#if (SL_BTMESH_HSL_SERVER_SATURATION_UI_UPDATE_PERIOD_CFG_VAL) < (SL_BTMESH_HSL_SERVER_SATURATION_UPDATE_PERIOD_CFG_VAL)
#error "The SL_BTMESH_HSL_SERVER_SATURATION_UPDATE_PERIOD_CFG_VAL shall be less than SL_BTMESH_HSL_SERVER_SATURATION_UI_UPDATE_PERIOD_CFG_VAL."
#endif

#endif // SL_BTMESH_HSL_SERVER_CONFIG_H
