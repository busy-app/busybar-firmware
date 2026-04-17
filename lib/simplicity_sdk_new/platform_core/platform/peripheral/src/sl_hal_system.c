/***************************************************************************//**
 * @file
 * @brief System API
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

#include "sl_hal_system.h"
#include "sl_hal_syscfg.h"
#include "em_device.h"
#include <stdbool.h>
#if defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_se_manager.h"
#include "sli_se_manager_device_data.h"
#endif
#include "sl_status.h"
#include "sl_assert.h"
#if defined(_SILICON_LABS_GECKO_INTERNAL_SDID_240)
#include "em_cmu.h"
#endif
/***************************************************************************//**
 * @addtogroup system SYSTEM - System Utils
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ************************************
 ******************************************************************************/

#define HFRCO_DPLL_FREQUENCY_TABLE_SIZE  11

#define DEVINFO_TEMPERATURE_CALTEMP_INTEGER_SHIFT  4

#if defined(_SILICON_LABS_32B_SERIES_3)
#define HAL_SYSTEM_CALIBRATION_SUPPORT
#endif

/*******************************************************************************
 *******************************   TYPEDEF   ***********************************
 ******************************************************************************/

#if defined(HAL_SYSTEM_CALIBRATION_SUPPORT)
typedef struct hfrco_dpll_cal_element {
  uint32_t min_freq;
  uint32_t max_freq;
} hfrco_dpll_cal_element_t;
#endif

/*******************************************************************************
 ******************************   CONSTANTS   **********************************
 ******************************************************************************/
const sl_hal_system_devinfo_adc_t SL_HAL_SYSTEM_DEVINFO_ADC_RESET_VALUES = {
  .cal_data = {
    .trim_vros0 = 0,
    .trim_vros1 = 0,
    .trim_gain_4x = 0,
    .trim_gain_0x3_int = 0
  },
  .offset = {
    .trim_off_1x = 0,
    .trim_off_2x = 0,
    .trim_off_4x = 0
  }
};

const sl_hal_system_devinfo_temperature_t SL_HAL_SYSTEM_DEVINFO_TEMPERATURE_RESET_VALUES = {
  .emu_temp_room = 0,
  .cal_temp = 0
};

#if defined(HAL_SYSTEM_CALIBRATION_SUPPORT)
static const hfrco_dpll_cal_element_t HFRCO_DPLL_FREQUENCY_TABLE[HFRCO_DPLL_FREQUENCY_TABLE_SIZE] = {
  { .min_freq = 16000000, .max_freq = 20000000 }, /// 18MHz calibration central frequency.
  { .min_freq = 20000000, .max_freq = 24500000 }, /// 22MHz calibration central frequency.
  { .min_freq = 24500000, .max_freq = 30000000 }, /// 27MHz calibration central frequency.
  { .min_freq = 30000000, .max_freq = 36000000 }, /// 33MHz calibration central frequency.
  { .min_freq = 36000000, .max_freq = 42500000 }, /// 39MHz calibration central frequency.
  { .min_freq = 42500000, .max_freq = 50500000 }, /// 46MHz calibration central frequency.
  { .min_freq = 50500000, .max_freq = 60000000 }, /// 55MHz calibration central frequency.
  { .min_freq = 60000000, .max_freq = 70000000 }, /// 65MHz calibration central frequency.
  { .min_freq = 70000000, .max_freq = 80000000 }, /// 75MHz calibration central frequency.
  { .min_freq = 80000000, .max_freq = 90000000 }, /// 85MHz calibration central frequency.
  { .min_freq = 90000000, .max_freq = 100000000 } /// 95MHz calibration central frequency.
};
#endif

/*******************************************************************************
 ******************************   UTILITY  *************************************
 ******************************************************************************/

#if defined(_SILICON_LABS_32B_SERIES_2)
/***************************************************************************//**
 * Get the nth ASCII character of a specified number.
 *
 * @param[in] input_number The number where the digit will be taken.
 * @param[in] position The digit position.
 *
 * @return The ASCII value of the specified digit.
 ******************************************************************************/
char sli_get_n_digit(uint16_t input_number,
                     uint8_t position)
{
  uint32_t exp[] = { 10, 100, 1000, 10000, 100000 };
  uint32_t number = input_number;

  if (position > 4) {
    EFM_ASSERT(false);
    return '0';
  }

  number = (number % exp[position]);

  if (position != 0) {
    number = number / (exp[position - 1]);
  }

  return (char)number + '0';
}
#endif

#if defined(_DEVINFO_PART0_DIECODE0_MASK) && defined(_SILICON_LABS_SECURITY_FEATURE_VAULT)
/***************************************************************************//**
 * Convert hexadecimal ASCII character to integer value.
 *
 * @param[in] character The character to be coverted to a number.
 *
 * @return The uint8_t value of the character given in parameter.
 ******************************************************************************/
uint8_t sli_hex_ascii_to_value(char character)
{
  if (character >= '0' && character <= '9') {
    return character - '0';
  } else if (character >= 'A' && character <= 'F') {
    return character - 'A';
  }

  return 0U;
}
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/**************************************************************************//**
 * Chip initialization routine for revision errata workarounds.
 *****************************************************************************/
void sl_hal_system_init(void)
{
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
  sl_hal_system_chip_revision_t chipRev;
  sl_hal_system_get_chip_revision(&chipRev);

  if (chipRev.major == 0x01 && (HFXO0->STATUS & HFXO_STATUS_ENS) == 0U) {
    // Change HFXO default peak detector settings.
    *(volatile uint32_t*)(HFXO0_BASE + 0x34U) =
      (*(volatile uint32_t*)(HFXO0_BASE + 0x34U) & 0xFF8000FFU)
      | 0x00178500U;
    // Change HFXO low power control settings.
    *(volatile uint32_t*)(HFXO0_BASE + 0x30U) =
      (*(volatile uint32_t*)(HFXO0_BASE + 0x30U) & 0xFFFF0FFFU)
      | 0x0000C000U;
    // Change default SQBUF bias current.
    *(volatile uint32_t*)(HFXO0_BASE + 0x30U) |= 0x700;
  }

  if (chipRev.major == 0x01 && chipRev.minor == 0x0) {
    // Trigger RAM read for each RAM instance.
    volatile uint32_t *dmem = (volatile uint32_t *) DMEM_RAM0_RAM_MEM_BASE;
    for (uint32_t i = 0U; i < DMEM_NUM_BANK; i++) {
      // Force memory read
      *dmem;
      dmem += (DMEM_BANK0_SIZE / 4U);
    }
  }

  // Set TRACE clock to intended reset value.
  CMU->TRACECLKCTRL = (CMU->TRACECLKCTRL & ~_CMU_TRACECLKCTRL_CLKSEL_MASK)
                      | CMU_TRACECLKCTRL_CLKSEL_HFRCOEM23;
#endif

#if defined(_SILICON_LABS_GECKO_INTERNAL_SDID_205)
#if defined(SL_TRUSTZONE_SECURE)
#define HFRCO_CLK_CFG_CLR_ADDR (0x40012020UL)
#else
#define HFRCO_CLK_CFG_CLR_ADDR (0x50012020UL)
#endif
#define HFRCO_CLK_CFG_CLKOUTDIS0 (0x4UL)
  if (sl_hal_system_get_prod_rev() == 1) {
    bool hfrcoClkIsOff = (CMU->CLKEN0 & CMU_CLKEN0_HFRCO0) == 0;
    CMU->CLKEN0_SET = CMU_CLKEN0_HFRCO0;
    // Enable HFRCO CLKOUT0.
    *(volatile uint32_t*)(HFRCO_CLK_CFG_CLR_ADDR) = HFRCO_CLK_CFG_CLKOUTDIS0;
    if (hfrcoClkIsOff) {
      CMU->CLKEN0_CLR = CMU_CLKEN0_HFRCO0;
    }
  }
#endif

// PM-3503.
#if defined(_SILICON_LABS_GECKO_INTERNAL_SDID_210)
  {
    bool syscfgClkIsOff = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) == 0);
    CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;

    bool dcdcClkIsOff = ((CMU->CLKEN0 & CMU_CLKEN0_DCDC) == 0);
    CMU->CLKEN0_SET = CMU_CLKEN0_DCDC;

    bool dcdcIsLock = ((DCDC->LOCKSTATUS & DCDC_LOCKSTATUS_LOCK_LOCKED) != 0);
    DCDC->LOCK = DCDC_LOCK_LOCKKEY_UNLOCKKEY;

    while (DCDC->SYNCBUSY & DCDC_SYNCBUSY_CTRL) {
      // Wait for previous synchronization to finish.
    }

    DCDC->CTRL_CLR = DCDC_CTRL_MODE;
    while ((DCDC->STATUS & DCDC_STATUS_BYPSW) == 0U) {
      // Wait for BYPASS switch enable.
    }

    if (dcdcIsLock) {
      DCDC->LOCK = ~DCDC_LOCK_LOCKKEY_UNLOCKKEY;
    }

    if (dcdcClkIsOff) {
      CMU->CLKEN0_CLR = CMU_CLKEN0_DCDC;
    }

    if (syscfgClkIsOff) {
      CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
    }
  }
#endif

// PM-5163.
#if defined(_SILICON_LABS_GECKO_INTERNAL_SDID_215)    \
  && defined(_SILICON_LABS_EFR32_2G4HZ_HP_PA_PRESENT) \
  && (_SILICON_LABS_EFR32_2G4HZ_HP_PA_MAX_OUTPUT_DBM == 20)
  sl_hal_system_chip_revision_t chipRev;
  sl_hal_system_get_chip_revision(&chipRev);

  if (chipRev.major == 0x01 && chipRev.minor == 0x00) {
    bool hfxo0ClkIsOff = (CMU->CLKEN0 & CMU_CLKEN0_HFXO0) == 0;
    CMU->CLKEN0_SET = CMU_CLKEN0_HFXO0;

    *(volatile uint32_t*)(HFXO0_BASE + 0x0034UL) =
      (*(volatile uint32_t*)(HFXO0_BASE + 0x0034UL) & 0xE3FFFFFFUL)
      | 0x0C000000UL;

    if (hfxo0ClkIsOff) {
      CMU->CLKEN0_CLR = CMU_CLKEN0_HFXO0;
    }
  }
#endif

#if defined(_SILICON_LABS_GECKO_INTERNAL_SDID_240)

  // Enable ICache out of reset.
  CMU->CLKEN1_SET = _CMU_CLKEN1_ICACHE0_MASK;
  ICACHE0->CTRL_CLR = _ICACHE_CTRL_CACHEDIS_MASK;
  CMU->CLKEN1_CLR = _CMU_CLKEN1_ICACHE0_MASK;

  CMU->CLKEN0_SET = _CMU_CLKEN0_HFRCO0_MASK;

  if (((HFRCO0->CAL & _HFRCO_CAL_TUNING_MASK) >> _HFRCO_CAL_TUNING_SHIFT) == _HFRCO_CAL_TUNING_MASK) {
    // temporary call CMU here, will be fixed when the cmu peripheral available.
    CMU_HFRCODPLLBandSet(cmuHFRCODPLLFreq_19M0Hz);
  }

  CMU->CLKEN0_CLR = _CMU_CLKEN0_HFRCO0_MASK;

#endif
}
/***************************************************************************//**
 * Get the chip revision.
 ******************************************************************************/
void sl_hal_system_get_chip_revision(sl_hal_system_chip_revision_t *rev)
{
#if defined(CMU_CLKEN0_SYSCFG)
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
#endif

  uint32_t chip_rev = sl_hal_syscfg_read_chip_rev();

  rev->minor = (chip_rev & _SYSCFG_CHIPREV_MINOR_MASK) >> _SYSCFG_CHIPREV_MINOR_SHIFT;
  rev->major = (chip_rev & _SYSCFG_CHIPREV_MAJOR_MASK) >> _SYSCFG_CHIPREV_MAJOR_SHIFT;
#if defined(_SYSCFG_CHIPREV_PARTNUMBER_MASK)
  rev->part_number = ((chip_rev & SYSCFG_CHIPREV_PARTNUMBER1) >> 5) | (chip_rev & SYSCFG_CHIPREV_PARTNUMBER0);
  rev->family = (uint16_t)0xFFFF;
#elif defined(_SYSCFG_CHIPREV_FAMILY_MASK)
  rev->part_number = (uint16_t)0xFFFF;
  rev->family = (chip_rev & _SYSCFG_CHIPREV_FAMILY_MASK) >> _SYSCFG_CHIPREV_FAMILY_SHIFT;
#else
  #error No Chip Revision Part Number or Family
#endif
}

/***************************************************************************//**
 * Get the MCU family identifier.
 ******************************************************************************/
sl_hal_system_part_family_t sl_hal_system_get_family(void)
{
#if defined(_DEVINFO_PART_FAMILY_MASK)
  return (DEVINFO->PART & (_DEVINFO_PART_FAMILY_MASK
                           | _DEVINFO_PART_FAMILYNUM_MASK));
#else
  return (DEVINFO->PART0 & (_DEVINFO_PART0_PROTOCOL_MASK
                            | _DEVINFO_PART0_SERIES_MASK
                            | _DEVINFO_PART0_DIECODE0_MASK));
#endif
}

/***************************************************************************//**
 * Get DEVINFO revision.
 ******************************************************************************/
uint8_t sl_hal_system_get_devinfo_rev(void)
{
#if defined(_DEVINFO_INFO_DEVINFOREV_MASK)
  return (uint8_t)((DEVINFO->INFO & _DEVINFO_INFO_DEVINFOREV_MASK)
                   >> _DEVINFO_INFO_DEVINFOREV_SHIFT);
#elif defined(_DEVINFO_REVISION_DEVINFOREV_MASK)
  return (uint8_t)((DEVINFO->REVISION & _DEVINFO_REVISION_DEVINFOREV_MASK)
                   >> _DEVINFO_REVISION_DEVINFOREV_SHIFT);
#else
#error (sl_hal_system.c): Location of devinfo revision is not defined.
#endif
}

/***************************************************************************//**
 * Get the default factory calibration value for HFRCO oscillator.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfrco_default_calibration(void)
{
#if defined(_DEVINFO_HFRCOCALDEFAULT_MASK)
  return DEVINFO->HFRCOCALDEFAULT;
#else
  return 0;
#endif
}

/***************************************************************************//**
 * Get the speed factory calibration value for HFRCO oscillator.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfrco_speed_calibration(void)
{
#if defined(_DEVINFO_HFRCOCALSPEED_MASK)
  return DEVINFO->HFRCOCALSPEED;
#else
  return 0;
#endif
}

/***************************************************************************//**
 * Get the HFRCO calibration based on the frequency band.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfrcodpll_band_calibration(uint32_t frequency)
{
#if defined(HAL_SYSTEM_CALIBRATION_SUPPORT)
  sl_status_t status;
  uint8_t band_index = 0xFF;
  sl_se_command_context_t se_command_ctx;
  sli_se_device_data_t otp_section_id = (sli_se_device_data_t)(SLI_SE_DEVICE_DATA_DI0 + DEVINFO_GP_FRAGMENT_INDEX);
  uint32_t offset;
  uint32_t calibration_value = 0;

  for (uint8_t i = 0; i < HFRCO_DPLL_FREQUENCY_TABLE_SIZE; i++) {
    if ((frequency >= HFRCO_DPLL_FREQUENCY_TABLE[i].min_freq)
        && (frequency <= HFRCO_DPLL_FREQUENCY_TABLE[i].max_freq)) {
      band_index = i;
      break;
    }
  }

  if (band_index >= HFRCO_DPLL_FREQUENCY_TABLE_SIZE) {
    return 0;
  }

  // Calculate memory offset based on the band index we want.
  offset = (band_index * 4) + DEVINFO_GP_HFRCODPLLBAND0_OFFSET;

  // Initialize command context.
  status = sl_se_init_command_context(&se_command_ctx);
  if (status != SL_STATUS_OK) {
    return 0;
  }

  // Send the SE command to retrieve the HFRCODPLL calibration for a given band from the DEVINFO OTP section.
  status = sli_se_device_data_read_word(&se_command_ctx, otp_section_id, offset, &calibration_value);
  if (status != SL_STATUS_OK) {
    return 0;
  }

  return calibration_value;
#elif defined(_SILICON_LABS_32B_SERIES_2)
  uint32_t calibration_value = 0;
  switch (frequency) {
    // 1, 2 and 4MHz share the same calibration word.
    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_1M0Hz:
    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_2M0Hz:
    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_4M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[0].HFRCODPLLCAL;
      break;

  #if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4) \
      || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7)  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9)
    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_5M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[1].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_10M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[4].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_20M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[9].HFRCODPLLCAL;
      break;
  #endif

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_7M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[3].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_13M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[6].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_16M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[7].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_19M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[8].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_26M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[10].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_32M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[11].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_38M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[12].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_48M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[13].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_56M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[14].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_64M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[15].HFRCODPLLCAL;
      break;

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_80M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[16].HFRCODPLLCAL;
      break;

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_100M0Hz:
      calibration_value = DEVINFO->HFRCODPLLCAL[17].HFRCODPLLCAL;
      break;
#endif

    case SL_HAL_SYSTEM_HFRCODPLL_FREQ_USER_DEFINED:
      break;

    default:
      return 0;
  }
  return calibration_value;
#else
  (void)frequency;
  return 0;
#endif
}

/***************************************************************************//**
 * Get a factory calibration value for HFRCOCEM23 oscillator.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfrcoem23_calibration(uint32_t frequency)
{
#if defined(HAL_SYSTEM_CALIBRATION_SUPPORT)
  sl_status_t status;
  sl_se_command_context_t se_command_ctx;
  sli_se_device_data_t otp_section_id = (sli_se_device_data_t)(SLI_SE_DEVICE_DATA_DI0 + DEVINFO_GP_FRAGMENT_INDEX);
  uint32_t offset;
  uint32_t calibration_value = 0;

  // Determine offset based on HFRCOEM23 frequency.
  if (frequency == 40000000UL) {
#if defined(DEVINFO_GP_HFRCOEM2340MHZ_OFFSET)
    offset = DEVINFO_GP_HFRCOEM2340MHZ_OFFSET;
#else
    // Default to 20Mhz.
    offset = DEVINFO_GP_HFRCOEM23DEFAULT_OFFSET;
#endif
  } else {
    offset = DEVINFO_GP_HFRCOEM23DEFAULT_OFFSET;
  }

  // Initialize command context.
  status = sl_se_init_command_context(&se_command_ctx);
  if (status != SL_STATUS_OK) {
    return 0;
  }

  // Send the SE command to retrieve the HFRCOEM23 calibration from the DEVINFO OTP section.
  status = sli_se_device_data_read_word(&se_command_ctx, otp_section_id, offset, &calibration_value);
  if (status != SL_STATUS_OK) {
    return 0;
  }

  return calibration_value;
#elif defined(_SILICON_LABS_32B_SERIES_2) && defined(HFRCOEM23_PRESENT)
  uint32_t calibration_value = 0;
  switch (frequency) {
    // 1, 2 and 4MHz share the same calibration word.
    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_1M0Hz:
    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_2M0Hz:
    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_4M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[0].HFRCOEM23CAL;
      break;

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5) \
      || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_13)
    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_5M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[1].HFRCOEM23CAL;
      break;
    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_10M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[4].HFRCOEM23CAL;
      break;
    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_20M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[9].HFRCOEM23CAL;
      break;
  #endif

    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_7M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[3].HFRCOEM23CAL;
      break;

    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_13M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[6].HFRCOEM23CAL;
      break;

    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_16M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[7].HFRCOEM23CAL;
      break;

    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_19M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[8].HFRCOEM23CAL;
      break;

    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_26M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[10].HFRCOEM23CAL;
      break;

    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_32M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[11].HFRCOEM23CAL;
      break;

    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_40M0Hz:
      calibration_value = DEVINFO->HFRCOEM23CAL[12].HFRCOEM23CAL;
      break;

    case SL_HAL_SYSTEM_HFRCOEM23_FREQ_USER_DEFINED:
      break;

    default:
      return 0;
  }
  return calibration_value;

#else
  (void)frequency;
  return 0;
#endif
}

/***************************************************************************//**
 * Get a factory calibration value for HFXOCAL.
 ******************************************************************************/
uint32_t sl_hal_system_get_hfxocal(void)
{
#if defined(_DEVINFO_HFXOCAL_MASK)
  return DEVINFO->HFXOCAL;
#else
  return 0;
#endif
}

/***************************************************************************//**
 * Get family security capability.
 ******************************************************************************/
sl_hal_system_security_capability_t sl_hal_system_get_security_capability(void)
{
  sl_hal_system_security_capability_t sc = SL_SYSTEM_SECURITY_CAPABILITY_UNKNOWN;

  uint16_t mcu_feature_set_major;
  uint16_t device_number;
  device_number = sl_hal_system_get_part_number();
  mcu_feature_set_major = 'A' + (device_number / 1000);
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)
  // override feature set since BRD4182A Rev A00 -> rev B02 are marked "A".
  mcu_feature_set_major = 'C';
#endif

  switch (mcu_feature_set_major) {
    case 'A':
      sc = SL_SYSTEM_SECURITY_CAPABILITY_SE;
      break;

    case 'B':
      sc = SL_SYSTEM_SECURITY_CAPABILITY_VAULT;
      break;

    case 'C':
      sc = SL_SYSTEM_SECURITY_CAPABILITY_ROT;
      break;

    default:
      sc = SL_SYSTEM_SECURITY_CAPABILITY_UNKNOWN;
      break;
  }

  return sc;
}

/***************************************************************************//**
 * Get the unique number for this device.
 ******************************************************************************/
uint64_t sl_hal_system_get_unique(void)
{
  uint32_t tmp = DEVINFO->EUI64L;
  return ((uint64_t)DEVINFO->EUI64H << 32) | tmp;
}

/***************************************************************************//**
 * Get the production revision for this part.
 ******************************************************************************/
uint8_t sl_hal_system_get_prod_rev(void)
{
#if defined(_DEVINFO_INFO_PRODREV_MASK)
  return (uint8_t)((DEVINFO->INFO & _DEVINFO_INFO_PRODREV_MASK)
                   >> _DEVINFO_INFO_PRODREV_SHIFT);
#elif defined(_DEVINFO_REVISION_PRODREV_MASK)
  return (uint8_t)((DEVINFO->REVISION & _DEVINFO_REVISION_PRODREV_MASK)
                   >> _DEVINFO_REVISION_PRODREV_SHIFT);
#else
#error (sl_hal_system.c): Location of production revision is not defined.
#endif
}

/***************************************************************************//**
 * Get the SRAM Base Address.
 ******************************************************************************/
uint32_t sl_hal_system_get_sram_base_address(void)
{
  return SRAM_BASE;
}

/***************************************************************************//**
 * Get the SRAM size (in KB).
 ******************************************************************************/
uint16_t sl_hal_system_get_sram_size(void)
{
#if defined(_DEVINFO_MSIZE_SRAM_MASK)
  return (uint16_t)((DEVINFO->MSIZE & _DEVINFO_MSIZE_SRAM_MASK)
                    >> _DEVINFO_MSIZE_SRAM_SHIFT);
#elif defined(_DEVINFO_EMBSIZE_RAM_MASK)
  return (uint16_t)((DEVINFO->EMBSIZE & _DEVINFO_EMBSIZE_RAM_MASK)
                    >> _DEVINFO_EMBSIZE_RAM_SHIFT);
#else
  #error (sl_hal_system.c): Location of SRAM Size is not defined.
#endif
}

/***************************************************************************//**
 * Get the flash size (in KB).
 ******************************************************************************/
uint16_t sl_hal_system_get_flash_size(void)
{
#if defined(_DEVINFO_MSIZE_FLASH_MASK)
  return (uint16_t)((DEVINFO->MSIZE & _DEVINFO_MSIZE_FLASH_MASK)
                    >> _DEVINFO_MSIZE_FLASH_SHIFT);
#elif defined(_DEVINFO_STACKMSIZE_FLASH_MASK)
  uint16_t stacked_flach_size = (uint16_t)((DEVINFO->STACKMSIZE & _DEVINFO_STACKMSIZE_FLASH_MASK)
                                           >> _DEVINFO_STACKMSIZE_FLASH_SHIFT);

  if (stacked_flach_size == 0) {
    // Defined in linker script for external flash provided by customers.
    extern uint32_t __flash_size__;
    // Get flash size in kB.
    stacked_flach_size = (uint16_t)((uintptr_t)&__flash_size__ / 1024);
  }

  return stacked_flach_size;
#endif
}

/***************************************************************************//**
 * Get the flash page size in bytes.
 ******************************************************************************/
uint32_t sl_hal_system_get_flash_page_size(void)
{
#if defined(_DEVINFO_MEMINFO_FLASHPAGESIZE_MASK)
  uint32_t tmp;
  tmp = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_FLASHPAGESIZE_MASK)
        >> _DEVINFO_MEMINFO_FLASHPAGESIZE_SHIFT;
  return 1UL << ((tmp + 10UL) & 0x1FUL);
#else
  // Defined in linker script for external flash provided by customers.
  extern uint32_t __flash_page_size__;
  return (uintptr_t)&__flash_page_size__;
#endif
}

/***************************************************************************//**
 * Get the MCU part number.
 ******************************************************************************/
uint16_t sl_hal_system_get_part_number(void)
{
#if defined(_DEVINFO_PART_DEVICENUM_MASK)
  return (uint16_t)((DEVINFO->PART & _DEVINFO_PART_DEVICENUM_MASK)
                    >> _DEVINFO_PART_DEVICENUM_SHIFT);
#elif defined(_DEVINFO_PART0_DIECODE0_MASK) && defined(_SILICON_LABS_SECURITY_FEATURE_VAULT)
  // Encode features to the series 2 format.
  // Add security level vault high for SIxG301.
  uint16_t device_number = 1000;
  uint32_t register_value = (DEVINFO->PART1 & _DEVINFO_PART1_FEATURE1_MASK) >> _DEVINFO_PART1_FEATURE1_SHIFT;

  device_number = sli_hex_ascii_to_value((char)register_value) * 100;

  register_value = (DEVINFO->PART1 & _DEVINFO_PART1_FEATURE2_MASK) >> _DEVINFO_PART1_FEATURE2_SHIFT;
  device_number += sli_hex_ascii_to_value((char)register_value) * 10;

  register_value = (DEVINFO->PART2 & _DEVINFO_PART2_FEATURE3_MASK) >> _DEVINFO_PART2_FEATURE3_SHIFT;
  device_number += sli_hex_ascii_to_value((char)register_value);

  return device_number;
#else
#error (em_system.c): Location of device part number is not defined.
#endif
}

/***************************************************************************//**
 * Get the SoC or MCU features.
 ******************************************************************************/
sl_hal_system_features_t sl_hal_system_get_part_features(void)
{
  sl_hal_system_features_t part_features = { .feature1 = '0', .feature2 = '0', .feature3 = '0' };

#if defined(_SILICON_LABS_32B_SERIES_2)
  uint16_t device_number = ((DEVINFO->PART & _DEVINFO_PART_DEVICENUM_MASK) >> _DEVINFO_PART_DEVICENUM_SHIFT);

  part_features.feature1 = sli_get_n_digit(device_number, 2);
  part_features.feature2 = sli_get_n_digit(device_number, 1);
  part_features.feature3 = sli_get_n_digit(device_number, 0);

#elif defined(_SILICON_LABS_32B_SERIES_3)

  part_features.feature1 = (DEVINFO->PART1 & _DEVINFO_PART1_FEATURE1_MASK) >> _DEVINFO_PART1_FEATURE1_SHIFT;
  part_features.feature2 = (DEVINFO->PART1 & _DEVINFO_PART1_FEATURE2_MASK) >> _DEVINFO_PART1_FEATURE2_SHIFT;
  part_features.feature3 = (DEVINFO->PART2 & _DEVINFO_PART2_FEATURE3_MASK) >> _DEVINFO_PART2_FEATURE3_SHIFT;

#else
#error Not defined for this die.
#endif

  return part_features;
}

/***************************************************************************//**
 * Get the temperature information.
 ******************************************************************************/
void sl_hal_system_get_temperature_info(sl_hal_system_devinfo_temperature_t *info)
{
#if defined(_DEVINFO_CALTEMP_MASK) || defined(_DEVINFO_EMUTEMP_MASK)
#if defined(_DEVINFO_CALTEMP_TEMP_MASK)
  info->cal_temp = ((DEVINFO->CALTEMP & _DEVINFO_CALTEMP_TEMP_MASK)
                    >> _DEVINFO_CALTEMP_TEMP_SHIFT);
#else
  info->cal_temp = 0;
#endif
#if defined(_DEVINFO_EMUTEMP_EMUTEMPROOM_MASK)
  info->emu_temp_room = ((DEVINFO->EMUTEMP & _DEVINFO_EMUTEMP_EMUTEMPROOM_MASK)
                         >> _DEVINFO_EMUTEMP_EMUTEMPROOM_SHIFT);
#else
  info->emu_temp_room = 0;
#endif
#elif defined(_SILICON_LABS_32B_SERIES_3)
  sl_status_t status;
  sl_se_command_context_t se_command_ctx;
  sli_se_device_data_t otp_section_id = (sli_se_device_data_t)(SLI_SE_DEVICE_DATA_DI0 + DEVINFO_GP_FRAGMENT_INDEX);
  uint32_t offset = DEVINFO_GP_TEMPERATURE_OFFSET;

  // Initialize command context.
  status = sl_se_init_command_context(&se_command_ctx);
  if (status != SL_STATUS_OK) {
    *info = SL_HAL_SYSTEM_DEVINFO_TEMPERATURE_RESET_VALUES;
    return;
  }

  // Send the SE command to retrieve the temperature information from the DEVINFO OTP section.
  status = sli_se_device_data_read_word(&se_command_ctx, otp_section_id, offset, (uint32_t*)info);
  if (status != SL_STATUS_OK) {
    *info = SL_HAL_SYSTEM_DEVINFO_TEMPERATURE_RESET_VALUES;
    return;
  }

  // Divide the temperature by 16 to retrieve only the integer part of the temperature value.
  info->cal_temp = info->cal_temp >> DEVINFO_TEMPERATURE_CALTEMP_INTEGER_SHIFT;
#else
  (void)info;
#endif
}

#if defined(__FPU_PRESENT) && (__FPU_PRESENT == 1)
/***************************************************************************//**
 * Set floating point co-processor (FPU) access mode.
 ******************************************************************************/
void sl_hal_system_fpu_set_access_mode(sl_hal_system_fpu_access_t access_mode)
{
  SCB->CPACR = (SCB->CPACR & ~(0xFUL << 20)) | access_mode;
}
#endif

/***************************************************************************//**
 * Get the ADC calibration info.
 ******************************************************************************/
void sl_hal_system_get_adc_calibration_info(sl_hal_system_devinfo_adc_t *info)
{
#if defined(HAL_SYSTEM_CALIBRATION_SUPPORT)
  sl_status_t status;
  sl_se_command_context_t se_command_ctx;
  sli_se_device_data_t otp_section_id = (sli_se_device_data_t)(SLI_SE_DEVICE_DATA_DI0 + DEVINFO_GP_FRAGMENT_INDEX);
  uint32_t offset = DEVINFO_GP_ADC0CALDATA_OFFSET;
  EFM_ASSERT(info != NULL);

  // Initialize command context.
  status = sl_se_init_command_context(&se_command_ctx);
  if (status != SL_STATUS_OK) {
    *info = SL_HAL_SYSTEM_DEVINFO_ADC_RESET_VALUES;
    return;
  }

  // Send the SE command to retrieve the ADC calibration from the DEVINFO OTP section.
  status = sli_se_device_data_read_chunk(&se_command_ctx,
                                         otp_section_id,
                                         offset,
                                         sizeof(sl_hal_system_devinfo_adc_t),
                                         info);
  if (status != SL_STATUS_OK) {
    *info = SL_HAL_SYSTEM_DEVINFO_ADC_RESET_VALUES;
    return;
  }
#else
  *info = SL_HAL_SYSTEM_DEVINFO_ADC_RESET_VALUES;
#endif
}

/***************************************************************************//**
 * Check if a debugger is connected (and debug session activated).
 ******************************************************************************/
bool sl_hal_system_is_debugger_connected(void)
{
  return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) ? true : false;
}

/** @} (end addtogroup system) */
