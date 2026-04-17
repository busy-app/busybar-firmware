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

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (47U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (110U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-350)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x4000 /* est_deci-dBm:-350 act_deci-dBm:-334*/, \
    0x4000 /* est_deci-dBm:-340 act_deci-dBm:-334*/, \
    0x4000 /* est_deci-dBm:-330 act_deci-dBm:-334*/, \
    0x4000 /* est_deci-dBm:-320 act_deci-dBm:-334*/, \
    0x4000 /* est_deci-dBm:-310 act_deci-dBm:-334*/, \
    0x4000 /* est_deci-dBm:-300 act_deci-dBm:-334*/, \
    0x4000 /* est_deci-dBm:-290 act_deci-dBm:-334*/, \
    0x4002 /* est_deci-dBm:-280 act_deci-dBm:-286*/, \
    0x4002 /* est_deci-dBm:-270 act_deci-dBm:-286*/, \
    0x4002 /* est_deci-dBm:-260 act_deci-dBm:-286*/, \
    0x4003 /* est_deci-dBm:-250 act_deci-dBm:-250*/, \
    0x4003 /* est_deci-dBm:-240 act_deci-dBm:-250*/, \
    0x4003 /* est_deci-dBm:-230 act_deci-dBm:-250*/, \
    0x4004 /* est_deci-dBm:-220 act_deci-dBm:-226*/, \
    0x4004 /* est_deci-dBm:-210 act_deci-dBm:-226*/, \
    0x4004 /* est_deci-dBm:-200 act_deci-dBm:-226*/, \
    0x4005 /* est_deci-dBm:-190 act_deci-dBm:-199*/, \
    0x4006 /* est_deci-dBm:-180 act_deci-dBm:-185*/, \
    0x4006 /* est_deci-dBm:-170 act_deci-dBm:-185*/, \
    0x4007 /* est_deci-dBm:-160 act_deci-dBm:-169*/, \
    0x4008 /* est_deci-dBm:-150 act_deci-dBm:-159*/, \
    0x4009 /* est_deci-dBm:-140 act_deci-dBm:-147*/, \
    0x400b /* est_deci-dBm:-130 act_deci-dBm:-131*/, \
    0x400c /* est_deci-dBm:-120 act_deci-dBm:-125*/, \
    0x400d /* est_deci-dBm:-110 act_deci-dBm:-114*/, \
    0x400f /* est_deci-dBm:-100 act_deci-dBm:-103*/, \
    0x4010 /* est_deci-dBm:-90 act_deci-dBm:-98*/,   \
    0x4013 /* est_deci-dBm:-80 act_deci-dBm:-81*/,   \
    0x4015 /* est_deci-dBm:-70 act_deci-dBm:-71*/,   \
    0x4018 /* est_deci-dBm:-60 act_deci-dBm:-61*/,   \
    0x401b /* est_deci-dBm:-50 act_deci-dBm:-50*/,   \
    0x401e /* est_deci-dBm:-40 act_deci-dBm:-42*/,   \
    0x4023 /* est_deci-dBm:-30 act_deci-dBm:-30*/,   \
    0x4027 /* est_deci-dBm:-20 act_deci-dBm:-21*/,   \
    0x402c /* est_deci-dBm:-10 act_deci-dBm:-11*/,   \
    0x4031 /* est_deci-dBm:0 act_deci-dBm:-1*/,      \
    0x4038 /* est_deci-dBm:10 act_deci-dBm:9*/,      \
    0x4040 /* est_deci-dBm:20 act_deci-dBm:20*/,     \
    0x4049 /* est_deci-dBm:30 act_deci-dBm:30*/,     \
    0x4053 /* est_deci-dBm:40 act_deci-dBm:40*/,     \
    0x405e /* est_deci-dBm:50 act_deci-dBm:50*/,     \
    0x406c /* est_deci-dBm:60 act_deci-dBm:60*/,     \
    0x407d /* est_deci-dBm:70 act_deci-dBm:70*/,     \
    0x4092 /* est_deci-dBm:80 act_deci-dBm:80*/,     \
    0x40ad /* est_deci-dBm:90 act_deci-dBm:90*/,     \
    0x40ce /* est_deci-dBm:100 act_deci-dBm:100*/,   \
    0x40eb /* est_deci-dBm:110 act_deci-dBm:109*/,   \
  }

#ifdef __cplusplus
}
#endif

#endif
