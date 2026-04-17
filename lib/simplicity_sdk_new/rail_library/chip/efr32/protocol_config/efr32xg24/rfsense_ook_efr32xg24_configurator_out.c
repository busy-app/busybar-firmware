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
#include "rfsense_ook_efr32xg24_configurator_out.h"

static const uint8_t irCalConfig[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 0, 2, 2, 0, 0, 0, 0, 5, 0, 1, 1, 0, 0, 0, 0, 0
};

static const int32_t timingConfig[] = {
  1103579, 1103579, 500000, 0
};

__ALIGNED(4) static const uint8_t hfxoRetimingConfigEntries[] = {
  2, 0, 0, 0, 0xc0, 0x17, 0x53, 0x02, 6, 20, 0, 0, 0x80, 0x2f, 0xa6, 0x04, 5, 56, 0, 0, 0xa0, 0x08, 0, 0, 0, 0, 0x58, 0x09, 1, 4, 7, 6, 0x10, 0x0a, 1, 4, 7, 7, 0xc8, 0x0a, 0, 4, 8, 7, 0x80, 0x0b, 0, 4, 8, 8, 0x38, 0x0c, 0, 4, 9, 8, 0x61, 0x08, 0, 0, 0, 0, 0x8a, 0x08, 0, 0, 0, 0, 0xc7, 0x09, 1, 4, 4, 3, 0x2c, 0x0b, 1, 4, 4, 4, 0x92, 0x0c, 1, 4, 5, 4
};

#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
__ALIGNED(4) static const uint8_t stackInfo_0[2] = { 0x00, 0x00 };
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

static const uint32_t phyInfo[] = {
  20UL,
  0x00000000UL, // 0.0
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig,
  0x00000000UL,
  0UL,
  22950000UL,
  1000UL,
  0x00F20201UL,
  0x03100433UL,
  (uint32_t) NULL,
  (uint32_t) hfxoRetimingConfigEntries,
  (uint32_t) NULL,
  0UL,
  0UL,
  2000UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

const uint32_t rfsense_ook_1kbps_modemConfigBase[] = {
  0x0002400CUL, 0x00000000UL, /* Write: FRC.DFLCTRL */
  /*    4010 */ 0x00004000UL, /* FRC.MAXLENGTH */
  0x00024020UL, 0x0000000FUL, /* Write: FRC.WCNTCMP0 */
  /*    4024 */ 0x00000000UL, /* FRC.WCNTCMP1 */
  0x00074030UL, 0x00000000UL, /* Write: FRC.WHITECTRL */
  /*    4034 */ 0x00000000UL, /* FRC.WHITEPOLY */
  /*    4038 */ 0x00000000UL, /* FRC.WHITEINIT */
  /*    403C */ 0x00000000UL, /* FRC.FECCTRL */
  /*    4040 */ 0x00000000UL, /* FRC.BLOCKRAMADDR */
  /*    4044 */ 0x00004000UL, /* FRC.CONVRAMADDR */
  /*    4048 */ 0x03000700UL, /* FRC.CTRL */
  0x00014050UL, 0x00000000UL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000000UL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000000UL, /* FRC.PUNCTCTRL */
  0x000140A8UL, 0x00000007UL, /* Write: FRC.AUTOCG */
  0x000440BCUL, 0x00000000UL, /* Write: FRC.WCNTCMP3 */
  /*    40C0 */ 0x00000000UL, /* FRC.BOICTRL */
  /*    40C4 */ 0x00000000UL, /* FRC.DSLCTRL */
  /*    40C8 */ 0x00000000UL, /* FRC.WCNTCMP4 */
  0x00044108UL, 0x000040FFUL, /* Write: FRC.FCD0 */
  /*    410C */ 0x00000000UL, /* FRC.FCD1 */
  /*    4110 */ 0x000041FFUL, /* FRC.FCD2 */
  /*    4114 */ 0x00000000UL, /* FRC.FCD3 */
  0x1001C020UL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C020UL, 0x01280100UL, /* OR: AGC.CTRL0 */
  0x1001C024UL, 0x000000FFUL, /* AND: AGC.CTRL1 */
  0x3001C024UL, 0x00001300UL, /* OR: AGC.CTRL1 */
  0x0008C028UL, 0x03B380ECUL, /* Write: AGC.CTRL2 */
  /*    C02C */ 0x51407543UL, /* AGC.CTRL3 */
  /*    C030 */ 0xF8000FA0UL, /* AGC.CTRL4 */
  /*    C034 */ 0x00004000UL, /* AGC.CTRL5 */
  /*    C038 */ 0x0007AAA8UL, /* AGC.CTRL6 */
  /*    C03C */ 0x00000000UL, /* AGC.CTRL7 */
  /*    C040 */ 0x00000000UL, /* AGC.RSSISTEPTHR */
  /*    C044 */ 0x00000000UL, /* AGC.MANGAIN */
  0x0010C054UL, 0x00302151UL, /* Write: AGC.GAINRANGE */
  /*    C058 */ 0xE6E300F4UL, /* AGC.AGCPERIOD0 */
  /*    C05C */ 0x000054A3UL, /* AGC.AGCPERIOD1 */
  /*    C060 */ 0xDCCFB786UL, /* AGC.HICNTREGION0 */
  /*    C064 */ 0x000000E3UL, /* AGC.HICNTREGION1 */
  /*    C068 */ 0x0002C688UL, /* AGC.STEPDWN */
  /*    C06C */ 0x00000740UL, /* AGC.GAINSTEPLIM0 */
  /*    C070 */ 0x000010BAUL, /* AGC.GAINSTEPLIM1 */
  /*    C074 */ 0x00200400UL, /* AGC.PNRFATT0 */
  /*    C078 */ 0x00801804UL, /* AGC.PNRFATT1 */
  /*    C07C */ 0x01203C0BUL, /* AGC.PNRFATT2 */
  /*    C080 */ 0x02107C18UL, /* AGC.PNRFATT3 */
  /*    C084 */ 0x06E0FC2FUL, /* AGC.PNRFATT4 */
  /*    C088 */ 0x0000007FUL, /* AGC.PNRFATT5 */
  /*    C08C */ 0x00000000UL, /* AGC.PNRFATT6 */
  /*    C090 */ 0x00000000UL, /* AGC.PNRFATT7 */
  0x0005C0A8UL, 0x15724BBDUL, /* Write: AGC.LNAMIXCODE0 */
  /*    C0AC */ 0x0518A311UL, /* AGC.LNAMIXCODE1 */
  /*    C0B0 */ 0x76543210UL, /* AGC.PGACODE0 */
  /*    C0B4 */ 0x00000A98UL, /* AGC.PGACODE1 */
  /*    C0B8 */ 0x00000000UL, /* AGC.LBT */
  0x0004C0CCUL, 0x00000001UL, /* Write: AGC.LNABOOST */
  /*    C0D0 */ 0x00000000UL, /* AGC.ANTDIV */
  /*    C0D4 */ 0x000A0001UL, /* AGC.DUALRFPKDTHD0 */
  /*    C0D8 */ 0x00280001UL, /* AGC.DUALRFPKDTHD1 */
  0x01010008UL, 0x00000744UL, /* Write: RFCRC.CTRL */
  0x01010018UL, 0x00000000UL, /* Write: RFCRC.INIT */
  0x01010020UL, 0x0000A001UL, /* Write: RFCRC.POLY */
  0x01254040UL, 0x00000000UL, /* Write: MODEM.AFCADJRX */
  /*    4044 */ 0x00000000UL, /* MODEM.AFCADJTX */
  /*    4048 */ 0x00000010UL, /* MODEM.MIXCTRL */
  /*    404C */ 0x00000191UL, /* MODEM.CTRL0 */
  /*    4050 */ 0x0602C001UL, /* MODEM.CTRL1 */
  /*    4054 */ 0x00003000UL, /* MODEM.CTRL2 */
  /*    4058 */ 0x00000000UL, /* MODEM.CTRL3 */
  /*    405C */ 0x03000000UL, /* MODEM.CTRL4 */
  /*    4060 */ 0x20000000UL, /* MODEM.CTRL5 */
  /*    4064 */ 0x00000000UL, /* MODEM.CTRL6 */
  /*    4068 */ 0x00067242UL, /* MODEM.TXBR */
  /*    406C */ 0x00000841UL, /* MODEM.RXBR */
  /*    4070 */ 0x000407F0UL, /* MODEM.CF */
  /*    4074 */ 0x00000010UL, /* MODEM.PRE */
  /*    4078 */ 0x00000000UL, /* MODEM.SYNC0 */
  /*    407C */ 0x00000000UL, /* MODEM.SYNC1 */
  /*    4080 */ 0x08AC0000UL, /* MODEM.TIMING */
  /*    4084 */ 0x00000000UL, /* MODEM.DSSS0 */
  /*    4088 */ 0x000F031CUL, /* MODEM.MODINDEX */
  /*    408C */ 0x60000000UL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x00000000UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x00000000UL, /* MODEM.SHAPING1 */
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
  /*    40C4 */ 0x00000000UL, /* MODEM.SHAPING12 */
  /*    40C8 */ 0x00000000UL, /* MODEM.SHAPING13 */
  /*    40CC */ 0x00000000UL, /* MODEM.SHAPING14 */
  /*    40D0 */ 0x00000000UL, /* MODEM.SHAPING15 */
  0x010140E0UL, 0x00000200UL, /* Write: MODEM.ANARAMPCTRL */
  0x01024110UL, 0x00051E33UL, /* Write: MODEM.DCCOMP */
  /*    4114 */ 0x00000000UL, /* MODEM.DCCOMPFILTINIT */
  0x0103411CUL, 0x8B561000UL, /* Write: MODEM.SRCCHF */
  /*    4120 */ 0x00000000UL, /* MODEM.INTAFC */
  /*    4124 */ 0x078304FFUL, /* MODEM.DSATHD0 */
  0x010A412CUL, 0x0C6606FFUL, /* Write: MODEM.DSATHD2 */
  /*    4130 */ 0x078304FFUL, /* MODEM.DSATHD3 */
  /*    4134 */ 0x03FF1388UL, /* MODEM.DSATHD4 */
  /*    4138 */ 0xF00A20BCUL, /* MODEM.DSACTRL */
  /*    413C */ 0x00503356UL, /* MODEM.DIGMIXCTRL */
  /*    4140 */ 0x00000000UL, /* MODEM.VITERBIDEMOD */
  /*    4144 */ 0x123556B7UL, /* MODEM.VTCORRCFG0 */
  /*    4148 */ 0x50000000UL, /* MODEM.VTCORRCFG1 */
  /*    414C */ 0x00003B80UL, /* MODEM.VTTRACK */
  /*    4150 */ 0x00000000UL, /* MODEM.VTBLETIMING */
  0x01024158UL, 0x00000000UL, /* Write: MODEM.AUTOCG */
  /*    415C */ 0x00000000UL, /* MODEM.CGCLKSTOP */
  0x010D4164UL, 0x0000010CUL, /* Write: MODEM.DIRECTMODE */
  /*    4168 */ 0x00FA53E8UL, /* MODEM.LONGRANGE */
  /*    416C */ 0x40000000UL, /* MODEM.LONGRANGE1 */
  /*    4170 */ 0x00000000UL, /* MODEM.LONGRANGE2 */
  /*    4174 */ 0x00000000UL, /* MODEM.LONGRANGE3 */
  /*    4178 */ 0x00000000UL, /* MODEM.LONGRANGE4 */
  /*    417C */ 0x00000000UL, /* MODEM.LONGRANGE5 */
  /*    4180 */ 0x00000000UL, /* MODEM.LONGRANGE6 */
  /*    4184 */ 0x00000101UL, /* MODEM.LRFRC */
  /*    4188 */ 0x00000000UL, /* MODEM.COH0 */
  /*    418C */ 0x00000000UL, /* MODEM.COH1 */
  /*    4190 */ 0x00000000UL, /* MODEM.COH2 */
  /*    4194 */ 0x00000000UL, /* MODEM.COH3 */
  0x010241A4UL, 0x00000000UL, /* Write: MODEM.SYNCPROPERTIES */
  /*    41A8 */ 0x00000000UL, /* MODEM.DIGIGAINCTRL */
  0x010241B0UL, 0x00000000UL, /* Write: MODEM.PADEBUG */
  /*    41B4 */ 0x00200000UL, /* MODEM.REALTIMCFE */
  0x010341BCUL, 0x00000000UL, /* Write: MODEM.ETSTIM */
  /*    41C0 */ 0x003C0000UL, /* MODEM.ANTSWCTRL */
  /*    41C4 */ 0x00069069UL, /* MODEM.ANTSWCTRL1 */
  0x010341D0UL, 0x00000000UL, /* Write: MODEM.TRECPMPATT */
  /*    41D4 */ 0x00000010UL, /* MODEM.TRECPMDET */
  /*    41D8 */ 0x00020000UL, /* MODEM.TRECSCFG */
  0x011641E0UL, 0x00000000UL, /* Write: MODEM.COCURRMODE */
  /*    41E4 */ 0x0BFFE7E6UL, /* MODEM.CHFCOE00 */
  /*    41E8 */ 0x000AA1CDUL, /* MODEM.CHFCOE01 */
  /*    41EC */ 0x006A06BDUL, /* MODEM.CHFCOE02 */
  /*    41F0 */ 0x004DB05EUL, /* MODEM.CHFCOE03 */
  /*    41F4 */ 0x0E42027DUL, /* MODEM.CHFCOE04 */
  /*    41F8 */ 0x0222B6A5UL, /* MODEM.CHFCOE05 */
  /*    41FC */ 0x34B225FFUL, /* MODEM.CHFCOE06 */
  /*    4200 */ 0x0BFFE7E6UL, /* MODEM.CHFCOE10 */
  /*    4204 */ 0x000AA1CDUL, /* MODEM.CHFCOE11 */
  /*    4208 */ 0x006A06BDUL, /* MODEM.CHFCOE12 */
  /*    420C */ 0x004DB05EUL, /* MODEM.CHFCOE13 */
  /*    4210 */ 0x0E42027DUL, /* MODEM.CHFCOE14 */
  /*    4214 */ 0x0222B6A5UL, /* MODEM.CHFCOE15 */
  /*    4218 */ 0x34B225FFUL, /* MODEM.CHFCOE16 */
  /*    421C */ 0x00000000UL, /* MODEM.CHFCTRL */
  /*    4220 */ 0x00000000UL, /* MODEM.CHFLATENCYCTRL */
  /*    4224 */ 0x00000005UL, /* MODEM.FRMSCHTIME */
  /*    4228 */ 0x00000000UL, /* MODEM.PREFILTCOEFF */
  /*    422C */ 0x40001860UL, /* MODEM.RXRESTART */
  /*    4230 */ 0x00000000UL, /* MODEM.SQ */
  /*    4234 */ 0x00000000UL, /* MODEM.SQEXT */
  0x0101423CUL, 0x00000000UL, /* Write: MODEM.ANTDIVCTRL */
  0x01014244UL, 0x00000014UL, /* Write: MODEM.PHDMODANTDIV */
  0x0101424CUL, 0x04000008UL, /* Write: MODEM.PHDMODCTRL */
  0x01014268UL, 0x00000000UL, /* Write: MODEM.CHFSWCTRL */
  0x01024280UL, 0x00000000UL, /* Write: MODEM.ADCTRL1 */
  /*    4284 */ 0x00000081UL, /* MODEM.ADCTRL2 */
  0x01054298UL, 0x0200003FUL, /* Write: MODEM.ADQUAL4 */
  /*    429C */ 0x0000FFFFUL, /* MODEM.ADQUAL5 */
  /*    42A0 */ 0x0000FFFFUL, /* MODEM.ADQUAL6 */
  /*    42A4 */ 0x000003FFUL, /* MODEM.ADQUAL7 */
  /*    42A8 */ 0x0000FFFFUL, /* MODEM.ADQUAL8 */
  0x010142B4UL, 0x00000000UL, /* Write: MODEM.ADFSM0 */
  0x010A4330UL, 0x01200040UL, /* Write: MODEM.ADPC1 */
  /*    4334 */ 0x000000A0UL, /* MODEM.ADPC2 */
  /*    4338 */ 0x01005008UL, /* MODEM.ADPC3 */
  /*    433C */ 0x1F1F1F1FUL, /* MODEM.ADPC4 */
  /*    4340 */ 0x1B1F1F1FUL, /* MODEM.ADPC5 */
  /*    4344 */ 0x11131518UL, /* MODEM.ADPC6 */
  /*    4348 */ 0x0C0D0E10UL, /* MODEM.ADPC7 */
  /*    434C */ 0x2F87C145UL, /* MODEM.ADPC8 */
  /*    4350 */ 0x00000000UL, /* MODEM.ADPC9 */
  /*    4354 */ 0x00000000UL, /* MODEM.ADPC10 */
  0x01018010UL, 0x00000003UL, /* Write: SYNTH.CTRL */
  0x01028038UL, 0x00100433UL, /* Write: SYNTH.IFFREQ */
  /*    803C */ 0x00000001UL, /* SYNTH.DIVCTRL */
  0x010280A0UL, 0x00037870UL, /* Write: SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x000000D0UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x01CB4205UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x00FD3E05UL, /* OR: SYNTH.LPFCTRL2TX */
  0x010280B0UL, 0x02000300UL, /* Write: SYNTH.DSMCTRLRX */
  /*    80B4 */ 0x01000037UL, /* SYNTH.DSMCTRLTX */
  0x0201009CUL, 0x04000C00UL, /* Write: RAC.SYNTHREGCTRL */
  0x020300D8UL, 0xAA400005UL, /* Write: RAC.CLKMULTEN0 */
  /*    00DC */ 0x00000188UL, /* RAC.CLKMULTEN1 */
  /*    00E0 */ 0x000000C0UL, /* RAC.CLKMULTCTRL */
  0x120100ECUL, 0x00000FE0UL, /* AND: RAC.IFADCTRIM0 */
  0x320100ECUL, 0x5151200CUL, /* OR: RAC.IFADCTRIM0 */
  0x020100F0UL, 0x0000012BUL, /* Write: RAC.IFADCTRIM1 */
  0x12010110UL, 0x000FFF00UL, /* AND: RAC.LNAMIXTRIM4 */
  0x32010110UL, 0x31000002UL, /* OR: RAC.LNAMIXTRIM4 */
  0x12010150UL, 0x0001C000UL, /* AND: RAC.PGACTRL */
  0x32010150UL, 0x00A200C1UL, /* OR: RAC.PGACTRL */
  0x02010174UL, 0x0C1B8169UL, /* Write: RAC.SYTRIM0 */
  0x12010178UL, 0x001C0000UL, /* AND: RAC.SYTRIM1 */
  0x32010178UL, 0xCFE00410UL, /* OR: RAC.SYTRIM1 */
  0x12010180UL, 0x00000779UL, /* AND: RAC.SYEN */
  0x32010180UL, 0x00000002UL, /* OR: RAC.SYEN */
  0x02020184UL, 0x00001000UL, /* Write: RAC.SYLOEN */
  /*    0188 */ 0x00000050UL, /* RAC.SYMMDCTRL */
  0x03014FFCUL, (uint32_t) &phyInfo, /* Write: SEQ.PHYINFO */
  0xFFFFFFFFUL,
};

const RAIL_ChannelConfigEntry_t rfsense_ook_1kbps_channels[] = {
  {
    .phyConfigDeltaAdd = NULL,
    .baseFrequency = 2450000000,
    .channelSpacing = 1000000,
    .physicalChannelOffset = 0,
    .channelNumberStart = 0,
    .channelNumberEnd = 0,
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

const RAIL_ChannelConfig_t rfsense_ook_1kbps_channelConfig = {
  .phyConfigBase = rfsense_ook_1kbps_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = rfsense_ook_1kbps_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 39000000UL,
};

uint32_t rfsenseAccelerationBuffer[431];
