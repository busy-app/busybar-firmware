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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG27_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG27_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (60U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (5U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (0U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-291)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x8000 /* est_deci-dBm:-291 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-286 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-281 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-276 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-271 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-266 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-261 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-256 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-251 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-246 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-241 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-236 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-231 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-226 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-221 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-216 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-211 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-206 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-201 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-196 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-191 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-186 act_deci-dBm:-291*/, \
    0x8000 /* est_deci-dBm:-181 act_deci-dBm:-291*/, \
    0x8001 /* est_deci-dBm:-176 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-171 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-166 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-161 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-156 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-151 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-146 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-141 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-136 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-131 act_deci-dBm:-178*/, \
    0x8001 /* est_deci-dBm:-126 act_deci-dBm:-178*/, \
    0x8002 /* est_deci-dBm:-121 act_deci-dBm:-125*/, \
    0x8002 /* est_deci-dBm:-116 act_deci-dBm:-125*/, \
    0x8002 /* est_deci-dBm:-111 act_deci-dBm:-125*/, \
    0x8002 /* est_deci-dBm:-106 act_deci-dBm:-125*/, \
    0x8002 /* est_deci-dBm:-101 act_deci-dBm:-125*/, \
    0x8002 /* est_deci-dBm:-96 act_deci-dBm:-125*/,  \
    0x8003 /* est_deci-dBm:-91 act_deci-dBm:-94*/,   \
    0x8003 /* est_deci-dBm:-86 act_deci-dBm:-94*/,   \
    0x8003 /* est_deci-dBm:-81 act_deci-dBm:-94*/,   \
    0x8003 /* est_deci-dBm:-76 act_deci-dBm:-94*/,   \
    0x8004 /* est_deci-dBm:-71 act_deci-dBm:-74*/,   \
    0x8004 /* est_deci-dBm:-66 act_deci-dBm:-74*/,   \
    0x8004 /* est_deci-dBm:-61 act_deci-dBm:-74*/,   \
    0x8005 /* est_deci-dBm:-56 act_deci-dBm:-60*/,   \
    0x8005 /* est_deci-dBm:-51 act_deci-dBm:-60*/,   \
    0x8006 /* est_deci-dBm:-46 act_deci-dBm:-49*/,   \
    0x8006 /* est_deci-dBm:-41 act_deci-dBm:-49*/,   \
    0x8007 /* est_deci-dBm:-36 act_deci-dBm:-40*/,   \
    0x8008 /* est_deci-dBm:-31 act_deci-dBm:-33*/,   \
    0x8009 /* est_deci-dBm:-26 act_deci-dBm:-28*/,   \
    0x800a /* est_deci-dBm:-21 act_deci-dBm:-23*/,   \
    0x800c /* est_deci-dBm:-16 act_deci-dBm:-16*/,   \
    0x800d /* est_deci-dBm:-11 act_deci-dBm:-12*/,   \
    0x800f /* est_deci-dBm:-6 act_deci-dBm:-8*/,     \
    0x800f /* est_deci-dBm:-1 act_deci-dBm:-8*/,     \
    0x800f /* est_deci-dBm:0 act_deci-dBm:-8*/,      \
  }

#ifdef __cplusplus
}
#endif

#endif
