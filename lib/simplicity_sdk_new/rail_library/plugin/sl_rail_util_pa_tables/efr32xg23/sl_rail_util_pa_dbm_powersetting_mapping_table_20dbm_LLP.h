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

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (40U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (-70)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-460)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0xc000 /* est_deci-dBm:-460 act_deci-dBm:-451*/, \
    0xc000 /* est_deci-dBm:-450 act_deci-dBm:-451*/, \
    0xc002 /* est_deci-dBm:-440 act_deci-dBm:-442*/, \
    0xc002 /* est_deci-dBm:-430 act_deci-dBm:-442*/, \
    0xc003 /* est_deci-dBm:-420 act_deci-dBm:-428*/, \
    0xc004 /* est_deci-dBm:-410 act_deci-dBm:-417*/, \
    0xc004 /* est_deci-dBm:-400 act_deci-dBm:-417*/, \
    0xc005 /* est_deci-dBm:-390 act_deci-dBm:-396*/, \
    0xc006 /* est_deci-dBm:-380 act_deci-dBm:-387*/, \
    0xc007 /* est_deci-dBm:-370 act_deci-dBm:-375*/, \
    0xc008 /* est_deci-dBm:-360 act_deci-dBm:-368*/, \
    0xc009 /* est_deci-dBm:-350 act_deci-dBm:-352*/, \
    0xc00a /* est_deci-dBm:-340 act_deci-dBm:-346*/, \
    0xc00c /* est_deci-dBm:-330 act_deci-dBm:-334*/, \
    0xc00e /* est_deci-dBm:-320 act_deci-dBm:-320*/, \
    0xc010 /* est_deci-dBm:-310 act_deci-dBm:-310*/, \
    0xc010 /* est_deci-dBm:-300 act_deci-dBm:-310*/, \
    0xc012 /* est_deci-dBm:-290 act_deci-dBm:-292*/, \
    0xc015 /* est_deci-dBm:-280 act_deci-dBm:-280*/, \
    0xc018 /* est_deci-dBm:-270 act_deci-dBm:-271*/, \
    0xc01b /* est_deci-dBm:-260 act_deci-dBm:-260*/, \
    0xc01e /* est_deci-dBm:-250 act_deci-dBm:-251*/, \
    0xc021 /* est_deci-dBm:-240 act_deci-dBm:-241*/, \
    0xc026 /* est_deci-dBm:-230 act_deci-dBm:-231*/, \
    0xc02b /* est_deci-dBm:-220 act_deci-dBm:-221*/, \
    0xc030 /* est_deci-dBm:-210 act_deci-dBm:-212*/, \
    0xc037 /* est_deci-dBm:-200 act_deci-dBm:-200*/, \
    0xc03e /* est_deci-dBm:-190 act_deci-dBm:-190*/, \
    0xc045 /* est_deci-dBm:-180 act_deci-dBm:-180*/, \
    0xc04e /* est_deci-dBm:-170 act_deci-dBm:-170*/, \
    0xc057 /* est_deci-dBm:-160 act_deci-dBm:-160*/, \
    0xc061 /* est_deci-dBm:-150 act_deci-dBm:-150*/, \
    0xc06d /* est_deci-dBm:-140 act_deci-dBm:-140*/, \
    0xc07a /* est_deci-dBm:-130 act_deci-dBm:-131*/, \
    0xc089 /* est_deci-dBm:-120 act_deci-dBm:-120*/, \
    0xc098 /* est_deci-dBm:-110 act_deci-dBm:-110*/, \
    0xc0ac /* est_deci-dBm:-100 act_deci-dBm:-100*/, \
    0xc0bf /* est_deci-dBm:-90 act_deci-dBm:-90*/,   \
    0xc0d7 /* est_deci-dBm:-80 act_deci-dBm:-80*/,   \
    0xc0e8 /* est_deci-dBm:-70 act_deci-dBm:-70*/,   \
  }

#ifdef __cplusplus
}
#endif

#endif
