/*
 * Copyright (c) 2023 Silicon Laboratories Inc. (www.silabs.com)
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this software
 * is governed by the terms of the Silicon Labs Master Software License Agreement (MSLA)
 * available at www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software is distributed to you in Object Code format and/or Source Code format and
 * is governed by the sections of the MSLA applicable to Object Code, Source Code and
 * Modified Open Source Code. By using this software, you agree to the terms of the MSLA.
 *
 * This software is a modified version of the ARM/Pelion Wi-SUN FAN software stack which is
 * licensed under Apache 2.0 (see below). Modifications to the ARM/Pelion Wi-SUN software stack
 * within this software are subject to the above copyright notice and licensed pursuant to the MSLA.
 *
 * The original ARM/Pelion Wi-SUN FAN software stack is subject to the following copyright notice.
 *
 * Copyright (c) 2014-2018, Pelion and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SL_WISUN_BR_LFN_PARAMS_API_H
#define SL_WISUN_BR_LFN_PARAMS_API_H

#include <stdint.h>
#include "sl_wisun_common.h"
#include "sl_common.h"
#include "sl_status.h"

/// API version used to check compatibility (do not edit when using this header)
#define SL_WISUN_BR_LFN_PARAMS_API_VERSION 0x0002

/**************************************************************************//**
 * @addtogroup SL_WISUN_TYPES
 * @{
 *****************************************************************************/

/// BR LFN parenting parameter set
SL_PACK_START(1)
typedef struct {
    /**
   * Version of this API, must be the first field.
   * This field allows to store the parameters in an NVM and check on reload
   * that they are compatible with the stack if there was an update.
   */
  uint32_t version;
  /// Time between LFN Broadcast windows (millisecond)
  /// Specification range [10000, 600000] (10s to 10m)
  uint32_t lfn_broadcast_interval_ms;
  /// Number of LFN Broadcast Intervals between broadcast sync messages
  /// Specification range [1, 60]
  uint8_t lfn_broadcast_sync_period;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_br_lfn_params_t;
SL_PACK_END()

/**************************************************************************//**
 * @addtogroup SL_WISUN_BR_LFN_PARAMETER_SETS Predefined BR LFN-parenting parameter sets
 *
 * Predefined LFN parameter sets for sl_wisun_br_set_lfn_parameters().
 *
 * These parameter sets can be used either as-is or used as an initialization value
 * for an application-specific parameter set.
 *
 * @{
 *****************************************************************************/

/// Profile for use during tests
static const sl_wisun_br_lfn_params_t SL_WISUN_BR_PARAMS_LFN_TEST = {
  .version = SL_WISUN_BR_LFN_PARAMS_API_VERSION,
  .lfn_broadcast_interval_ms = SEC_TO_MS(10),
  .lfn_broadcast_sync_period = 5,
  .reserved = { 0 }
};

/// Profile providing balance between battery life and performance
static const sl_wisun_br_lfn_params_t SL_WISUN_BR_PARAMS_LFN_BALANCED = {
  .version = SL_WISUN_BR_LFN_PARAMS_API_VERSION,
  .lfn_broadcast_interval_ms = SEC_TO_MS(60),
  .lfn_broadcast_sync_period = 15,
  .reserved = { 0 }
};

/// Profile with a higher battery life
static const sl_wisun_br_lfn_params_t SL_WISUN_BR_PARAMS_LFN_ECO = {
  .version = SL_WISUN_BR_LFN_PARAMS_API_VERSION,
  .lfn_broadcast_interval_ms = SEC_TO_MS(600),
  .lfn_broadcast_sync_period = 60,
  .reserved = { 0 }
};

/** @} */

/** @} (end SL_WISUN_TYPES) */

#endif
