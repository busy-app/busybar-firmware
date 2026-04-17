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
#include "ble_efr32xg21_configurator_out.h"

static const uint8_t irCalConfig[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int32_t timingConfig_0[] = {
  50000, 9500, 500, 500
};

static const int32_t timingConfig_1[] = {
  50000, 5500, 1500, 1500
};

static const int32_t timingConfig_2[] = {
  187125, 9500, 500, 500
};

static const int32_t timingConfig_3[] = {
  49125, 9500, 500, 500
};

#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
__ALIGNED(4) static const uint8_t stackInfo_0[2] = { 0x03, 0x01 };
__ALIGNED(4) static const uint8_t stackInfo_1[2] = { 0x03, 0x02 };
__ALIGNED(4) static const uint8_t stackInfo_2[2] = { 0x03, 0x03 };
__ALIGNED(4) static const uint8_t stackInfo_3[2] = { 0x03, 0x04 };
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
  8793864UL,
  32000000UL,
  1000000UL,
  0x00000101UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  999978UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_1[] = {
  20UL,
  0x00800000UL, // 128.0
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig_1,
  0x00000000UL,
  8800000UL,
  32000000UL,
  2000000UL,
  0x00000101UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  1999957UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_2[] = {
  20UL,
  0x00800000UL, // 128.0
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig_2,
  0x00000000UL,
  9840000UL,
  32000000UL,
  1000000UL,
  0x00000801UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  999998UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_3[] = {
  20UL,
  0x00800000UL, // 128.0
  (uint32_t) NULL,
  (uint32_t) irCalConfig,
  (uint32_t) timingConfig_3,
  0x00000000UL,
  9840000UL,
  32000000UL,
  1000000UL,
  0x00000201UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  999998UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

const uint32_t ble_viterbi1M_modemConfigBase[] = {
  0x0002400CUL, 0x00018101UL, /* Write: FRC.DFLCTRL */
  /*    4010 */ 0x00004100UL, /* FRC.MAXLENGTH */
  0x00024020UL, 0x00000000UL, /* Write: FRC.WCNTCMP0 */
  /*    4024 */ 0x00000001UL, /* FRC.WCNTCMP1 */
  0x00034030UL, 0x00000000UL, /* Write: FRC.WHITECTRL */
  /*    4034 */ 0x00000000UL, /* FRC.WHITEPOLY */
  /*    4038 */ 0x00000000UL, /* FRC.WHITEINIT */
  0x00024040UL, 0x00000000UL, /* Write: FRC.BLOCKRAMADDR */
  /*    4044 */ 0x00000000UL, /* FRC.CONVRAMADDR */
  0x000140A8UL, 0x00000007UL, /* Write: FRC.AUTOCG */
  0x000340B8UL, 0x00004CFFUL, /* Write: FRC.FCD1 */
  /*    40BC */ 0x00004DFFUL, /* FRC.FCD2 */
  /*    40C0 */ 0x00004DFFUL, /* FRC.FCD3 */
  0x0001C008UL, 0x00000000UL, /* Write: AGC.STATUS0 */
  0x1001C01CUL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C01CUL, 0x000802F5UL, /* OR: AGC.CTRL0 */
  0x1001C020UL, 0x000000FFUL, /* AND: AGC.CTRL1 */
  0x3001C020UL, 0x00001300UL, /* OR: AGC.CTRL1 */
  0x0004C024UL, 0x03B380ECUL, /* Write: AGC.CTRL2 */
  /*    C028 */ 0x02A87543UL, /* AGC.CTRL3 */
  /*    C02C */ 0x00000000UL, /* AGC.RSSISTEPTHR */
  /*    C030 */ 0x0000022EUL, /* AGC.MANGAIN */
  0x0001C050UL, 0x0C304187UL, /* Write: AGC.GAINRANGE */
  0x0002C058UL, 0x08070654UL, /* Write: AGC.HICNTREGION */
  /*    C05C */ 0x0002B6D1UL, /* AGC.STEPDWN */
  0x0008C064UL, 0x00003004UL, /* Write: AGC.GAINSTEPLIM */
  /*    C068 */ 0x06102040UL, /* AGC.PNRFATT0 */
  /*    C06C */ 0x1848F2C8UL, /* AGC.PNRFATT1 */
  /*    C070 */ 0x00F6E81FUL, /* AGC.PNRFATT2 */
  /*    C074 */ 0x15724BBDUL, /* AGC.LNAMIXCODE0 */
  /*    C078 */ 0x0518A311UL, /* AGC.LNAMIXCODE1 */
  /*    C07C */ 0x76543210UL, /* AGC.PGACODE0 */
  /*    C080 */ 0x00000A98UL, /* AGC.PGACODE1 */
  0x0001C088UL, 0x00000000UL, /* Write: AGC.LBT */
  0x01010008UL, 0x00000708UL, /* Write: RFCRC.CTRL */
  0x01010018UL, 0x00AAAAAAUL, /* Write: RFCRC.INIT */
  0x01010020UL, 0x00DA6000UL, /* Write: RFCRC.POLY */
  0x0101401CUL, 0x00000010UL, /* Write: MODEM.MIXCTRL */
  0x01034030UL, 0x03000000UL, /* Write: MODEM.CTRL4 */
  /*    4034 */ 0x00000000UL, /* MODEM.CTRL5 */
  /*    4038 */ 0x00000000UL, /* MODEM.CTRL6 */
  0x01024068UL, 0x71764129UL, /* Write: MODEM.SYNC0 */
  /*    406C */ 0x00000000UL, /* MODEM.SYNC1 */
  0x010240E0UL, 0x00000033UL, /* Write: MODEM.DCCOMP */
  /*    40E4 */ 0x00000000UL, /* MODEM.DCCOMPFILTINIT */
  0x0101410CUL, 0x001255B8UL, /* Write: MODEM.DIGMIXCTRL */
  0x01014124UL, 0x00000000UL, /* Write: MODEM.AUTOCG */
  0x01054154UL, 0x00000001UL, /* Write: MODEM.LRFRC */
  /*    4158 */ 0x00000000UL, /* MODEM.COH0 */
  /*    415C */ 0x00000000UL, /* MODEM.COH1 */
  /*    4160 */ 0x00000000UL, /* MODEM.COH2 */
  /*    4164 */ 0x00000000UL, /* MODEM.COH3 */
  0x01024238UL, 0x00000000UL, /* Write: MODEM.SYNCPROPERTIES */
  /*    423C */ 0x00000000UL, /* MODEM.DIGIGAINCTRL */
  0x01014244UL, 0x00000000UL, /* Write: MODEM.PADEBUG */
  0x01018010UL, 0x00000003UL, /* Write: SYNTH.CTRL */
  0x01028038UL, 0x00104911UL, /* Write: SYNTH.IFFREQ */
  /*    803C */ 0x00000001UL, /* SYNTH.DIVCTRL */
  0x0101809CUL, 0x000240EBUL, /* Write: SYNTH.LPFCTRL1CAL */
  0x010280B0UL, 0x02000300UL, /* Write: SYNTH.DSMCTRLRX */
  /*    80B4 */ 0x01000037UL, /* SYNTH.DSMCTRLTX */
  0x02020088UL, 0x04000C00UL, /* Write: RAC.SYNTHREGCTRL */
  /*    008C */ 0x0000004CUL, /* RAC.VCOCTRL */
  0x02010094UL, 0x00000400UL, /* Write: RAC.SYNTHCTRL */
  0x020100D4UL, 0x08960290UL, /* Write: RAC.IFADCTRIM */
  0x020100E4UL, 0x0442093DUL, /* Write: RAC.LNAMIXTRIM0 */
  0x02010118UL, 0x02510060UL, /* Write: RAC.PGACTRL */
  0xFFFFFFFFUL,
};

const uint32_t ble_viterbi1M_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_0, /* Write: SEQ.PHYINFO */
  0x0001403CUL, 0x00000000UL, /* Write: FRC.FECCTRL */
  0x00014048UL, 0x000007A0UL, /* Write: FRC.CTRL */
  0x00014050UL, 0x00000000UL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000000UL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000000UL, /* FRC.PUNCTCTRL */
  0x000140B4UL, 0x00004CFFUL, /* Write: FRC.FCD0 */
  0x0001C054UL, 0xE6092D0EUL, /* Write: AGC.AGCPERIOD */
  0x01044020UL, 0x86000000UL, /* Write: MODEM.CTRL0 */
  /*    4024 */ 0x0001C01FUL, /* MODEM.CTRL1 */
  /*    4028 */ 0x00006000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x00008000UL, /* MODEM.CTRL3 */
  0x01044058UL, 0x00050018UL, /* Write: MODEM.TXBR */
  /*    405C */ 0x00000880UL, /* MODEM.RXBR */
  /*    4060 */ 0x00000009UL, /* MODEM.CF */
  /*    4064 */ 0x00040091UL, /* MODEM.PRE */
  0x010C4080UL, 0x000C008CUL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x00000000UL, /* MODEM.DSSS0 */
  /*    4088 */ 0x00220013UL, /* MODEM.MODINDEX */
  /*    408C */ 0x00000000UL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x0D050200UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x53422F1CUL, /* MODEM.SHAPING1 */
  /*    409C */ 0x60686962UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x1F2E4053UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0x84208291UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x22334567UL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x00024912UL, /* MODEM.SHAPING6 */
  0x010740ECUL, 0xAC2C3890UL, /* Write: MODEM.SRCCHF */
  /*    40F0 */ 0x00000000UL, /* MODEM.INTAFC */
  /*    40F4 */ 0x0A08105AUL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x4ACA07D0UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x00481051UL, /* MODEM.DSACTRL */
  /*    4100 */ 0x801E54F9UL, /* MODEM.VITERBIDEMOD */
  /*    4104 */ 0x4A413747UL, /* MODEM.VTCORRCFG0 */
  0x01024110UL, 0x054A1930UL, /* Write: MODEM.VTCORRCFG1 */
  /*    4114 */ 0x0040BB89UL, /* MODEM.VTTRACK */
  0x01014128UL, 0x0000FFFFUL, /* Write: MODEM.CGCLKSTOP */
  0x01094130UL, 0x430450F1UL, /* Write: MODEM.DSATHD2 */
  /*    4134 */ 0x0000010CUL, /* MODEM.DIRECTMODE */
  /*    4138 */ 0x00FA53E8UL, /* MODEM.LONGRANGE */
  /*    413C */ 0x00080000UL, /* MODEM.LONGRANGE1 */
  /*    4140 */ 0x221C1610UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x3A342E28UL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x43214640UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x0BA98765UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0xB4C00000UL, /* MODEM.LONGRANGE6 */
  0x01034168UL, 0x0A08105AUL, /* Write: MODEM.DSATHD3 */
  /*    416C */ 0x044107D0UL, /* MODEM.DSATHD4 */
  /*    4170 */ 0x000003C1UL, /* MODEM.VTBLETIMING */
  0x01024230UL, 0x0FA39389UL, /* Write: MODEM.BLEIQDSA */
  /*    4234 */ 0x0E5DC25EUL, /* MODEM.BLEIQDSAEXT1 */
  0x010280A0UL, 0x00037870UL, /* Write: SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x000000D0UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x01CB4205UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x00FD3E05UL, /* OR: SYNTH.LPFCTRL2TX */
  0xFFFFFFFFUL,
};

const uint32_t ble_viterbi2M_0_34_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_1, /* Write: SEQ.PHYINFO */
  0x0001403CUL, 0x00000000UL, /* Write: FRC.FECCTRL */
  0x00014048UL, 0x000007A0UL, /* Write: FRC.CTRL */
  0x00014050UL, 0x00000000UL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000000UL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000000UL, /* FRC.PUNCTCTRL */
  0x000140B4UL, 0x00004CFFUL, /* Write: FRC.FCD0 */
  0x0001C054UL, 0xE6092D0AUL, /* Write: AGC.AGCPERIOD */
  0x01044020UL, 0x06000000UL, /* Write: MODEM.CTRL0 */
  /*    4024 */ 0x0000C01FUL, /* MODEM.CTRL1 */
  /*    4028 */ 0x00006000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x0032D000UL, /* MODEM.CTRL3 */
  0x01044058UL, 0x00FF0264UL, /* Write: MODEM.TXBR */
  /*    405C */ 0x00000880UL, /* MODEM.RXBR */
  /*    4060 */ 0x00000001UL, /* MODEM.CF */
  /*    4064 */ 0x00080011UL, /* MODEM.PRE */
  0x010C4080UL, 0x00000408UL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x00000000UL, /* MODEM.DSSS0 */
  /*    4088 */ 0x002203D9UL, /* MODEM.MODINDEX */
  /*    408C */ 0x00000000UL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x11070201UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x5A4B3721UL, /* MODEM.SHAPING1 */
  /*    409C */ 0x54626967UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x05152940UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0x20800000UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x33334443UL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x24A49223UL, /* MODEM.SHAPING6 */
  0x010740ECUL, 0xAC2C3890UL, /* Write: MODEM.SRCCHF */
  /*    40F0 */ 0x00000000UL, /* MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A209DUL, /* MODEM.DSACTRL */
  /*    4100 */ 0x80206101UL, /* MODEM.VITERBIDEMOD */
  /*    4104 */ 0x4A413747UL, /* MODEM.VTCORRCFG0 */
  0x01024110UL, 0x024A142AUL, /* Write: MODEM.VTCORRCFG1 */
  /*    4114 */ 0x0AC0BB89UL, /* MODEM.VTTRACK */
  0x01014128UL, 0x0000FFFFUL, /* Write: MODEM.CGCLKSTOP */
  0x01094130UL, 0x0C660664UL, /* Write: MODEM.DSATHD2 */
  /*    4134 */ 0x0000010CUL, /* MODEM.DIRECTMODE */
  /*    4138 */ 0x00FA53E8UL, /* MODEM.LONGRANGE */
  /*    413C */ 0x00080000UL, /* MODEM.LONGRANGE1 */
  /*    4140 */ 0x241E1812UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x3C36302AUL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x43214842UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x0BA98765UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0xC4E00000UL, /* MODEM.LONGRANGE6 */
  0x01034168UL, 0x07830464UL, /* Write: MODEM.DSATHD3 */
  /*    416C */ 0x00821388UL, /* MODEM.DSATHD4 */
  /*    4170 */ 0x000008C1UL, /* MODEM.VTBLETIMING */
  0x01024230UL, 0x106B9645UL, /* Write: MODEM.BLEIQDSA */
  /*    4234 */ 0x0E67225CUL, /* MODEM.BLEIQDSAEXT1 */
  0x010280A0UL, 0x00037870UL, /* Write: SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x0000C0D5UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x01CB4205UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x008D2205UL, /* OR: SYNTH.LPFCTRL2TX */
  0xFFFFFFFFUL,
};

const uint32_t ble_LR_DSA_125kb_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_2, /* Write: SEQ.PHYINFO */
  0x0001403CUL, 0x00000010UL, /* Write: FRC.FECCTRL */
  0x00014048UL, 0x00000720UL, /* Write: FRC.CTRL */
  0x00014050UL, 0x00000400UL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000F0DUL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000101UL, /* FRC.PUNCTCTRL */
  0x000140B4UL, 0x0001C003UL, /* Write: FRC.FCD0 */
  0x0001C054UL, 0xE6092D0EUL, /* Write: AGC.AGCPERIOD */
  0x01044020UL, 0x46000030UL, /* Write: MODEM.CTRL0 */
  /*    4024 */ 0x0001C81FUL, /* MODEM.CTRL1 */
  /*    4028 */ 0x10002000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x00008000UL, /* MODEM.CTRL3 */
  0x01044058UL, 0x00050018UL, /* Write: MODEM.TXBR */
  /*    405C */ 0x00000840UL, /* MODEM.RXBR */
  /*    4060 */ 0x00000009UL, /* MODEM.CF */
  /*    4064 */ 0x000A0092UL, /* MODEM.PRE */
  0x010C4080UL, 0x022C008CUL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x00000000UL, /* MODEM.DSSS0 */
  /*    4088 */ 0x00120012UL, /* MODEM.MODINDEX */
  /*    408C */ 0x1B8C805BUL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x0E050200UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x53432F1CUL, /* MODEM.SHAPING1 */
  /*    409C */ 0x61686962UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x04324355UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0xC6B8E416UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x3345678AUL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x04924A22UL, /* MODEM.SHAPING6 */
  0x010740ECUL, 0x8CB1B080UL, /* Write: MODEM.SRCCHF */
  /*    40F0 */ 0x00000000UL, /* MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A209CUL, /* MODEM.DSACTRL */
  /*    4100 */ 0x80206101UL, /* MODEM.VITERBIDEMOD */
  /*    4104 */ 0x123556B7UL, /* MODEM.VTCORRCFG0 */
  0x01024110UL, 0x29041BA0UL, /* Write: MODEM.VTCORRCFG1 */
  /*    4114 */ 0x00408808UL, /* MODEM.VTTRACK */
  0x01014128UL, 0x00000000UL, /* Write: MODEM.CGCLKSTOP */
  0x01094130UL, 0x0C660664UL, /* Write: MODEM.DSATHD2 */
  /*    4134 */ 0x0000010CUL, /* MODEM.DIRECTMODE */
  /*    4138 */ 0x3BE8D44CUL, /* MODEM.LONGRANGE */
  /*    413C */ 0x306F1904UL, /* MODEM.LONGRANGE1 */
  /*    4140 */ 0x1A140E08UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x322C2620UL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x32103E38UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x0A987654UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0x00005F1DUL, /* MODEM.LONGRANGE6 */
  0x01034168UL, 0x07830464UL, /* Write: MODEM.DSATHD3 */
  /*    416C */ 0x00821388UL, /* MODEM.DSATHD4 */
  /*    4170 */ 0x00000000UL, /* MODEM.VTBLETIMING */
  0x01024230UL, 0x00000000UL, /* Write: MODEM.BLEIQDSA */
  /*    4234 */ 0x0E000800UL, /* MODEM.BLEIQDSAEXT1 */
  0x010280A0UL, 0x00033870UL, /* Write: SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x000000D0UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x01FD5A05UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x00FD3E05UL, /* OR: SYNTH.LPFCTRL2TX */
  0xFFFFFFFFUL,
};

const uint32_t ble_LR_DSA_500kb_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_3, /* Write: SEQ.PHYINFO */
  0x0001403CUL, 0x00000010UL, /* Write: FRC.FECCTRL */
  0x00014048UL, 0x00001720UL, /* Write: FRC.CTRL */
  0x00014050UL, 0x00000401UL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000F0DUL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000101UL, /* FRC.PUNCTCTRL */
  0x000140B4UL, 0x0001C003UL, /* Write: FRC.FCD0 */
  0x0001C054UL, 0xE6092D0EUL, /* Write: AGC.AGCPERIOD */
  0x01044020UL, 0x46000030UL, /* Write: MODEM.CTRL0 */
  /*    4024 */ 0x0001C81FUL, /* MODEM.CTRL1 */
  /*    4028 */ 0x10002000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x00008000UL, /* MODEM.CTRL3 */
  0x01044058UL, 0x00050018UL, /* Write: MODEM.TXBR */
  /*    405C */ 0x00000840UL, /* MODEM.RXBR */
  /*    4060 */ 0x00000009UL, /* MODEM.CF */
  /*    4064 */ 0x000A0092UL, /* MODEM.PRE */
  0x010C4080UL, 0x022C008CUL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x00000000UL, /* MODEM.DSSS0 */
  /*    4088 */ 0x00120012UL, /* MODEM.MODINDEX */
  /*    408C */ 0x1B8C805BUL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x0E050200UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x53432F1CUL, /* MODEM.SHAPING1 */
  /*    409C */ 0x61686962UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x04324355UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0xC6B8E416UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x3345678AUL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x04924A22UL, /* MODEM.SHAPING6 */
  0x010740ECUL, 0x8CB1B080UL, /* Write: MODEM.SRCCHF */
  /*    40F0 */ 0x00000000UL, /* MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A209CUL, /* MODEM.DSACTRL */
  /*    4100 */ 0x80206101UL, /* MODEM.VITERBIDEMOD */
  /*    4104 */ 0x123556B7UL, /* MODEM.VTCORRCFG0 */
  0x01024110UL, 0x29041BA0UL, /* Write: MODEM.VTCORRCFG1 */
  /*    4114 */ 0x00408808UL, /* MODEM.VTTRACK */
  0x01014128UL, 0x00000000UL, /* Write: MODEM.CGCLKSTOP */
  0x01094130UL, 0x0C660664UL, /* Write: MODEM.DSATHD2 */
  /*    4134 */ 0x0000010CUL, /* MODEM.DIRECTMODE */
  /*    4138 */ 0x3BE8D44CUL, /* MODEM.LONGRANGE */
  /*    413C */ 0x306F1904UL, /* MODEM.LONGRANGE1 */
  /*    4140 */ 0x1C16100AUL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x342E2822UL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x3210403AUL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x0A987654UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0xA4805F1DUL, /* MODEM.LONGRANGE6 */
  0x01034168UL, 0x07830464UL, /* Write: MODEM.DSATHD3 */
  /*    416C */ 0x00821388UL, /* MODEM.DSATHD4 */
  /*    4170 */ 0x00000000UL, /* MODEM.VTBLETIMING */
  0x01024230UL, 0x00000000UL, /* Write: MODEM.BLEIQDSA */
  /*    4234 */ 0x0E000800UL, /* MODEM.BLEIQDSAEXT1 */
  0x010280A0UL, 0x00033870UL, /* Write: SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x000000D0UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x01FD5A05UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x00FD3E05UL, /* OR: SYNTH.LPFCTRL2TX */
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

const RAIL_ChannelConfigEntry_t ble_LR_DSA_125kb_channels[] = {
  {
    .phyConfigDeltaAdd = ble_LR_DSA_125kb_modemConfig,
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
    .stackInfo = stackInfo_2,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfigEntry_t ble_LR_DSA_500kb_channels[] = {
  {
    .phyConfigDeltaAdd = ble_LR_DSA_500kb_modemConfig,
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
    .stackInfo = stackInfo_3,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfig_t ble_viterbi1M_channelConfig = {
  .phyConfigBase = ble_viterbi1M_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ble_viterbi1M_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

const RAIL_ChannelConfig_t ble_viterbi2M_channelConfig = {
  .phyConfigBase = ble_viterbi1M_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ble_viterbi2M_channels,
  .length = 6U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

const RAIL_ChannelConfig_t ble_LR_DSA_125kb_channelConfig = {
  .phyConfigBase = ble_viterbi1M_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ble_LR_DSA_125kb_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

const RAIL_ChannelConfig_t ble_LR_DSA_500kb_channelConfig = {
  .phyConfigBase = ble_viterbi1M_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ble_LR_DSA_500kb_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

uint32_t bleAccelerationBuffer[135];
