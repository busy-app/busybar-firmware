/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config  -  DO NOT EDIT
 *   Radio Configurator Version: 2504.4.6
 *   RAIL Adapter Version: 2.4.33
 *   RAIL Compatibility: 2.x
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
#include "em_device.h"
#include "sl_rail_ble_config_39MHz.h"

static const uint8_t irCalConfig_0[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint8_t irCalConfig_1[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int32_t timingConfig_0[] = {
  50000, 9980, 3210, 1106
};

static const int32_t timingConfig_1[] = {
  12492, 5460, 1520, 416
};

static const int32_t timingConfig_2[] = {
  7000, 7930, 3182, 1058
};

static const int32_t timingConfig_3[] = {
  6750, 7930, 3182, 1058
};

static const int32_t timingConfig_4[] = {
  50000, 10700, 2000, 2000
};

__ALIGNED(4) static const uint8_t hfxoRetimingConfigEntries[] = {
  2, 0, 0, 0, 0xc0, 0x17, 0x53, 0x02, 6, 20, 0, 0, 0x80, 0x2f, 0xa6, 0x04, 5, 56, 0, 0, 0xa0, 0x08, 0, 0, 0, 0, 0x58, 0x09, 1, 4, 7, 6, 0x10, 0x0a, 1, 4, 7, 7, 0xc8, 0x0a, 0, 4, 8, 7, 0x80, 0x0b, 0, 4, 8, 8, 0x38, 0x0c, 0, 4, 9, 8, 0x61, 0x08, 0, 0, 0, 0, 0x8a, 0x08, 0, 0, 0, 0, 0xc7, 0x09, 1, 4, 4, 3, 0x2c, 0x0b, 1, 4, 4, 4, 0x92, 0x0c, 1, 4, 5, 4
};

#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
__ALIGNED(4) static const uint8_t stackInfo_0[2] = { 0x03, 0x09 };
__ALIGNED(4) static const uint8_t stackInfo_1[2] = { 0x03, 0x01 };
__ALIGNED(4) static const uint8_t stackInfo_2[2] = { 0x03, 0x02 };
__ALIGNED(4) static const uint8_t stackInfo_3[2] = { 0x03, 0x03 };
__ALIGNED(4) static const uint8_t stackInfo_4[2] = { 0x03, 0x04 };
#endif // RADIO_CONFIG_ENABLE_STACK_INFO

static RAIL_ChannelConfigEntryAttr_t channelConfigEntryAttr_0 = {
#if RAIL_SUPPORTS_OFDM_PA
  {
#ifdef RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
    { 0xFFFFFFFFUL, 0xFFFFFFFFUL, },
#else
    { 0xFFFFFFFFUL },
#endif // RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
    { 0xFFFFFFFFUL, 0xFFFFFFFFUL }
  }
#else // RAIL_SUPPORTS_OFDM_PA
#ifdef RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
  { 0xFFFFFFFFUL, 0xFFFFFFFFUL, },
#else
  { 0xFFFFFFFFUL },
#endif // RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
#endif // RAIL_SUPPORTS_OFDM_PA
};

static RAIL_ChannelConfigEntryAttr_t channelConfigEntryAttr_1 = {
#if RAIL_SUPPORTS_OFDM_PA
  {
#ifdef RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
    { 0xFFFFFFFFUL, 0xFFFFFFFFUL, },
#else
    { 0xFFFFFFFFUL },
#endif // RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
    { 0xFFFFFFFFUL, 0xFFFFFFFFUL }
  }
#else // RAIL_SUPPORTS_OFDM_PA
#ifdef RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
  { 0xFFFFFFFFUL, 0xFFFFFFFFUL, },
#else
  { 0xFFFFFFFFUL },
#endif // RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
#endif // RAIL_SUPPORTS_OFDM_PA
};

static const uint32_t phyInfo_0[] = {
  20UL,
  0x00924924UL, // 146.28571428571428
  (uint32_t) NULL,
  (uint32_t) irCalConfig_0,
  (uint32_t) timingConfig_0,
  0x00000000UL,
  0UL,
  16000000UL,
  999998UL,
  0x00F00101UL,
  0x035024C2UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  999979UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_1[] = {
  20UL,
  0x00666666UL, // 102.39999999999999
  (uint32_t) NULL,
  (uint32_t) irCalConfig_0,
  (uint32_t) timingConfig_1,
  0x00000000UL,
  0UL,
  32000000UL,
  1999995UL,
  0x00F00101UL,
  0x025047F1UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  1999996UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_2[] = {
  20UL,
  0x0071C71CUL, // 113.77777777777776
  (uint32_t) NULL,
  (uint32_t) irCalConfig_1,
  (uint32_t) timingConfig_2,
  0x00000000UL,
  0UL,
  36000000UL,
  999998UL,
  0x00EE0801UL,
  0x021047F1UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  999978UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_3[] = {
  20UL,
  0x0071C71CUL, // 113.77777777777776
  (uint32_t) NULL,
  (uint32_t) irCalConfig_1,
  (uint32_t) timingConfig_3,
  0x00000000UL,
  0UL,
  36000000UL,
  999998UL,
  0x00EE0201UL,
  0x021047F1UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  999978UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_4[] = {
  20UL,
  0x0071C71CUL, // 113.77777777777776
  (uint32_t) NULL,
  (uint32_t) irCalConfig_1,
  (uint32_t) timingConfig_4,
  0x00000000UL,
  0UL,
  36000000UL,
  999998UL,
  0x00EE0801UL,
  0x021047F1UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  999978UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

const uint32_t sl_rail_ble_phy_1Mbps_viterbi_39MHz_modemConfigBase[] = {
  0x03020000UL, 0x0000803FUL,
  /*    0004 */ 0x0001007EUL,
  0x0001400CUL, 0x00018101UL,
  0x00024020UL, 0x00000000UL,
  /*    4024 */ 0x00000001UL,
  0x00034030UL, 0x00000000UL,
  /*    4034 */ 0x00000000UL,
  /*    4038 */ 0x00000000UL,
  0x00024040UL, 0x00000000UL,
  /*    4044 */ 0xA000E000UL,
  0x00014060UL, 0x00000101UL,
  0x000140A8UL, 0x00000007UL,
  0x000440BCUL, 0x00000000UL,
  /*    40C0 */ 0x00000000UL,
  /*    40C4 */ 0x00000000UL,
  /*    40C8 */ 0x00000000UL,
  0x00034114UL, 0x00004CFFUL,
  /*    4118 */ 0x00004DFFUL,
  /*    411C */ 0x00004DFFUL,
  0x1001C020UL, 0x0007F800UL,
  0x3001C020UL, 0x012802F5UL,
  0x1001C024UL, 0x000000FFUL,
  0x3001C024UL, 0x04001300UL,
  0x0008C028UL, 0x03B380ECUL,
  /*    C02C */ 0x51407543UL,
  /*    C030 */ 0xB8000FA0UL,
  /*    C034 */ 0x00004000UL,
  /*    C038 */ 0x0007AAA8UL,
  /*    C03C */ 0x00000000UL,
  /*    C040 */ 0x00000000UL,
  /*    C044 */ 0x00000000UL,
  0x0009C070UL, 0x0000EFB9UL,
  /*    C074 */ 0x00003000UL,
  /*    C078 */ 0x0000B006UL,
  /*    C07C */ 0x00018010UL,
  /*    C080 */ 0x0002301FUL,
  /*    C084 */ 0x00060033UL,
  /*    C088 */ 0x0009F071UL,
  /*    C08C */ 0x006FF0FFUL,
  /*    C090 */ 0x00FFFFFFUL,
  0x0005C0A8UL, 0x083198AEUL,
  /*    C0AC */ 0x000420C6UL,
  /*    C0B0 */ 0x76543210UL,
  /*    C0B4 */ 0x00000A98UL,
  /*    C0B8 */ 0x00000000UL,
  0x0003C0D0UL, 0x00000000UL,
  /*    C0D4 */ 0x000A0001UL,
  /*    C0D8 */ 0x00280001UL,
  0x0003C0ECUL, 0x00200000UL,
  /*    C0F0 */ 0x00044222UL,
  /*    C0F4 */ 0x000001F8UL,
  0x0007C10CUL, 0x10171E26UL,
  /*    C110 */ 0xF0F8FF04UL,
  /*    C114 */ 0xCED7E0E8UL,
  /*    C118 */ 0xB5B5BDC5UL,
  /*    C11C */ 0x0C141C24UL,
  /*    C120 */ 0xEBF4FC04UL,
  /*    C124 */ 0x0000EBEBUL,
  0x0004C134UL, 0x01E10088UL,
  /*    C138 */ 0x0180D070UL,
  /*    C13C */ 0x0120A058UL,
  /*    C140 */ 0x00007040UL,
  0x01010008UL, 0x00000708UL,
  0x01010018UL, 0x00AAAAAAUL,
  0x01010020UL, 0x00DA6000UL,
  0x0102406CUL, 0x03000000UL,
  /*    4070 */ 0x20000000UL,
  0x01024078UL, 0x0000D20DUL,
  /*    407C */ 0x00000840UL,
  0x01024088UL, 0x000C008CUL,
  /*    408C */ 0x00000000UL,
  0x010440CCUL, 0x00000000UL,
  /*    40D0 */ 0x00000000UL,
  /*    40D4 */ 0x00000000UL,
  /*    40D8 */ 0x00000000UL,
  0x0101411CUL, 0x00000000UL,
  0x01014128UL, 0x00000000UL,
  0x01014158UL, 0x00000000UL,
  0x01014164UL, 0x0000010CUL,
  0x01054184UL, 0x0007D101UL,
  /*    4188 */ 0x00000000UL,
  /*    418C */ 0x00000000UL,
  /*    4190 */ 0x00000000UL,
  /*    4194 */ 0x00000000UL,
  0x010241A4UL, 0x00000000UL,
  /*    41A8 */ 0x00000000UL,
  0x010341BCUL, 0x00000000UL,
  /*    41C0 */ 0x003C0000UL,
  /*    41C4 */ 0x00069069UL,
  0x0103422CUL, 0x40001860UL,
  /*    4230 */ 0x00000000UL,
  /*    4234 */ 0x00000000UL,
  0x0101423CUL, 0x00000000UL,
  0x01014244UL, 0x00000014UL,
  0x010242A0UL, 0x00000000UL,
  /*    42A4 */ 0x00000081UL,
  0x010542B8UL, 0x0200003FUL,
  /*    42BC */ 0x0000FFFFUL,
  /*    42C0 */ 0x0000FFFFUL,
  /*    42C4 */ 0x000003FFUL,
  /*    42C8 */ 0x0000FFFFUL,
  0x010142D4UL, 0x00000000UL,
  0x010A4350UL, 0x01200040UL,
  /*    4354 */ 0x000000A0UL,
  /*    4358 */ 0x01005008UL,
  /*    435C */ 0x1F1F1F1FUL,
  /*    4360 */ 0x1B1F1F1FUL,
  /*    4364 */ 0x11131518UL,
  /*    4368 */ 0x0C0D0E10UL,
  /*    436C */ 0x2F87C145UL,
  /*    4370 */ 0x00000000UL,
  /*    4374 */ 0x00000000UL,
  0x01014414UL, 0x00082220UL,
  0x0102441CUL, 0x2BC32080UL,
  /*    4420 */ 0x31626258UL,
  0x01014470UL, 0x00000000UL,
  0x01054480UL, 0x71764129UL,
  /*    4484 */ 0x00000000UL,
  /*    4488 */ 0x00000000UL,
  /*    448C */ 0x00000000UL,
  /*    4490 */ 0x00000000UL,
  0x01024528UL, 0x00000000UL,
  /*    452C */ 0x123556B7UL,
  0x01018028UL, 0x00000003UL,
  0x01018038UL, 0x00000001UL,
  0x01018054UL, 0x00800005UL,
  0x01018060UL, 0x44455500UL,
  0x0103806CUL, 0x0000803FUL,
  /*    8070 */ 0x18E95287UL,
  /*    8074 */ 0x190952C7UL,
  0x11018078UL, 0x0007FFFCUL,
  0x01028080UL, 0x6777BBBFUL,
  /*    8084 */ 0x008F0BFFUL,
  0x01028090UL, 0x00069018UL,
  /*    8094 */ 0x00000018UL,
  0x0103809CUL, 0x00B22489UL,
  /*    80A0 */ 0x00823344UL,
  /*    80A4 */ 0x246D2240UL,
  0x120100DCUL, 0x05AFF02FUL,
  0x320100DCUL, 0x00400300UL,
  0x120100E0UL, 0x00007FC0UL,
  0x320100E0UL, 0x01600018UL,
  0x120100E4UL, 0x000007E4UL,
  0x320100E4UL, 0x00002008UL,
  0x12010268UL, 0x3FC00380UL,
  0x32010268UL, 0x000E2400UL,
  0x02010270UL, 0x00000000UL,
  0x0201027CUL, 0x00000000UL,
  0x02010284UL, 0x00000052UL,
  0x020102A0UL, 0x1443004DUL,
  0x120102BCUL, 0x0000F000UL,
  0x320102BCUL, 0x006207DFUL,
  0x020202CCUL, 0x0440005DUL,
  /*    02D0 */ 0x00000080UL,
  0x020102D8UL, 0x00000080UL,
  0xFFFFFFFFUL,
};

const uint32_t sl_rail_ble_phy_1Mbps_viterbi_39MHz_modemConfig[] = {
  0x0301001CUL, (uint32_t) &phyInfo_0,
  0x03050008UL, 0x000779C5UL,
  /*    000C */ 0x00077AC5UL,
  /*    0010 */ 0x00000000UL,
  /*    0014 */ 0x00000004UL,
  /*    0018 */ 0x00000000UL,
  0x00014010UL, 0x00004101UL,
  0x0001403CUL, 0x00000000UL,
  0x00014048UL, 0x0B0087A0UL,
  0x00014050UL, 0x00000000UL,
  0x0001405CUL, 0x00000000UL,
  0x00014110UL, 0x00004CFFUL,
  0x0007C054UL, 0x00302155UL,
  /*    C058 */ 0xE60D060EUL,
  /*    C05C */ 0x00000046UL,
  /*    C060 */ 0x0C0A0806UL,
  /*    C064 */ 0x0000000DUL,
  /*    C068 */ 0x0002B688UL,
  /*    C06C */ 0x00000560UL,
  0x01074050UL, 0x51500000UL,
  /*    4054 */ 0x11A00000UL,
  /*    4058 */ 0x00000010UL,
  /*    405C */ 0x86000000UL,
  /*    4060 */ 0x0081C01FUL,
  /*    4064 */ 0x20000000UL,
  /*    4068 */ 0x00000000UL,
  0x01014074UL, 0x40000000UL,
  0x01024080UL, 0x00000002UL,
  /*    4084 */ 0x00040091UL,
  0x010F4090UL, 0x000779C5UL,
  /*    4094 */ 0xE1000000UL,
  /*    4098 */ 0x00000000UL,
  /*    409C */ 0xFC02FF01UL,
  /*    40A0 */ 0x58381C15UL,
  /*    40A4 */ 0x797E7F71UL,
  /*    40A8 */ 0x24405B6CUL,
  /*    40AC */ 0x05FCFF09UL,
  /*    40B0 */ 0x07080EFCUL,
  /*    40B4 */ 0x05060607UL,
  /*    40B8 */ 0x04040505UL,
  /*    40BC */ 0x03030404UL,
  /*    40C0 */ 0x02020303UL,
  /*    40C4 */ 0x02020202UL,
  /*    40C8 */ 0x01010202UL,
  0x01014118UL, 0x00051E43UL,
  0x01014124UL, 0x8CB16000UL,
  0x0101413CUL, 0x005262F6UL,
  0x0101415CUL, 0x0000DDFFUL,
  0x01074168UL, 0x00FA53E8UL,
  /*    416C */ 0x40000000UL,
  /*    4170 */ 0x00000000UL,
  /*    4174 */ 0x00000000UL,
  /*    4178 */ 0x00000000UL,
  /*    417C */ 0x00000000UL,
  /*    4180 */ 0x00000000UL,
  0x011341E0UL, 0x00000004UL,
  /*    41E4 */ 0x0A00ABFFUL,
  /*    41E8 */ 0x000FF15CUL,
  /*    41EC */ 0x000241D3UL,
  /*    41F0 */ 0x00B1ED95UL,
  /*    41F4 */ 0x0FD87B19UL,
  /*    41F8 */ 0x04B90812UL,
  /*    41FC */ 0x1F6D1BEAUL,
  /*    4200 */ 0x0AC2B7C9UL,
  /*    4204 */ 0x003B200BUL,
  /*    4208 */ 0x00000643UL,
  /*    420C */ 0x00F36259UL,
  /*    4210 */ 0x00967E72UL,
  /*    4214 */ 0x00193DA7UL,
  /*    4218 */ 0x3BF122D0UL,
  /*    421C */ 0xA000AA00UL,
  /*    4220 */ 0x00000000UL,
  /*    4224 */ 0x00100038UL,
  /*    4228 */ 0x00000000UL,
  0x0101424CUL, 0x04030008UL,
  0x01014268UL, 0x00000001UL,
  0x01014288UL, 0x00000080UL,
  0x010144A0UL, 0x00000018UL,
  0x01094500UL, 0x40985B05UL,
  /*    4504 */ 0x94826E8EUL,
  /*    4508 */ 0x5248781BUL,
  /*    450C */ 0x00403B89UL,
  /*    4510 */ 0x800003C0UL,
  /*    4514 */ 0xC07E9258UL,
  /*    4518 */ 0xAA000000UL,
  /*    451C */ 0x00000340UL,
  /*    4520 */ 0x03050004UL,
  0x01018034UL, 0x001024C2UL,
  0x0101807CUL, 0x9AAACCCFUL,
  0x0201026CUL, 0x00000002UL,
  0x020102ACUL, 0x00800000UL,
  0xFFFFFFFFUL,
};

const uint32_t sl_rail_ble_phy_2Mbps_viterbi_39MHz_0_34_modemConfig[] = {
  0x0301001CUL, (uint32_t) &phyInfo_1,
  0x03050008UL, 0x00055C2DUL,
  /*    000C */ 0x00055D2DUL,
  /*    0010 */ 0x00000000UL,
  /*    0014 */ 0x00000004UL,
  /*    0018 */ 0x00000000UL,
  0x00014010UL, 0x00004101UL,
  0x0001403CUL, 0x00000000UL,
  0x00014048UL, 0x0B0087A0UL,
  0x00014050UL, 0x00000000UL,
  0x0001405CUL, 0x00000000UL,
  0x00014110UL, 0x00004CFFUL,
  0x0007C054UL, 0x00302155UL,
  /*    C058 */ 0xE60D060EUL,
  /*    C05C */ 0x00000046UL,
  /*    C060 */ 0x0C0A0806UL,
  /*    C064 */ 0x0000000DUL,
  /*    C068 */ 0x0002B688UL,
  /*    C06C */ 0x00000560UL,
  0x01074050UL, 0x51500000UL,
  /*    4054 */ 0x21A00000UL,
  /*    4058 */ 0x00000010UL,
  /*    405C */ 0x86000000UL,
  /*    4060 */ 0x0081C01FUL,
  /*    4064 */ 0x20000000UL,
  /*    4068 */ 0x00000000UL,
  0x01014074UL, 0x40000000UL,
  0x01024080UL, 0x00000002UL,
  /*    4084 */ 0x00080091UL,
  0x010F4090UL, 0x00055C2DUL,
  /*    4094 */ 0x61000000UL,
  /*    4098 */ 0x00000000UL,
  /*    409C */ 0x13FD01FFUL,
  /*    40A0 */ 0x6B513419UL,
  /*    40A4 */ 0x757D7F7BUL,
  /*    40A8 */ 0x1C344F67UL,
  /*    40AC */ 0x030C090FUL,
  /*    40B0 */ 0x090A0A11UL,
  /*    40B4 */ 0x06060708UL,
  /*    40B8 */ 0x04040505UL,
  /*    40BC */ 0x03030303UL,
  /*    40C0 */ 0x02020202UL,
  /*    40C4 */ 0x01010202UL,
  /*    40C8 */ 0x01010101UL,
  0x01014118UL, 0x00051E33UL,
  0x01014124UL, 0x8CB1B000UL,
  0x0101413CUL, 0x005255B6UL,
  0x0101415CUL, 0x0000DDFFUL,
  0x01074168UL, 0x00FA53E8UL,
  /*    416C */ 0x40000000UL,
  /*    4170 */ 0x00000000UL,
  /*    4174 */ 0x00000000UL,
  /*    4178 */ 0x00000000UL,
  /*    417C */ 0x00000000UL,
  /*    4180 */ 0x00000000UL,
  0x011341E0UL, 0x00000004UL,
  /*    41E4 */ 0x0A00ABFFUL,
  /*    41E8 */ 0x000FF15CUL,
  /*    41EC */ 0x000241D3UL,
  /*    41F0 */ 0x00B1ED95UL,
  /*    41F4 */ 0x0FD87B19UL,
  /*    41F8 */ 0x04B90812UL,
  /*    41FC */ 0x1F6D1BEAUL,
  /*    4200 */ 0x0A00ABFFUL,
  /*    4204 */ 0x000FF15CUL,
  /*    4208 */ 0x000241D3UL,
  /*    420C */ 0x00B1ED95UL,
  /*    4210 */ 0x0FD87B19UL,
  /*    4214 */ 0x04B90812UL,
  /*    4218 */ 0x1F6D1BEAUL,
  /*    421C */ 0x0000AA00UL,
  /*    4220 */ 0x00000000UL,
  /*    4224 */ 0x00100040UL,
  /*    4228 */ 0x00000000UL,
  0x0101424CUL, 0x04030008UL,
  0x01014268UL, 0x00000000UL,
  0x01014288UL, 0x00000080UL,
  0x010144A0UL, 0x0000001AUL,
  0x01094500UL, 0x40985B05UL,
  /*    4504 */ 0x94826E8EUL,
  /*    4508 */ 0x52483C1DUL,
  /*    450C */ 0x00403B89UL,
  /*    4510 */ 0x800003C0UL,
  /*    4514 */ 0xC07E9258UL,
  /*    4518 */ 0xAAAA0000UL,
  /*    451C */ 0x00000340UL,
  /*    4520 */ 0x03050004UL,
  0x01018034UL, 0x001047F1UL,
  0x0101807CUL, 0xCDDDEEEFUL,
  0x0201026CUL, 0x00000000UL,
  0x020102ACUL, 0x00A00000UL,
  0xFFFFFFFFUL,
};

const uint32_t sl_rail_ble_phy_125kbps_39MHz_modemConfig[] = {
  0x0301001CUL, (uint32_t) &phyInfo_2,
  0x03050008UL, 0x000259C5UL,
  /*    000C */ 0x00025AC5UL,
  /*    0010 */ 0x00000000UL,
  /*    0014 */ 0x00000024UL,
  /*    0018 */ 0x00000000UL,
  0x00014010UL, 0x00004100UL,
  0x0001403CUL, 0x00000010UL,
  0x00014048UL, 0x0B008720UL,
  0x00014050UL, 0x00000400UL,
  0x0001405CUL, 0x00000F0DUL,
  0x00014110UL, 0x0001C003UL,
  0x0007C054UL, 0x00301155UL,
  /*    C058 */ 0xE60D060EUL,
  /*    C05C */ 0x0000002AUL,
  /*    C060 */ 0x0D0C0B08UL,
  /*    C064 */ 0x0000000DUL,
  /*    C068 */ 0x0002B6D1UL,
  /*    C06C */ 0x00000580UL,
  0x01074050UL, 0x70200000UL,
  /*    4054 */ 0x11A00000UL,
  /*    4058 */ 0x00000010UL,
  /*    405C */ 0x46000030UL,
  /*    4060 */ 0x0081C81FUL,
  /*    4064 */ 0x30002000UL,
  /*    4068 */ 0x00008000UL,
  0x01014074UL, 0x00000000UL,
  0x01024080UL, 0x00000010UL,
  /*    4084 */ 0x000A0092UL,
  0x010F4090UL, 0x000259C5UL,
  /*    4094 */ 0x60000000UL,
  /*    4098 */ 0x00000000UL,
  /*    409C */ 0xFC02FF01UL,
  /*    40A0 */ 0x58381C15UL,
  /*    40A4 */ 0x797E7F71UL,
  /*    40A8 */ 0x24405B6CUL,
  /*    40AC */ 0x05FCFF09UL,
  /*    40B0 */ 0x07080EFCUL,
  /*    40B4 */ 0x05060607UL,
  /*    40B8 */ 0x04040505UL,
  /*    40BC */ 0x03030404UL,
  /*    40C0 */ 0x02020303UL,
  /*    40C4 */ 0x02020202UL,
  /*    40C8 */ 0x01010202UL,
  0x01014118UL, 0x00051E33UL,
  0x01014124UL, 0x8C2C3000UL,
  0x0101413CUL, 0x0051C049UL,
  0x0101415CUL, 0x00000000UL,
  0x01074168UL, 0x33E8D44CUL,
  /*    416C */ 0x700D1904UL,
  /*    4170 */ 0x1A140E08UL,
  /*    4174 */ 0x322C2620UL,
  /*    4178 */ 0x32103E38UL,
  /*    417C */ 0x0A987654UL,
  /*    4180 */ 0xB440691DUL,
  0x011341E0UL, 0x00000004UL,
  /*    41E4 */ 0x2CDDCFE2UL,
  /*    41E8 */ 0x003AB67EUL,
  /*    41EC */ 0x00190960UL,
  /*    41F0 */ 0x00E22251UL,
  /*    41F4 */ 0x0E51B94CUL,
  /*    41F8 */ 0x04A98239UL,
  /*    41FC */ 0x299B22D8UL,
  /*    4200 */ 0x2CDDCFE2UL,
  /*    4204 */ 0x003AB67EUL,
  /*    4208 */ 0x00190960UL,
  /*    420C */ 0x00E22251UL,
  /*    4210 */ 0x0E51B94CUL,
  /*    4214 */ 0x04A98239UL,
  /*    4218 */ 0x299B22D8UL,
  /*    421C */ 0x0000AA00UL,
  /*    4220 */ 0x00000000UL,
  /*    4224 */ 0x00100044UL,
  /*    4228 */ 0x3C3C3C3CUL,
  0x0101424CUL, 0x04000008UL,
  0x01014268UL, 0x00000000UL,
  0x01014288UL, 0x00000086UL,
  0x010144A0UL, 0x00000018UL,
  0x01094500UL, 0x40000001UL,
  /*    4504 */ 0x123556B7UL,
  /*    4508 */ 0x50087800UL,
  /*    450C */ 0x00400809UL,
  /*    4510 */ 0x800003C0UL,
  /*    4514 */ 0xC07E9258UL,
  /*    4518 */ 0x00000000UL,
  /*    451C */ 0x00000340UL,
  /*    4520 */ 0x03010000UL,
  0x01018034UL, 0x001047F1UL,
  0x0101807CUL, 0x9AAACCCFUL,
  0x0201026CUL, 0x00000000UL,
  0x020102ACUL, 0x00A00000UL,
  0xFFFFFFFFUL,
};

const uint32_t sl_rail_ble_phy_500kbps_39MHz_modemConfig[] = {
  0x0301001CUL, (uint32_t) &phyInfo_3,
  0x03050008UL, 0x000259C5UL,
  /*    000C */ 0x00025AC5UL,
  /*    0010 */ 0x00000000UL,
  /*    0014 */ 0x00000024UL,
  /*    0018 */ 0x00000000UL,
  0x00014010UL, 0x00004100UL,
  0x0001403CUL, 0x00000010UL,
  0x00014048UL, 0x0B009720UL,
  0x00014050UL, 0x00000401UL,
  0x0001405CUL, 0x00000F0DUL,
  0x00014110UL, 0x0001C003UL,
  0x0007C054UL, 0x00301155UL,
  /*    C058 */ 0xE60D060EUL,
  /*    C05C */ 0x0000002AUL,
  /*    C060 */ 0x0D0C0B08UL,
  /*    C064 */ 0x0000000DUL,
  /*    C068 */ 0x0002B6D1UL,
  /*    C06C */ 0x00000580UL,
  0x01074050UL, 0x70200000UL,
  /*    4054 */ 0x11A00000UL,
  /*    4058 */ 0x00000010UL,
  /*    405C */ 0x46000030UL,
  /*    4060 */ 0x0081C81FUL,
  /*    4064 */ 0x30002000UL,
  /*    4068 */ 0x00008000UL,
  0x01014074UL, 0x00000000UL,
  0x01024080UL, 0x00000010UL,
  /*    4084 */ 0x000A0092UL,
  0x010F4090UL, 0x000259C5UL,
  /*    4094 */ 0x60000000UL,
  /*    4098 */ 0x00000000UL,
  /*    409C */ 0xFC02FF01UL,
  /*    40A0 */ 0x58381C15UL,
  /*    40A4 */ 0x797E7F71UL,
  /*    40A8 */ 0x24405B6CUL,
  /*    40AC */ 0x05FCFF09UL,
  /*    40B0 */ 0x07080EFCUL,
  /*    40B4 */ 0x05060607UL,
  /*    40B8 */ 0x04040505UL,
  /*    40BC */ 0x03030404UL,
  /*    40C0 */ 0x02020303UL,
  /*    40C4 */ 0x02020202UL,
  /*    40C8 */ 0x01010202UL,
  0x01014118UL, 0x00051E33UL,
  0x01014124UL, 0x8C2C3000UL,
  0x0101413CUL, 0x0051C049UL,
  0x0101415CUL, 0x00000000UL,
  0x01074168UL, 0x33E8D44CUL,
  /*    416C */ 0x700D1904UL,
  /*    4170 */ 0x1C16100AUL,
  /*    4174 */ 0x342E2822UL,
  /*    4178 */ 0x3210403AUL,
  /*    417C */ 0x0A987654UL,
  /*    4180 */ 0xA480691DUL,
  0x011341E0UL, 0x00000004UL,
  /*    41E4 */ 0x2CDDCFE2UL,
  /*    41E8 */ 0x003AB67EUL,
  /*    41EC */ 0x00190960UL,
  /*    41F0 */ 0x00E22251UL,
  /*    41F4 */ 0x0E51B94CUL,
  /*    41F8 */ 0x04A98239UL,
  /*    41FC */ 0x299B22D8UL,
  /*    4200 */ 0x2CDDCFE2UL,
  /*    4204 */ 0x003AB67EUL,
  /*    4208 */ 0x00190960UL,
  /*    420C */ 0x00E22251UL,
  /*    4210 */ 0x0E51B94CUL,
  /*    4214 */ 0x04A98239UL,
  /*    4218 */ 0x299B22D8UL,
  /*    421C */ 0x0000AA00UL,
  /*    4220 */ 0x00000000UL,
  /*    4224 */ 0x00100044UL,
  /*    4228 */ 0x3C3C3C3CUL,
  0x0101424CUL, 0x04000008UL,
  0x01014268UL, 0x00000000UL,
  0x01014288UL, 0x00000086UL,
  0x010144A0UL, 0x00000018UL,
  0x01094500UL, 0x40000001UL,
  /*    4504 */ 0x123556B7UL,
  /*    4508 */ 0x50087800UL,
  /*    450C */ 0x00400809UL,
  /*    4510 */ 0x800003C0UL,
  /*    4514 */ 0xC07E9258UL,
  /*    4518 */ 0x00000000UL,
  /*    451C */ 0x00000340UL,
  /*    4520 */ 0x03010000UL,
  0x01018034UL, 0x001047F1UL,
  0x0101807CUL, 0x9AAACCCFUL,
  0x0201026CUL, 0x00000000UL,
  0x020102ACUL, 0x00A00000UL,
  0xFFFFFFFFUL,
};

const uint32_t sl_rail_ble_phy_simulscan_39MHz_modemConfig[] = {
  0x0301001CUL, (uint32_t) &phyInfo_4,
  0x03050008UL, 0x000259C5UL,
  /*    000C */ 0x00025AC5UL,
  /*    0010 */ 0x00000000UL,
  /*    0014 */ 0x00000024UL,
  /*    0018 */ 0x00000000UL,
  0x00014010UL, 0x00004100UL,
  0x0001403CUL, 0x00000000UL,
  0x00014048UL, 0x0B008720UL,
  0x00014050UL, 0x00000000UL,
  0x0001405CUL, 0x00000F0DUL,
  0x00014110UL, 0x00004CFFUL,
  0x0007C054UL, 0x00301155UL,
  /*    C058 */ 0xE60D060EUL,
  /*    C05C */ 0x0000002AUL,
  /*    C060 */ 0x0D0C0B08UL,
  /*    C064 */ 0x0000000DUL,
  /*    C068 */ 0x0002B6D1UL,
  /*    C06C */ 0x000004E0UL,
  0x01074050UL, 0x70200000UL,
  /*    4054 */ 0x11A00000UL,
  /*    4058 */ 0x00000010UL,
  /*    405C */ 0x46000000UL,
  /*    4060 */ 0x0081C01FUL,
  /*    4064 */ 0x20000000UL,
  /*    4068 */ 0x00008000UL,
  0x01014074UL, 0x00000000UL,
  0x01024080UL, 0x00000010UL,
  /*    4084 */ 0x00040091UL,
  0x010F4090UL, 0x000259C5UL,
  /*    4094 */ 0x60000000UL,
  /*    4098 */ 0x00000000UL,
  /*    409C */ 0xFC02FF01UL,
  /*    40A0 */ 0x58381C15UL,
  /*    40A4 */ 0x797E7F71UL,
  /*    40A8 */ 0x24405B6CUL,
  /*    40AC */ 0x05FCFF09UL,
  /*    40B0 */ 0x07080EFCUL,
  /*    40B4 */ 0x05060607UL,
  /*    40B8 */ 0x04040505UL,
  /*    40BC */ 0x03030404UL,
  /*    40C0 */ 0x02020303UL,
  /*    40C4 */ 0x02020202UL,
  /*    40C8 */ 0x01010202UL,
  0x01014118UL, 0x00051E33UL,
  0x01014124UL, 0x8C2C3000UL,
  0x0101413CUL, 0x0051C049UL,
  0x0101415CUL, 0x0000DDFFUL,
  0x01074168UL, 0x33E8D44CUL,
  /*    416C */ 0x500D1904UL,
  /*    4170 */ 0x1A140E08UL,
  /*    4174 */ 0x322C2620UL,
  /*    4178 */ 0x32103E38UL,
  /*    417C */ 0x0A987654UL,
  /*    4180 */ 0xB440001DUL,
  0x011341E0UL, 0x80000004UL,
  /*    41E4 */ 0x2CDDCFE2UL,
  /*    41E8 */ 0x003AB67EUL,
  /*    41EC */ 0x00190960UL,
  /*    41F0 */ 0x00E22251UL,
  /*    41F4 */ 0x0E51B94CUL,
  /*    41F8 */ 0x04A98239UL,
  /*    41FC */ 0x299B22D8UL,
  /*    4200 */ 0x2CDDCFE2UL,
  /*    4204 */ 0x003AB67EUL,
  /*    4208 */ 0x00190960UL,
  /*    420C */ 0x00E22251UL,
  /*    4210 */ 0x0E51B94CUL,
  /*    4214 */ 0x04A98239UL,
  /*    4218 */ 0x299B22D8UL,
  /*    421C */ 0x0000AA00UL,
  /*    4220 */ 0x00000000UL,
  /*    4224 */ 0x00100044UL,
  /*    4228 */ 0x3C3C3C3CUL,
  0x0101424CUL, 0x04030008UL,
  0x01014268UL, 0x00000000UL,
  0x01014288UL, 0x00000086UL,
  0x010144A0UL, 0x00000018UL,
  0x01094500UL, 0x40A05B05UL,
  /*    4504 */ 0x94826E8EUL,
  /*    4508 */ 0x52488224UL,
  /*    450C */ 0x00400809UL,
  /*    4510 */ 0x80000DC0UL,
  /*    4514 */ 0xC07E9258UL,
  /*    4518 */ 0x00000000UL,
  /*    451C */ 0x00000340UL,
  /*    4520 */ 0x03050004UL,
  0x01018034UL, 0x001047F1UL,
  0x0101807CUL, 0x9AAACCCFUL,
  0x0201026CUL, 0x00000000UL,
  0x020102ACUL, 0x00A00000UL,
  0xFFFFFFFFUL,
};

const RAIL_ChannelConfigEntry_t sl_rail_ble_phy_1Mbps_viterbi_39MHz_channels[] = {
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_1Mbps_viterbi_39MHz_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 39,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr_0,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_0,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfigEntry_t sl_rail_ble_phy_2Mbps_viterbi_39MHz_channels[] = {
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_2Mbps_viterbi_39MHz_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 34,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr_0,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_2Mbps_viterbi_39MHz_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 35,
    .channelNumberEnd = 35,
    .maxPower = 182,
    .attr = &channelConfigEntryAttr_0,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_2Mbps_viterbi_39MHz_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 36,
    .channelNumberEnd = 36,
    .maxPower = 157,
    .attr = &channelConfigEntryAttr_0,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_2Mbps_viterbi_39MHz_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 37,
    .channelNumberEnd = 37,
    .maxPower = 125,
    .attr = &channelConfigEntryAttr_0,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_2Mbps_viterbi_39MHz_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 38,
    .channelNumberEnd = 38,
    .maxPower = 83,
    .attr = &channelConfigEntryAttr_0,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_2Mbps_viterbi_39MHz_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 39,
    .channelNumberEnd = 39,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr_0,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfigEntry_t sl_rail_ble_phy_125kbps_39MHz_channels[] = {
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_125kbps_39MHz_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 39,
    .maxPower = 140,
    .attr = &channelConfigEntryAttr_1,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_2,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfigEntry_t sl_rail_ble_phy_500kbps_39MHz_channels[] = {
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_500kbps_39MHz_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 39,
    .maxPower = 140,
    .attr = &channelConfigEntryAttr_1,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_3,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfigEntry_t sl_rail_ble_phy_simulscan_39MHz_channels[] = {
  {
    .phyConfigDeltaAdd = sl_rail_ble_phy_simulscan_39MHz_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 39,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr_1,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_4,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfig_t sl_rail_ble_phy_1Mbps_viterbi_39MHz_channelConfig = {
  .phyConfigBase = sl_rail_ble_phy_1Mbps_viterbi_39MHz_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = sl_rail_ble_phy_1Mbps_viterbi_39MHz_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 39000000UL,
};

const RAIL_ChannelConfig_t sl_rail_ble_phy_2Mbps_viterbi_39MHz_channelConfig = {
  .phyConfigBase = sl_rail_ble_phy_1Mbps_viterbi_39MHz_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = sl_rail_ble_phy_2Mbps_viterbi_39MHz_channels,
  .length = 6U,
  .signature = 0UL,
  .xtalFrequencyHz = 39000000UL,
};

const RAIL_ChannelConfig_t sl_rail_ble_phy_125kbps_39MHz_channelConfig = {
  .phyConfigBase = sl_rail_ble_phy_1Mbps_viterbi_39MHz_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = sl_rail_ble_phy_125kbps_39MHz_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 39000000UL,
};

const RAIL_ChannelConfig_t sl_rail_ble_phy_500kbps_39MHz_channelConfig = {
  .phyConfigBase = sl_rail_ble_phy_1Mbps_viterbi_39MHz_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = sl_rail_ble_phy_500kbps_39MHz_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 39000000UL,
};

const RAIL_ChannelConfig_t sl_rail_ble_phy_simulscan_39MHz_channelConfig = {
  .phyConfigBase = sl_rail_ble_phy_1Mbps_viterbi_39MHz_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = sl_rail_ble_phy_simulscan_39MHz_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 39000000UL,
};
