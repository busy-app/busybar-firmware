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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG28_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG28_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_1_NUM_VALUES  (45U)
#define SL_RAIL_PA_TABLE_1_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_1_MAX_POWER_DDBM     (130U)
#define SL_RAIL_PA_TABLE_1_MIN_POWER_DDBM     (-310)
#define SL_RAIL_PA_TABLE_1                           \
  {                                                  \
    0x4000 /* est_deci-dBm:-310 act_deci-dBm:-303*/, \
    0x4000 /* est_deci-dBm:-300 act_deci-dBm:-303*/, \
    0x4000 /* est_deci-dBm:-290 act_deci-dBm:-303*/, \
    0x4000 /* est_deci-dBm:-280 act_deci-dBm:-303*/, \
    0x4000 /* est_deci-dBm:-270 act_deci-dBm:-303*/, \
    0x4002 /* est_deci-dBm:-260 act_deci-dBm:-262*/, \
    0x4002 /* est_deci-dBm:-250 act_deci-dBm:-262*/, \
    0x4002 /* est_deci-dBm:-240 act_deci-dBm:-262*/, \
    0x4003 /* est_deci-dBm:-230 act_deci-dBm:-239*/, \
    0x4003 /* est_deci-dBm:-220 act_deci-dBm:-239*/, \
    0x4004 /* est_deci-dBm:-210 act_deci-dBm:-210*/, \
    0x4004 /* est_deci-dBm:-200 act_deci-dBm:-210*/, \
    0x4005 /* est_deci-dBm:-190 act_deci-dBm:-196*/, \
    0x4006 /* est_deci-dBm:-180 act_deci-dBm:-180*/, \
    0x4007 /* est_deci-dBm:-170 act_deci-dBm:-170*/, \
    0x4007 /* est_deci-dBm:-160 act_deci-dBm:-170*/, \
    0x4008 /* est_deci-dBm:-150 act_deci-dBm:-154*/, \
    0x4009 /* est_deci-dBm:-140 act_deci-dBm:-147*/, \
    0x400b /* est_deci-dBm:-130 act_deci-dBm:-132*/, \
    0x400c /* est_deci-dBm:-120 act_deci-dBm:-123*/, \
    0x400e /* est_deci-dBm:-110 act_deci-dBm:-111*/, \
    0x400f /* est_deci-dBm:-100 act_deci-dBm:-106*/, \
    0x4011 /* est_deci-dBm:-90 act_deci-dBm:-93*/,   \
    0x4013 /* est_deci-dBm:-80 act_deci-dBm:-85*/,   \
    0x4016 /* est_deci-dBm:-70 act_deci-dBm:-72*/,   \
    0x4019 /* est_deci-dBm:-60 act_deci-dBm:-60*/,   \
    0x401c /* est_deci-dBm:-50 act_deci-dBm:-51*/,   \
    0x4020 /* est_deci-dBm:-40 act_deci-dBm:-41*/,   \
    0x4024 /* est_deci-dBm:-30 act_deci-dBm:-31*/,   \
    0x4029 /* est_deci-dBm:-20 act_deci-dBm:-20*/,   \
    0x402f /* est_deci-dBm:-10 act_deci-dBm:-10*/,   \
    0x4034 /* est_deci-dBm:0 act_deci-dBm:0*/,       \
    0x403a /* est_deci-dBm:10 act_deci-dBm:9*/,      \
    0x4043 /* est_deci-dBm:20 act_deci-dBm:20*/,     \
    0x404b /* est_deci-dBm:30 act_deci-dBm:30*/,     \
    0x4054 /* est_deci-dBm:40 act_deci-dBm:40*/,     \
    0x405f /* est_deci-dBm:50 act_deci-dBm:50*/,     \
    0x406b /* est_deci-dBm:60 act_deci-dBm:60*/,     \
    0x4079 /* est_deci-dBm:70 act_deci-dBm:70*/,     \
    0x4089 /* est_deci-dBm:80 act_deci-dBm:80*/,     \
    0x409b /* est_deci-dBm:90 act_deci-dBm:90*/,     \
    0x40af /* est_deci-dBm:100 act_deci-dBm:100*/,   \
    0x40c8 /* est_deci-dBm:110 act_deci-dBm:110*/,   \
    0x40e0 /* est_deci-dBm:120 act_deci-dBm:120*/,   \
    0x40ed /* est_deci-dBm:130 act_deci-dBm:128*/,   \
  }

#if (_SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_DUALBAND)
#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (33U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (120U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-200)
#define SL_RAIL_PA_TABLE_0                        \
  {                                               \
    0x0 /* est_deci-dBm:-200 act_deci-dBm:-199*/, \
    0x0 /* est_deci-dBm:-190 act_deci-dBm:-199*/, \
    0x0 /* est_deci-dBm:-180 act_deci-dBm:-199*/, \
    0x0 /* est_deci-dBm:-170 act_deci-dBm:-199*/, \
    0x2 /* est_deci-dBm:-160 act_deci-dBm:-164*/, \
    0x2 /* est_deci-dBm:-150 act_deci-dBm:-164*/, \
    0x3 /* est_deci-dBm:-140 act_deci-dBm:-141*/, \
    0x3 /* est_deci-dBm:-130 act_deci-dBm:-141*/, \
    0x4 /* est_deci-dBm:-120 act_deci-dBm:-123*/, \
    0x4 /* est_deci-dBm:-110 act_deci-dBm:-123*/, \
    0x5 /* est_deci-dBm:-100 act_deci-dBm:-107*/, \
    0x6 /* est_deci-dBm:-90 act_deci-dBm:-94*/,   \
    0x7 /* est_deci-dBm:-80 act_deci-dBm:-84*/,   \
    0x8 /* est_deci-dBm:-70 act_deci-dBm:-75*/,   \
    0xa /* est_deci-dBm:-60 act_deci-dBm:-60*/,   \
    0xb /* est_deci-dBm:-50 act_deci-dBm:-53*/,   \
    0xd /* est_deci-dBm:-40 act_deci-dBm:-41*/,   \
    0xf /* est_deci-dBm:-30 act_deci-dBm:-31*/,   \
    0x11 /* est_deci-dBm:-20 act_deci-dBm:-22*/,  \
    0x14 /* est_deci-dBm:-10 act_deci-dBm:-11*/,  \
    0x17 /* est_deci-dBm:0 act_deci-dBm:-2*/,     \
    0x1b /* est_deci-dBm:10 act_deci-dBm:8*/,     \
    0x1f /* est_deci-dBm:20 act_deci-dBm:18*/,    \
    0x23 /* est_deci-dBm:30 act_deci-dBm:30*/,    \
    0x2a /* est_deci-dBm:40 act_deci-dBm:40*/,    \
    0x31 /* est_deci-dBm:50 act_deci-dBm:50*/,    \
    0x39 /* est_deci-dBm:60 act_deci-dBm:60*/,    \
    0x43 /* est_deci-dBm:70 act_deci-dBm:70*/,    \
    0x52 /* est_deci-dBm:80 act_deci-dBm:80*/,    \
    0x64 /* est_deci-dBm:90 act_deci-dBm:90*/,    \
    0x84 /* est_deci-dBm:100 act_deci-dBm:100*/,  \
    0xb8 /* est_deci-dBm:110 act_deci-dBm:110*/,  \
    0xed /* est_deci-dBm:120 act_deci-dBm:117*/,  \
  }
#endif // (_SILICON_LABS_EFR32_RADIO_TYPE == _SILICON_LABS_EFR32_RADIO_DUALBAND)

#ifdef __cplusplus
}
#endif

#endif
