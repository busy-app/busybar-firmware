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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG21_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG21_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (35U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (100U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-235)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x8001 /* est_deci-dBm:-235 act_deci-dBm:-234*/, \
    0x8003 /* est_deci-dBm:-225 act_deci-dBm:-233*/, \
    0x8003 /* est_deci-dBm:-215 act_deci-dBm:-233*/, \
    0x8003 /* est_deci-dBm:-205 act_deci-dBm:-233*/, \
    0x8004 /* est_deci-dBm:-195 act_deci-dBm:-203*/, \
    0x8004 /* est_deci-dBm:-185 act_deci-dBm:-203*/, \
    0x8005 /* est_deci-dBm:-175 act_deci-dBm:-179*/, \
    0x8005 /* est_deci-dBm:-165 act_deci-dBm:-179*/, \
    0x8006 /* est_deci-dBm:-155 act_deci-dBm:-160*/, \
    0x8006 /* est_deci-dBm:-145 act_deci-dBm:-160*/, \
    0x8007 /* est_deci-dBm:-135 act_deci-dBm:-143*/, \
    0x8008 /* est_deci-dBm:-125 act_deci-dBm:-130*/, \
    0x8009 /* est_deci-dBm:-115 act_deci-dBm:-118*/, \
    0x800a /* est_deci-dBm:-105 act_deci-dBm:-109*/, \
    0x800b /* est_deci-dBm:-95 act_deci-dBm:-100*/,  \
    0x800c /* est_deci-dBm:-85 act_deci-dBm:-92*/,   \
    0x800d /* est_deci-dBm:-75 act_deci-dBm:-79*/,   \
    0x800f /* est_deci-dBm:-65 act_deci-dBm:-68*/,   \
    0x8012 /* est_deci-dBm:-55 act_deci-dBm:-55*/,   \
    0x8015 /* est_deci-dBm:-45 act_deci-dBm:-45*/,   \
    0x8018 /* est_deci-dBm:-35 act_deci-dBm:-37*/,   \
    0x801d /* est_deci-dBm:-25 act_deci-dBm:-26*/,   \
    0x8024 /* est_deci-dBm:-15 act_deci-dBm:-15*/,   \
    0x802e /* est_deci-dBm:-5 act_deci-dBm:-5*/,     \
    0x400a /* est_deci-dBm:5 act_deci-dBm:-1*/,      \
    0x400c /* est_deci-dBm:15 act_deci-dBm:13*/,     \
    0x400e /* est_deci-dBm:25 act_deci-dBm:25*/,     \
    0x4010 /* est_deci-dBm:35 act_deci-dBm:34*/,     \
    0x4012 /* est_deci-dBm:45 act_deci-dBm:43*/,     \
    0x4015 /* est_deci-dBm:55 act_deci-dBm:53*/,     \
    0x401a /* est_deci-dBm:65 act_deci-dBm:65*/,     \
    0x401f /* est_deci-dBm:75 act_deci-dBm:74*/,     \
    0x4028 /* est_deci-dBm:85 act_deci-dBm:85*/,     \
    0x4035 /* est_deci-dBm:95 act_deci-dBm:95*/,     \
    0x4041 /* est_deci-dBm:100 act_deci-dBm:100*/,   \
  }

#ifdef __cplusplus
}
#endif

#endif
