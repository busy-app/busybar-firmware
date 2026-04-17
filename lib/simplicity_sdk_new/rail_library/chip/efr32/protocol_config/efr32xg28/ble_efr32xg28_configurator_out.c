/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config  -  DO NOT EDIT
 *   Radio Configurator Version: 2504.4.1
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
#include "ble_efr32xg28_configurator_out.h"

static const uint8_t irCalConfig[] = {
  20, 41, 2, 0, 0, 57, 19, 0, 0, 0, 1, 0, 2, 100, 0, 1, 1, 47, 0, 0, 7
};

static const int32_t timingConfig_0[] = {
  50000, 10010, 0, 0
};

static const int32_t timingConfig_1[] = {
  14500, 4300, 0, 0
};

__ALIGNED(4) static const uint8_t hfxoRetimingConfigEntries[] = {
  1, 0, 0, 0, 0xc0, 0x17, 0x53, 0x02, 6, 12, 0, 0, 0xa0, 0x08, 0, 0, 0, 0, 0x58, 0x09, 1, 4, 7, 6, 0x10, 0x0a, 1, 4, 7, 7, 0xc8, 0x0a, 0, 4, 8, 7, 0x80, 0x0b, 0, 4, 8, 8, 0x38, 0x0c, 0, 4, 9, 8
};

#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
__ALIGNED(4) static const uint8_t stackInfo_0[2] = { 0x03, 0x01 };
__ALIGNED(4) static const uint8_t stackInfo_1[2] = { 0x03, 0x02 };
#endif // RADIO_CONFIG_ENABLE_STACK_INFO

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

static const uint32_t phyInfo_0[] = {
  20UL,
  0x00400000UL, // 64.0
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig_0,
  0x00000000UL,
  16UL,
  128000000UL,
  1000000UL,
  0x00FD0101UL,
  0x03501F81UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  999990UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_1[] = {
  20UL,
  0x00400000UL, // 64.0
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig_1,
  0x00000000UL,
  8UL,
  256000000UL,
  2000000UL,
  0x00FD0101UL,
  0x02503F03UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  1999958UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

const uint32_t ble_viterbi1M_modemConfigBase[] = {
  0x0002400CUL, 0x00018101UL,
  /*    4010 */ 0x00004100UL,
  0x00024020UL, 0x00000000UL,
  /*    4024 */ 0x00000001UL,
  0x00074030UL, 0x00000000UL,
  /*    4034 */ 0x00000000UL,
  /*    4038 */ 0x00000000UL,
  /*    403C */ 0x00000000UL,
  /*    4040 */ 0x00000000UL,
  /*    4044 */ 0x00006000UL,
  /*    4048 */ 0x000007A0UL,
  0x00014050UL, 0x00000000UL,
  0x0002405CUL, 0x00000000UL,
  /*    4060 */ 0x00000101UL,
  0x000140A8UL, 0x00000007UL,
  0x000440BCUL, 0x00000000UL,
  /*    40C0 */ 0x00000000UL,
  /*    40C4 */ 0x00000000UL,
  /*    40C8 */ 0x00000000UL,
  0x00044108UL, 0x00004CFFUL,
  /*    410C */ 0x00004CFFUL,
  /*    4110 */ 0x00004DFFUL,
  /*    4114 */ 0x00004DFFUL,
  0x1001C024UL, 0x000000FFUL,
  0x3001C024UL, 0x00001300UL,
  0x0008C028UL, 0x03B380ECUL,
  /*    C02C */ 0x51407543UL,
  /*    C030 */ 0xF8000FA0UL,
  /*    C034 */ 0x00004000UL,
  /*    C038 */ 0x0007AAA8UL,
  /*    C03C */ 0x00000000UL,
  /*    C040 */ 0x00000000UL,
  /*    C044 */ 0x00000000UL,
  0x0003C060UL, 0x07060504UL,
  /*    C064 */ 0x00000008UL,
  /*    C068 */ 0x0002B6D1UL,
  0x000DC070UL, 0x00000EBAUL,
  /*    C074 */ 0x00660000UL,
  /*    C078 */ 0x0293016DUL,
  /*    C07C */ 0x067A043CUL,
  /*    C080 */ 0x0C6C0956UL,
  /*    C084 */ 0x165E10C5UL,
  /*    C088 */ 0x26A61D67UL,
  /*    C08C */ 0x3FFF3278UL,
  /*    C090 */ 0x3FFF3FFFUL,
  /*    C094 */ 0x3FFF3FFFUL,
  /*    C098 */ 0x3FFF3FFFUL,
  /*    C09C */ 0x3FFF3FFFUL,
  /*    C0A0 */ 0x00003FFFUL,
  0x0005C0A8UL, 0x15724BBDUL,
  /*    C0AC */ 0x0518A311UL,
  /*    C0B0 */ 0x76543210UL,
  /*    C0B4 */ 0x00000A98UL,
  /*    C0B8 */ 0x00000000UL,
  0x0004C0CCUL, 0x00000001UL,
  /*    C0D0 */ 0x00000000UL,
  /*    C0D4 */ 0x000A0001UL,
  /*    C0D8 */ 0x00280001UL,
  0x01010008UL, 0x00000708UL,
  0x01010018UL, 0x00AAAAAAUL,
  0x01010020UL, 0x00DA6000UL,
  0x01014040UL, 0x51B00000UL,
  0x01084048UL, 0x00000010UL,
  /*    404C */ 0x86000000UL,
  /*    4050 */ 0x0081C01FUL,
  /*    4054 */ 0x20000000UL,
  /*    4058 */ 0x00000000UL,
  /*    405C */ 0x03000000UL,
  /*    4060 */ 0x20000000UL,
  /*    4064 */ 0x40000000UL,
  0x0102406CUL, 0x00000840UL,
  /*    4070 */ 0x00000002UL,
  0x01044078UL, 0x71764129UL,
  /*    407C */ 0x00000000UL,
  /*    4080 */ 0x000C008CUL,
  /*    4084 */ 0x00000000UL,
  0x0102408CUL, 0x60000000UL,
  /*    4090 */ 0x00000000UL,
  0x010740BCUL, 0x00000000UL,
  /*    40C0 */ 0x00000000UL,
  /*    40C4 */ 0x00000000UL,
  /*    40C8 */ 0x00000000UL,
  /*    40CC */ 0x00000000UL,
  /*    40D0 */ 0x00000000UL,
  /*    40D4 */ 0x00000000UL,
  0x110140E0UL, 0x000001F8UL,
  0x310140E0UL, 0x00000201UL,
  0x01024110UL, 0x00051E33UL,
  /*    4114 */ 0x00000000UL,
  0x01024120UL, 0x00000000UL,
  /*    4124 */ 0x078304FFUL,
  0x0104412CUL, 0x0C6606FFUL,
  /*    4130 */ 0x078304FFUL,
  /*    4134 */ 0x03FF1388UL,
  /*    4138 */ 0xF00A20BCUL,
  0x01024140UL, 0x40985B05UL,
  /*    4144 */ 0x94826E8EUL,
  0x0102414CUL, 0x00403B89UL,
  /*    4150 */ 0x800003C0UL,
  0x01024158UL, 0x00000000UL,
  /*    415C */ 0x0000DDFFUL,
  0x01014164UL, 0x0000010CUL,
  0x010B416CUL, 0x40000000UL,
  /*    4170 */ 0x00000000UL,
  /*    4174 */ 0x00000000UL,
  /*    4178 */ 0x00000000UL,
  /*    417C */ 0x00000000UL,
  /*    4180 */ 0x00000000UL,
  /*    4184 */ 0x00000101UL,
  /*    4188 */ 0x00000000UL,
  /*    418C */ 0x00000000UL,
  /*    4190 */ 0x00000000UL,
  /*    4194 */ 0x00000000UL,
  0x010241A4UL, 0x00000000UL,
  /*    41A8 */ 0x00000000UL,
  0x010841B0UL, 0x00000000UL,
  /*    41B4 */ 0xC03E9258UL,
  /*    41B8 */ 0x00000000UL,
  /*    41BC */ 0x00000000UL,
  /*    41C0 */ 0x003C0000UL,
  /*    41C4 */ 0x0006AAAAUL,
  /*    41C8 */ 0x00000000UL,
  /*    41CC */ 0x00000000UL,
  0x010341D4UL, 0x000000D0UL,
  /*    41D8 */ 0x00120004UL,
  /*    41DC */ 0x00000000UL,
  0x011041E4UL, 0x0A00ABFFUL,
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
  /*    421C */ 0x00000000UL,
  /*    4220 */ 0x00000000UL,
  0x01044228UL, 0x00000000UL,
  /*    422C */ 0x40001860UL,
  /*    4230 */ 0x00000000UL,
  /*    4234 */ 0x00000000UL,
  0x0101423CUL, 0x00000000UL,
  0x01014244UL, 0x00000014UL,
  0x0101424CUL, 0x04030008UL,
  0x01014254UL, 0x00000080UL,
  0x010F4330UL, 0x00000000UL,
  /*    4334 */ 0x00000000UL,
  /*    4338 */ 0x00002000UL,
  /*    433C */ 0x00000000UL,
  /*    4340 */ 0x00000000UL,
  /*    4344 */ 0x00000000UL,
  /*    4348 */ 0x00000000UL,
  /*    434C */ 0x00000000UL,
  /*    4350 */ 0x00000000UL,
  /*    4354 */ 0x00000000UL,
  /*    4358 */ 0x00000000UL,
  /*    435C */ 0x38000000UL,
  /*    4360 */ 0x00000000UL,
  /*    4364 */ 0x00000000UL,
  /*    4368 */ 0x58FF0000UL,
  0x01018010UL, 0x00000003UL,
  0x0101803CUL, 0x00000001UL,
  0x010180A0UL, 0x00037870UL,
  0x010280B0UL, 0x02000300UL,
  /*    80B4 */ 0x01000037UL,
  0x02030098UL, 0x00000000UL,
  /*    009C */ 0x04000C00UL,
  /*    00A0 */ 0x0000044CUL,
  0x020200D8UL, 0xAA400005UL,
  /*    00DC */ 0x00000188UL,
  0x020100F0UL, 0x0000052BUL,
  0x02010100UL, 0x00000118UL,
  0x12010104UL, 0x00000000UL,
  0x32010104UL, 0x00000118UL,
  0x12010110UL, 0x000FFF00UL,
  0x32010110UL, 0x31000002UL,
  0x1201012CUL, 0x001FFC00UL,
  0x3201012CUL, 0x008000A9UL,
  0x02010140UL, 0x0000003FUL,
  0x12010150UL, 0x0000C000UL,
  0x32010150UL, 0x00510060UL,
  0x02020168UL, 0x00060000UL,
  /*    016C */ 0x00012000UL,
  0x02010174UL, 0x0C1B8169UL,
  0x12010178UL, 0x001C0000UL,
  0x32010178UL, 0xCFE00440UL,
  0x12010180UL, 0x00000779UL,
  0x32010180UL, 0x00000002UL,
  0x02010188UL, 0x00000050UL,
  0x020101B0UL, 0x00000000UL,
  0x03017FF8UL, 0x00000000UL,
  0xFFFFFFFFUL,
};

const uint32_t ble_viterbi1M_modemConfig[] = {
  0x03017FFCUL, (uint32_t) &phyInfo_0,
  0x1001C020UL, 0x0007F800UL,
  0x3001C020UL, 0x002802F5UL,
  0x0003C054UL, 0x00301141UL,
  /*    C058 */ 0xE609000EUL,
  /*    C05C */ 0x0000002DUL,
  0x0001C06CUL, 0x00003564UL,
  0x01014044UL, 0x11A00000UL,
  0x01014068UL, 0x00F804B9UL,
  0x01014074UL, 0x00040092UL,
  0x01014088UL, 0x001003B9UL,
  0x010A4094UL, 0x0D050200UL,
  /*    4098 */ 0x53422F1BUL,
  /*    409C */ 0x5F686962UL,
  /*    40A0 */ 0x1A2A3D51UL,
  /*    40A4 */ 0x0302040CUL,
  /*    40A8 */ 0x03030404UL,
  /*    40AC */ 0x01020202UL,
  /*    40B0 */ 0x00010101UL,
  /*    40B4 */ 0x00000000UL,
  /*    40B8 */ 0x00000000UL,
  0x0101411CUL, 0x8CB15000UL,
  0x0101413CUL, 0x00520BA8UL,
  0x01014148UL, 0x5248781CUL,
  0x010141D0UL, 0xAA000000UL,
  0x01014224UL, 0x00000038UL,
  0x01018038UL, 0x00101F81UL,
  0x010180A4UL, 0x000000D0UL,
  0x110180A8UL, 0x000001F6UL,
  0x310180A8UL, 0x01CB4201UL,
  0x110180ACUL, 0x000001F6UL,
  0x310180ACUL, 0x00FD3E01UL,
  0x120100ECUL, 0x00000FE0UL,
  0x320100ECUL, 0x5151200CUL,
  0x0201018CUL, 0x00002000UL,
  0x120101ACUL, 0x000001F8UL,
  0x320101ACUL, 0x00010004UL,
  0xFFFFFFFFUL,
};

const uint32_t ble_viterbi2M_0_34_modemConfig[] = {
  0x03017FFCUL, (uint32_t) &phyInfo_1,
  0x1001C020UL, 0x0007F800UL,
  0x3001C020UL, 0x002802FEUL,
  0x0003C054UL, 0x00302141UL,
  /*    C058 */ 0xE60F0010UL,
  /*    C05C */ 0x00000030UL,
  0x0001C06CUL, 0x00000560UL,
  0x01014044UL, 0x21A00000UL,
  0x01014068UL, 0x00F00249UL,
  0x01014074UL, 0x00080092UL,
  0x01014088UL, 0x001003DAUL,
  0x010A4094UL, 0x11070201UL,
  /*    4098 */ 0x5A4B3721UL,
  /*    409C */ 0x54626967UL,
  /*    40A0 */ 0x05152940UL,
  /*    40A4 */ 0x00000000UL,
  /*    40A8 */ 0x04030201UL,
  /*    40AC */ 0x03030404UL,
  /*    40B0 */ 0x02030303UL,
  /*    40B4 */ 0x02020202UL,
  /*    40B8 */ 0x01010102UL,
  0x0101411CUL, 0x8CB1A000UL,
  0x0101413CUL, 0x00520B8FUL,
  0x01014148UL, 0x52483C1CUL,
  0x010141D0UL, 0xAAAA0000UL,
  0x01014224UL, 0x00000040UL,
  0x01018038UL, 0x00103F03UL,
  0x010180A4UL, 0x0000C0D5UL,
  0x110180A8UL, 0x000001F6UL,
  0x310180A8UL, 0x01CB4201UL,
  0x110180ACUL, 0x000001F6UL,
  0x310180ACUL, 0x008D2201UL,
  0x120100ECUL, 0x00000FE0UL,
  0x320100ECUL, 0x1151200CUL,
  0x0201018CUL, 0x00000000UL,
  0x120101ACUL, 0x000001F8UL,
  0x320101ACUL, 0x00011004UL,
  0xFFFFFFFFUL,
};

const RAIL_ChannelConfigEntry_t ble_viterbi1M_channels[] = {
  {
    .phyConfigDeltaAdd = ble_viterbi1M_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 39,
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

const RAIL_ChannelConfigEntry_t ble_viterbi2M_channels[] = {
  {
    .phyConfigDeltaAdd = ble_viterbi2M_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 34,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = ble_viterbi2M_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 35,
    .channelNumberEnd = 35,
    .maxPower = 182,
    .attr = &channelConfigEntryAttr,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = ble_viterbi2M_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 36,
    .channelNumberEnd = 36,
    .maxPower = 157,
    .attr = &channelConfigEntryAttr,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = ble_viterbi2M_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 37,
    .channelNumberEnd = 37,
    .maxPower = 125,
    .attr = &channelConfigEntryAttr,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = ble_viterbi2M_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 38,
    .channelNumberEnd = 38,
    .maxPower = 83,
    .attr = &channelConfigEntryAttr,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
  {
    .phyConfigDeltaAdd = ble_viterbi2M_0_34_modemConfig,
    .baseFrequency = 2402000000,
    .channelSpacing = 2000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 39,
    .channelNumberEnd = 39,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfig_t ble_viterbi1M_channelConfig = {
  .phyConfigBase = ble_viterbi1M_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ble_viterbi1M_channels,
  .length = 1U,
  .signature = 1665503061UL,
  .xtalFrequencyHz = 39000000UL,
};

const RAIL_ChannelConfig_t ble_viterbi2M_channelConfig = {
  .phyConfigBase = ble_viterbi1M_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ble_viterbi2M_channels,
  .length = 6U,
  .signature = 211303326UL,
  .xtalFrequencyHz = 39000000UL,
};
