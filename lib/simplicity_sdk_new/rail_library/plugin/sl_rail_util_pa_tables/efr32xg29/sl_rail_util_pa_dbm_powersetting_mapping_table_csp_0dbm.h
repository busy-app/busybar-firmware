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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG29_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG29_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (29U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (0U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-292)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x8000 /* est_deci-dBm:-272 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-262 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-252 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-242 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-232 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-222 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-212 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-202 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-192 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-182 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-172 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-162 act_deci-dBm:-272*/, \
    0x8000 /* est_deci-dBm:-152 act_deci-dBm:-272*/, \
    0x8001 /* est_deci-dBm:-142 act_deci-dBm:-150*/, \
    0x8001 /* est_deci-dBm:-132 act_deci-dBm:-150*/, \
    0x8001 /* est_deci-dBm:-122 act_deci-dBm:-150*/, \
    0x8001 /* est_deci-dBm:-112 act_deci-dBm:-150*/, \
    0x8001 /* est_deci-dBm:-102 act_deci-dBm:-150*/, \
    0x8002 /* est_deci-dBm:-92 act_deci-dBm:-97*/,   \
    0x8002 /* est_deci-dBm:-82 act_deci-dBm:-97*/,   \
    0x8002 /* est_deci-dBm:-72 act_deci-dBm:-97*/,   \
    0x8003 /* est_deci-dBm:-62 act_deci-dBm:-68*/,   \
    0x8003 /* est_deci-dBm:-52 act_deci-dBm:-68*/,   \
    0x8004 /* est_deci-dBm:-42 act_deci-dBm:-50*/,   \
    0x8005 /* est_deci-dBm:-32 act_deci-dBm:-36*/,   \
    0x8006 /* est_deci-dBm:-22 act_deci-dBm:-26*/,   \
    0x8008 /* est_deci-dBm:-12 act_deci-dBm:-14*/,   \
    0x800b /* est_deci-dBm:-2 act_deci-dBm:-2*/,     \
    0x800c /* est_deci-dBm:0 act_deci-dBm:0*/,       \
  }

#ifdef __cplusplus
}
#endif

#endif
