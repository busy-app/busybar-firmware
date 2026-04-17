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

#define SL_RAIL_PA_TABLE_1_NUM_VALUES  (37U)
#define SL_RAIL_PA_TABLE_1_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_1_MAX_POWER_DDBM     (210U)
#define SL_RAIL_PA_TABLE_1_MIN_POWER_DDBM     (-145)
#define SL_RAIL_PA_TABLE_1                        \
  {                                               \
    0x0 /* est_deci-dBm:-145 act_deci-dBm:-150*/, \
    0x0 /* est_deci-dBm:-135 act_deci-dBm:-150*/, \
    0x0 /* est_deci-dBm:-125 act_deci-dBm:-150*/, \
    0x2 /* est_deci-dBm:-115 act_deci-dBm:-115*/, \
    0x2 /* est_deci-dBm:-105 act_deci-dBm:-115*/, \
    0x2 /* est_deci-dBm:-95 act_deci-dBm:-115*/,  \
    0x3 /* est_deci-dBm:-85 act_deci-dBm:-91*/,   \
    0x3 /* est_deci-dBm:-75 act_deci-dBm:-91*/,   \
    0x4 /* est_deci-dBm:-65 act_deci-dBm:-72*/,   \
    0x5 /* est_deci-dBm:-55 act_deci-dBm:-56*/,   \
    0x5 /* est_deci-dBm:-45 act_deci-dBm:-56*/,   \
    0x6 /* est_deci-dBm:-35 act_deci-dBm:-44*/,   \
    0x7 /* est_deci-dBm:-25 act_deci-dBm:-33*/,   \
    0x8 /* est_deci-dBm:-15 act_deci-dBm:-23*/,   \
    0xa /* est_deci-dBm:-5 act_deci-dBm:-6*/,     \
    0xb /* est_deci-dBm:5 act_deci-dBm:1*/,       \
    0xd /* est_deci-dBm:15 act_deci-dBm:13*/,     \
    0xf /* est_deci-dBm:25 act_deci-dBm:23*/,     \
    0x11 /* est_deci-dBm:35 act_deci-dBm:33*/,    \
    0x14 /* est_deci-dBm:45 act_deci-dBm:45*/,    \
    0x16 /* est_deci-dBm:55 act_deci-dBm:52*/,    \
    0x1a /* est_deci-dBm:65 act_deci-dBm:65*/,    \
    0x1e /* est_deci-dBm:75 act_deci-dBm:75*/,    \
    0x21 /* est_deci-dBm:85 act_deci-dBm:83*/,    \
    0x26 /* est_deci-dBm:95 act_deci-dBm:95*/,    \
    0x2b /* est_deci-dBm:105 act_deci-dBm:104*/,  \
    0x31 /* est_deci-dBm:115 act_deci-dBm:114*/,  \
    0x39 /* est_deci-dBm:125 act_deci-dBm:125*/,  \
    0x42 /* est_deci-dBm:135 act_deci-dBm:135*/,  \
    0x4c /* est_deci-dBm:145 act_deci-dBm:145*/,  \
    0x5a /* est_deci-dBm:155 act_deci-dBm:155*/,  \
    0x6b /* est_deci-dBm:165 act_deci-dBm:165*/,  \
    0x82 /* est_deci-dBm:175 act_deci-dBm:175*/,  \
    0xa2 /* est_deci-dBm:185 act_deci-dBm:185*/,  \
    0xd4 /* est_deci-dBm:195 act_deci-dBm:195*/,  \
    0xec /* est_deci-dBm:205 act_deci-dBm:200*/,  \
    0xec /* est_deci-dBm:210 act_deci-dBm:200*/,  \
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
