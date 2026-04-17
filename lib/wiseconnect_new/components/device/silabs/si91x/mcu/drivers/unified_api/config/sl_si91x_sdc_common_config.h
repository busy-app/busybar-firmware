/***************************************************************************/ /**
 * @file sl_si91x_sdc_common_config.h
 * @brief SDC common configuration file.
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

#ifndef SL_SI91X_SDC_COMMON_CONFIG_H
#define SL_SI91X_SDC_COMMON_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************** SDC Peripheral CommonConfiguration **********************/

// <<< Use Configuration Wizard in Context Menu >>>
// <h>SDC Peripheral Common Configuration

// <o SDC_NUMBER_OF_CHANNELS> Number of channels
// <1=> 1
// <2=> 2
// <4=> 4
// <i> Default: 1
#define SDC_NUMBER_OF_CHANNELS 1

// <o SL_SDC_SAMPLING_INTERVAL> Sampling Interval <1-1024>
// <i> Default: 1
#define SL_SDC_SAMPLING_INTERVAL 100

// <o SL_SDC_SAMPLING_THRESHOLD> Sampling Threshold <1-15>
// <i> Default: 15
#define SL_SDC_SAMPLING_THRESHOLD 15

// <q SL_SDC_MILLI_SEC_ENABLE> Enable Milli seconds as Interval
// <i> Default: 1
#define SL_SDC_MILLI_SEC_ENABLE 1

// </h>
// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif // SL_SDC
#endif /* SL_SI91X_SDC_COMMON_CONFIG_H */
