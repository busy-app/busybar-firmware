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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG26_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG26_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (55U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (200U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-334)

#define SL_RAIL_PA_TABLE_0                        \
  {                                               \
    0x0 /* est_deci-dBm:-334 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-324 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-314 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-304 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-294 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-284 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-274 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-264 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-254 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-244 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-234 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-224 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-214 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-204 act_deci-dBm:-334*/, \
    0x0 /* est_deci-dBm:-194 act_deci-dBm:-334*/, \
    0x1 /* est_deci-dBm:-184 act_deci-dBm:-191*/, \
    0x1 /* est_deci-dBm:-174 act_deci-dBm:-191*/, \
    0x1 /* est_deci-dBm:-164 act_deci-dBm:-191*/, \
    0x1 /* est_deci-dBm:-154 act_deci-dBm:-191*/, \
    0x1 /* est_deci-dBm:-144 act_deci-dBm:-191*/, \
    0x1 /* est_deci-dBm:-134 act_deci-dBm:-191*/, \
    0x2 /* est_deci-dBm:-124 act_deci-dBm:-133*/, \
    0x2 /* est_deci-dBm:-114 act_deci-dBm:-133*/, \
    0x2 /* est_deci-dBm:-104 act_deci-dBm:-133*/, \
    0x3 /* est_deci-dBm:-94 act_deci-dBm:-98*/,   \
    0x3 /* est_deci-dBm:-84 act_deci-dBm:-98*/,   \
    0x4 /* est_deci-dBm:-74 act_deci-dBm:-74*/,   \
    0x4 /* est_deci-dBm:-64 act_deci-dBm:-74*/,   \
    0x5 /* est_deci-dBm:-54 act_deci-dBm:-56*/,   \
    0x5 /* est_deci-dBm:-44 act_deci-dBm:-56*/,   \
    0x6 /* est_deci-dBm:-34 act_deci-dBm:-40*/,   \
    0x7 /* est_deci-dBm:-24 act_deci-dBm:-28*/,   \
    0x8 /* est_deci-dBm:-14 act_deci-dBm:-17*/,   \
    0x9 /* est_deci-dBm:-4 act_deci-dBm:-8*/,     \
    0xa /* est_deci-dBm:6 act_deci-dBm:1*/,       \
    0xc /* est_deci-dBm:16 act_deci-dBm:14*/,     \
    0xd /* est_deci-dBm:26 act_deci-dBm:21*/,     \
    0xf /* est_deci-dBm:36 act_deci-dBm:33*/,     \
    0x12 /* est_deci-dBm:46 act_deci-dBm:45*/,    \
    0x14 /* est_deci-dBm:56 act_deci-dBm:55*/,    \
    0x16 /* est_deci-dBm:66 act_deci-dBm:66*/,    \
    0x19 /* est_deci-dBm:76 act_deci-dBm:76*/,    \
    0x1c /* est_deci-dBm:86 act_deci-dBm:85*/,    \
    0x1f /* est_deci-dBm:96 act_deci-dBm:94*/,    \
    0x23 /* est_deci-dBm:106 act_deci-dBm:105*/,  \
    0x27 /* est_deci-dBm:116 act_deci-dBm:115*/,  \
    0x2d /* est_deci-dBm:126 act_deci-dBm:125*/,  \
    0x33 /* est_deci-dBm:136 act_deci-dBm:135*/,  \
    0x3d /* est_deci-dBm:146 act_deci-dBm:146*/,  \
    0x46 /* est_deci-dBm:156 act_deci-dBm:156*/,  \
    0x51 /* est_deci-dBm:166 act_deci-dBm:166*/,  \
    0x60 /* est_deci-dBm:176 act_deci-dBm:176*/,  \
    0x73 /* est_deci-dBm:186 act_deci-dBm:186*/,  \
    0x8e /* est_deci-dBm:196 act_deci-dBm:196*/,  \
    0x9e /* est_deci-dBm:200 act_deci-dBm:200*/,  \
  }

#ifdef __cplusplus
}
#endif

#endif
