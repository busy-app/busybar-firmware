/***************************************************************************/ /**
 * @file sl_si91x_opamp_common_config.h
 * @brief OPAMP common configuration file.
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

#ifndef SL_SI91X_OPAMP_COMMON_CONFIG_H
#define SL_SI91X_OPAMP_COMMON_CONFIG_H

#include "sl_si91x_opamp.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************** OPAMP Common Configuration ******************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h>OPAMP Common Configuration

// <o OPAMP_REF_VOLT> OPAMP Reference Voltage
//   <2500=> 2.5 V
//   <3300=> 3.3 V
// <i> Selection of OPAMP Reference Voltage
#define OPAMP_REF_VOLT 3300

// <q INPUT_DAC_NEG_INPUT_EXTERNAL> DAC Enable
// <i> Default: 0
#define INPUT_DAC_NEG_INPUT_EXTERNAL 0

// </h>
// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif // SL_OPAMP
#endif /* SL_SI91X_OPAMP_COMMON_CONFIG_H */