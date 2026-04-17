/***************************************************************************//**
 * @file
 * @brief DEVICE_INIT_DCDC Config
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

#ifndef SL_DEVICE_INIT_DCDC_CONFIG_H
#define SL_DEVICE_INIT_DCDC_CONFIG_H

#include "sl_device_init_dcdc.h"
#include "em_device.h"

// DC/DC Type
#define SL_DEVICE_INIT_DCDC_TYPE         SL_DEVICE_INIT_DCDC_TYPE_BOOST

// <<< Use Configuration Wizard in Context Menu >>>

// <h> DC/DC Boost Settings

// <q SL_DEVICE_INIT_DCDC_ENABLE> Enable DC/DC Converter
// <i>
// <i> Default: 1
#define SL_DEVICE_INIT_DCDC_ENABLE         1

// <o SL_DEVICE_INIT_DCDC_BOOST_OUTPUT> DC/DC Boost Output Voltage Settings
// <emuDcdcBoostOutputVoltage_1v8=> 1.8 V
// <emuDcdcBoostOutputVoltage_1v9=> 1.9 V
// <emuDcdcBoostOutputVoltage_2v0=> 2.0 V
// <emuDcdcBoostOutputVoltage_2v1=> 2.1 V
// <emuDcdcBoostOutputVoltage_2v2=> 2.2 V
// <emuDcdcBoostOutputVoltage_2v3=> 2.3 V
// <emuDcdcBoostOutputVoltage_2v4=> 2.4 V
// <i> Default: emuDcdcBoostOutputVoltage_1v8
#define SL_DEVICE_INIT_DCDC_BOOST_OUTPUT        emuDcdcBoostOutputVoltage_1v8

// </h>

// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_DCDC_CONFIG_H
