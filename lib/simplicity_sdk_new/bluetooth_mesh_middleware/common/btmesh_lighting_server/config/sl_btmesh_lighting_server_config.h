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
#ifndef SL_BTMESH_LIGHTING_SERVER_CONFIG_H
#define SL_BTMESH_LIGHTING_SERVER_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Light Lightness Server configuration

// <o SL_BTMESH_LIGHTING_SERVER_NVM_SAVE_TIME_CFG_VAL> Timeout [ms] for saving States of the model to NVM.
// <i> Default: 5000
// <i> Timeout [ms] for saving States of the model to NVM.
#define SL_BTMESH_LIGHTING_SERVER_NVM_SAVE_TIME_CFG_VAL   (5000)

// <o SL_BTMESH_LIGHTING_SERVER_PS_KEY_CFG_VAL> PS Key for NVM Page where the States of the Lighting Model are saved.
// <i> Default: 0x4004
// <i> PS Key for NVM Page where the States of the Lighting Model are saved.
#define SL_BTMESH_LIGHTING_SERVER_PS_KEY_CFG_VAL   (0x4004)

// <o SL_BTMESH_LIGHTING_SERVER_PWM_UPDATE_PERIOD_CFG_VAL> Periodicity [ms] for updating the PWM duty cycle during a transition.
// <i> Default: 10
// <i> Periodicity [ms] for updating the PWM duty cycle during a transition.
#define SL_BTMESH_LIGHTING_SERVER_PWM_UPDATE_PERIOD_CFG_VAL   (10)

// <o SL_BTMESH_LIGHTING_SERVER_UI_UPDATE_PERIOD_CFG_VAL> for updating the UI with lightness level during a transition.
// <i> Default: 100
// <i> Periodicity [ms] for updating the UI with lightness level during a transition.
#define SL_BTMESH_LIGHTING_SERVER_UI_UPDATE_PERIOD_CFG_VAL   (100)

// <o SL_BTMESH_LIGHTING_SERVER_PWM_MINIMUM_BRIGHTNESS_CFG_VAL> Timer value for minimum PWM duty cycle.
// <i> Default: 1
// <i> Timer value for minimum PWM duty cycle.
#define SL_BTMESH_LIGHTING_SERVER_PWM_MINIMUM_BRIGHTNESS_CFG_VAL   (1)

// <o SL_BTMESH_LIGHTING_SERVER_PWM_MAXIMUM_BRIGHTNESS_CFG_VAL> Timer value for maximum PWM duty cycle.
// <i> Default: 0xFFFE
// <i> Timer value for minimum PWM duty cycle.
#define SL_BTMESH_LIGHTING_SERVER_PWM_MAXIMUM_BRIGHTNESS_CFG_VAL   (0xFFFE)

// <h> Lightness Range

// <o SL_BTMESH_LIGHTING_SERVER_LIGHTNESS_MIN_CFG_VAL> Minimum lightness value <0x0001-0xFFFF>
// <i> Determines the minimum non-zero lightness an element is configured to emit.
// <i> Default: 0x0001
#define SL_BTMESH_LIGHTING_SERVER_LIGHTNESS_MIN_CFG_VAL   0x0001

// <o SL_BTMESH_LIGHTING_SERVER_LIGHTNESS_MAX_CFG_VAL> Maximum lightness value <0x0001-0xFFFF>
// <i> Determines the maximum lightness an element is configured to emit.
// <i> The value of the Light Lightness Range Max state shall be greater than
// <i> or equal to the value of the Light Lightness Range Min state.
// <i> Default: 0xFFFF
#define SL_BTMESH_LIGHTING_SERVER_LIGHTNESS_MAX_CFG_VAL   0xFFFF

//</h>

// <e SL_BTMESH_LIGHTING_SERVER_LOGGING_CFG_VAL> Enable Logging
// <i> Default: 1
// <i> Enable / disable Logging for Lighting Server model specific messages for this component.
#define SL_BTMESH_LIGHTING_SERVER_LOGGING_CFG_VAL   (1)

// <q SL_BTMESH_LIGHTING_SERVER_DEBUG_PRINTS_FOR_STATE_CHANGE_EVENTS_CFG_VAL> Enable debug prints for each server state changed event.
// <i> Default: 0
// <i> Enable debug prints for each server state changed event.
#define SL_BTMESH_LIGHTING_SERVER_DEBUG_PRINTS_FOR_STATE_CHANGE_EVENTS_CFG_VAL   (0)

// </e>

// </h>

// <h> Lighting Server Metadata Settings

// <o SL_BTMESH_METADATA_LIGHT_PURPOSE_1> Light Purpose metadata
//   <0x0000=> Uplight
//   <0x0001=> UplightLeft
//   <0x0002=> UplightCenter
//   <0x0003=> UplightRight
//   <0x0004=> Downlight
//   <0x0005=> DownlightLeft
//   <0x0006=> DownlightCenter
//   <0x0007=> DownlightRight
//   <0x0008=> Inside
//   <0x0009=> Outside
//   <0x000A=> Backlight
//   <0x000B=> Floodlight
//   <0x000C=> Tasklight
//   <0x000D=> TasklightLeft
//   <0x000E=> TasklightCenter
//   <0x000F=> TasklightRight
//   <0x0010=> WarmingLight
//   <0x0011=> EmergencyLight
//   <0x0012=> NightLight
//   <0x0013=> IndicatorLight
//   <0x0014=> UndercabinetLight
//   <0x0015=> AccentLight
//   <0x0016=> StripLight
//   <0x0017=> TrofferLight
//   <0x0018=> HighBayLight
//   <0x0019=> WallPackLight
// <i> This metadata indicates the purpose of the current light source.
// <i> It must be one of the Bluetooth assigned numbers for Light Purpose.
// <i> Default: 0x0000

#define SL_BTMESH_METADATA_LIGHT_PURPOSE_1   0x0000

// <o SL_BTMESH_METADATA_LIGHT_LIGHTNESS_RANGE_1> Light Lightness Range metadata - Minimum Lightness value <0x0001-0xFFFF>
// <i> Determines the minimum non-zero lightness the physical device is capable to emit.
// <i> This metadata is based on the physical capabilities of the device and
// <i> restricts the Light Lightness Setup Server Lightness Range minimum value.
// <i> Default: 0x0001
#define SL_BTMESH_METADATA_LIGHT_LIGHTNESS_RANGE_1   0x0001

// <o SL_BTMESH_METADATA_LIGHT_LIGHTNESS_RANGE_2> Light Lightness Range metadata - Maximum Lightness value <0x0001-0xFFFF>
// <i> Determines the maximum non-zero lightness the physical device is capable to emit.
// <i> This metadata is based on the physical capabilities of the device and
// <i> restricts the Light Lightness Setup Server Lightness Range maximum value.
// <i> Default: 0xFFFF
#define SL_BTMESH_METADATA_LIGHT_LIGHTNESS_RANGE_2   0xFFFF

// </h>
// <<< end of configuration section >>>

// Models Metadata additional info
#define SL_BTMESH_METADATA_LIGHT_PURPOSE_ID           0x0002
#define SL_BTMESH_METADATA_LIGHT_LIGHTNESS_RANGE_ID   0x0003

// The PWM update period shall not be greater than the UI update period
#if (SL_BTMESH_LIGHTING_SERVER_UI_UPDATE_PERIOD_CFG_VAL) < (SL_BTMESH_LIGHTING_SERVER_PWM_UPDATE_PERIOD_CFG_VAL)
#error "The SL_BTMESH_LIGHTING_SERVER_PWM_UPDATE_PERIOD_CFG_VAL shall be less than SL_BTMESH_LIGHTING_SERVER_UI_UPDATE_PERIOD_CFG_VAL."
#endif

// Lightness maximum value cannot be less than minimum value
#if (SL_BTMESH_LIGHTING_SERVER_LIGHTNESS_MAX_CFG_VAL) < (SL_BTMESH_LIGHTING_SERVER_LIGHTNESS_MIN_CFG_VAL)
#error The value of the Lightness Range Max shall be greater than or equal to \
  the value of the Lightness Range Min state.
#endif // (SL_BTMESH_LIGHTING_SERVER_LIGHTNESS_MAX_CFG_VAL) < (SL_BTMESH_LIGHTING_SERVER_LIGHTNESS_MIN_CFG_VAL)

#endif // SL_BTMESH_LIGHTING_SERVER_CONFIG_H
