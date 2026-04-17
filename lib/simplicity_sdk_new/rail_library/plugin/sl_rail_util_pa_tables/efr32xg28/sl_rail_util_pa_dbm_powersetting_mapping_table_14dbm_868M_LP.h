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

#define SL_RAIL_PA_TABLE_1_NUM_VALUES  (41U)
#define SL_RAIL_PA_TABLE_1_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_1_MAX_POWER_DDBM     (30U)
#define SL_RAIL_PA_TABLE_1_MIN_POWER_DDBM     (-370)
#define SL_RAIL_PA_TABLE_1                           \
  {                                                  \
    0x8000 /* est_deci-dBm:-370 act_deci-dBm:-370*/, \
    0x8000 /* est_deci-dBm:-360 act_deci-dBm:-370*/, \
    0x8002 /* est_deci-dBm:-350 act_deci-dBm:-354*/, \
    0x8002 /* est_deci-dBm:-340 act_deci-dBm:-354*/, \
    0x8003 /* est_deci-dBm:-330 act_deci-dBm:-339*/, \
    0x8003 /* est_deci-dBm:-320 act_deci-dBm:-339*/, \
    0x8004 /* est_deci-dBm:-310 act_deci-dBm:-313*/, \
    0x8005 /* est_deci-dBm:-300 act_deci-dBm:-302*/, \
    0x8005 /* est_deci-dBm:-290 act_deci-dBm:-302*/, \
    0x8006 /* est_deci-dBm:-280 act_deci-dBm:-286*/, \
    0x8007 /* est_deci-dBm:-270 act_deci-dBm:-277*/, \
    0x8007 /* est_deci-dBm:-260 act_deci-dBm:-277*/, \
    0x8009 /* est_deci-dBm:-250 act_deci-dBm:-252*/, \
    0x800a /* est_deci-dBm:-240 act_deci-dBm:-244*/, \
    0x800b /* est_deci-dBm:-230 act_deci-dBm:-238*/, \
    0x800d /* est_deci-dBm:-220 act_deci-dBm:-223*/, \
    0x800f /* est_deci-dBm:-210 act_deci-dBm:-211*/, \
    0x800f /* est_deci-dBm:-200 act_deci-dBm:-211*/, \
    0x8012 /* est_deci-dBm:-190 act_deci-dBm:-191*/, \
    0x8014 /* est_deci-dBm:-180 act_deci-dBm:-183*/, \
    0x8017 /* est_deci-dBm:-170 act_deci-dBm:-172*/, \
    0x801a /* est_deci-dBm:-160 act_deci-dBm:-160*/, \
    0x801d /* est_deci-dBm:-150 act_deci-dBm:-151*/, \
    0x8022 /* est_deci-dBm:-140 act_deci-dBm:-140*/, \
    0x8026 /* est_deci-dBm:-130 act_deci-dBm:-130*/, \
    0x802b /* est_deci-dBm:-120 act_deci-dBm:-121*/, \
    0x802f /* est_deci-dBm:-110 act_deci-dBm:-112*/, \
    0x8036 /* est_deci-dBm:-100 act_deci-dBm:-100*/, \
    0x803c /* est_deci-dBm:-90 act_deci-dBm:-90*/,   \
    0x8044 /* est_deci-dBm:-80 act_deci-dBm:-80*/,   \
    0x804c /* est_deci-dBm:-70 act_deci-dBm:-71*/,   \
    0x8057 /* est_deci-dBm:-60 act_deci-dBm:-60*/,   \
    0x8062 /* est_deci-dBm:-50 act_deci-dBm:-50*/,   \
    0x806f /* est_deci-dBm:-40 act_deci-dBm:-41*/,   \
    0x807e /* est_deci-dBm:-30 act_deci-dBm:-30*/,   \
    0x808f /* est_deci-dBm:-20 act_deci-dBm:-20*/,   \
    0x80a4 /* est_deci-dBm:-10 act_deci-dBm:-10*/,   \
    0x80b8 /* est_deci-dBm:0 act_deci-dBm:0*/,       \
    0x80d5 /* est_deci-dBm:10 act_deci-dBm:10*/,     \
    0x80e9 /* est_deci-dBm:20 act_deci-dBm:20*/,     \
    0x80ee /* est_deci-dBm:30 act_deci-dBm:23*/,     \
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
