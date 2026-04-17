/***************************************************************************//**
 * @file sl_wisun_regdb.c
 * @brief Wi-SUN PHY and channels parameters
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_wisun_regdb.h"
#include "sl_wisun_types.h"

const sl_wisun_chan_params_t chan_params_table[] = {
  /*              operating_class -.                  chan_plan_id -.   chan_spacing_hz -.    ,- chan_count
  *      regulatory_domain        |     regional_regulation        | chan0_freq_hz      |    |       valid phy_mode_ids         chan_allowed */
  { SL_WISUN_REGULATORY_DOMAIN_AZ, 1, SL_WISUN_REGULATION_NONE,    48,  915200000,  200000,  64, {  2,  3, 18, 19              }, NULL }, // SL_WISUN_REGULATORY_DOMAIN_AZ and SL_WISUN_REGULATORY_DOMAIN_NZ share the same ID
  { SL_WISUN_REGULATORY_DOMAIN_AZ, 2, SL_WISUN_REGULATION_NONE,    49,  915400000,  400000,  32, {  5,  6,  8, 21, 22, 24      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_BZ, 1, SL_WISUN_REGULATION_NONE,     1,  902200000,  200000, 129, {  2,  3, 18, 19, 84, 85, 86, }, "0-25,65-128", },
  { SL_WISUN_REGULATORY_DOMAIN_BZ, 2, SL_WISUN_REGULATION_NONE,     2,  902400000,  400000,  64, {  5,  6, 21, 22, 68, 69, 70, }, "0-11,33-63", },
  { SL_WISUN_REGULATORY_DOMAIN_BZ, 3, SL_WISUN_REGULATION_NONE,     3,  902600000,  600000,  42, {  8, 24,                     }, "0-7,22-41", },
  { SL_WISUN_REGULATORY_DOMAIN_BZ, 0, SL_WISUN_REGULATION_NONE,     4,  902800000,  800000,  31, { 51, 52, 53, 54,             }, "0-4,17-30", },
  { SL_WISUN_REGULATORY_DOMAIN_BZ, 0, SL_WISUN_REGULATION_NONE,     5,  903200000, 1200000,  20, { 34, 35, 36, 37, 38,         }, "0-2,11-19", },
  { SL_WISUN_REGULATORY_DOMAIN_CN, 1, SL_WISUN_REGULATION_NONE,   160,  470200000,  200000, 199, {  2,  3,  5, 18, 19, 21      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_CN, 2, SL_WISUN_REGULATION_NONE,  0xff,  779200000,  200000,  39, {  2,  3,                     }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_CN, 3, SL_WISUN_REGULATION_NONE,  0xff,  779400000,  400000,  19, {  5,  6,  8,                 }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_CN, 4, SL_WISUN_REGULATION_NONE,   128,  920625000,  250000,  16, {  2,  3,  5, 18, 19, 21      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_EU, 1, SL_WISUN_REGULATION_ETSI,  0xff,  863100000,  100000,  69, {  1,                         }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_EU, 2, SL_WISUN_REGULATION_ETSI,  0xff,  863100000,  200000,  35, {  3,  5,                     }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_EU, 0, SL_WISUN_REGULATION_ETSI,    32,  863100000,  100000,  69, {  1, 17,                     }, "0-54,57-60,64,67-68", },
  { SL_WISUN_REGULATORY_DOMAIN_EU, 0, SL_WISUN_REGULATION_ETSI,    33,  863100000,  200000,  35, {  3,  5, 19, 21, 84, 85, 86, }, "0-26,29,34", },
  { SL_WISUN_REGULATORY_DOMAIN_EU, 3, SL_WISUN_REGULATION_ETSI,    34,  870100000,  100000,  55, {  1, 17,                     }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_EU, 4, SL_WISUN_REGULATION_ETSI,    35,  870200000,  200000,  27, {  3,  5, 19, 21, 84, 85, 86, }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_EU, 0, SL_WISUN_REGULATION_ETSI,    36,  863100000,  100000, 125, {  1, 17,                     }, "0-54,57-60,64,67-124", },
  { SL_WISUN_REGULATORY_DOMAIN_EU, 0, SL_WISUN_REGULATION_ETSI,    37,  863100000,  200000,  62, {  3,  5, 19, 21, 84, 85, 86, }, "0-26,29,34-61", },
  { SL_WISUN_REGULATORY_DOMAIN_HK, 1, SL_WISUN_REGULATION_NONE,    64,  920200000,  200000,  24, {  2,  3, 18, 19              }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_HK, 2, SL_WISUN_REGULATION_NONE,    65,  920400000,  400000,  12, {  5,  6,  8, 21, 22, 24      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_IN, 1, SL_WISUN_REGULATION_WPC ,    39,  865100000,  100000,  29, {  1, 17                      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_IN, 2, SL_WISUN_REGULATION_WPC ,    40,  865100000,  200000,  15, {  3,  5, 19, 21, 84, 85, 86  }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_JP, 1, SL_WISUN_REGULATION_ARIB,    21,  920600000,  200000,  38, {  2, 18, 84, 85, 86,         }, "9-37", },
  { SL_WISUN_REGULATORY_DOMAIN_JP, 2, SL_WISUN_REGULATION_ARIB,    22,  920900000,  400000,  18, {  4,  5, 20, 21, 68, 69, 70, }, "4-17", },
  { SL_WISUN_REGULATORY_DOMAIN_JP, 3, SL_WISUN_REGULATION_ARIB,    23,  920800000,  600000,  12, {  7,  8, 23, 24,             }, "3-11", },
  { SL_WISUN_REGULATORY_DOMAIN_JP, 0, SL_WISUN_REGULATION_ARIB,    24,  921100000,  800000,   9, { 51, 52, 53, 54,             }, "2-8", },
  { SL_WISUN_REGULATORY_DOMAIN_KR, 1, SL_WISUN_REGULATION_NONE,    96,  917100000,  200000,  32, {  2,  3, 18, 19              }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_KR, 2, SL_WISUN_REGULATION_NONE,    97,  917300000,  400000,  16, {  5,  6,  8, 21, 22, 24      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_MX, 1, SL_WISUN_REGULATION_NONE,     1,  902200000,  200000, 129, {  2,  3, 18, 19              }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_MX, 2, SL_WISUN_REGULATION_NONE,     2,  902400000,  400000,  64, {  5,  6,  8, 21, 22, 24      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_MY, 1, SL_WISUN_REGULATION_NONE,    80,  919200000,  200000,  19, {  2,  3, 18, 19              }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_MY, 2, SL_WISUN_REGULATION_NONE,    81,  919200000,  400000,  10, {  5,  6,  8, 21, 22, 24      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_NA, 1, SL_WISUN_REGULATION_NONE,     1,  902200000,  200000, 129, {  2,  3, 18, 19, 84, 85, 86, }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_NA, 2, SL_WISUN_REGULATION_NONE,     2,  902400000,  400000,  64, {  5,  6, 21, 22, 68, 69, 70, }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_NA, 3, SL_WISUN_REGULATION_NONE,     3,  902600000,  600000,  42, {  8, 24,                     }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_NA, 0, SL_WISUN_REGULATION_NONE,     4,  902800000,  800000,  31, { 51, 52, 53, 54,             }, "0-30", },
  { SL_WISUN_REGULATORY_DOMAIN_NA, 0, SL_WISUN_REGULATION_NONE,     5,  903200000, 1200000,  20, { 34, 35, 36, 37, 38,         }, "0-19", },
  { SL_WISUN_REGULATORY_DOMAIN_PH, 1, SL_WISUN_REGULATION_NONE,    48,  915200000,  200000,  64, {  2,  3,  18, 19             }, "0-13", },
  { SL_WISUN_REGULATORY_DOMAIN_PH, 2, SL_WISUN_REGULATION_NONE,    49,  915400000,  400000,  32, {  5,  6,  8,  21, 22, 24     }, "0-6", },
  { SL_WISUN_REGULATORY_DOMAIN_SG, 1, SL_WISUN_REGULATION_NONE,    41,  866100000,  100000,  29, {  1,                         }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_SG, 2, SL_WISUN_REGULATION_NONE,    42,  866100000,  200000,  15, {  3,  5,                     }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_SG, 3, SL_WISUN_REGULATION_NONE,    43,  866300000,  400000,   7, {  6,  8,                     }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_SG, 4, SL_WISUN_REGULATION_NONE,    98,  917200000,  200000,  39, {  2,  3, 18, 19              }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_SG, 5, SL_WISUN_REGULATION_NONE,    99,  917400000,  400000,  19, {  5,  6,  8, 21, 22, 24      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_TH, 1, SL_WISUN_REGULATION_NONE,    64,  920200000,  200000,  24, {  2,  3, 18, 19              }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_TH, 2, SL_WISUN_REGULATION_NONE,    65,  920400000,  400000,  12, {  5,  6,  8, 21, 22, 24      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_VN, 1, SL_WISUN_REGULATION_NONE,    64,  920200000,  200000,  24, {  2,  3, 18, 19              }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_VN, 2, SL_WISUN_REGULATION_NONE,    65,  920400000,  400000,  12, {  5,  6,  8, 21, 22, 24      }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_WW, 1, SL_WISUN_REGULATION_NONE,   112, 2400200000,  200000, 416, {  2,  3, 18, 19              }, NULL },
  { SL_WISUN_REGULATORY_DOMAIN_WW, 2, SL_WISUN_REGULATION_NONE,   113, 2400400000,  400000, 207, {  5,  6,  8, 21, 22, 24      }, NULL },
  {                             0, 0, SL_WISUN_REGULATION_NONE,  0xff,          0,       0,   0, { 0xff                        }, NULL },
};
