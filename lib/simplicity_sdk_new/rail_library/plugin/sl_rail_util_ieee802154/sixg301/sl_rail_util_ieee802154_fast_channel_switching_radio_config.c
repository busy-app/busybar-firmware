/***************************************************************************//**
 * @file
 * @brief
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
#include "sl_rail_util_ieee802154_fast_channel_switching_radio_config.h"

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
static const uint8_t irCalConfig[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0
};

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
static const int32_t timingConfig[] = {
  6625, 6625, 500, 0
};

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
__ALIGNED(4) static const uint8_t hfxoRetimingConfigEntries[] = {
  2, 0, 0, 0, 0x00, 0xf0, 0x49, 0x02, 6, 20, 0, 0, 0x00, 0xe0, 0x93, 0x04, 5, 56, 0, 0, 0xa0, 0x08, 0, 0, 0, 0, 0x58, 0x09, 1, 4, 7, 6, 0x10, 0x0a, 1, 4, 7, 7, 0xc8, 0x0a, 0, 4, 8, 7, 0x80, 0x0b, 0, 4, 8, 8, 0x38, 0x0c, 0, 4, 9, 8, 0x61, 0x08, 0, 0, 0, 0, 0x68, 0x08, 0, 0, 0, 0, 0xc7, 0x09, 1, 4, 4, 3, 0x2c, 0x0b, 1, 4, 4, 4, 0x92, 0x0c, 1, 4, 5, 4
};

#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
__ALIGNED(4) static const uint8_t stackInfo_0[2] = { 0x05, 0x00 };
#endif // RADIO_CONFIG_ENABLE_STACK_INFO

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
static RAIL_ChannelConfigEntryAttr_t channelConfigEntryAttr = {
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

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
static const uint32_t phyInfo[] = {
  20UL,
  0x00800000UL, // 128.0
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig,
  0x00000000UL,
  0UL,
  32000000UL,
  1999988UL,
  0x00F02004UL,
  0x02504911UL,
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

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
const uint32_t RAIL_IEEE802154_Phy2ChFs_modemConfigBase[] = {
  0x03080000UL, 0x0000803FUL,
  /*    0004 */ 0x0001007EUL,
  /*    0008 */ 0x0001E201UL,
  /*    000C */ 0x0001E301UL,
  /*    0010 */ 0x00000000UL,
  /*    0014 */ 0x00000440UL,
  /*    0018 */ 0x00000000UL,
  /*    001C */ (uint32_t) &phyInfo,
  0x0002400CUL, 0x00148001UL,
  /*    4010 */ 0x0000407FUL,
  0x00024020UL, 0x00000000UL,
  /*    4024 */ 0x00000000UL,
  0x00074030UL, 0x00000825UL,
  /*    4034 */ 0x00000100UL,
  /*    4038 */ 0x00000000UL,
  /*    403C */ 0x00010340UL,
  /*    4040 */ 0x00000000UL,
  /*    4044 */ 0xA000E000UL,
  /*    4048 */ 0x0B0087A0UL,
  0x00014050UL, 0x00000000UL,
  0x0002405CUL, 0x00000D0FUL,
  /*    4060 */ 0x00000101UL,
  0x000140A8UL, 0x00000007UL,
  0x000440BCUL, 0x00000000UL,
  /*    40C0 */ 0x00000000UL,
  /*    40C4 */ 0x00000000UL,
  /*    40C8 */ 0x00000000UL,
  0x00044110UL, 0x00004000UL,
  /*    4114 */ 0x00004CFFUL,
  /*    4118 */ 0x00004100UL,
  /*    411C */ 0x00004DFFUL,
  0x1001C020UL, 0x0007F800UL,
  0x3001C020UL, 0x012802FEUL,
  0x1001C024UL, 0x000000FFUL,
  0x3001C024UL, 0x04001300UL,
  0x0008C028UL, 0x03B380ECUL,
  /*    C02C */ 0x51407543UL,
  /*    C030 */ 0xB8000FA0UL,
  /*    C034 */ 0x00004000UL,
  /*    C038 */ 0x0007AAA8UL,
  /*    C03C */ 0x01000402UL,
  /*    C040 */ 0x00000000UL,
  /*    C044 */ 0x00000000UL,
  0x0010C054UL, 0x00303155UL,
  /*    C058 */ 0xE608040EUL,
  /*    C05C */ 0x0000002AUL,
  /*    C060 */ 0x07060504UL,
  /*    C064 */ 0x00000008UL,
  /*    C068 */ 0x0002D6D1UL,
  /*    C06C */ 0x000005A0UL,
  /*    C070 */ 0x0000EFB9UL,
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
  0x01010008UL, 0x00000704UL,
  0x01010018UL, 0x00000000UL,
  0x01010020UL, 0x00008408UL,
  0x01234050UL, 0x31B00000UL,
  /*    4054 */ 0x21A00000UL,
  /*    4058 */ 0x00000010UL,
  /*    405C */ 0x0413F920UL,
  /*    4060 */ 0x0052C007UL,
  /*    4064 */ 0x00000000UL,
  /*    4068 */ 0x00000000UL,
  /*    406C */ 0x03000000UL,
  /*    4070 */ 0x20000000UL,
  /*    4074 */ 0x00000000UL,
  /*    4078 */ 0x0000D555UL,
  /*    407C */ 0x00000840UL,
  /*    4080 */ 0x00000002UL,
  /*    4084 */ 0x00080030UL,
  /*    4088 */ 0x0000114BUL,
  /*    408C */ 0x744AC39BUL,
  /*    4090 */ 0x0001E201UL,
  /*    4094 */ 0x60000000UL,
  /*    4098 */ 0x00000000UL,
  /*    409C */ 0x30100101UL,
  /*    40A0 */ 0x7F7F7050UL,
  /*    40A4 */ 0x70777770UL,
  /*    40A8 */ 0x1E334C61UL,
  /*    40AC */ 0x0102060FUL,
  /*    40B0 */ 0x00000000UL,
  /*    40B4 */ 0x00000000UL,
  /*    40B8 */ 0x00000000UL,
  /*    40BC */ 0x00000000UL,
  /*    40C0 */ 0x00000000UL,
  /*    40C4 */ 0x00000000UL,
  /*    40C8 */ 0x00000000UL,
  /*    40CC */ 0x00000000UL,
  /*    40D0 */ 0x00000000UL,
  /*    40D4 */ 0x00000000UL,
  /*    40D8 */ 0x00000000UL,
  0x01024118UL, 0x00051E33UL,
  /*    411C */ 0x00000000UL,
  0x01024124UL, 0x8CB330C3UL,
  /*    4128 */ 0x00000000UL,
  0x01014130UL, 0x00000000UL,
  0x0101413CUL, 0x145254FAUL,
  0x01024158UL, 0x00000001UL,
  /*    415C */ 0x00001003UL,
  0x010D4164UL, 0x0000010CUL,
  /*    4168 */ 0x00FA53E8UL,
  /*    416C */ 0x40600000UL,
  /*    4170 */ 0x00000000UL,
  /*    4174 */ 0x00000000UL,
  /*    4178 */ 0x00000000UL,
  /*    417C */ 0x00000000UL,
  /*    4180 */ 0x00000000UL,
  /*    4184 */ 0x0007D101UL,
  /*    4188 */ 0x0840D400UL,
  /*    418C */ 0x00000000UL,
  /*    4190 */ 0x00000000UL,
  /*    4194 */ 0x00000000UL,
  0x010241A4UL, 0x00000000UL,
  /*    41A8 */ 0x00000000UL,
  0x010341BCUL, 0x00000000UL,
  /*    41C0 */ 0x003C0000UL,
  /*    41C4 */ 0x0006AAAAUL,
  0x011641E0UL, 0x00012C08UL,
  /*    41E4 */ 0x0A00ABFFUL,
  /*    41E8 */ 0x000FF15CUL,
  /*    41EC */ 0x000241D3UL,
  /*    41F0 */ 0x00B1ED95UL,
  /*    41F4 */ 0x0FD87B19UL,
  /*    41F8 */ 0x04B90812UL,
  /*    41FC */ 0x1F6D1BEAUL,
  /*    4200 */ 0x09B15C1EUL,
  /*    4204 */ 0x000170A9UL,
  /*    4208 */ 0x006986F4UL,
  /*    420C */ 0x00B92BA8UL,
  /*    4210 */ 0x00ECFE18UL,
  /*    4214 */ 0x05088BACUL,
  /*    4218 */ 0x1D0C1A9DUL,
  /*    421C */ 0x8000AA00UL,
  /*    4220 */ 0x00000000UL,
  /*    4224 */ 0x40100028UL,
  /*    4228 */ 0x3675EE07UL,
  /*    422C */ 0x40001860UL,
  /*    4230 */ 0x00600001UL,
  /*    4234 */ 0x002C0000UL,
  0x0101423CUL, 0x00000000UL,
  0x01064244UL, 0x00000014UL,
  /*    4248 */ 0x00000000UL,
  /*    424C */ 0x04080003UL,
  /*    4250 */ 0x00000000UL,
  /*    4254 */ 0x00000000UL,
  /*    4258 */ 0x00000000UL,
  0x01024264UL, 0x00000000UL,
  /*    4268 */ 0x00000000UL,
  0x01014288UL, 0x00000080UL,
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
  0x01054414UL, 0x70196221UL,
  /*    4418 */ 0x00000000UL,
  /*    441C */ 0x2BC2E480UL,
  /*    4420 */ 0x635B59F4UL,
  /*    4424 */ 0x30000578UL,
  0x01014470UL, 0x00000001UL,
  0x01054480UL, 0x000000A7UL,
  /*    4484 */ 0x0000002FUL,
  /*    4488 */ 0x00000000UL,
  /*    448C */ 0x00000000UL,
  /*    4490 */ 0x40000000UL,
  0x010144A0UL, 0x0000001AUL,
  0x01094500UL, 0x409F6100UL,
  /*    4504 */ 0xC18DE12AUL,
  /*    4508 */ 0x4248231FUL,
  /*    450C */ 0x00403B89UL,
  /*    4510 */ 0x00000000UL,
  /*    4514 */ 0xC07F9258UL,
  /*    4518 */ 0x55550000UL,
  /*    451C */ 0x00000340UL,
  /*    4520 */ 0x83050004UL,
  0x01024528UL, 0x00000000UL,
  /*    452C */ 0x123556B7UL,
  0x01018028UL, 0x00000003UL,
  0x01028034UL, 0x00104911UL,
  /*    8038 */ 0x00000001UL,
  0x01018054UL, 0x00800005UL,
  0x01018060UL, 0x44455500UL,
  0x0103806CUL, 0x0000803FUL,
  /*    8070 */ 0x18E95287UL,
  /*    8074 */ 0x190952C7UL,
  0x11018078UL, 0x0007FFFCUL,
  0x0103807CUL, 0xBDDDEEEFUL,
  /*    8080 */ 0x8AAACCCFUL,
  /*    8084 */ 0x008F0BFFUL,
  0x01028090UL, 0x00069098UL,
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
  0x0202026CUL, 0x00000000UL,
  /*    0270 */ 0x00000000UL,
  0x0201027CUL, 0x00000000UL,
  0x02010284UL, 0x00000052UL,
  0x020102A0UL, 0x1443004DUL,
  0x020102ACUL, 0x00A00000UL,
  0x120102BCUL, 0x0000F000UL,
  0x320102BCUL, 0x006207DFUL,
  0x020202CCUL, 0x0440005DUL,
  /*    02D0 */ 0x00000080UL,
  0x020102D8UL, 0x00000080UL,
  0xFFFFFFFFUL,
};

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
const RAIL_ChannelConfigEntry_t RAIL_IEEE802154_Phy2ChFs_channels[] = {
  {
    .phyConfigDeltaAdd = NULL,
    .baseFrequency = 2405000000,
    .channelSpacing = 5000000,
    .physicalChannelOffset = 11,
    .channelNumberStart = 11,
    .channelNumberEnd = 26,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_0,
#endif
    .alternatePhy = NULL,
  },
};

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
const RAIL_ChannelConfig_t RAIL_IEEE802154_Phy2ChFs_channelConfig = {
  .phyConfigBase = RAIL_IEEE802154_Phy2ChFs_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = RAIL_IEEE802154_Phy2ChFs_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_UTIL_IEEE802154_PHY_SELECT, SL_CODE_CLASS_TIME_CRITICAL)
const RAIL_ChannelConfig_t * const RAIL_IEEE802154_Phy2p4GHzRxChSwitching =
  &RAIL_IEEE802154_Phy2ChFs_channelConfig;
