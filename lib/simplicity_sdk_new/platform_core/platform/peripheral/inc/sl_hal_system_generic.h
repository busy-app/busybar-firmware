/***************************************************************************//**
 * @file
 * @brief System API (Generic)
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

#ifndef _SL_HAL_SYSTEM_GENERIC_H
#define _SL_HAL_SYSTEM_GENERIC_H

#include "sl_enum.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup system SYSTEM - System Utils
 * @{
 ******************************************************************************/

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Family security capability.
SL_ENUM(sl_hal_system_security_capability_t) {
  /// Unknown security capability.
  SL_SYSTEM_SECURITY_CAPABILITY_UNKNOWN,
  /// Security capability not applicable.
  SL_SYSTEM_SECURITY_CAPABILITY_NA,
  /// Basic security capability.
  SL_SYSTEM_SECURITY_CAPABILITY_BASIC,
  /// Root of Trust security capability.
  SL_SYSTEM_SECURITY_CAPABILITY_ROT,
  /// Secure Element security capability.
  SL_SYSTEM_SECURITY_CAPABILITY_SE,
  /// Secure Vault security capability.
  SL_SYSTEM_SECURITY_CAPABILITY_VAULT
};

/// Floating point co-processor access modes.
SL_ENUM_GENERIC(sl_hal_system_fpu_access_t, uint32_t) {
  /// Access denied, any attempted access generates a NOCP UsageFault.
  SL_SYSTEM_FPU_ACCESS_DENIED          = (0x0 << 20),
  /// Privileged access only, an unprivileged access generates a NOCP UsageFault.
  SL_SYSTEM_FPU_ACCESS_PRIVILEGED_ONLY = (0x5 << 20),
  /// Reserved.
  SL_SYSTEM_FPU_ACCESS_RESERVED        = (0xA << 20),
  /// Full access.
  SL_SYSTEM_FPU_ACCESS_FULL            = (0xF << 20)
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// Chip revision details.
typedef struct {
  uint8_t minor;        ///< Minor revision number.
  uint8_t major;        ///< Major revision number.
  uint16_t part_number; ///< Device part number. (0xFFFF if unavailable)
  uint16_t family;      ///< Device family number. (0xFFFF if unavailable)
} sl_hal_system_chip_revision_t;

/// ADC calibration DEVINFO structure.
typedef struct sl_hal_system_devinfo_adc_cal_data_t {
  uint8_t trim_vros0;        ///< Trim VROS0.
  uint8_t trim_vros1;        ///< Trim VROS1.
  uint8_t trim_gain_4x;      ///< Trim gain 4x.
  uint8_t trim_gain_0x3_int; ///< Trim gain 0.325x internal.
} sl_hal_system_devinfo_adc_cal_data_t;

/// ADC offset DEVINFO structure.
typedef struct sl_hal_system_devinfo_adc_offset_t {
  uint8_t trim_off_1x; ///< Trim offset 1x.
  uint8_t trim_off_2x; ///< Trim offset 2x.
  uint8_t trim_off_4x; ///< Trim offset 4x.
  uint8_t dummy_byte;  ///< Dummy byte.
} sl_hal_system_devinfo_adc_offset_t;

/// ADC DEVINFO structure.
typedef struct sl_hal_system_devinfo_adc_t {
  sl_hal_system_devinfo_adc_cal_data_t cal_data; ///< DEVINFO calibration data.
  sl_hal_system_devinfo_adc_offset_t   offset;   ///< DEVINFO offset data.
} sl_hal_system_devinfo_adc_t;

/// Temperature DEVINFO Structure.
typedef struct sl_hal_system_devinfo_temperature_t {
  uint16_t emu_temp_room; ///< Temperature value.
  uint16_t cal_temp;      ///< Temperature calibration.
} sl_hal_system_devinfo_temperature_t;

/// Chip features Structure.
typedef struct sl_hal_system_features {
  char feature1; ///< Chip feature 1.
  char feature2; ///< Chip feature 2.
  char feature3; ///< Chip feature 3.
} sl_hal_system_features_t;

/*******************************************************************************
 **************************   GLOBAL CONSTANTS   *******************************
 ******************************************************************************/
/// DEVINFO ADC reset values.
extern const sl_hal_system_devinfo_adc_t SL_HAL_SYSTEM_DEVINFO_ADC_RESET_VALUES;
/// DEVINFO temperature reset values.
extern const sl_hal_system_devinfo_temperature_t SL_HAL_SYSTEM_DEVINFO_TEMPERATURE_RESET_VALUES;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/**************************************************************************//**
 * @brief
 *   Chip initialization routine for revision errata workarounds.
 *
 * @note
 *   This function must be called immediately in main().
 *
 * This initialization function configures the device to a state
 * as similar to later revisions as possible to improve software compatibility
 * with newer parts. See the device-specific errata for details.
 *****************************************************************************/
void sl_hal_system_init(void);

/*******************************************************************************
 * @brief
 *   Get the chip revision.
 *
 * @param[out] rev
 *   Pointer to return the chip revision to.
 *
 * @warning
 *   The chip revision structure may be returned with either the partnumber or
 *   family unpopulated (0xFFFF) depending on the device.
 ******************************************************************************/
void sl_hal_system_get_chip_revision(sl_hal_system_chip_revision_t *rev);

/***************************************************************************//**
 * @brief
 *   Get DEVINFO revision.
 *
 * @return
 *   Revision of the DEVINFO contents.
 ******************************************************************************/
uint8_t sl_hal_system_get_devinfo_rev(void);

/***************************************************************************//**
 * @brief
 *    Get the default factory calibration value for HFRCO oscillator.
 *
 * @return
 *    HFRCOCAL default value.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfrco_default_calibration(void);

/***************************************************************************//**
 * @brief
 *    Get the speed factory calibration value for HFRCO oscillator.
 *
 * @return
 *    HFRCOCAL speed value.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfrco_speed_calibration(void);

/***************************************************************************//**
 * @brief Get the HFRCO calibration based on the frequency band.
 *
 * @param[in] frequency
 *    Frequency for which to retrieve calibration.
 *
 * @return
 *    HFRCOCAL value for the given band.
 *
 * @note
 *    Those calibrations are only valid for the HFRCO oscillator when used with
 *    the DPLL module.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfrcodpll_band_calibration(uint32_t frequency);

/***************************************************************************//**
 * @brief
 *    Get a factory calibration value for HFRCOEM23 oscillator.
 *
 * @param[in] frequency
 *    HFRCOEM23 frequency for which to retrieve calibration.
 *
 * @return
 *    HFRCOEM23 calibration value.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfrcoem23_calibration(uint32_t frequency);

/***************************************************************************//**
 * @brief
 *    Get a factory calibration value for HFXOCAL.
 *
 * @return
 *    HFXOCAL value.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfxocal(void);

/***************************************************************************//**
 * @brief
 *   Get family security capability.
 *
 * @note
 *   This function retrieves the family security capability based on the
 *   device number.
 *
 * @return
 *   Security capability of MCU.
 ******************************************************************************/
sl_hal_system_security_capability_t sl_hal_system_get_security_capability(void);

/***************************************************************************//**
 * @brief
 *   Get the unique number for this device.
 *
 * @return
 *   Unique number for this device.
 ******************************************************************************/
uint64_t sl_hal_system_get_unique(void);

/***************************************************************************//**
 * @brief
 *   Get the production revision for this part.
 *
 * @return
 *   Production revision for this part.
 ******************************************************************************/
uint8_t sl_hal_system_get_prod_rev(void);

/***************************************************************************//**
 * @brief
 *   Get the SRAM Base Address.
 *
 * @return
 *   Base address SRAM (32-bit unsigned integer).
 ******************************************************************************/
uint32_t sl_hal_system_get_sram_base_address(void);

/***************************************************************************//**
 * @brief
 *   Get the SRAM size (in KB).
 *
 * @note
 *   This function retrieves SRAM size by reading the chip device
 *   info structure. If your binary is made for one specific device only,
 *   use SRAM_SIZE instead.
 *
 * @return
 *   Size of internal SRAM (in KB).
 ******************************************************************************/
uint16_t sl_hal_system_get_sram_size(void);

/***************************************************************************//**
 * @brief
 *   Get the flash size (in KB).
 *
 * @note
 *   This function retrieves flash size by reading the chip device info structure or
 *   DEVINFO->EMBMSIZE (embedded flash. not the case for S3 for now) or
 *   user config (external flash).
 *
 * @return
 *   Size of flash (in KB).
 ******************************************************************************/
uint16_t sl_hal_system_get_flash_size(void);

/***************************************************************************//**
 * @brief
 *   Get the flash page size in bytes.
 *
 * @note
 *   This function retrieves flash page size by reading the SE or
 *   user config (external flash)
 *
 * @return
 *   Page size of flash in bytes.
 ******************************************************************************/
uint32_t sl_hal_system_get_flash_page_size(void);

/***************************************************************************//**
 * @brief
 *   Get the MCU part number.
 *
 * @return
 *   The part number of MCU.
 ******************************************************************************/
uint16_t sl_hal_system_get_part_number(void);

/***************************************************************************//**
 * @brief
 *   Get the SoC or MCU features.
 *
 * @return
 *   The features of the current SoC or MCU.
 *
 * @note The features can be decoded by referring to the SoC or MCU datasheet.
 ******************************************************************************/
sl_hal_system_features_t sl_hal_system_get_part_features(void);

/***************************************************************************//**
 * @brief
 *   Get the temperature information.
 *
 * @param[out] info
 *   Pointer to variable where to store the temperature info.
 ******************************************************************************/
void sl_hal_system_get_temperature_info(sl_hal_system_devinfo_temperature_t *info);

/***************************************************************************//**
 * @brief
 *   Set floating point co-processor (FPU) access mode.
 *
 * @param[in] access_mode
 *   Floating point co-processor access mode.
 ******************************************************************************/
void sl_hal_system_fpu_set_access_mode(sl_hal_system_fpu_access_t access_mode);

/***************************************************************************//**
 * @brief Get the ADC calibration info.
 *
 * @param[out] info
 *   Pointer to variable where to store the adc calibration info.
 ******************************************************************************/
void sl_hal_system_get_adc_calibration_info(sl_hal_system_devinfo_adc_t *info);

/***************************************************************************//**
 * @brief
 *   Check if a debugger is connected (and debug session activated).
 *
 * @details
 *   Used to make run-time decisions depending on whether or not a debug session
 *   has been active since last reset, i.e., using a debug probe or similar. In
 *   some cases, special handling is required in that scenario.
 *
 * @return
 *   True if a debug session is active since last reset, otherwise false.
 ******************************************************************************/
bool sl_hal_system_is_debugger_connected(void);

/** @} (end addtogroup system) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/****************************************************************************//**
 * @addtogroup system SYSTEM - System Utils
 * @{
 *
 * @n @section system_example Example
 *
 *  @code{.c}
 *  {
 *    // Get chip revision information
 *    sl_hal_system_chip_revision_t rev;
 *    sl_hal_system_get_chip_revision(&rev);
 *
 *    // Print chip revision info
 *    printf("Chip revision: %d.%d, Part number: 0x%04X, Family: 0x%04X\n",
 *           rev.major, rev.minor, rev.part_number, rev.family);
 *
 *    // Get unique device identifier
 *    uint64_t unique_id = sl_hal_system_get_unique();
 *
 *    // Get production revision
 *    uint8_t prod_rev = sl_hal_system_get_prod_rev();
 *
 *    // Get memory sizes
 *    uint16_t flash_size_kb = sl_hal_system_get_flash_size();
 *    uint16_t sram_size_kb = sl_hal_system_get_sram_size();
 *    uint32_t sram_base = sl_hal_system_get_sram_base_address();
 *
 *    // Get security capability
 *    sl_hal_system_security_capability_t sec_cap = sl_hal_system_get_security_capability();
 *
 *    // The system must be initialized at the beginning of main()
 *    sl_hal_system_init();
 *  }
 *  @endcode
 *
 * @} (end addtogroup system)
 *******************************************************************************/
/* *INDENT-ON* */

#endif /* #ifndef _SL_HAL_SYSTEM_GENERIC_H */
