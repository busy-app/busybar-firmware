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
#include "ieee802154_efr32xg21_configurator_out.h"

static const uint8_t irCalConfig_0[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0
};

static const uint8_t irCalConfig_1[] = {
  25, 63, 1, 6, 4, 16, 1, 0, 0, 1, 1, 6, 0, 16, 39, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int32_t timingConfig_0[] = {
  0, 0, 0, 0
};

static const int32_t timingConfig_1[] = {
  50000, 5500, 1500, 1500
};

static const int32_t timingConfig_2[] = {
  6625, 6625, 0, 0
};

#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
__ALIGNED(4) static const uint8_t stackInfo_0[2] = { 0x00, 0x00 };
__ALIGNED(4) static const uint8_t stackInfo_1[2] = { 0x03, 0x00 };
__ALIGNED(4) static const uint8_t stackInfo_2[2] = { 0x05, 0x02 };
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
  0x00666666UL, // 102.4
  (uint32_t) NULL,
  (uint32_t) irCalConfig_0,
  (uint32_t) timingConfig_0,
  0x00000000UL,
  10099200UL,
  40000000UL,
  2000000UL,
  0x00FE2004UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  2000049UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_1[] = {
  20UL,
  0x00666666UL, // 102.4
  (uint32_t) NULL,
  (uint32_t) irCalConfig_1,
  (uint32_t) timingConfig_1,
  0x00000000UL,
  8800000UL,
  40000000UL,
  2000000UL,
  0x00000101UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  1999937UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_2[] = {
  20UL,
  0x00666666UL, // 102.4
  (uint32_t) NULL,
  (uint32_t) irCalConfig_0,
  (uint32_t) timingConfig_2,
  0x00000C00UL,
  10099200UL,
  40000000UL,
  2000000UL,
  0x00002004UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  2000049UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_3[] = {
  20UL,
  0x00666666UL, // 102.4
  (uint32_t) NULL,
  (uint32_t) irCalConfig_0,
  (uint32_t) timingConfig_2,
  0x00000A00UL,
  10099200UL,
  40000000UL,
  2000000UL,
  0x00002004UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  2000049UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

static const uint32_t phyInfo_4[] = {
  20UL,
  0x00666666UL, // 102.4
  (uint32_t) NULL,
  (uint32_t) irCalConfig_0,
  (uint32_t) timingConfig_2,
  0x00000000UL,
  10099200UL,
  40000000UL,
  2000000UL,
  0x00FE2004UL,
  0x02504911UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  0UL,
  0UL,
  2000049UL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) NULL,
  (uint32_t) 0UL,
};

const uint32_t ieee802154_2p4_2mbps_modemConfigBase[] = {
  0x00024020UL, 0x00000000UL, /* Write: FRC.WCNTCMP0 */
  /*    4024 */ 0x00000000UL, /* FRC.WCNTCMP1 */
  0x00034040UL, 0x00000000UL, /* Write: FRC.BLOCKRAMADDR */
  /*    4044 */ 0x00000000UL, /* FRC.CONVRAMADDR */
  /*    4048 */ 0x000007A0UL, /* FRC.CTRL */
  0x0001C008UL, 0x00000000UL, /* Write: AGC.STATUS0 */
  0x0004C024UL, 0x03B380ECUL, /* Write: AGC.CTRL2 */
  /*    C028 */ 0x02A87543UL, /* AGC.CTRL3 */
  /*    C02C */ 0x00000000UL, /* AGC.RSSISTEPTHR */
  /*    C030 */ 0x0000022EUL, /* AGC.MANGAIN */
  0x0003C054UL, 0xE6092D0EUL, /* Write: AGC.AGCPERIOD */
  /*    C058 */ 0x08070654UL, /* AGC.HICNTREGION */
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
  0x010240E0UL, 0x00000033UL, /* Write: MODEM.DCCOMP */
  /*    40E4 */ 0x00000000UL, /* MODEM.DCCOMPFILTINIT */
  0x01024100UL, 0x00206100UL, /* Write: MODEM.VITERBIDEMOD */
  /*    4104 */ 0x123556B7UL, /* MODEM.VTCORRCFG0 */
  0x0103410CUL, 0x001254F9UL, /* Write: MODEM.DIGMIXCTRL */
  /*    4110 */ 0x29043020UL, /* MODEM.VTCORRCFG1 */
  /*    4114 */ 0x0040BB88UL, /* MODEM.VTTRACK */
  0x01034130UL, 0x0C660664UL, /* Write: MODEM.DSATHD2 */
  /*    4134 */ 0x0000010CUL, /* MODEM.DIRECTMODE */
  /*    4138 */ 0x00FA53E8UL, /* MODEM.LONGRANGE */
  0x01034168UL, 0x07830464UL, /* Write: MODEM.DSATHD3 */
  /*    416C */ 0x00821388UL, /* MODEM.DSATHD4 */
  /*    4170 */ 0x00000000UL, /* MODEM.VTBLETIMING */
  0x01034230UL, 0x00000000UL, /* Write: MODEM.BLEIQDSA */
  /*    4234 */ 0x0E000000UL, /* MODEM.BLEIQDSAEXT1 */
  /*    4238 */ 0x00000000UL, /* MODEM.SYNCPROPERTIES */
  0x01014244UL, 0x00000000UL, /* Write: MODEM.PADEBUG */
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
  0x02020088UL, 0x04000C00UL, /* Write: RAC.SYNTHREGCTRL */
  /*    008C */ 0x0000004CUL, /* RAC.VCOCTRL */
  0x02010094UL, 0x00000400UL, /* Write: RAC.SYNTHCTRL */
  0x020100D4UL, 0x08960290UL, /* Write: RAC.IFADCTRIM */
  0x020100E4UL, 0x0442093DUL, /* Write: RAC.LNAMIXTRIM0 */
  0x02010118UL, 0x02510060UL, /* Write: RAC.PGACTRL */
  0xFFFFFFFFUL,
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
  /*    4044 */ 0x00000000UL, /* FRC.CONVRAMADDR */
  /*    4048 */ 0x000007A0UL, /* FRC.CTRL */
  0x00014050UL, 0x00000000UL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000000UL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000000UL, /* FRC.PUNCTCTRL */
  0x000140A8UL, 0x00000007UL, /* Write: FRC.AUTOCG */
  0x000440B4UL, 0x00004000UL, /* Write: FRC.FCD0 */
  /*    40B8 */ 0x00004CFFUL, /* FRC.FCD1 */
  /*    40BC */ 0x00004100UL, /* FRC.FCD2 */
  /*    40C0 */ 0x00004DFFUL, /* FRC.FCD3 */
  0x0001C008UL, 0x00000000UL, /* Write: AGC.STATUS0 */
  0x1001C020UL, 0x000000FFUL, /* AND: AGC.CTRL1 */
  0x3001C020UL, 0x00004300UL, /* OR: AGC.CTRL1 */
  0x0004C024UL, 0x03B380ECUL, /* Write: AGC.CTRL2 */
  /*    C028 */ 0x02A87543UL, /* AGC.CTRL3 */
  /*    C02C */ 0x00000000UL, /* AGC.RSSISTEPTHR */
  /*    C030 */ 0x0000022EUL, /* AGC.MANGAIN */
  0x0003C054UL, 0xE6092D0EUL, /* Write: AGC.AGCPERIOD */
  /*    C058 */ 0x08070654UL, /* AGC.HICNTREGION */
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
  0x01010008UL, 0x00000704UL, /* Write: RFCRC.CTRL */
  0x01010018UL, 0x00000000UL, /* Write: RFCRC.INIT */
  0x01010020UL, 0x00008408UL, /* Write: RFCRC.POLY */
  0x0102401CUL, 0x00000010UL, /* Write: MODEM.MIXCTRL */
  /*    4020 */ 0x0413F920UL, /* MODEM.CTRL0 */
  0x0105405CUL, 0x00000841UL, /* Write: MODEM.RXBR */
  /*    4060 */ 0x00000001UL, /* MODEM.CF */
  /*    4064 */ 0x000807B0UL, /* MODEM.PRE */
  /*    4068 */ 0x000000A7UL, /* MODEM.SYNC0 */
  /*    406C */ 0x00000000UL, /* MODEM.SYNC1 */
  0x01074094UL, 0x30100101UL, /* Write: MODEM.SHAPING0 */
  /*    4098 */ 0x7F7F7050UL, /* MODEM.SHAPING1 */
  /*    409C */ 0x00000000UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x00000000UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0x00000000UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x00000000UL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x00000000UL, /* MODEM.SHAPING6 */
  0x010140E4UL, 0x00000000UL, /* Write: MODEM.DCCOMPFILTINIT */
  0x010140ECUL, 0x8BD9A87DUL, /* Write: MODEM.SRCCHF */
  0x01024100UL, 0x00206100UL, /* Write: MODEM.VITERBIDEMOD */
  /*    4104 */ 0x123556B7UL, /* MODEM.VTCORRCFG0 */
  0x0103410CUL, 0x001254F9UL, /* Write: MODEM.DIGMIXCTRL */
  /*    4110 */ 0x29043020UL, /* MODEM.VTCORRCFG1 */
  /*    4114 */ 0x0040BB88UL, /* MODEM.VTTRACK */
  0x01014124UL, 0x00000000UL, /* Write: MODEM.AUTOCG */
  0x01034130UL, 0x0C660664UL, /* Write: MODEM.DSATHD2 */
  /*    4134 */ 0x0000010CUL, /* MODEM.DIRECTMODE */
  /*    4138 */ 0x00FA53E8UL, /* MODEM.LONGRANGE */
  0x01034168UL, 0x07830464UL, /* Write: MODEM.DSATHD3 */
  /*    416C */ 0x00821388UL, /* MODEM.DSATHD4 */
  /*    4170 */ 0x00000000UL, /* MODEM.VTBLETIMING */
  0x01034230UL, 0x00000000UL, /* Write: MODEM.BLEIQDSA */
  /*    4234 */ 0x0E000000UL, /* MODEM.BLEIQDSAEXT1 */
  /*    4238 */ 0x00000000UL, /* MODEM.SYNCPROPERTIES */
  0x01014244UL, 0x00000000UL, /* Write: MODEM.PADEBUG */
  0x01018010UL, 0x00000003UL, /* Write: SYNTH.CTRL */
  0x01028038UL, 0x00104911UL, /* Write: SYNTH.IFFREQ */
  /*    803C */ 0x00000001UL, /* SYNTH.DIVCTRL */
  0x0101809CUL, 0x000240EBUL, /* Write: SYNTH.LPFCTRL1CAL */
  0x010180B4UL, 0x01000037UL, /* Write: SYNTH.DSMCTRLTX */
  0x02020088UL, 0x04000C00UL, /* Write: RAC.SYNTHREGCTRL */
  /*    008C */ 0x0000004CUL, /* RAC.VCOCTRL */
  0x02010094UL, 0x00000400UL, /* Write: RAC.SYNTHCTRL */
  0x020100D4UL, 0x08960290UL, /* Write: RAC.IFADCTRIM */
  0x020100E4UL, 0x0442093DUL, /* Write: RAC.LNAMIXTRIM0 */
  0x02010118UL, 0x02510060UL, /* Write: RAC.PGACTRL */
  0xFFFFFFFFUL,
};

const uint32_t ieee802154_2p4_dual_sync_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_0, /* Write: SEQ.PHYINFO */
  0x0002400CUL, 0x00148001UL, /* Write: FRC.DFLCTRL */
  /*    4010 */ 0x0000407FUL, /* FRC.MAXLENGTH */
  0x00044030UL, 0x00000000UL, /* Write: FRC.WHITECTRL */
  /*    4034 */ 0x00000000UL, /* FRC.WHITEPOLY */
  /*    4038 */ 0x00000000UL, /* FRC.WHITEINIT */
  /*    403C */ 0x00000000UL, /* FRC.FECCTRL */
  0x00014050UL, 0x00000000UL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000000UL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000000UL, /* FRC.PUNCTCTRL */
  0x000140A8UL, 0x00000007UL, /* Write: FRC.AUTOCG */
  0x000440B4UL, 0x00004000UL, /* Write: FRC.FCD0 */
  /*    40B8 */ 0x00004CFFUL, /* FRC.FCD1 */
  /*    40BC */ 0x00004100UL, /* FRC.FCD2 */
  /*    40C0 */ 0x00004DFFUL, /* FRC.FCD3 */
  0x1001C01CUL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C01CUL, 0x00080214UL, /* OR: AGC.CTRL0 */
  0x1001C020UL, 0x000000FFUL, /* AND: AGC.CTRL1 */
  0x3001C020UL, 0x00004300UL, /* OR: AGC.CTRL1 */
  0x0001C050UL, 0x04304187UL, /* Write: AGC.GAINRANGE */
  0x01010008UL, 0x00000704UL, /* Write: RFCRC.CTRL */
  0x01010018UL, 0x00000000UL, /* Write: RFCRC.INIT */
  0x01010020UL, 0x00008408UL, /* Write: RFCRC.POLY */
  0x0108401CUL, 0x00000010UL, /* Write: MODEM.MIXCTRL */
  /*    4020 */ 0x0413F920UL, /* MODEM.CTRL0 */
  /*    4024 */ 0x00620207UL, /* MODEM.CTRL1 */
  /*    4028 */ 0x00007038UL, /* MODEM.CTRL2 */
  /*    402C */ 0x00000000UL, /* MODEM.CTRL3 */
  /*    4030 */ 0x03000000UL, /* MODEM.CTRL4 */
  /*    4034 */ 0x0F016800UL, /* MODEM.CTRL5 */
  /*    4038 */ 0x4024B840UL, /* MODEM.CTRL6 */
  0x01064058UL, 0x006900FCUL, /* Write: MODEM.TXBR */
  /*    405C */ 0x00000841UL, /* MODEM.RXBR */
  /*    4060 */ 0x00000001UL, /* MODEM.CF */
  /*    4064 */ 0x000807B0UL, /* MODEM.PRE */
  /*    4068 */ 0x000000A7UL, /* MODEM.SYNC0 */
  /*    406C */ 0x0000002FUL, /* MODEM.SYNC1 */
  0x010C4080UL, 0x11E06323UL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x744AC39BUL, /* MODEM.DSSS0 */
  /*    4088 */ 0x000003F0UL, /* MODEM.MODINDEX */
  /*    408C */ 0x00008003UL, /* MODEM.AFC */
  /*    4090 */ 0x00000ABEUL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x30100101UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x7F7F7050UL, /* MODEM.SHAPING1 */
  /*    409C */ 0x00000000UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x00000000UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0x00000000UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x00000000UL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x00000000UL, /* MODEM.SHAPING6 */
  0x010540ECUL, 0x8BD9A87DUL, /* Write: MODEM.SRCCHF */
  /*    40F0 */ 0x00000B59UL, /* MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A2090UL, /* MODEM.DSACTRL */
  0x01024124UL, 0x00000000UL, /* Write: MODEM.AUTOCG */
  /*    4128 */ 0x00001E00UL, /* MODEM.CGCLKSTOP */
  0x010B413CUL, 0x008C1400UL, /* Write: MODEM.LONGRANGE1 */
  /*    4140 */ 0x28211A14UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x423B342EUL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x55435049UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x0CBA9876UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0x00006928UL, /* MODEM.LONGRANGE6 */
  /*    4154 */ 0x00000001UL, /* MODEM.LRFRC */
  /*    4158 */ 0x64282107UL, /* MODEM.COH0 */
  /*    415C */ 0x001A1714UL, /* MODEM.COH1 */
  /*    4160 */ 0x69C80420UL, /* MODEM.COH2 */
  /*    4164 */ 0x093A20A1UL, /* MODEM.COH3 */
  0x0101423CUL, 0x00000029UL, /* Write: MODEM.DIGIGAINCTRL */
  0xFFFFFFFFUL,
};

const uint32_t ieee802154_2p4_high_speed_2mbps_coherent_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_1, /* Write: SEQ.PHYINFO */
  0x0002400CUL, 0x0011B10CUL, /* Write: FRC.DFLCTRL */
  /*    4010 */ 0x00004100UL, /* FRC.MAXLENGTH */
  0x00044030UL, 0x00000825UL, /* Write: FRC.WHITECTRL */
  /*    4034 */ 0x00000100UL, /* FRC.WHITEPOLY */
  /*    4038 */ 0x000000FFUL, /* FRC.WHITEINIT */
  /*    403C */ 0x00010301UL, /* FRC.FECCTRL */
  0x00014050UL, 0x0000000BUL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000D0FUL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000101UL, /* FRC.PUNCTCTRL */
  0x000440B4UL, 0x00004001UL, /* Write: FRC.FCD0 */
  /*    40B8 */ 0x00000CFFUL, /* FRC.FCD1 */
  /*    40BC */ 0x00004101UL, /* FRC.FCD2 */
  /*    40C0 */ 0x00000DFFUL, /* FRC.FCD3 */
  0x1001C01CUL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C01CUL, 0x000802F5UL, /* OR: AGC.CTRL0 */
  0x1001C020UL, 0x000000FFUL, /* AND: AGC.CTRL1 */
  0x3001C020UL, 0x00001300UL, /* OR: AGC.CTRL1 */
  0x0001C050UL, 0x0C304187UL, /* Write: AGC.GAINRANGE */
  0x01010008UL, 0x0000170EUL, /* Write: RFCRC.CTRL */
  0x01010018UL, 0xFFFFFFFFUL, /* Write: RFCRC.INIT */
  0x01010020UL, 0xEDB88320UL, /* Write: RFCRC.POLY */
  0x0108401CUL, 0x00000010UL, /* Write: MODEM.MIXCTRL */
  /*    4020 */ 0x04000000UL, /* MODEM.CTRL0 */
  /*    4024 */ 0x0001C01FUL, /* MODEM.CTRL1 */
  /*    4028 */ 0x00003000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x000A1000UL, /* MODEM.CTRL3 */
  /*    4030 */ 0x03000000UL, /* MODEM.CTRL4 */
  /*    4034 */ 0x00000000UL, /* MODEM.CTRL5 */
  /*    4038 */ 0x00000000UL, /* MODEM.CTRL6 */
  0x01064058UL, 0x00FF0264UL, /* Write: MODEM.TXBR */
  /*    405C */ 0x00000841UL, /* MODEM.RXBR */
  /*    4060 */ 0x00000001UL, /* MODEM.CF */
  /*    4064 */ 0x00080012UL, /* MODEM.PRE */
  /*    4068 */ 0xAB784E7CUL, /* MODEM.SYNC0 */
  /*    406C */ 0x00000000UL, /* MODEM.SYNC1 */
  0x010C4080UL, 0x08AC0032UL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x00000000UL, /* MODEM.DSSS0 */
  /*    4088 */ 0x002A03DFUL, /* MODEM.MODINDEX */
  /*    408C */ 0x00000000UL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x11070201UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x5A4B3721UL, /* MODEM.SHAPING1 */
  /*    409C */ 0x54626967UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x05152940UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0x20800000UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x33334443UL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x24A49223UL, /* MODEM.SHAPING6 */
  0x010540ECUL, 0x8B57A890UL, /* Write: MODEM.SRCCHF */
  /*    40F0 */ 0x00000000UL, /* MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A209CUL, /* MODEM.DSACTRL */
  0x01024124UL, 0x00000000UL, /* Write: MODEM.AUTOCG */
  /*    4128 */ 0x00000000UL, /* MODEM.CGCLKSTOP */
  0x010B413CUL, 0x00000000UL, /* Write: MODEM.LONGRANGE1 */
  /*    4140 */ 0x00000000UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x00000000UL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x00000000UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x00000000UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0x00000000UL, /* MODEM.LONGRANGE6 */
  /*    4154 */ 0x00000000UL, /* MODEM.LRFRC */
  /*    4158 */ 0x00000000UL, /* MODEM.COH0 */
  /*    415C */ 0x00000000UL, /* MODEM.COH1 */
  /*    4160 */ 0x00000000UL, /* MODEM.COH2 */
  /*    4164 */ 0x00000000UL, /* MODEM.COH3 */
  0x0101423CUL, 0x00000000UL, /* Write: MODEM.DIGIGAINCTRL */
  0xFFFFFFFFUL,
};

const uint32_t ieee802154_2p4_high_speed_1mbps_fec_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_1, /* Write: SEQ.PHYINFO */
  0x0002400CUL, 0x0011B10CUL, /* Write: FRC.DFLCTRL */
  /*    4010 */ 0x00004100UL, /* FRC.MAXLENGTH */
  0x00044030UL, 0x00000825UL, /* Write: FRC.WHITECTRL */
  /*    4034 */ 0x00000100UL, /* FRC.WHITEPOLY */
  /*    4038 */ 0x000000FFUL, /* FRC.WHITEINIT */
  /*    403C */ 0x00010714UL, /* FRC.FECCTRL */
  0x00014050UL, 0x0000000BUL, /* Write: FRC.TRAILTXDATACTRL */
  0x0002405CUL, 0x00000D0FUL, /* Write: FRC.CONVGENERATOR */
  /*    4060 */ 0x00000101UL, /* FRC.PUNCTCTRL */
  0x000440B4UL, 0x00004001UL, /* Write: FRC.FCD0 */
  /*    40B8 */ 0x00000CFFUL, /* FRC.FCD1 */
  /*    40BC */ 0x00004101UL, /* FRC.FCD2 */
  /*    40C0 */ 0x00000DFFUL, /* FRC.FCD3 */
  0x1001C01CUL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C01CUL, 0x000802F5UL, /* OR: AGC.CTRL0 */
  0x1001C020UL, 0x000000FFUL, /* AND: AGC.CTRL1 */
  0x3001C020UL, 0x00001300UL, /* OR: AGC.CTRL1 */
  0x0001C050UL, 0x0C304187UL, /* Write: AGC.GAINRANGE */
  0x01010008UL, 0x0000170EUL, /* Write: RFCRC.CTRL */
  0x01010018UL, 0xFFFFFFFFUL, /* Write: RFCRC.INIT */
  0x01010020UL, 0xEDB88320UL, /* Write: RFCRC.POLY */
  0x0108401CUL, 0x00000010UL, /* Write: MODEM.MIXCTRL */
  /*    4020 */ 0x04000000UL, /* MODEM.CTRL0 */
  /*    4024 */ 0x0001C01FUL, /* MODEM.CTRL1 */
  /*    4028 */ 0x00003000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x000A1000UL, /* MODEM.CTRL3 */
  /*    4030 */ 0x03000000UL, /* MODEM.CTRL4 */
  /*    4034 */ 0x00000000UL, /* MODEM.CTRL5 */
  /*    4038 */ 0x00000000UL, /* MODEM.CTRL6 */
  0x01064058UL, 0x00FF0264UL, /* Write: MODEM.TXBR */
  /*    405C */ 0x00000841UL, /* MODEM.RXBR */
  /*    4060 */ 0x00000001UL, /* MODEM.CF */
  /*    4064 */ 0x00080012UL, /* MODEM.PRE */
  /*    4068 */ 0x5487B183UL, /* MODEM.SYNC0 */
  /*    406C */ 0x00000000UL, /* MODEM.SYNC1 */
  0x010C4080UL, 0x08AC0032UL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x00000000UL, /* MODEM.DSSS0 */
  /*    4088 */ 0x002A03DFUL, /* MODEM.MODINDEX */
  /*    408C */ 0x00000000UL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  /*    4094 */ 0x11070201UL, /* MODEM.SHAPING0 */
  /*    4098 */ 0x5A4B3721UL, /* MODEM.SHAPING1 */
  /*    409C */ 0x54626967UL, /* MODEM.SHAPING2 */
  /*    40A0 */ 0x05152940UL, /* MODEM.SHAPING3 */
  /*    40A4 */ 0x20800000UL, /* MODEM.SHAPING4 */
  /*    40A8 */ 0x33334443UL, /* MODEM.SHAPING5 */
  /*    40AC */ 0x24A49223UL, /* MODEM.SHAPING6 */
  0x010540ECUL, 0x8B57A890UL, /* Write: MODEM.SRCCHF */
  /*    40F0 */ 0x00000000UL, /* MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A209CUL, /* MODEM.DSACTRL */
  0x01024124UL, 0x00000000UL, /* Write: MODEM.AUTOCG */
  /*    4128 */ 0x00000000UL, /* MODEM.CGCLKSTOP */
  0x010B413CUL, 0x00000000UL, /* Write: MODEM.LONGRANGE1 */
  /*    4140 */ 0x00000000UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x00000000UL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x00000000UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x00000000UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0x00000000UL, /* MODEM.LONGRANGE6 */
  /*    4154 */ 0x00000000UL, /* MODEM.LRFRC */
  /*    4158 */ 0x00000000UL, /* MODEM.COH0 */
  /*    415C */ 0x00000000UL, /* MODEM.COH1 */
  /*    4160 */ 0x00000000UL, /* MODEM.COH2 */
  /*    4164 */ 0x00000000UL, /* MODEM.COH3 */
  0x0101423CUL, 0x00000000UL, /* Write: MODEM.DIGIGAINCTRL */
  0xFFFFFFFFUL,
};

const uint32_t ieee802154_2p4_antdiv_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_2, /* Write: SEQ.PHYINFO */
  0x1001C01CUL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C01CUL, 0x000802F5UL, /* OR: AGC.CTRL0 */
  0x0001C050UL, 0x0C304187UL, /* Write: AGC.GAINRANGE */
  0x01064024UL, 0x0052C007UL, /* Write: MODEM.CTRL1 */
  /*    4028 */ 0x00000000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x00000000UL, /* MODEM.CTRL3 */
  /*    4030 */ 0x03000000UL, /* MODEM.CTRL4 */
  /*    4034 */ 0x00000000UL, /* MODEM.CTRL5 */
  /*    4038 */ 0x00000000UL, /* MODEM.CTRL6 */
  0x01014058UL, 0x00FF0264UL, /* Write: MODEM.TXBR */
  0x01054080UL, 0x08A0014BUL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x744AC39BUL, /* MODEM.DSSS0 */
  /*    4088 */ 0x000003F0UL, /* MODEM.MODINDEX */
  /*    408C */ 0x00000000UL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  0x010140E0UL, 0x00000033UL, /* Write: MODEM.DCCOMP */
  0x010440F0UL, 0x00000000UL, /* Write: MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A209CUL, /* MODEM.DSACTRL */
  0x01014128UL, 0x00001003UL, /* Write: MODEM.CGCLKSTOP */
  0x010B413CUL, 0x00000000UL, /* Write: MODEM.LONGRANGE1 */
  /*    4140 */ 0x00000000UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x00000000UL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x00000000UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x00000000UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0x00000000UL, /* MODEM.LONGRANGE6 */
  /*    4154 */ 0x00000001UL, /* MODEM.LRFRC */
  /*    4158 */ 0x00000000UL, /* MODEM.COH0 */
  /*    415C */ 0x00000000UL, /* MODEM.COH1 */
  /*    4160 */ 0x00000000UL, /* MODEM.COH2 */
  /*    4164 */ 0x00000000UL, /* MODEM.COH3 */
  0x0101423CUL, 0x00000000UL, /* Write: MODEM.DIGIGAINCTRL */
  0x010280A0UL, 0x00037870UL, /* Write: SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x0000C0D5UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x01CB4205UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x008D2205UL, /* OR: SYNTH.LPFCTRL2TX */
  0x010180B0UL, 0x02000300UL, /* Write: SYNTH.DSMCTRLRX */
  0xFFFFFFFFUL,
};

const uint32_t ieee802154_2p4_rx_channel_switching_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_3, /* Write: SEQ.PHYINFO */
  0x1001C01CUL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C01CUL, 0x000802F5UL, /* OR: AGC.CTRL0 */
  0x0001C050UL, 0x0C304187UL, /* Write: AGC.GAINRANGE */
  0x01064024UL, 0x0042C007UL, /* Write: MODEM.CTRL1 */
  /*    4028 */ 0x00000000UL, /* MODEM.CTRL2 */
  /*    402C */ 0x00000000UL, /* MODEM.CTRL3 */
  /*    4030 */ 0x03000000UL, /* MODEM.CTRL4 */
  /*    4034 */ 0x00000000UL, /* MODEM.CTRL5 */
  /*    4038 */ 0x00003820UL, /* MODEM.CTRL6 */
  0x01014058UL, 0x00FF0264UL, /* Write: MODEM.TXBR */
  0x01054080UL, 0x08A0025CUL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x744AC39BUL, /* MODEM.DSSS0 */
  /*    4088 */ 0x000003F0UL, /* MODEM.MODINDEX */
  /*    408C */ 0x00000000UL, /* MODEM.AFC */
  /*    4090 */ 0x00000000UL, /* MODEM.AFCADJLIM */
  0x010140E0UL, 0x00000073UL, /* Write: MODEM.DCCOMP */
  0x010440F0UL, 0x00000111UL, /* Write: MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A209CUL, /* MODEM.DSACTRL */
  0x01014128UL, 0x00001003UL, /* Write: MODEM.CGCLKSTOP */
  0x010B413CUL, 0x00000000UL, /* Write: MODEM.LONGRANGE1 */
  /*    4140 */ 0x00000000UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x00000000UL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x00000000UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x00000000UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0x00000000UL, /* MODEM.LONGRANGE6 */
  /*    4154 */ 0x00000001UL, /* MODEM.LRFRC */
  /*    4158 */ 0x00000000UL, /* MODEM.COH0 */
  /*    415C */ 0x00000000UL, /* MODEM.COH1 */
  /*    4160 */ 0x00000000UL, /* MODEM.COH2 */
  /*    4164 */ 0x00000000UL, /* MODEM.COH3 */
  0x0101423CUL, 0x00000000UL, /* Write: MODEM.DIGIGAINCTRL */
  0x010280A0UL, 0x0000C0D5UL, /* Write: SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x0000C0D5UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x008D2205UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x008D2205UL, /* OR: SYNTH.LPFCTRL2TX */
  0x010180B0UL, 0x01000037UL, /* Write: SYNTH.DSMCTRLRX */
  0xFFFFFFFFUL,
};

const uint32_t ieee802154_2p4_coherent_modemConfig[] = {
  0x03011FFCUL, (uint32_t) &phyInfo_4, /* Write: SEQ.PHYINFO */
  0x1001C01CUL, 0x0007F800UL, /* AND: AGC.CTRL0 */
  0x3001C01CUL, 0x00080214UL, /* OR: AGC.CTRL0 */
  0x0001C050UL, 0x04304187UL, /* Write: AGC.GAINRANGE */
  0x01064024UL, 0x00620007UL, /* Write: MODEM.CTRL1 */
  /*    4028 */ 0x00007038UL, /* MODEM.CTRL2 */
  /*    402C */ 0x00000000UL, /* MODEM.CTRL3 */
  /*    4030 */ 0x03000000UL, /* MODEM.CTRL4 */
  /*    4034 */ 0x0F016800UL, /* MODEM.CTRL5 */
  /*    4038 */ 0x4024B840UL, /* MODEM.CTRL6 */
  0x01014058UL, 0x006900FCUL, /* Write: MODEM.TXBR */
  0x01054080UL, 0x11E06323UL, /* Write: MODEM.TIMING */
  /*    4084 */ 0x744AC39BUL, /* MODEM.DSSS0 */
  /*    4088 */ 0x000003F0UL, /* MODEM.MODINDEX */
  /*    408C */ 0x00008003UL, /* MODEM.AFC */
  /*    4090 */ 0x00000ABEUL, /* MODEM.AFCADJLIM */
  0x010140E0UL, 0x00000033UL, /* Write: MODEM.DCCOMP */
  0x010440F0UL, 0x00000B59UL, /* Write: MODEM.INTAFC */
  /*    40F4 */ 0x07830464UL, /* MODEM.DSATHD0 */
  /*    40F8 */ 0x3AC81388UL, /* MODEM.DSATHD1 */
  /*    40FC */ 0x000A2090UL, /* MODEM.DSACTRL */
  0x01014128UL, 0x00001E00UL, /* Write: MODEM.CGCLKSTOP */
  0x010B413CUL, 0x008C1400UL, /* Write: MODEM.LONGRANGE1 */
  /*    4140 */ 0x28211A14UL, /* MODEM.LONGRANGE2 */
  /*    4144 */ 0x423B342EUL, /* MODEM.LONGRANGE3 */
  /*    4148 */ 0x55435049UL, /* MODEM.LONGRANGE4 */
  /*    414C */ 0x0CBA9876UL, /* MODEM.LONGRANGE5 */
  /*    4150 */ 0x00006928UL, /* MODEM.LONGRANGE6 */
  /*    4154 */ 0x00000001UL, /* MODEM.LRFRC */
  /*    4158 */ 0x64282107UL, /* MODEM.COH0 */
  /*    415C */ 0x001A1714UL, /* MODEM.COH1 */
  /*    4160 */ 0x69C80420UL, /* MODEM.COH2 */
  /*    4164 */ 0x093A20A1UL, /* MODEM.COH3 */
  0x0101423CUL, 0x00000029UL, /* Write: MODEM.DIGIGAINCTRL */
  0x010280A0UL, 0x00037870UL, /* Write: SYNTH.LPFCTRL1RX */
  /*    80A4 */ 0x0000C0D5UL, /* SYNTH.LPFCTRL1TX */
  0x110180A8UL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2RX */
  0x310180A8UL, 0x01CB4205UL, /* OR: SYNTH.LPFCTRL2RX */
  0x110180ACUL, 0x000001F0UL, /* AND: SYNTH.LPFCTRL2TX */
  0x310180ACUL, 0x008D2205UL, /* OR: SYNTH.LPFCTRL2TX */
  0x010180B0UL, 0x02000300UL, /* Write: SYNTH.DSMCTRLRX */
  0xFFFFFFFFUL,
};

const RAIL_ChannelConfigEntry_t ieee802154_2p4_2mbps_channels[] = {
  {
    .phyConfigDeltaAdd = ieee802154_2p4_dual_sync_modemConfig,
    .baseFrequency = 2405000000,
    .channelSpacing = 5000000,
    .physicalChannelOffset = 11,
    .channelNumberStart = 11,
    .channelNumberEnd = 26,
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
  {
    .phyConfigDeltaAdd = ieee802154_2p4_high_speed_2mbps_coherent_modemConfig,
    .baseFrequency = 2405000000,
    .channelSpacing = 5000000,
    .physicalChannelOffset = 27,
    .channelNumberStart = 27,
    .channelNumberEnd = 42,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr_1,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfigEntry_t ieee802154_2p4_1mbps_fec_channels[] = {
  {
    .phyConfigDeltaAdd = ieee802154_2p4_dual_sync_modemConfig,
    .baseFrequency = 2405000000,
    .channelSpacing = 5000000,
    .physicalChannelOffset = 11,
    .channelNumberStart = 11,
    .channelNumberEnd = 26,
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
  {
    .phyConfigDeltaAdd = ieee802154_2p4_high_speed_1mbps_fec_modemConfig,
    .baseFrequency = 2405000000,
    .channelSpacing = 5000000,
    .physicalChannelOffset = 27,
    .channelNumberStart = 27,
    .channelNumberEnd = 42,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr_1,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_1,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfigEntry_t ieee802154_2p4_antdiv_channels[] = {
  {
    .phyConfigDeltaAdd = ieee802154_2p4_antdiv_modemConfig,
    .baseFrequency = 2405000000,
    .channelSpacing = 5000000,
    .physicalChannelOffset = 11,
    .channelNumberStart = 11,
    .channelNumberEnd = 26,
    .maxPower = RAIL_TX_POWER_MAX,
    .attr = &channelConfigEntryAttr_0,
#ifdef RADIO_CONFIG_ENABLE_CONC_PHY
    .entryType = 0,
#endif
#ifdef RADIO_CONFIG_ENABLE_STACK_INFO
    .stackInfo = stackInfo_2,
#endif
    .alternatePhy = NULL,
  },
};

const RAIL_ChannelConfigEntry_t ieee802154_2p4_rx_channel_switching_channels[] = {
  {
    .phyConfigDeltaAdd = ieee802154_2p4_rx_channel_switching_modemConfig,
    .baseFrequency = 2405000000,
    .channelSpacing = 5000000,
    .physicalChannelOffset = 11,
    .channelNumberStart = 11,
    .channelNumberEnd = 26,
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

const RAIL_ChannelConfigEntry_t ieee802154_2p4_coherent_channels[] = {
  {
    .phyConfigDeltaAdd = ieee802154_2p4_coherent_modemConfig,
    .baseFrequency = 2405000000,
    .channelSpacing = 5000000,
    .physicalChannelOffset = 11,
    .channelNumberStart = 11,
    .channelNumberEnd = 26,
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

const RAIL_ChannelConfig_t ieee802154_2p4_2mbps_channelConfig = {
  .phyConfigBase = ieee802154_2p4_2mbps_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ieee802154_2p4_2mbps_channels,
  .length = 2U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

const RAIL_ChannelConfig_t ieee802154_2p4_1mbps_fec_channelConfig = {
  .phyConfigBase = ieee802154_2p4_2mbps_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ieee802154_2p4_1mbps_fec_channels,
  .length = 2U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

const RAIL_ChannelConfig_t ieee802154_2p4_antdiv_channelConfig = {
  .phyConfigBase = ieee802154_2p4_antdiv_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ieee802154_2p4_antdiv_channels,
  .length = 1U,
  .signature = 0UL,
  .xtalFrequencyHz = 38400000UL,
};

const RAIL_ChannelConfig_t ieee802154_2p4_rx_channel_switching_channelConfig = {
  .phyConfigBase = ieee802154_2p4_antdiv_modemConfigBase,
  .phyConfigDeltaSubtract = NULL,
  .configs = ieee802154_2p4_rx_channel_switching_channels,
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

uint32_t ieee802154AccelerationBuffer[169];
