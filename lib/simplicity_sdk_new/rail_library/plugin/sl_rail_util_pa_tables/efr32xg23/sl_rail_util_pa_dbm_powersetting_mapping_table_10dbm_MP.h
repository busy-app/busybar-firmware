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

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (44U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (110U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-320)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x4000 /* est_deci-dBm:-320 act_deci-dBm:-315*/, \
    0x4000 /* est_deci-dBm:-310 act_deci-dBm:-315*/, \
    0x4000 /* est_deci-dBm:-300 act_deci-dBm:-315*/, \
    0x4000 /* est_deci-dBm:-290 act_deci-dBm:-315*/, \
    0x4000 /* est_deci-dBm:-280 act_deci-dBm:-315*/, \
    0x4000 /* est_deci-dBm:-270 act_deci-dBm:-315*/, \
    0x4000 /* est_deci-dBm:-260 act_deci-dBm:-315*/, \
    0x4002 /* est_deci-dBm:-250 act_deci-dBm:-254*/, \
    0x4002 /* est_deci-dBm:-240 act_deci-dBm:-254*/, \
    0x4002 /* est_deci-dBm:-230 act_deci-dBm:-254*/, \
    0x4002 /* est_deci-dBm:-220 act_deci-dBm:-254*/, \
    0x4003 /* est_deci-dBm:-210 act_deci-dBm:-218*/, \
    0x4003 /* est_deci-dBm:-200 act_deci-dBm:-218*/, \
    0x4004 /* est_deci-dBm:-190 act_deci-dBm:-192*/, \
    0x4004 /* est_deci-dBm:-180 act_deci-dBm:-192*/, \
    0x4004 /* est_deci-dBm:-170 act_deci-dBm:-192*/, \
    0x4005 /* est_deci-dBm:-160 act_deci-dBm:-167*/, \
    0x4006 /* est_deci-dBm:-150 act_deci-dBm:-152*/, \
    0x4006 /* est_deci-dBm:-140 act_deci-dBm:-152*/, \
    0x4007 /* est_deci-dBm:-130 act_deci-dBm:-138*/, \
    0x4008 /* est_deci-dBm:-120 act_deci-dBm:-127*/, \
    0x4009 /* est_deci-dBm:-110 act_deci-dBm:-113*/, \
    0x400a /* est_deci-dBm:-100 act_deci-dBm:-105*/, \
    0x400c /* est_deci-dBm:-90 act_deci-dBm:-90*/,   \
    0x400d /* est_deci-dBm:-80 act_deci-dBm:-82*/,   \
    0x400f /* est_deci-dBm:-70 act_deci-dBm:-70*/,   \
    0x4010 /* est_deci-dBm:-60 act_deci-dBm:-65*/,   \
    0x4012 /* est_deci-dBm:-50 act_deci-dBm:-53*/,   \
    0x4015 /* est_deci-dBm:-40 act_deci-dBm:-40*/,   \
    0x4018 /* est_deci-dBm:-30 act_deci-dBm:-30*/,   \
    0x401b /* est_deci-dBm:-20 act_deci-dBm:-20*/,   \
    0x401e /* est_deci-dBm:-10 act_deci-dBm:-12*/,   \
    0x4023 /* est_deci-dBm:0 act_deci-dBm:0*/,       \
    0x4028 /* est_deci-dBm:10 act_deci-dBm:9*/,      \
    0x402e /* est_deci-dBm:20 act_deci-dBm:20*/,     \
    0x4034 /* est_deci-dBm:30 act_deci-dBm:29*/,     \
    0x403d /* est_deci-dBm:40 act_deci-dBm:40*/,     \
    0x4047 /* est_deci-dBm:50 act_deci-dBm:50*/,     \
    0x4053 /* est_deci-dBm:60 act_deci-dBm:60*/,     \
    0x4062 /* est_deci-dBm:70 act_deci-dBm:70*/,     \
    0x4078 /* est_deci-dBm:80 act_deci-dBm:80*/,     \
    0x4095 /* est_deci-dBm:90 act_deci-dBm:90*/,     \
    0x40c8 /* est_deci-dBm:100 act_deci-dBm:100*/,   \
    0x40ec /* est_deci-dBm:110 act_deci-dBm:106*/,   \
  }

#ifdef __cplusplus
}
#endif

#endif
