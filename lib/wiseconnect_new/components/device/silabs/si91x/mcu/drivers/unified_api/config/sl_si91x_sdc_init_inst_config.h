/***************************************************************************/ /**
 * @file sl_si91x_sdc_init_inst_config.h
 * @brief SDC configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_SDC_INSTANCE_CONFIG_H
#define SL_SI91X_SDC_INSTANCE_CONFIG_H
#include "sl_si91x_sdc.h"
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************* ADC Configuration **************************/

// <<< Use Configuration Wizard in Context Menu >>>
// <h>SDC ADC Configuration

// <o SL_SDC_INSTANCE_INPUT_TYPE> Input Type
//   <SL_SDC_SINGLE_ENDED_MODE=>  Single ended
//   <SL_SDC_DIFFERENTIAL_MODE=> Differential
// <i> Selection of the SDC input type.
#define SL_SDC_INSTANCE_INPUT_TYPE SL_SDC_SINGLE_ENDED_MODE

// <q SL_SDC_INSTANCE_AUTO_BUFFER_RESET_ENABLE> Auto Buffer Reset Enable
// <i> Default: 1
#define SL_SDC_INSTANCE_AUTO_BUFFER_RESET_ENABLE 1

// </h>
// <<< end of configuration section >>>

#define SL_SDC_INSTANCE_POS_INPUT_CHNL_SEL 4
#define SL_SDC_INSTANCE_NEG_INPUT_CHNL_SEL 0

#ifdef __cplusplus
}
#endif // SL_SDC
#endif /* SL_SI91X_SDC_INSTANCE_CONFIG_H */
