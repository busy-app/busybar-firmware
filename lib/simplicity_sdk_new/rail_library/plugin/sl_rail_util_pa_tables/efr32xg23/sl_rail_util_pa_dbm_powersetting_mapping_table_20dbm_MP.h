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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG23_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG23_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (50U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (130U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-360)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x4000 /* est_deci-dBm:-360 act_deci-dBm:-354*/, \
    0x4000 /* est_deci-dBm:-350 act_deci-dBm:-354*/, \
    0x4000 /* est_deci-dBm:-340 act_deci-dBm:-354*/, \
    0x4000 /* est_deci-dBm:-330 act_deci-dBm:-354*/, \
    0x4000 /* est_deci-dBm:-320 act_deci-dBm:-354*/, \
    0x4000 /* est_deci-dBm:-310 act_deci-dBm:-354*/, \
    0x4000 /* est_deci-dBm:-300 act_deci-dBm:-354*/, \
    0x4002 /* est_deci-dBm:-290 act_deci-dBm:-299*/, \
    0x4002 /* est_deci-dBm:-280 act_deci-dBm:-299*/, \
    0x4002 /* est_deci-dBm:-270 act_deci-dBm:-299*/, \
    0x4002 /* est_deci-dBm:-260 act_deci-dBm:-299*/, \
    0x4003 /* est_deci-dBm:-250 act_deci-dBm:-258*/, \
    0x4003 /* est_deci-dBm:-240 act_deci-dBm:-258*/, \
    0x4004 /* est_deci-dBm:-230 act_deci-dBm:-235*/, \
    0x4004 /* est_deci-dBm:-220 act_deci-dBm:-235*/, \
    0x4004 /* est_deci-dBm:-210 act_deci-dBm:-235*/, \
    0x4005 /* est_deci-dBm:-200 act_deci-dBm:-206*/, \
    0x4006 /* est_deci-dBm:-190 act_deci-dBm:-193*/, \
    0x4006 /* est_deci-dBm:-180 act_deci-dBm:-193*/, \
    0x4007 /* est_deci-dBm:-170 act_deci-dBm:-177*/, \
    0x4008 /* est_deci-dBm:-160 act_deci-dBm:-167*/, \
    0x4009 /* est_deci-dBm:-150 act_deci-dBm:-151*/, \
    0x400a /* est_deci-dBm:-140 act_deci-dBm:-144*/, \
    0x400c /* est_deci-dBm:-130 act_deci-dBm:-130*/, \
    0x400d /* est_deci-dBm:-120 act_deci-dBm:-120*/, \
    0x400e /* est_deci-dBm:-110 act_deci-dBm:-115*/, \
    0x4010 /* est_deci-dBm:-100 act_deci-dBm:-103*/, \
    0x4011 /* est_deci-dBm:-90 act_deci-dBm:-92*/,   \
    0x4014 /* est_deci-dBm:-80 act_deci-dBm:-80*/,   \
    0x4016 /* est_deci-dBm:-70 act_deci-dBm:-72*/,   \
    0x4018 /* est_deci-dBm:-60 act_deci-dBm:-65*/,   \
    0x401c /* est_deci-dBm:-50 act_deci-dBm:-51*/,   \
    0x4020 /* est_deci-dBm:-40 act_deci-dBm:-40*/,   \
    0x4024 /* est_deci-dBm:-30 act_deci-dBm:-31*/,   \
    0x4028 /* est_deci-dBm:-20 act_deci-dBm:-22*/,   \
    0x402e /* est_deci-dBm:-10 act_deci-dBm:-10*/,   \
    0x4033 /* est_deci-dBm:0 act_deci-dBm:-1*/,      \
    0x403a /* est_deci-dBm:10 act_deci-dBm:10*/,     \
    0x4042 /* est_deci-dBm:20 act_deci-dBm:20*/,     \
    0x404a /* est_deci-dBm:30 act_deci-dBm:30*/,     \
    0x4054 /* est_deci-dBm:40 act_deci-dBm:40*/,     \
    0x405e /* est_deci-dBm:50 act_deci-dBm:50*/,     \
    0x406a /* est_deci-dBm:60 act_deci-dBm:60*/,     \
    0x4078 /* est_deci-dBm:70 act_deci-dBm:70*/,     \
    0x4088 /* est_deci-dBm:80 act_deci-dBm:80*/,     \
    0x409b /* est_deci-dBm:90 act_deci-dBm:90*/,     \
    0x40af /* est_deci-dBm:100 act_deci-dBm:100*/,   \
    0x40c7 /* est_deci-dBm:110 act_deci-dBm:110*/,   \
    0x40e0 /* est_deci-dBm:120 act_deci-dBm:120*/,   \
    0x40ee /* est_deci-dBm:130 act_deci-dBm:129*/,   \
  }

#ifdef __cplusplus
}
#endif

#endif
