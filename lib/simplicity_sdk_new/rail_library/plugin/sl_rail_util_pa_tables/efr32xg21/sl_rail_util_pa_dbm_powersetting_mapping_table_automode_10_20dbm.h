/***************************************************************************//**
 * @file
 * @brief PA power conversion curves used by Silicon Labs PA power conversion
 *   functions.
 * @details This file contains the curves needed convert PA power levels to
 *   dBm powers.
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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG21_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG21_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (41U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (200U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-192)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x4001 /* est_deci-dBm:-192 act_deci-dBm:-192*/, \
    0x4001 /* est_deci-dBm:-182 act_deci-dBm:-192*/, \
    0x4001 /* est_deci-dBm:-172 act_deci-dBm:-192*/, \
    0x4001 /* est_deci-dBm:-162 act_deci-dBm:-192*/, \
    0x4001 /* est_deci-dBm:-152 act_deci-dBm:-192*/, \
    0x4001 /* est_deci-dBm:-142 act_deci-dBm:-192*/, \
    0x4002 /* est_deci-dBm:-132 act_deci-dBm:-134*/, \
    0x4002 /* est_deci-dBm:-122 act_deci-dBm:-134*/, \
    0x4002 /* est_deci-dBm:-112 act_deci-dBm:-134*/, \
    0x4002 /* est_deci-dBm:-102 act_deci-dBm:-134*/, \
    0x4003 /* est_deci-dBm:-92 act_deci-dBm:-100*/,  \
    0x4003 /* est_deci-dBm:-82 act_deci-dBm:-100*/,  \
    0x4004 /* est_deci-dBm:-72 act_deci-dBm:-76*/,   \
    0x4004 /* est_deci-dBm:-62 act_deci-dBm:-76*/,   \
    0x4005 /* est_deci-dBm:-52 act_deci-dBm:-57*/,   \
    0x4006 /* est_deci-dBm:-42 act_deci-dBm:-42*/,   \
    0x4006 /* est_deci-dBm:-32 act_deci-dBm:-42*/,   \
    0x4007 /* est_deci-dBm:-22 act_deci-dBm:-29*/,   \
    0x4008 /* est_deci-dBm:-12 act_deci-dBm:-19*/,   \
    0x4009 /* est_deci-dBm:-2 act_deci-dBm:-9*/,     \
    0x400b /* est_deci-dBm:8 act_deci-dBm:6*/,       \
    0x400c /* est_deci-dBm:18 act_deci-dBm:13*/,     \
    0x400e /* est_deci-dBm:28 act_deci-dBm:25*/,     \
    0x4010 /* est_deci-dBm:38 act_deci-dBm:34*/,     \
    0x4013 /* est_deci-dBm:48 act_deci-dBm:46*/,     \
    0x4017 /* est_deci-dBm:58 act_deci-dBm:58*/,     \
    0x401b /* est_deci-dBm:68 act_deci-dBm:67*/,     \
    0x4021 /* est_deci-dBm:78 act_deci-dBm:77*/,     \
    0x402b /* est_deci-dBm:88 act_deci-dBm:88*/,     \
    0x403c /* est_deci-dBm:98 act_deci-dBm:98*/,     \
    0x27 /* est_deci-dBm:108 act_deci-dBm:107*/,     \
    0x2c /* est_deci-dBm:118 act_deci-dBm:117*/,     \
    0x32 /* est_deci-dBm:128 act_deci-dBm:127*/,     \
    0x3a /* est_deci-dBm:138 act_deci-dBm:138*/,     \
    0x40 /* est_deci-dBm:148 act_deci-dBm:147*/,     \
    0x4c /* est_deci-dBm:158 act_deci-dBm:158*/,     \
    0x58 /* est_deci-dBm:168 act_deci-dBm:168*/,     \
    0x67 /* est_deci-dBm:178 act_deci-dBm:178*/,     \
    0x7c /* est_deci-dBm:188 act_deci-dBm:188*/,     \
    0x9b /* est_deci-dBm:198 act_deci-dBm:198*/,     \
    0xa6 /* est_deci-dBm:208 act_deci-dBm:200*/,     \
  }

#ifdef __cplusplus
}
#endif

#endif
