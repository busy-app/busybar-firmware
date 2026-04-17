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

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (46U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (50U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-400)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x8000 /* est_deci-dBm:-400 act_deci-dBm:-395*/, \
    0x8000 /* est_deci-dBm:-390 act_deci-dBm:-395*/, \
    0x8000 /* est_deci-dBm:-380 act_deci-dBm:-395*/, \
    0x8000 /* est_deci-dBm:-370 act_deci-dBm:-395*/, \
    0x8002 /* est_deci-dBm:-360 act_deci-dBm:-368*/, \
    0x8002 /* est_deci-dBm:-350 act_deci-dBm:-368*/, \
    0x8002 /* est_deci-dBm:-340 act_deci-dBm:-368*/, \
    0x8003 /* est_deci-dBm:-330 act_deci-dBm:-339*/, \
    0x8003 /* est_deci-dBm:-320 act_deci-dBm:-339*/, \
    0x8004 /* est_deci-dBm:-310 act_deci-dBm:-316*/, \
    0x8004 /* est_deci-dBm:-300 act_deci-dBm:-316*/, \
    0x8004 /* est_deci-dBm:-290 act_deci-dBm:-316*/, \
    0x8005 /* est_deci-dBm:-280 act_deci-dBm:-285*/, \
    0x8006 /* est_deci-dBm:-270 act_deci-dBm:-271*/, \
    0x8006 /* est_deci-dBm:-260 act_deci-dBm:-271*/, \
    0x8007 /* est_deci-dBm:-250 act_deci-dBm:-255*/, \
    0x8008 /* est_deci-dBm:-240 act_deci-dBm:-244*/, \
    0x8008 /* est_deci-dBm:-230 act_deci-dBm:-244*/, \
    0x800a /* est_deci-dBm:-220 act_deci-dBm:-220*/, \
    0x800b /* est_deci-dBm:-210 act_deci-dBm:-213*/, \
    0x800c /* est_deci-dBm:-200 act_deci-dBm:-206*/, \
    0x800e /* est_deci-dBm:-190 act_deci-dBm:-190*/, \
    0x800f /* est_deci-dBm:-180 act_deci-dBm:-183*/, \
    0x8010 /* est_deci-dBm:-170 act_deci-dBm:-178*/, \
    0x8012 /* est_deci-dBm:-160 act_deci-dBm:-162*/, \
    0x8014 /* est_deci-dBm:-150 act_deci-dBm:-154*/, \
    0x8017 /* est_deci-dBm:-140 act_deci-dBm:-141*/, \
    0x801a /* est_deci-dBm:-130 act_deci-dBm:-130*/, \
    0x801d /* est_deci-dBm:-120 act_deci-dBm:-120*/, \
    0x8021 /* est_deci-dBm:-110 act_deci-dBm:-111*/, \
    0x8026 /* est_deci-dBm:-100 act_deci-dBm:-100*/, \
    0x802a /* est_deci-dBm:-90 act_deci-dBm:-91*/,   \
    0x8030 /* est_deci-dBm:-80 act_deci-dBm:-80*/,   \
    0x8036 /* est_deci-dBm:-70 act_deci-dBm:-70*/,   \
    0x803c /* est_deci-dBm:-60 act_deci-dBm:-61*/,   \
    0x8045 /* est_deci-dBm:-50 act_deci-dBm:-50*/,   \
    0x804f /* est_deci-dBm:-40 act_deci-dBm:-40*/,   \
    0x8059 /* est_deci-dBm:-30 act_deci-dBm:-30*/,   \
    0x8065 /* est_deci-dBm:-20 act_deci-dBm:-20*/,   \
    0x8073 /* est_deci-dBm:-10 act_deci-dBm:-10*/,   \
    0x8085 /* est_deci-dBm:0 act_deci-dBm:0*/,       \
    0x809a /* est_deci-dBm:10 act_deci-dBm:10*/,     \
    0x80b1 /* est_deci-dBm:20 act_deci-dBm:20*/,     \
    0x80ce /* est_deci-dBm:30 act_deci-dBm:30*/,     \
    0x80ea /* est_deci-dBm:40 act_deci-dBm:40*/,     \
    0x80ed /* est_deci-dBm:50 act_deci-dBm:42*/,     \
  }

#ifdef __cplusplus
}
#endif

#endif
