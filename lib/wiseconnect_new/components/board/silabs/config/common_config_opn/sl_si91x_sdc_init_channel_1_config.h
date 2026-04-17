/***************************************************************************/ /**
 * @file sl_si91x_sdc_init_channel_1_config.h
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

#ifndef SL_SI91X_SDC_CHANNEL_1_CONFIG_H
#define SL_SI91X_SDC_CHANNEL_1_CONFIG_H
#include "sl_si91x_sdc.h"
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************* SDC Configuration **************************/

// <<< Use Configuration Wizard in Context Menu >>>
// <h>SDC ADC Configuration

// <o SL_SDC_CHANNEL_1_INPUT_TYPE> Input Type
//   <SL_SDC_SINGLE_ENDED_MODE=>  Single ended
//   <SL_SDC_DIFFERENTIAL_MODE=> Differential
// <i> Selection of the SDC input type.
#define SL_SDC_CHANNEL_1_INPUT_TYPE SL_SDC_SINGLE_ENDED_MODE

// <q SL_SDC_CHANNEL_1_AUTO_BUFFER_RESET_ENABLE> Auto Buffer Reset Enable
// <i> Default: 1
#define SL_SDC_CHANNEL_1_AUTO_BUFFER_RESET_ENABLE 1

// </h>
// <<< end of configuration section >>>
#if USER_CONFIGURATION_ENABLE
// <<< sl:start pin_tool >>>
// <sdc_ch1 signal=P,(N)> SL_SDC_CH1
// $[SDC_CH1_SL_SDC_CH1]

#ifndef SL_SDC_CH1_PERIPHERAL
#define SL_SDC_CH1_PERIPHERAL SDC_CH1
#endif

// SDC_CH1 P on ULP_GPIO_1/GPIO_65
#ifndef SL_SDC_CH1_P_PORT
#define SL_SDC_CH1_P_PORT ULP
#endif
#ifndef SL_SDC_CH1_P_PIN
#define SL_SDC_CH1_P_PIN 8
#endif
#ifndef SL_SDC_CH1_P_LOC
#define SL_SDC_CH1_P_LOC 0
#endif

// [SDC_CH1_SL_SDC_CH1]$
// <<< sl:end pin_tool >>>

#if (!defined(SL_SDC_CH1_N_PORT) && (SL_SDC_CHANNEL_1_INPUT_TYPE == SL_SDC_DIFFERENTIAL_MODE))
#warning "Channel_1's SDC_N pin is not set up for a differential input type"
#endif

#else

#warning \
  "SDC Channel 1 peripheral pins are not configured. To configure, either install [ENABLE USER CONFIGURATION] component or define USER_CONFIGURATION_ENABLE macro to 1, then configure the pins as per the Custom board."

#endif // USER_CONFIGURATION_ENABLE

// Positive Input Channel Selection
#ifdef SL_SDC_CH1_P_PIN
#define SL_SDC_CHANNEL_1_POS_INPUT_CHNL_SEL \
  ((SL_SDC_CH1_P_PIN == 0)    ? 0           \
   : (SL_SDC_CH1_P_PIN == 2)  ? 1           \
   : (SL_SDC_CH1_P_PIN == 4)  ? 2           \
   : (SL_SDC_CH1_P_PIN == 6)  ? 3           \
   : (SL_SDC_CH1_P_PIN == 8)  ? 4           \
   : (SL_SDC_CH1_P_PIN == 10) ? 5           \
   : (SL_SDC_CH1_P_PIN == 25) ? 6           \
   : (SL_SDC_CH1_P_PIN == 27) ? 7           \
   : (SL_SDC_CH1_P_PIN == 29) ? 8           \
   : (SL_SDC_CH1_P_PIN == 1)  ? 10          \
   : (SL_SDC_CH1_P_PIN == 3)  ? 11          \
   : (SL_SDC_CH1_P_PIN == 5)  ? 12          \
   : (SL_SDC_CH1_P_PIN == 11) ? 13          \
   : (SL_SDC_CH1_P_PIN == 9)  ? 14          \
   : (SL_SDC_CH1_P_PIN == 7)  ? 15          \
   : (SL_SDC_CH1_P_PIN == 26) ? 16          \
   : (SL_SDC_CH1_P_PIN == 28) ? 17          \
   : (SL_SDC_CH1_P_PIN == 30) ? 18          \
                              : -1)
#else
#define SL_SDC_CHANNEL_1_POS_INPUT_CHNL_SEL 4
#endif
// Negative Input Channel Selection
#ifdef SL_SDC_CH1_N_PIN
#define SL_SDC_CHANNEL_1_NEG_INPUT_CHNL_SEL \
  ((SL_SDC_CH1_N_PIN == 1)    ? 0           \
   : (SL_SDC_CH1_N_PIN == 3)  ? 1           \
   : (SL_SDC_CH1_N_PIN == 5)  ? 2           \
   : (SL_SDC_CH1_N_PIN == 11) ? 3           \
   : (SL_SDC_CH1_N_PIN == 9)  ? 4           \
   : (SL_SDC_CH1_N_PIN == 7)  ? 5           \
   : (SL_SDC_CH1_N_PIN == 26) ? 6           \
   : (SL_SDC_CH1_N_PIN == 28) ? 7           \
   : (SL_SDC_CH1_N_PIN == 30) ? 8           \
                              : -1)
#else
#define SL_SDC_CHANNEL_1_NEG_INPUT_CHNL_SEL 0
#endif

#ifdef __cplusplus
}
#endif // SL_SDC
#endif /* SL_SI91X_SDC_CHANNEL_1_CONFIG_H */