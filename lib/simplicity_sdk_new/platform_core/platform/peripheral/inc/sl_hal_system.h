/***************************************************************************//**
 * @file
 * @brief System API
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_HAL_SYSTEM_H
#define SL_HAL_SYSTEM_H

#include "em_device.h"
#include "sl_hal_system_generic.h"
#include "sl_enum.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup system SYSTEM - System Utils
 * @brief System API
 * @details
 *  This module contains functions to read information such as RAM and Flash size,
 *  device unique ID, chip revision, family, and part number from DEVINFO and
 *  SCB blocks. Functions to configure and read status from FPU are available for
 *  compatible devices.
 * @{
 ******************************************************************************/

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
#if defined(_SILICON_LABS_32B_SERIES_2)

#if defined(_CMU_HFRCOCTRL_BAND_MASK)
/// High-frequency system RCO bands.
SL_ENUM_GENERIC(sl_hal_system_hfrco_band_t, uint32_t) {
  SL_HAL_SYSTEM_HFRCO_BAND_1MHz  = _CMU_HFRCOCTRL_BAND_1MHZ,     ///< 1 MHz HFRCO band.
  SL_HAL_SYSTEM_HFRCO_BAND_7MHz  = _CMU_HFRCOCTRL_BAND_7MHZ,     ///< 7 MHz HFRCO band.
  SL_HAL_SYSTEM_HFRCO_BAND_11MHz = _CMU_HFRCOCTRL_BAND_11MHZ,    ///< 11 MHz HFRCO band.
  SL_HAL_SYSTEM_HFRCO_BAND_14MHz = _CMU_HFRCOCTRL_BAND_14MHZ,    ///< 14 MHz HFRCO band.
  SL_HAL_SYSTEM_HFRCO_BAND_21MHz = _CMU_HFRCOCTRL_BAND_21MHZ,    ///< 21 MHz HFRCO band.
#if defined(CMU_HFRCOCTRL_BAND_28MHZ)
  SL_HAL_SYSTEM_HFRCO_BAND_28MHz = _CMU_HFRCOCTRL_BAND_28MHZ,    ///< 28 MHz HFRCO band.
#endif
};
#elif defined(_CMU_HFRCOCTRL_FREQRANGE_MASK)
/// High-frequency system RCO bands.
SL_ENUM_GENERIC(sl_hal_system_hfrco_freq_t, uint32_t) {
  SL_HAL_SYSTEM_HFRCO_FREQ_1M0Hz            = 1000000U,             ///< 1 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_2M0Hz            = 2000000U,             ///< 2 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_4M0Hz            = 4000000U,             ///< 4 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_7M0Hz            = 7000000U,             ///< 7 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_13M0Hz           = 13000000U,            ///< 13 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_16M0Hz           = 16000000U,            ///< 16 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_19M0Hz           = 19000000U,            ///< 19 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_26M0Hz           = 26000000U,            ///< 26 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_32M0Hz           = 32000000U,            ///< 32 MHz RC band.
  SL_HAL_SYSTEM_HFRCO_FREQ_38M0Hz           = 38000000U,            ///< 38 MHz RC band.
#if defined(_DEVINFO_HFRCOCAL13_MASK)
  SL_HAL_SYSTEM_HFRCO_FREQ_48M0Hz           = 48000000U,            ///< 48 MHz RC band.
#endif
#if defined(_DEVINFO_HFRCOCAL14_MASK)
  SL_HAL_SYSTEM_HFRCO_FREQ_56M0Hz           = 56000000U,            ///< 56 MHz RC band.
#endif
#if defined(_DEVINFO_HFRCOCAL15_MASK)
  SL_HAL_SYSTEM_HFRCO_FREQ_64M0Hz           = 64000000U,            ///< 64 MHz RC band.
#endif
#if defined(_DEVINFO_HFRCOCAL16_MASK)
  SL_HAL_SYSTEM_HFRCO_FREQ_72M0Hz           = 72000000U,            ///< 72 MHz RC band.
#endif
  SL_HAL_SYSTEM_HFRCO_FREQ_USER_DEFINED     = 0,
};

/// HFRCO minimum frequency.
#define SL_HAL_SYSTEM_HFRCO_MIN           SL_HAL_SYSTEM_HFRCO_FREQ_1M0Hz
#if defined(_DEVINFO_HFRCOCAL16_MASK)
/// HFRCO maximum frequency.
#define SL_HAL_SYSTEM_HFRCO_MAX           SL_HAL_SYSTEM_HFRCO_FREQ_72M0Hz
#elif defined(_DEVINFO_HFRCOCAL15_MASK)
/// HFRCO maximum frequency.
#define SL_HAL_SYSTEM_HFRCO_MAX           SL_HAL_SYSTEM_HFRCO_FREQ_64M0Hz
#elif defined(_DEVINFO_HFRCOCAL14_MASK)
/// HFRCO maximum frequency.
#define SL_HAL_SYSTEM_HFRCO_MAX           SL_HAL_SYSTEM_HFRCO_FREQ_56M0Hz
#elif defined(_DEVINFO_HFRCOCAL13_MASK)
/// HFRCO maximum frequency.
#define SL_HAL_SYSTEM_HFRCO_MAX           SL_HAL_SYSTEM_HFRCO_FREQ_48M0Hz
#else
/// HFRCO maximum frequency.
#define SL_HAL_SYSTEM_HFRCO_MAX           SL_HAL_SYSTEM_HFRCO_FREQ_38M0Hz
#endif
#endif /* _CMU_HFRCOCTRL_BAND_MASK */

/** HFRCODPLL frequency bands */
SL_ENUM_GENERIC(sl_hal_system_hfrcodpll_freq_t, uint32_t) {
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_1M0Hz            = 1000000U,         /**< 1MHz RC band.  */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_2M0Hz            = 2000000U,         /**< 2MHz RC band.  */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_4M0Hz            = 4000000U,         /**< 4MHz RC band.  */
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7)  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9)
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_5M0Hz            = 5000000U,         /**< 5MHz RC band.  */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_10M0Hz           = 10000000U,        /**< 10MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_20M0Hz           = 20000000U,        /**< 20MHz RC band. */
#endif
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_7M0Hz            = 7000000U,         /**< 7MHz RC band.  */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_13M0Hz           = 13000000U,        /**< 13MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_16M0Hz           = 16000000U,        /**< 16MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_19M0Hz           = 19000000U,        /**< 19MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_26M0Hz           = 26000000U,        /**< 26MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_32M0Hz           = 32000000U,        /**< 32MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_38M0Hz           = 38000000U,        /**< 38MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_48M0Hz           = 48000000U,        /**< 48MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_56M0Hz           = 56000000U,        /**< 56MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_64M0Hz           = 64000000U,        /**< 64MHz RC band. */
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_80M0Hz           = 80000000U,        /**< 80MHz RC band. */
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_100M0Hz          = 100000000U,       /**< 100MHz RC band. */
#endif
  SL_HAL_SYSTEM_HFRCODPLL_FREQ_USER_DEFINED     = 0,
};

/** HFRCODPLL minimum frequency */
#define SL_HAL_SYSTEM_HFRCODPLL_MIN       SL_HAL_SYSTEM_HFRCODPLL_FREQ_1M0Hz
/** HFRCODPLL maximum frequency */
#define SL_HAL_SYSTEM_HFRCODPLL_MAX       SL_HAL_SYSTEM_HFRCODPLL_FREQ_80M0Hz

#if defined(HFRCOEM23_PRESENT)
/** HFRCOEM23 frequency bands */
SL_ENUM_GENERIC(sl_hal_system_hfrcoem23_freq_t, uint32_t) {
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_1M0Hz            = 1000000U,         /**< 1MHz RC band.  */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_2M0Hz            = 2000000U,         /**< 2MHz RC band.  */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_4M0Hz            = 4000000U,         /**< 4MHz RC band.  */
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_13)
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_5M0Hz            = 5000000U,         /**< 5MHz RC band.  */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_10M0Hz           = 10000000U,        /**< 10MHz RC band. */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_20M0Hz           = 20000000U,        /**< 20MHz RC band. */
#endif
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_7M0Hz            = 7000000U,         /**< 7MHz RC band.  */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_13M0Hz           = 13000000U,        /**< 13MHz RC band. */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_16M0Hz           = 16000000U,        /**< 16MHz RC band. */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_19M0Hz           = 19000000U,        /**< 19MHz RC band. */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_26M0Hz           = 26000000U,        /**< 26MHz RC band. */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_32M0Hz           = 32000000U,        /**< 32MHz RC band. */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_40M0Hz           = 40000000U,        /**< 40MHz RC band. */
  SL_HAL_SYSTEM_HFRCOEM23_FREQ_USER_DEFINED     = 0,
};

/** HFRCOEM23 minimum frequency */
#define SL_HAL_SYSTEM_HFRCOEM23_MIN         SL_HAL_SYSTEM_HFRCOEM23_FREQ_1M0Hz
/** HFRCOEM23 maximum frequency */
#define SL_HAL_SYSTEM_HFRCOEM23_MAX         SL_HAL_SYSTEM_HFRCOEM23_FREQ_40M0Hz

#endif
#endif

/// Family identifiers.
SL_ENUM_GENERIC(sl_hal_system_part_family_t, uint32_t) {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
  SL_HAL_SYSTEM_PART_FAMILY_MIGHTY_21 = DEVINFO_PART_FAMILY_MG | (21 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFR32 Mighty Gecko Series 2 Config 1 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_FLEX_21 = DEVINFO_PART_FAMILY_FG | (21 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Flex Gecko Series 2 Config 1 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_BLUE_21 = DEVINFO_PART_FAMILY_BG | (21 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Blue Gecko Series 2 Config 1 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_MIGHTY_RCP_21 = DEVINFO_PART_FAMILY_MR | (21 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFR32 Mighty RCP Series 2 Config 1 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)
  SL_HAL_SYSTEM_PART_FAMILY_MIGHTY_22 = DEVINFO_PART_FAMILY_MG | (22 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFR32 Mighty Gecko Series 2 Config 2 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_FLEX_22 = DEVINFO_PART_FAMILY_FG | (22 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Flex Gecko Series 2 Config 2 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_BLUE_22 = DEVINFO_PART_FAMILY_BG | (22 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Blue Gecko Series 2 Config 2 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_EFM32_PEARL_22 = DEVINFO_PART_FAMILY_PG | (22 << _DEVINFO_PART_FAMILYNUM_SHIFT),     ///< EFM32 Pearl Gecko Series 2 Config 2 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)
  SL_HAL_SYSTEM_PART_FAMILY_FLEX_23 = DEVINFO_PART_FAMILY_FG | (23 << _DEVINFO_PART_FAMILYNUM_SHIFT),       ///< EFR32 Flex Gecko Series 2 Config 3 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_ZEN_23 = DEVINFO_PART_FAMILY_ZG | (23 << _DEVINFO_PART_FAMILYNUM_SHIFT),        ///< EFR32 Zen Gecko Series 2 Config 3 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_EFM32_PEARL_23 = DEVINFO_PART_FAMILY_PG | (23 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFM32 Pearl Gecko Series 2 Config 3 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_SIDEWALK_23 = DEVINFO_PART_FAMILY_SG | (23 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Side Walk Gecko Series 2 Config 3 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4)
  SL_HAL_SYSTEM_PART_FAMILY_MIGHTY_24 = DEVINFO_PART_FAMILY_MG | (24 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFR32 Mighty Gecko Series 2 Config 4 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_FLEX_24 = DEVINFO_PART_FAMILY_FG | (24 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Flex Gecko Series 2 Config 4 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_BLUE_24 = DEVINFO_PART_FAMILY_BG | (24 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Blue Gecko Series 2 Config 4 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
  SL_HAL_SYSTEM_PART_FAMILY_FLEX_25 = DEVINFO_PART_FAMILY_FG | (25 << _DEVINFO_PART_FAMILYNUM_SHIFT),       ///< EFR32 Flex Gecko Series 2 Config 5 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)
  SL_HAL_SYSTEM_PART_FAMILY_MIGHTY_26 = DEVINFO_PART_FAMILY_MG | (26 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFR32 Mighty Gecko Series 2 Config 6 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_BLUE_26 = DEVINFO_PART_FAMILY_BG | (26 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Blue Gecko Series 2 Config 6 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_EFM32_PEARL_26 = DEVINFO_PART_FAMILY_PG | (26 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFM32 Pearl Gecko Series 2 Config 6 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7)
  SL_HAL_SYSTEM_PART_FAMILY_MIGHTY_27 = DEVINFO_PART_FAMILY_MG | (27 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFR32 Mighty Gecko Series 2 Config 7 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_BLUE_27 = DEVINFO_PART_FAMILY_BG | (27 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Blue Gecko Series 2 Config 7 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8)
  SL_HAL_SYSTEM_PART_FAMILY_FLEX_28 = DEVINFO_PART_FAMILY_FG | (28 << _DEVINFO_PART_FAMILYNUM_SHIFT),  ///< EFR32 Flex Gecko Series 2 Config 8 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_ZEN_28 = DEVINFO_PART_FAMILY_ZG | (28 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Zen Gecko Series 2 Config 8 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_SIDEWALK_28 = DEVINFO_PART_FAMILY_SG | (28 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Side Walk Gecko Series 2 Config 8 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_EFM32_PEARL_28 = DEVINFO_PART_FAMILY_PG | (28 << _DEVINFO_PART_FAMILYNUM_SHIFT),  ///< EFM32 Pearl Gecko Series 2 Config 8 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9)
  SL_HAL_SYSTEM_PART_FAMILY_MIGHTY_29 = DEVINFO_PART_FAMILY_MG | (29 << _DEVINFO_PART_FAMILYNUM_SHIFT), ///< EFR32 Mighty Gecko Series 2 Config 9 Value Device Family
  SL_HAL_SYSTEM_PART_FAMILY_BLUE_29 = DEVINFO_PART_FAMILY_BG | (29 << _DEVINFO_PART_FAMILYNUM_SHIFT),   ///< EFR32 Blue Gecko Series 2 Config 9 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_13)
  SL_HAL_SYSTEM_PART_FAMILY_FLEX_2D = DEVINFO_PART_FAMILY_FG | (33 << _DEVINFO_PART_FAMILYNUM_SHIFT),  ///< EFR32 Flex Gecko Series 2 Config 13 Value Device Family
#endif
#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  SL_HAL_SYSTEM_PART_FAMILY_BLUETOOTH_301 = DEVINFO_PART0_PROTOCOL_BLUETOOTH        \
                                            | (0x33 << _DEVINFO_PART0_SERIES_SHIFT) \
                                            | (0x30 << _DEVINFO_PART0_DIECODE0_SHIFT), ///< SI Series 3 Bluetooth Config 1 Value Device Family (BG)
  SL_HAL_SYSTEM_PART_FAMILY_PROPRIETARY_301 = DEVINFO_PART0_PROTOCOL_PROPRIETARY      \
                                              | (0x33 << _DEVINFO_PART0_SERIES_SHIFT) \
                                              | (0x30 << _DEVINFO_PART0_DIECODE0_SHIFT), ///< SI Series 3 Proprietary Config 1 Value Device Family (FG)
  SL_HAL_SYSTEM_PART_FAMILY_FIFTEENPFOUR_301 = DEVINFO_PART0_PROTOCOL_FIFTEENPFOUR     \
                                               | (0x33 << _DEVINFO_PART0_SERIES_SHIFT) \
                                               | (0x30 << _DEVINFO_PART0_DIECODE0_SHIFT), ///< SI Series 3 15.4 Config 1 Value Device Family (MG)
  SL_HAL_SYSTEM_PART_FAMILY_PEARL_301 = DEVINFO_PART0_PROTOCOL_PEARL            \
                                        | (0x33 << _DEVINFO_PART0_SERIES_SHIFT) \
                                        | (0x30 << _DEVINFO_PART0_DIECODE0_SHIFT), ///< SI Series 3 Pearl Config 1 Value Device Family (PG)
  SL_HAL_SYSTEM_PART_FAMILY_WIFI_301 = DEVINFO_PART0_PROTOCOL_WIFI             \
                                       | (0x33 << _DEVINFO_PART0_SERIES_SHIFT) \
                                       | (0x30 << _DEVINFO_PART0_DIECODE0_SHIFT), ///< SI Series 3 Wifi Config 1 Value Device Family (WG)
  SL_HAL_SYSTEM_PART_FAMILY_ZWAVE_301 = DEVINFO_PART0_PROTOCOL_ZWAVE            \
                                        | (0x33 << _DEVINFO_PART0_SERIES_SHIFT) \
                                        | (0x30 << _DEVINFO_PART0_DIECODE0_SHIFT), ///< SI Series 3 Zwave Config 1 Value Device Family (ZG)
#endif
  SL_HAL_SYSTEM_PART_FAMILY_UNKNOWN = 0xFF  ///< Unknown Device Family. Family ID is missing on unprogrammed parts.
};

/***************************************************************************//**
 * @brief
 *   Get the MCU family identifier.
 *
 * @return
 *   Family identifier of MCU.
 *
 * @note
 *   This function retrieves family ID by reading the chip's device info
 *   structure in flash memory. Users can retrieve family ID directly
 *   by reading DEVINFO->PART item and decode with mask and shift
 *   \#defines defined in \<part_family\>_devinfo.h (refer to code
 *   below for details).
 ******************************************************************************/
sl_hal_system_part_family_t sl_hal_system_get_family(void);

/** @} (end addtogroup system) */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SL_HAL_SYSTEM_H */
