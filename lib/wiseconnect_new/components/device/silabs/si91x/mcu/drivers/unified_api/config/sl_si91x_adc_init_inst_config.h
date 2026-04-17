/***************************************************************************/ /**
 * @file sl_si91x_adc_init_inst_config.h
 * @brief ADC configuration file.
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

#ifndef SL_SI91X_ADC_INSTANCE_CONFIG_H
#define SL_SI91X_ADC_INSTANCE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************* ADC Configuration **************************/

/* NOTE: User should configure all macros defined below, while creating an instance other than pre-defined one */

#if USER_CONFIGURATION_ENABLE

// <<< Use Configuration Wizard in Context Menu >>>
// <h>ADC Configuration

// <o SL_ADC_INSTANCE_INPUT_TYPE> Input Type
//   <SL_ADC_SINGLE_ENDED=>  Single ended
//   <SL_ADC_DIFFERENTIAL=> Differential
// <i> Selection of the ADC input type.
#define SL_ADC_INSTANCE_INPUT_TYPE SL_ADC_SINGLE_ENDED

// <o SL_ADC_INSTANCE_SAMPLING_RATE> Sampling Rate <1-10000000>
// <i> Default: 100000
#define SL_ADC_INSTANCE_SAMPLING_RATE 100000

// <o SL_ADC_INSTANCE_SAMPLE_LENGTH> Sample Length <1-1023>
// <i> Default: 1023
#define SL_ADC_INSTANCE_SAMPLE_LENGTH 1023

// </h>
// <<< end of configuration section >>>

#define SL_ADC_INSTANCE_POS_INPUT_CHNL_SEL 4
#define SL_ADC_INSTANCE_NEG_INPUT_CHNL_SEL 0

#else

#warning \
  "The sl_si91x_adc_init_INSTANCE_config.h file requires specifying the INSTANCE name for custom ADC configurations. To properly configure ADC either enable user configuration by installing the [ENABLE USER CONFIGURATION] component or set USER_CONFIGURATION_ENABLE to 1, and then define all required MACROS in sl_si91x_adc_init_INSTANCE_config.h according to your board specifications."

#endif // USER_CONFIGURATION_ENABLE

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_ADC_INSTANCE_CONFIG_H */
