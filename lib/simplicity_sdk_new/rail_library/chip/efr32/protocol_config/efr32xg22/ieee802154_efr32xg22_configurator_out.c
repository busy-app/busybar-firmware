/***************************************************************************//**
 * @brief RAIL Configuration
 * @details
 *   WARNING: Auto-Generated Radio Config  -  DO NOT EDIT
 *   Radio Configurator Version: 2504.4.1 (2ab5c852a7)
 *   RAIL Adapter Version: 2.4.33 (330b556b3a)
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
#include "ieee802154_efr32xg22_configurator_out.h"

static const uint8_t irCalConfig[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int32_t timingConfig[] = {
  6125, 6125, 0, 0
};

__ALIGNED(4) static const uint8_t hfxoRetimingConfigEntries[] = {
  2, 0, 0, 0, 0x00, 0xf0, 0x49, 0x02, 6, 20, 0, 0, 0x00, 0xe0, 0x93, 0x04, 5, 56, 0, 0, 0xa0, 0x08, 0, 0, 0, 0, 0x58, 0x09, 1, 4, 7, 6, 0x10, 0x0a, 1, 4, 7, 7, 0xc8, 0x0a, 0, 4, 8, 7, 0x80, 0x0b, 0, 4, 8, 8, 0x38, 0x0c, 0, 4, 9, 8, 0x61, 0x08, 0, 0, 0, 0, 0x68, 0x08, 0, 0, 0, 0, 0xc7, 0x09, 1, 4, 4, 3, 0x2c, 0x0b, 1, 4, 4, 4, 0x92, 0x0c, 1, 4, 5, 4
};

#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
__ALIGNED(4) static const uint8_t stackInfo_0[2] = { 0x05, 0x01 };
#endif // RADIO_CONFIG_ENABLE_STACK_INFO

static RAIL_ChannelConfigEntryAttr_t channelConfigEntryAttr = {
#if RAIL_SUPPORTS_OFDM_PA
  {
    { 0xFFFFFFFFUL },
    { 0xFFFFFFFFUL, 0xFFFFFFFFUL }
  }
#else // RAIL_SUPPORTS_OFDM_PA
  { 0xFFFFFFFFUL },
#endif // RAIL_SUPPORTS_OFDM_PA
};

static const uint32_t phyInfo[] = {
  20UL,
  0x00666666UL, // 102.4
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig,
  0x00000000UL,
  10099200UL,
  40000000UL,
  2000000UL,
  0x00F52004UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  2000049UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

const uint32_t ieee802154_2p4_antdiv_modemConfigBase[] = {
  0x0002400CUL, 0x00148001UL, /* Write: FRC.DFLCTRL */
  /*    4010 */ 0x0000407FUL, /* FRC.MAXLENGTH */
  0x00024020UL, 0x00000000UL, /* Write: FRC.WCNTCMP0 */
  /*    4024 */ 0x00000000UL, /* FRC.WCNTCMP1 */
  0x00074030UL, 0x00000000UL, /* Write: FRC.WHITECTRL */
  /*    4034 */ 0x00000000UL, /* FRC.WHITEPOLY */
  /*    4038 */ 0x00000000UL, /* FRC.WHITEINIT */
  /*    403C */ 0x00000000UL, /* FRC.FECCTRL */
  /*    4040 */ 0x00000000UL, /* FRC.BLOCKRAMADDR */
  /*    4044 */ 0x00004000UL, /* FRC.CONVRAMADDR */
  /*    4048 */ 0x000007A0UL, /* FRC.CTRL */
  0x00014050UL, 0x00000000UL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000000UL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000000UL, /* FRC.PUNCTCTRL */
  0x000140A8UL, 0x00000007UL, /* Write: FRC.AUTOCG */
  0x000440BCUL, 0x00000000UL, /* Write: FRC.WCNTCMP3 */
  /*    40C0 */ 0x00000000UL, /* FRC.BOICTRL */
  /*    40C4 */ 0x00000000UL, /* FRC.DSLCTRL */
  /*    40C8 */ 0x00000000UL, /* FRC.WCNTCMP4 */
  0x00044104UL, 0x00004000UL, /* Write: FRC.FCD0 */
  /*    4108 */ 0x00004CFFUL, /* FRC.FCD1 */
  /*    410C */ 0x00005DFFUL, /* FRC.FCD2 */
  /*    4110 */ 0x00004DFFUL, /* FRC.FCD3 */
  0x1001C020UL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C020UL, 0x000802F5UL, /* OR: AGC.CTRL0 */
  0x1001C024UL, 0x000000FFUL, /* AND: AGC.CTRL1 */
  0x3001C024UL, 0x00001800UL, /* OR: AGC.CTRL1 */
  0x0007C028UL, 0x83B380ECUL, /* Write: AGC.CTRL2 */
  /*    C02C */ 0x51407543UL, /* AGC.CTRL3 */
  /*    C030 */ 0x48000FA0UL, /* AGC.CTRL4 */
  /*    C034 */ 0x00004030UL, /* AGC.CTRL5 */
  /*    C038 */ 0x00000000UL, /* AGC.CTRL6 */
  /*    C03C */ 0x00000000UL, /* AGC.RSSISTEPTHR */
  /*    C040 */ 0x0000022EUL, /* AGC.MANGAIN */
  0x0004C050UL, 0x04301151UL, /* Write: AGC.GAINRANGE */
  /*    C054 */ 0xE6092D0EUL, /* AGC.AGCPERIOD */
  /*    C058 */ 0x08070654UL, /* AGC.HICNTREGION */
  /*    C05C */ 0x0002B6D1UL, /* AGC.STEPDWN */
  0x000AC064UL, 0x1C003004UL, /* Write: AGC.GAINSTEPLIM */
  /*    C068 */ 0x09183040UL, /* AGC.PNRFATT0 */
  /*    C06C */ 0x2079640DUL, /* AGC.PNRFATT1 */
  /*    C070 */ 0x01FBFCEBUL, /* AGC.PNRFATT2 */
  /*    C074 */ 0x03E8F67FUL, /* AGC.PNRFATT3 */
  /*    C078 */ 0x15724BBDUL, /* AGC.LNAMIXCODE0 */
  /*    C07C */ 0x0518A311UL, /* AGC.LNAMIXCODE1 */
  /*    C080 */ 0x76543210UL, /* AGC.PGACODE0 */
  /*    C084 */ 0x00000A98UL, /* AGC.PGACODE1 */
  /*    C088 */ 0x00000000UL, /* AGC.LBT */
  0x01010008UL, 0x00000704UL, /* Write: RFCRC.CTRL */
  0x01010018UL, 0x00000000UL, /* Write: RFCRC.INIT */
  0x01010020UL, 0x00008408UL, /* Write: RFCRC.POLY */
  0x0108401CUL, 0x00000010UL, /* Write: MODEM.MIXCTRL */
  /*    4020 */ 0x0413F920UL, /* MODEM.CTRL0 */
  /*    4024 */ 0x0052C007UL, /* MODEM.CTRL1 */
  /*    4028 */ 0x00000000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x00000000UL, /* MODEM.CTRL3 */
  /*    4030 */ 0x03000000UL, /* MODEM.CTRL4 */
  /*    4034 */ 0x00000000UL, /* MODEM.CTRL5 */
  /*    4038 */ 0x00000000UL, /* MODEM.CTRL6 */
  0x01064058UL, 0x00FF0264UL, /* Write: MODEM.TXBR */
  /*    405C */ 0x00000841UL, /* MODEM.RXBR */
  /*    4060 */ 0x00000001UL, /* MODEM.CF */
  /*    4064 */ 0x000807B0UL, /* MODEM.PRE */
  /*    4068 */ 0x000000A7UL, /* MODEM.SYNC0 */
  /*    406C */ 0x00000000UL, /* MODEM.SYNC1 */
  0x01114080UL, 0x08A0014BUL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x744AC39BUL, /* MODEM.DSSS0 */
  /*    4088 */ 0x000003F0UL, /* MODEM.MODINDEX */
  /*    408C */ 0x00000000UL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x30100101UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x7F7F7050UL, /* MODEM.SHAPING1 */
  /*    409C */ 0x00000000UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x00000000UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0x00000000UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x00000000UL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x00000000UL, /* MODEM.SHAPING6 */
  /*    40B0 */ 0x00000000UL, /* MODEM.SHAPING7 */
  /*    40B4 */ 0x00000000UL, /* MODEM.SHAPING8 */
  /*    40B8 */ 0x00000000UL, /* MODEM.SHAPING9 */
  /*    40BC */ 0x00000000UL, /* MODEM.SHAPING10 */
  /*    40C0 */ 0x00000000UL, /* MODEM.SHAPING11 */
  0x010240E0UL, 0x00000033UL, /* Write: MODEM.DCCOMP */
  /*    40E4 */ 0x00000000UL, /* MODEM.DCCOMPFILTINIT */
  0x010140ECUL, 0x8BD9A87DUL, /* Write: MODEM.SRCCHF */
  0x010540F4UL, 0x07830464UL, /* Write: MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A209CUL, /* MODEM.DSACTRL */
  /*    4100 */ 0x00206100UL, /* MODEM.VITERBIDEMOD */
  /*    4104 */ 0x123556B7UL, /* MODEM.VTCORRCFG0 */
  0x0103410CUL, 0x001254F9UL, /* Write: MODEM.DIGMIXCTRL */
  /*    4110 */ 0x29043020UL, /* MODEM.VTCORRCFG1 */
  /*    4114 */ 0x4040BB88UL, /* MODEM.VTTRACK */
  0x01024124UL, 0x00000000UL, /* Write: MODEM.AUTOCG */
  /*    4128 */ 0x00001003UL, /* MODEM.CGCLKSTOP */
  0x01094134UL, 0x0000010CUL, /* Write: MODEM.DIRECTMODE */
  /*    4138 */ 0x00FA53E8UL, /* MODEM.LONGRANGE */
  /*    413C */ 0x00000000UL, /* MODEM.LONGRANGE1 */
  /*    4140 */ 0x00000000UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x00000000UL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x00000000UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x00000000UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0x00000000UL, /* MODEM.LONGRANGE6 */
  /*    4154 */ 0x00000101UL, /* MODEM.LRFRC */
  0x01034168UL, 0x07830464UL, /* Write: MODEM.DSATHD3 */
  /*    416C */ 0x00821388UL, /* MODEM.DSATHD4 */
  /*    4170 */ 0x00000000UL, /* MODEM.VTBLETIMING */
  0x01044230UL, 0x00000000UL, /* Write: MODEM.BLEIQDSA */
  /*    4234 */ 0x0E000000UL, /* MODEM.BLEIQDSAEXT1 */
  /*    4238 */ 0x00000000UL, /* MODEM.SYNCPROPERTIES */
  /*    423C */ 0x00000000UL, /* MODEM.DIGIGAINCTRL */
  0x01024244UL, 0x00000000UL, /* Write: MODEM.PADEBUG */
  /*    4248 */ 0x001F81F4UL, /* MODEM.REALTIMCFE */
  0x010C4254UL, 0x00000000UL, /* Write: MODEM.ETSCTRL */
  /*    4258 */ 0x003C0000UL, /* MODEM.ANTSWCTRL */
  /*    425C */ 0x00000000UL, /* MODEM.ANTSWSTART */
  /*    4260 */ 0x00000000UL, /* MODEM.ANTSWEND */
  /*    4264 */ 0x55555555UL, /* MODEM.TRECPMPATT */
  /*    4268 */ 0x00000017UL, /* MODEM.TRECPMDET */
  /*    426C */ 0x00000000UL, /* MODEM.CFGANTPATT */
  /*    4270 */ 0x00000000UL, /* MODEM.ETSTIM */
  /*    4274 */ 0x0006AAAAUL, /* MODEM.ANTSWCTRL1 */
  /*    4278 */ 0x00000000UL, /* MODEM.COCURRMODE */
  /*    427C */ 0x00000000UL, /* MODEM.ANTDIVCTRL */
  /*    4280 */ 0x00000000UL, /* MODEM.BLEIQDSAEXT2 */
  0x01018010UL, 0x00000003UL, /* Write: SYNTH.CTRL */
  0x01028038UL, 0x00104911UL, /* Write: SYNTH.IFFREQ */
  /*    803C */ 0x00000001UL, /* SYNTH.DIVCTRL */
  0x0103809CUL, 0x000240EBUL, /* Write: SYNTH.LPFCTRL1CAL */
  /*    80A0 */ 0x00037870UL, /* SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x0000C0D5UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x01CB4205UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x008D2205UL, /* OR: SYNTH.LPFCTRL2TX */
  0x010280B0UL, 0x02000300UL, /* Write: SYNTH.DSMCTRLRX */
  /*    80B4 */ 0x01000037UL, /* SYNTH.DSMCTRLTX */
  0x02020098UL, 0x04000C00UL, /* Write: RAC.SYNTHREGCTRL */
  /*    009C */ 0x0000004CUL, /* RAC.VCOCTRL */
  0x020100A4UL, 0x00000400UL, /* Write: RAC.SYNTHCTRL */
  0x020200D0UL, 0xAA400005UL, /* Write: RAC.CLKMULTEN0 */
  /*    00D4 */ 0x00000188UL, /* RAC.CLKMULTEN1 */
  0x020100E4UL, 0x11512C6CUL, /* Write: RAC.IFADCTRIM0 */
  0x020200F4UL, 0x00000000UL, /* Write: RAC.LNAMIXDEBUG */
  /*    00F8 */ 0x1108213DUL, /* RAC.LNAMIXTRIM0 */
  0x120100FCUL, 0x0000003FUL, /* AND: RAC.LNAMIXTRIM1 */
  0x320100FCUL, 0x00045400UL, /* OR: RAC.LNAMIXTRIM1 */
  0x02010130UL, 0x02510060UL, /* Write: RAC.PGACTRL */
  0x02010154UL, 0x00003FC4UL, /* Write: RAC.SYTRIM1 */
  0x02010168UL, 0x00000400UL, /* Write: RAC.SYMMDCTRL */
  0x03014FFCUL, (uint32_t) &phyInfo, /* Write: SEQ.PHYINFO */
  0xFFFFFFFFUL,
};

const RAIL_ChannelConfigEntry_t ieee802154_2p4_antdiv_channels[] = {
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

const RAIL_ChannelConfigEntry_t ieee802154_2p4_coherent_channels[] = {
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

const RAIL_ChannelConfig_t ieee802154_2p4_antdiv_channelConfig = {
  .phyConfigBase = ieee802154_2p4_antdiv_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ieee802154_2p4_antdiv_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

const RAIL_ChannelConfig_t ieee802154_2p4_coherent_channelConfig = {
  .phyConfigBase = ieee802154_2p4_antdiv_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ieee802154_2p4_coherent_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

uint32_t ieee802154AccelerationBuffer[303];
