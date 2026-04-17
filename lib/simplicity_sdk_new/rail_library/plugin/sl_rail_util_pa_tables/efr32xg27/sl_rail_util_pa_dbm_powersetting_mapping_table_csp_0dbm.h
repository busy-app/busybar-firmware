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

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (57U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (5U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (0U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-277)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x8000 /* est_deci-dBm:-277 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-272 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-267 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-262 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-257 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-252 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-247 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-242 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-237 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-232 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-227 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-222 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-217 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-212 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-207 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-202 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-197 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-192 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-187 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-182 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-177 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-172 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-167 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-162 act_deci-dBm:-277*/, \
    0x8001 /* est_deci-dBm:-157 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-152 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-147 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-142 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-137 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-132 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-127 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-122 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-117 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-112 act_deci-dBm:-161*/, \
    0x8002 /* est_deci-dBm:-107 act_deci-dBm:-108*/, \
    0x8002 /* est_deci-dBm:-102 act_deci-dBm:-108*/, \
    0x8002 /* est_deci-dBm:-97 act_deci-dBm:-108*/,  \
    0x8002 /* est_deci-dBm:-92 act_deci-dBm:-108*/,  \
    0x8002 /* est_deci-dBm:-87 act_deci-dBm:-108*/,  \
    0x8002 /* est_deci-dBm:-82 act_deci-dBm:-108*/,  \
    0x8003 /* est_deci-dBm:-77 act_deci-dBm:-78*/,   \
    0x8003 /* est_deci-dBm:-72 act_deci-dBm:-78*/,   \
    0x8003 /* est_deci-dBm:-67 act_deci-dBm:-78*/,   \
    0x8003 /* est_deci-dBm:-62 act_deci-dBm:-78*/,   \
    0x8004 /* est_deci-dBm:-57 act_deci-dBm:-58*/,   \
    0x8004 /* est_deci-dBm:-52 act_deci-dBm:-58*/,   \
    0x8004 /* est_deci-dBm:-47 act_deci-dBm:-58*/,   \
    0x8005 /* est_deci-dBm:-42 act_deci-dBm:-43*/,   \
    0x8005 /* est_deci-dBm:-37 act_deci-dBm:-43*/,   \
    0x8006 /* est_deci-dBm:-32 act_deci-dBm:-33*/,   \
    0x8006 /* est_deci-dBm:-27 act_deci-dBm:-33*/,   \
    0x8007 /* est_deci-dBm:-22 act_deci-dBm:-24*/,   \
    0x8008 /* est_deci-dBm:-17 act_deci-dBm:-17*/,   \
    0x8009 /* est_deci-dBm:-12 act_deci-dBm:-12*/,   \
    0x800a /* est_deci-dBm:-7 act_deci-dBm:-7*/,     \
    0x800b /* est_deci-dBm:-2 act_deci-dBm:-4*/,     \
    0x800c /* est_deci-dBm:0 act_deci-dBm:-1*/,      \
  }

#ifdef __cplusplus
}
#endif

#endif
