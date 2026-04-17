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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG24_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG24_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (55U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (200U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-338)

#define SL_RAIL_PA_TABLE_0                        \
  {                                               \
    0x0 /* est_deci-dBm:-338 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-328 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-318 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-308 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-298 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-288 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-278 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-268 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-258 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-248 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-238 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-228 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-218 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-208 act_deci-dBm:-338*/, \
    0x0 /* est_deci-dBm:-198 act_deci-dBm:-338*/, \
    0x1 /* est_deci-dBm:-188 act_deci-dBm:-192*/, \
    0x1 /* est_deci-dBm:-178 act_deci-dBm:-192*/, \
    0x1 /* est_deci-dBm:-168 act_deci-dBm:-192*/, \
    0x1 /* est_deci-dBm:-158 act_deci-dBm:-192*/, \
    0x1 /* est_deci-dBm:-148 act_deci-dBm:-192*/, \
    0x1 /* est_deci-dBm:-138 act_deci-dBm:-192*/, \
    0x2 /* est_deci-dBm:-128 act_deci-dBm:-134*/, \
    0x2 /* est_deci-dBm:-118 act_deci-dBm:-134*/, \
    0x2 /* est_deci-dBm:-108 act_deci-dBm:-134*/, \
    0x3 /* est_deci-dBm:-98 act_deci-dBm:-99*/,   \
    0x3 /* est_deci-dBm:-88 act_deci-dBm:-99*/,   \
    0x3 /* est_deci-dBm:-78 act_deci-dBm:-99*/,   \
    0x4 /* est_deci-dBm:-68 act_deci-dBm:-75*/,   \
    0x4 /* est_deci-dBm:-58 act_deci-dBm:-75*/,   \
    0x5 /* est_deci-dBm:-48 act_deci-dBm:-57*/,   \
    0x6 /* est_deci-dBm:-38 act_deci-dBm:-42*/,   \
    0x7 /* est_deci-dBm:-28 act_deci-dBm:-29*/,   \
    0x8 /* est_deci-dBm:-18 act_deci-dBm:-19*/,   \
    0x9 /* est_deci-dBm:-8 act_deci-dBm:-10*/,    \
    0xa /* est_deci-dBm:2 act_deci-dBm:-1*/,      \
    0xb /* est_deci-dBm:12 act_deci-dBm:6*/,      \
    0xd /* est_deci-dBm:22 act_deci-dBm:19*/,     \
    0xf /* est_deci-dBm:32 act_deci-dBm:30*/,     \
    0x11 /* est_deci-dBm:42 act_deci-dBm:41*/,    \
    0x13 /* est_deci-dBm:52 act_deci-dBm:51*/,    \
    0x15 /* est_deci-dBm:62 act_deci-dBm:60*/,    \
    0x18 /* est_deci-dBm:72 act_deci-dBm:71*/,    \
    0x1b /* est_deci-dBm:82 act_deci-dBm:80*/,    \
    0x1f /* est_deci-dBm:92 act_deci-dBm:91*/,    \
    0x23 /* est_deci-dBm:102 act_deci-dBm:101*/,  \
    0x28 /* est_deci-dBm:112 act_deci-dBm:112*/,  \
    0x2e /* est_deci-dBm:122 act_deci-dBm:122*/,  \
    0x34 /* est_deci-dBm:132 act_deci-dBm:132*/,  \
    0x3b /* est_deci-dBm:142 act_deci-dBm:142*/,  \
    0x44 /* est_deci-dBm:152 act_deci-dBm:152*/,  \
    0x4e /* est_deci-dBm:162 act_deci-dBm:162*/,  \
    0x5c /* est_deci-dBm:172 act_deci-dBm:172*/,  \
    0x6d /* est_deci-dBm:182 act_deci-dBm:182*/,  \
    0x86 /* est_deci-dBm:192 act_deci-dBm:192*/,  \
    0xa5 /* est_deci-dBm:200 act_deci-dBm:200*/,  \
  }

#ifdef __cplusplus
}
#endif

#endif
