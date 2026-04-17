/***************************************************************************/ /**
 * @file sl_si91x_adc_common_config.h
 * @brief ADC common configuration file.
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

#ifndef SL_SI91X_ADC_COMMON_CONFIG_H
#define SL_SI91X_ADC_COMMON_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************** ADC Peripheral CommonConfiguration **********************/

// <<< Use Configuration Wizard in Context Menu >>>
// <h>ADC Peripheral Common Configuration

// <o NUMBER_OF_CHANNEL> Number of channel(s) <1-16>
// <i> Default: 1
#define NUMBER_OF_CHANNEL 1

// <o SL_ADC_OPERATION_MODE> ADC operation mode
//   <SL_ADC_FIFO_MODE=> FIFO mode
//   <SL_ADC_STATIC_MODE=> Static mode
// <i> Selection of ADC operation mode.
#define SL_ADC_OPERATION_MODE SL_ADC_FIFO_MODE

// </h>
// <<< end of configuration section >>>

sl_adc_config_t sl_adc_config = {
  .operation_mode        = SL_ADC_OPERATION_MODE,
  .num_of_channel_enable = NUMBER_OF_CHANNEL,
};

#ifdef __cplusplus
}
#endif // SL_ADC
#endif /* SL_SI91X_ADC_COMMON_CONFIG_H */
