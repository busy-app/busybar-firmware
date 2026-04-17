/***************************************************************************//**
 * @file
 * @brief ADC - Successive Approximation Registers peripheral API
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

#include "sl_hal_adc.h"
#include "sl_hal_system.h"
#include "sl_common.h"
#include "sl_status.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static void adc_calibrate_config(ADC_TypeDef *adc,
                                 const sl_hal_adc_init_t *init,
                                 uint8_t config_id);
static sl_status_t adc_calculate_prescalers(uint32_t branch_clock_freq,
                                            uint8_t* hsclkrate,
                                            uint8_t* adcprescale);
#if defined(_ADC_OFFSETSE_MASK)
static uint32_t adc_calculate_offset(uint8_t trim);
#endif
static sl_hal_adc_result_t adc_decode_result(uint32_t data,
                                             sl_hal_adc_alignment_t alignment);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE void sl_hal_adc_enable(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_disable(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_start(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_stop(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_enable_timer(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_disable_timer(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_flush_fifo(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_wait_sync(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_wait_ready(ADC_TypeDef *adc);
extern __INLINE uint32_t sl_hal_adc_get_status(ADC_TypeDef *adc);
extern __INLINE void sl_hal_adc_enable_interrupts(ADC_TypeDef *adc,
                                                  uint32_t flags);
extern __INLINE void sl_hal_adc_disable_interrupts(ADC_TypeDef *adc,
                                                   uint32_t flags);
extern __INLINE void sl_hal_adc_clear_interrupts(ADC_TypeDef *adc,
                                                 uint32_t flags);
extern __INLINE void sl_hal_adc_set_interrupts(ADC_TypeDef *adc,
                                               uint32_t flags);
extern __INLINE uint32_t sl_hal_adc_get_pending_interrupts(ADC_TypeDef *adc);
extern __INLINE uint32_t sl_hal_adc_get_enabled_pending_interrupts(ADC_TypeDef *adc);

/***************************************************************************//**
 * @addtogroup adc ADC - Successive Approximation Registers ADC
 * @brief Successive Approximation Registers ADC peripheral API
 * @{
 ******************************************************************************/

#define SLI_HAL_ADC_CLK_CORE_MAX     22000000UL
#define SLI_HAL_ADC_CLK_CORE_MIN     17000000UL
#define SLI_HAL_ADC_CLK_SRC_MAX      40000000UL
#define SLI_HAL_ADC_HSCLKRATE_MAX    5U
#define SLI_HAL_ADC_ADCPRESCALE_MAX  1U

#define SLI_HAL_ADC_GAIN_TRIM_G6(x) ((uint32_t)(((uint32_t)x & 0x40UL) >> 6))
#define SLI_HAL_ADC_GAIN_TRIM_G5(x) ((uint32_t)(((uint32_t)x & 0x20UL) >> 5))
#define SLI_HAL_ADC_GAIN_TRIM_G4(x) ((uint32_t)(((uint32_t)x & 0x10UL) >> 4))
#define SLI_HAL_ADC_GAIN_TRIM_G3(x) ((uint32_t)(((uint32_t)x & 0x08UL) >> 3))
#define SLI_HAL_ADC_GAIN_TRIM_G2(x) ((uint32_t)(((uint32_t)x & 0x04UL) >> 2))
#define SLI_HAL_ADC_GAIN_TRIM_G1(x) ((uint32_t)(((uint32_t)x & 0x02UL) >> 1))
#define SLI_HAL_ADC_GAIN_TRIM_G0(x) ((uint32_t)(((uint32_t)x & 0x01UL) >> 0))

// Define to replace the default hardware value for the 0.325x gain trim.
#define SLI_HAL_ADC_0X325_DEFAULT_GAIN_TRIM 32

/***************************************************************************//**
 * @brief
 *   Initialize the ADC.
 ******************************************************************************/
void sl_hal_adc_init(ADC_TypeDef *adc,
                     const sl_hal_adc_init_t *init,
                     uint32_t branch_clock_freq)
{
  // Assert that the adc reference is valid and the adc is disabled.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  EFM_ASSERT(!(adc->EN & _ADC_EN_EN_MASK));

  uint8_t hsclkrate;
  uint8_t adcprescale;
  EFM_ASSERT(adc_calculate_prescalers(branch_clock_freq, &hsclkrate, &adcprescale) == SL_STATUS_OK);
  adc->CTRL = ((hsclkrate << _ADC_CTRL_HSCLKRATE_SHIFT) & _ADC_CTRL_HSCLKRATE_MASK)
              | ((adcprescale << _ADC_CTRL_ADCPRESCALE_SHIFT) & _ADC_CTRL_ADCPRESCALE_MASK);

  // Set the CTRL.TIMEBASE according to the resulting prescaled ADC core clock.
  // This TIMEBASE needs to be set to the number of ADC core clock ticks
  // required for a duration of 1us.
  uint32_t clock_prescaled = ((branch_clock_freq / (uint32_t)(hsclkrate + 1)) / (uint32_t)(adcprescale + 1));
  uint32_t ns_per_tick = (1000000000UL / clock_prescaled);
  adc->CTRL_SET = (((1000U /* ns */ / ns_per_tick) << _ADC_CTRL_TIMEBASE_SHIFT) & _ADC_CTRL_TIMEBASE_MASK);

  // Set remaining CTRL register bits.
  adc->CTRL_SET = (((init->debug_halt ? 1U : 0U) << _ADC_CTRL_DBGHALT_SHIFT)
#if defined(_ADC_CTRL_ADCCLKSUSPEND_MASK)
                   | ((init->clk_suspend ? 1U : 0U) << _ADC_CTRL_ADCCLKSUSPEND_SHIFT)
#endif
#if defined(_ADC_CTRL_RESOLUTIONSEL_MASK)
                   | (((init->scan_resolution) << _ADC_CTRL_RESOLUTIONSEL_SHIFT) & _ADC_CTRL_RESOLUTIONSEL_MASK)
#endif
                   | ((init->warmup_mode ? 1U : 0U) << _ADC_CTRL_WARMUPMODE_SHIFT)
                   | (((uint32_t)init->voltage_reference << _ADC_CTRL_REFSEL_SHIFT) & _ADC_CTRL_REFSEL_MASK));

  // Set SCANFIFOCFG according to the init structure.
  adc->SCANFIFOCFG = ((init->show_id ? 1U : 0U) << _ADC_SCANFIFOCFG_SHOWID_SHIFT)
                     | (((uint32_t)init->alignment << _ADC_SCANFIFOCFG_ALIGNMENT_SHIFT) & _ADC_SCANFIFOCFG_ALIGNMENT_MASK)
                     | (((uint32_t)init->data_valid << _ADC_SCANFIFOCFG_DVL_SHIFT) & _ADC_SCANFIFOCFG_DVL_MASK);

  // Set ADC compare thresholds.
  adc->CMPTHR = (((uint32_t)init->greater_than << _ADC_CMPTHR_ADGT_SHIFT) & _ADC_CMPTHR_ADGT_MASK)
                | (((uint32_t)init->less_than << _ADC_CMPTHR_ADLT_SHIFT) & _ADC_CMPTHR_ADLT_MASK);

  // Select a TRIGGER to start a scan sequence, and the action mode once a scan
  // sequence is triggered.
  adc->TRIGGER = (((uint32_t)init->scan_trigger << _ADC_TRIGGER_SCANTRIGSEL_SHIFT) & _ADC_TRIGGER_SCANTRIGSEL_MASK)
#if defined(_ADC_TRIGGER_REPDELAY_MASK)
                 | (((uint32_t)init->repetition_delay << _ADC_TRIGGER_REPDELAY_SHIFT) & _ADC_TRIGGER_REPDELAY_MASK)
#endif
                 | (((uint32_t)init->scan_trigger_action << _ADC_TRIGGER_SCANTRIGACTION_SHIFT) & _ADC_TRIGGER_SCANTRIGACTION_MASK);

  // Set ADC configurations and calibrations.
  for (uint8_t i = 0; i < ADC_CONFIGNUM(ADC_NUM(adc)); i++) {
    // The ATIME bitfield is 10bits wide, let's make sure that the init value
    // conforms to this bitfield's width.
    EFM_ASSERT((uint32_t)init->config[i].acquisition_time <= (uint32_t)(_ADC_CFG_ATIME_MASK >> _ADC_CFG_ATIME_SHIFT));

#if defined(_ADC_CFG_AVERAGESEL_MASK)
    // Due to hardware limitations, the ADC cannot be configured for hardware
    // averaging if the adcprescale is 0 (divide by 1).
    if ( adcprescale == 0 ) {
      EFM_ASSERT(init->config[i].average == SL_HAL_ADC_AVERAGE_X1);
    }
#else
    // No averaging is only supported for Nyquist mode.
    if (init->config[i].oversampling_mode != SL_HAL_ADC_OS_MODE_0) {
      EFM_ASSERT(init->config[i].oversampling_rate != SL_HAL_ADC_OS_RATE_X1);
    }
#endif

    adc->CFG[i].CFG = (((uint32_t)init->config[i].gain << _ADC_CFG_ANALOGGAIN_SHIFT) & _ADC_CFG_ANALOGGAIN_MASK)
#if defined(_ADC_CFG_AVERAGESEL_MASK)
                      | (((uint32_t)init->config[i].average << _ADC_CFG_AVERAGESEL_SHIFT) & _ADC_CFG_AVERAGESEL_MASK)
#else
                      | (((uint32_t)init->config[i].oversampling_rate << _ADC_CFG_OSR_SHIFT) & _ADC_CFG_OSR_MASK)
                      | (((uint32_t)init->config[i].oversampling_mode << _ADC_CFG_OSMODE_SHIFT) & _ADC_CFG_OSMODE_MASK)
                      | (((uint32_t)init->config[i].output_polarity << _ADC_CFG_TWOSCMPL_SHIFT) & _ADC_CFG_TWOSCMPL_MASK)
                      | ((init->config[i].double_frequency ? 1U : 0U) << _ADC_CFG_FREQDOUBLE_SHIFT)
#endif
                      | (((uint32_t)init->config[i].acquisition_time << _ADC_CFG_ATIME_SHIFT) & _ADC_CFG_ATIME_MASK);

    adc_calibrate_config(adc, init, i);
  }

  // Set initial ADC scan table entries.
  for (uint8_t i = 0; i < ADC_CHANNELS(ADC_NUM(adc)); i++) {
    sl_hal_adc_update_scan_entry(adc, i, &init->entries[i]);
  }
}

/***************************************************************************//**
 * @brief
 *   Reset the ADC.
 ******************************************************************************/
void sl_hal_adc_reset(ADC_TypeDef *adc)
{
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  sl_hal_adc_enable(adc);

  adc->MASKREQ = _ADC_MASKREQ_RESETVALUE;

  sl_hal_adc_disable(adc);
  sl_hal_adc_wait_ready(adc);

  adc->CTRL = _ADC_CTRL_RESETVALUE;
  adc->TIMER = _ADC_TIMER_RESETVALUE;
  adc->CMPTHR = _ADC_CMPTHR_RESETVALUE;
  adc->IF = _ADC_IF_RESETVALUE;
  adc->IEN = _ADC_IEN_RESETVALUE;
  adc->TRIGGER = _ADC_TRIGGER_RESETVALUE;
  adc->SCANFIFOCFG = _ADC_SCANFIFOCFG_RESETVALUE;

  for (uint32_t i = 0; i < ADC_CONFIGNUM(ADC_NUM(adc)); i++) {
    adc->CFG[i].CFG = _ADC_CFG_RESETVALUE;
  }

  for (uint32_t i = 0; i < ADC_CHANNELS(ADC_NUM(adc)); i++) {
    adc->SCANTABLE[i].SCAN = _ADC_SCAN_RESETVALUE;
  }
}

/***************************************************************************//**
 * @brief
 *   Set a new Scan Table mask.
 ******************************************************************************/
void sl_hal_adc_set_scan_mask(ADC_TypeDef *adc,
                              uint32_t mask)
{
  EFM_ASSERT(adc->EN & _ADC_EN_EN_MASK);
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  adc->MASKREQ = (mask << _ADC_MASKREQ_MASKREQ_SHIFT) & _ADC_MASKREQ_MASKREQ_MASK;
}

/***************************************************************************//**
 * @brief
 *   Get the working scan table mask.
 ******************************************************************************/
uint32_t sl_hal_adc_get_scan_mask(ADC_TypeDef *adc)
{
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  sl_hal_adc_wait_sync(adc);
  return adc->STMASK;
}

/***************************************************************************//**
 * @brief
 *   Update an entry in the scan table.
 ******************************************************************************/
void sl_hal_adc_update_scan_entry(ADC_TypeDef *adc,
                                  sl_hal_adc_channel_id_t id,
                                  const sl_hal_adc_scan_entry_t* entry)
{
  EFM_ASSERT(!(adc->EN & _ADC_EN_EN_MASK));
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  EFM_ASSERT(id < ADC_CHANNELS(ADC_NUM(adc)));

  adc->SCANTABLE[id].SCAN = ((entry->compare ? 1U : 0U) << _ADC_SCAN_CMP_SHIFT)
                            | (((uint32_t)entry->config_id << _ADC_SCAN_CFG_SHIFT) & _ADC_SCAN_CFG_MASK)
                            | (((uint32_t)entry->neg_port << _ADC_SCAN_PORTNEG_SHIFT) & _ADC_SCAN_PORTNEG_MASK)
                            | (((uint32_t)entry->neg_pin << _ADC_SCAN_PINNEG_SHIFT) & _ADC_SCAN_PINNEG_MASK)
                            | (((uint32_t)entry->pos_port << _ADC_SCAN_PORTPOS_SHIFT) & _ADC_SCAN_PORTPOS_MASK)
                            | (((uint32_t)entry->pos_pin << _ADC_SCAN_PINPOS_SHIFT) & _ADC_SCAN_PINPOS_MASK);
}

/***************************************************************************//**
 * @brief
 *   Pull the value at the top of the FIFO. FIFO count reduced.
 ******************************************************************************/
sl_hal_adc_result_t sl_hal_adc_pull(const ADC_TypeDef *adc)
{
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  sl_hal_adc_alignment_t alignment =
    (adc->SCANFIFOCFG & _ADC_SCANFIFOCFG_ALIGNMENT_MASK)
    >> _ADC_SCANFIFOCFG_ALIGNMENT_SHIFT;

  return adc_decode_result(adc->SCANFIFODATA, alignment);
}

/***************************************************************************//**
 * @brief
 *   Peek at the top of the FIFO without removing the data.
 ******************************************************************************/
sl_hal_adc_result_t sl_hal_adc_peek(const ADC_TypeDef *adc)
{
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  sl_hal_adc_alignment_t alignment =
    (adc->SCANFIFOCFG & _ADC_SCANFIFOCFG_ALIGNMENT_MASK)
    >> _ADC_SCANFIFOCFG_ALIGNMENT_SHIFT;

  return adc_decode_result(adc->SCANDATA, alignment);
}

/***************************************************************************//**
 * @brief
 *   Get the ADC's current FIFO count.
 ******************************************************************************/
uint32_t sl_hal_adc_get_fifo_count(const ADC_TypeDef *adc)
{
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  return ((adc->SCANFIFOSTAT
           & _ADC_SCANFIFOSTAT_FIFOREADCNT_MASK)
          >> _ADC_SCANFIFOSTAT_FIFOREADCNT_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Get the ADC's internal reference voltage in millivolts.
 ******************************************************************************/
uint32_t sl_hal_adc_get_internal_reference_voltage(const ADC_TypeDef *adc)
{
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  // ADC reference voltages VDDA, VREFPL, VREFPH and VREFPBUF are all
  // application specific and cannot be known in advance.

  sl_hal_system_chip_revision_t rev;
  sl_hal_system_get_chip_revision(&rev);
  uint16_t rev_combined = (uint16_t)rev.minor | (uint16_t)(rev.major << 8U);

  // Only Rainier for now, but this can be extended to other series 3 device
  // families with pre-processors. Also could be less memory intensive to
  // determine this internal reference voltage from configuration data.
  if ( rev_combined == 0x0100 ) {   // Revision A0
    return 1210UL;   // 1.21V
  } else {
    return 1200UL;   // 1.20V
  }
}

/***************************************************************************//**
 * @brief
 *   Calculate and return the clock frequency of the ADC's local timer.
 ******************************************************************************/
uint32_t sl_hal_adc_get_timer_frequency(const ADC_TypeDef *adc,
                                        uint32_t branch_clock_freq)
{
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  uint32_t hsclkrate = (adc->CTRL & _ADC_CTRL_HSCLKRATE_MASK) >> _ADC_CTRL_HSCLKRATE_SHIFT;
  return (branch_clock_freq / (hsclkrate + 1));
}

/***************************************************************************//**
 * @brief
 *   Set the timer period to periodically trigger a timer event.
 ******************************************************************************/
void sl_hal_adc_set_timer_period(ADC_TypeDef *adc,
                                 uint16_t    timer_period)
{
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  adc->TIMER = timer_period;
}

/***************************************************************************//**
 * @brief
 *   Set both ADC prescalers.
 ******************************************************************************/
void sl_hal_adc_set_clock_prescalers(ADC_TypeDef *adc,
                                     uint32_t branch_clock_freq,
                                     uint8_t hsclkrate,
                                     uint8_t adcprescale)
{
  uint32_t clock_prescaled;

  // Assert that the adc reference is valid and the adc is disabled.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  EFM_ASSERT(!(adc->EN & _ADC_EN_EN_MASK));

  // Verify clock prescalers are suitable for the operation of the ADC with
  // regards to the branch clock frequency.
  EFM_ASSERT(hsclkrate <= SLI_HAL_ADC_HSCLKRATE_MAX);
  clock_prescaled = branch_clock_freq / (hsclkrate + 1);
  EFM_ASSERT(clock_prescaled <= SLI_HAL_ADC_CLK_SRC_MAX);

  EFM_ASSERT(adcprescale <= SLI_HAL_ADC_ADCPRESCALE_MAX);
  clock_prescaled = clock_prescaled / (adcprescale + 1);
  EFM_ASSERT(clock_prescaled >= SLI_HAL_ADC_CLK_CORE_MIN && clock_prescaled <= SLI_HAL_ADC_CLK_CORE_MAX);

  adc->CTRL_CLR = _ADC_CTRL_HSCLKRATE_MASK | _ADC_CTRL_ADCPRESCALE_MASK;
  adc->CTRL_SET = ((hsclkrate << _ADC_CTRL_HSCLKRATE_SHIFT) & _ADC_CTRL_HSCLKRATE_MASK)
                  | ((adcprescale << _ADC_CTRL_ADCPRESCALE_SHIFT) & _ADC_CTRL_ADCPRESCALE_MASK);

  // Set the CTRL.TIMEBASE according to the resulting prescaled ADC core clock.
  // This TIMEBASE needs to be set to the number of ADC core clock ticks
  // required for a duration of 1us.
  uint32_t ns_per_tick = (1000000000UL / clock_prescaled);
  adc->CTRL_SET = (((1000U /* ns */ / ns_per_tick) << _ADC_CTRL_TIMEBASE_SHIFT) & _ADC_CTRL_TIMEBASE_MASK);
}

/***************************************************************************//**
 * @brief
 *   Apply calibrations to a given configuration.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @param[in] init
 *   A pointer to the ADC initialization structure.
 *
 * @param[in] config_id
 *   Identifies the ADC configuration to calibrate.
 ******************************************************************************/
static void adc_calibrate_config(ADC_TypeDef *adc,
                                 const sl_hal_adc_init_t *init,
                                 uint8_t config_id)
{
#if defined(_ADC_OFFSETSE_MASK)
  sl_hal_system_devinfo_adc_t adc_devinfo;
  sl_hal_system_get_adc_calibration_info(&adc_devinfo);

  switch (init->config[config_id].gain) {
    case SL_HAL_ADC_ANALOG_GAIN_0_3125:
      if ( init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPL
           || init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPH ) {
        // Apply devinfo 0.325x gain trim for unbuffered external reference voltage.
        adc->CFG[config_id].GAINCAL = (((uint32_t)adc_devinfo.cal_data.trim_gain_0x3_int << _ADC_GAINCAL_SEPOS_SHIFT) & _ADC_GAINCAL_SEPOS_MASK)
                                      | (((uint32_t)adc_devinfo.cal_data.trim_gain_0x3_int << _ADC_GAINCAL_SENEG_SHIFT) & _ADC_GAINCAL_SENEG_MASK)
                                      | (((uint32_t)adc_devinfo.cal_data.trim_gain_0x3_int << _ADC_GAINCAL_DIFF_SHIFT) & _ADC_GAINCAL_DIFF_MASK);
      } else {
        // Apply default 0.325x gain trim for other reference voltages.
        adc->CFG[config_id].GAINCAL = ((SLI_HAL_ADC_0X325_DEFAULT_GAIN_TRIM << _ADC_GAINCAL_SEPOS_SHIFT) & _ADC_GAINCAL_SEPOS_MASK)
                                      | ((SLI_HAL_ADC_0X325_DEFAULT_GAIN_TRIM << _ADC_GAINCAL_SENEG_SHIFT) & _ADC_GAINCAL_SENEG_MASK)
                                      | ((SLI_HAL_ADC_0X325_DEFAULT_GAIN_TRIM << _ADC_GAINCAL_DIFF_SHIFT) & _ADC_GAINCAL_DIFF_MASK);
      }
      // Offset trims (0.3x, 0.5x or 1x) will be applied after this switch statement.
      break;
    case SL_HAL_ADC_ANALOG_GAIN_0_5:
      // Apply 0.5x gain trim for unbuffered external reference voltage.
      if ( init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPL
           || init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPH ) {
        // Translate 4x gain trim to 0.5x gain trim with formula
        // b = [g6, 1, !g6, !g6, g6^g5, g4, g3]
        uint32_t translated_trim =  (SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) << 6)
                                   | (0x1UL << 5)
                                   | ((SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) ^ 0x1) << 4)
                                   | ((SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) ^ 0x1) << 3)
                                   | ((SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) ^ SLI_HAL_ADC_GAIN_TRIM_G5(adc_devinfo.cal_data.trim_gain_4x)) << 2)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G4(adc_devinfo.cal_data.trim_gain_4x) << 1)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G3(adc_devinfo.cal_data.trim_gain_4x) << 0);
        adc->CFG[config_id].GAINCAL = ((translated_trim << _ADC_GAINCAL_SEPOS_SHIFT) & _ADC_GAINCAL_SEPOS_MASK)
                                      | ((translated_trim << _ADC_GAINCAL_SENEG_SHIFT) & _ADC_GAINCAL_SENEG_MASK)
                                      | ((translated_trim << _ADC_GAINCAL_DIFF_SHIFT) & _ADC_GAINCAL_DIFF_MASK);
      }
      // Offset trims (0.3x, 0.5x or 1x) will be applied after this switch statement.
      break;
    case SL_HAL_ADC_ANALOG_GAIN_1:
      // Apply 1x gain trim for unbuffered external reference voltage.
      if ( init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPL
           || init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPH ) {
        // Translate 4x gain trim to 1x gain trim with formula
        // b = [g6, 1, !g6, g6^g5, g4, g3, g2]
        uint32_t translated_trim = (SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) << 6)
                                   | (0x1UL << 5)
                                   | ((SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) ^ 0x1) << 4)
                                   | ((SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) ^ SLI_HAL_ADC_GAIN_TRIM_G5(adc_devinfo.cal_data.trim_gain_4x)) << 3)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G4(adc_devinfo.cal_data.trim_gain_4x) << 2)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G3(adc_devinfo.cal_data.trim_gain_4x) << 1)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G2(adc_devinfo.cal_data.trim_gain_4x) << 0);
        adc->CFG[config_id].GAINCAL = ((translated_trim << _ADC_GAINCAL_SEPOS_SHIFT) & _ADC_GAINCAL_SEPOS_MASK)
                                      | ((translated_trim << _ADC_GAINCAL_SENEG_SHIFT) & _ADC_GAINCAL_SENEG_MASK)
                                      | ((translated_trim << _ADC_GAINCAL_DIFF_SHIFT) & _ADC_GAINCAL_DIFF_MASK);
      }
      // Offset trims (0.3x, 0.5x or 1x) will be applied after this switch statement.
      break;
    case SL_HAL_ADC_ANALOG_GAIN_2:
      // Apply 2x gain trim for unbuffered external reference voltage.
      if ( init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPL
           || init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPH ) {
        // Translate 4x gain trim to 2x gain trim with formula
        // b = [g6, 1, g6^g5, g4, g3, g2, g1]
        uint32_t translated_trim = (SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) << 6)
                                   | (0x1UL << 5)
                                   | ((SLI_HAL_ADC_GAIN_TRIM_G6(adc_devinfo.cal_data.trim_gain_4x) ^ SLI_HAL_ADC_GAIN_TRIM_G5(adc_devinfo.cal_data.trim_gain_4x)) << 4)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G4(adc_devinfo.cal_data.trim_gain_4x) << 3)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G3(adc_devinfo.cal_data.trim_gain_4x) << 2)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G2(adc_devinfo.cal_data.trim_gain_4x) << 1)
                                   | (SLI_HAL_ADC_GAIN_TRIM_G1(adc_devinfo.cal_data.trim_gain_4x) << 0);
        adc->CFG[config_id].GAINCAL = ((translated_trim << _ADC_GAINCAL_SEPOS_SHIFT) & _ADC_GAINCAL_SEPOS_MASK)
                                      | ((translated_trim << _ADC_GAINCAL_SENEG_SHIFT) & _ADC_GAINCAL_SENEG_MASK)
                                      | ((translated_trim << _ADC_GAINCAL_DIFF_SHIFT) & _ADC_GAINCAL_DIFF_MASK);
      }
      // Apply offset trims (2x).
      adc->CFG[config_id].OFFSETSE = ((adc_calculate_offset(adc_devinfo.offset.trim_off_2x) << _ADC_OFFSETSE_SEPOS_SHIFT) & _ADC_OFFSETSE_SEPOS_MASK)
                                     | ((adc_calculate_offset(adc_devinfo.offset.trim_off_2x) << _ADC_OFFSETSE_SENEG_SHIFT) & _ADC_OFFSETSE_SENEG_MASK);
      adc->CFG[config_id].OFFSETDIFF = (((adc_calculate_offset(adc_devinfo.offset.trim_off_2x) + 1) << _ADC_OFFSETDIFF_DIFF_SHIFT) & _ADC_OFFSETDIFF_DIFF_MASK);
      break;
    case SL_HAL_ADC_ANALOG_GAIN_4:
      if ( init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPL
           || init->voltage_reference == SL_HAL_ADC_REFERENCE_VREFPH ) {
        // Apply 4x gain trim for unbuffered external reference voltage.
        adc->CFG[config_id].GAINCAL = (((uint32_t)adc_devinfo.cal_data.trim_gain_4x << _ADC_GAINCAL_SEPOS_SHIFT) & _ADC_GAINCAL_SEPOS_MASK)
                                      | (((uint32_t)adc_devinfo.cal_data.trim_gain_4x << _ADC_GAINCAL_SENEG_SHIFT) & _ADC_GAINCAL_SENEG_MASK)
                                      | (((uint32_t)adc_devinfo.cal_data.trim_gain_4x << _ADC_GAINCAL_DIFF_SHIFT) & _ADC_GAINCAL_DIFF_MASK);
      }
      // Apply offset trims (4x).
      adc->CFG[config_id].OFFSETSE = ((adc_calculate_offset(adc_devinfo.offset.trim_off_4x) << _ADC_OFFSETSE_SEPOS_SHIFT) & _ADC_OFFSETSE_SEPOS_MASK)
                                     | ((adc_calculate_offset(adc_devinfo.offset.trim_off_4x) << _ADC_OFFSETSE_SENEG_SHIFT) & _ADC_OFFSETSE_SENEG_MASK);
      adc->CFG[config_id].OFFSETDIFF = (((adc_calculate_offset(adc_devinfo.offset.trim_off_4x) + 1) << _ADC_OFFSETDIFF_DIFF_SHIFT) & _ADC_OFFSETDIFF_DIFF_MASK);
      break;
    default:
      break;
  }

  // Apply offset trim applicable to 0.3x, 0.5x and 1x gains.
  if ( init->config[config_id].gain == SL_HAL_ADC_ANALOG_GAIN_0_3125
       || init->config[config_id].gain == SL_HAL_ADC_ANALOG_GAIN_0_5
       || init->config[config_id].gain == SL_HAL_ADC_ANALOG_GAIN_1 ) {
    adc->CFG[config_id].OFFSETSE = ((adc_calculate_offset(adc_devinfo.offset.trim_off_1x) << _ADC_OFFSETSE_SEPOS_SHIFT) & _ADC_OFFSETSE_SEPOS_MASK)
                                   | ((adc_calculate_offset(adc_devinfo.offset.trim_off_1x) << _ADC_OFFSETSE_SENEG_SHIFT) & _ADC_OFFSETSE_SENEG_MASK);
    adc->CFG[config_id].OFFSETDIFF = (((adc_calculate_offset(adc_devinfo.offset.trim_off_1x) + 1) << _ADC_OFFSETDIFF_DIFF_SHIFT) & _ADC_OFFSETDIFF_DIFF_MASK);
  }
#else
  (void)adc;
  (void)init;
  (void)config_id;
#endif
}

/***************************************************************************//**
 * @brief
 *   Calculate sensible ADC prescalers based on branch_clock_freq.
 *
 * @param[in] branch_clock_freq
 *   Incoming peripheral frequency. Should be retrieved using the Clock Manager
 *   service.
 *
 * @param[in] hsclkrate
 *   Pointer to return hsclkrate prescaler.
 *
 * @param[in] adcprescale
 *   Pointer to return adcprescale prescaler.
 *
 * @return
 *   SL_STATUS_OK if computation succeded, SL_STATUS_FAIL otherwise.
 ******************************************************************************/
static sl_status_t adc_calculate_prescalers(uint32_t branch_clock_freq,
                                            uint8_t* hsclkrate,
                                            uint8_t* adcprescale)
{
  *hsclkrate = 127;
  *adcprescale = 127;

  // Set clock prescalers to reasonable defaults with regards to the ADC's
  // branch clock frequency.
  uint32_t clock_prescaled;
  for (uint8_t i = 0; i <= SLI_HAL_ADC_HSCLKRATE_MAX; i++) {
    clock_prescaled = branch_clock_freq / (i + 1);
    if ( clock_prescaled <= SLI_HAL_ADC_CLK_SRC_MAX ) {
      if ( clock_prescaled >= SLI_HAL_ADC_CLK_CORE_MIN
           && clock_prescaled <= SLI_HAL_ADC_CLK_CORE_MAX ) {
        *hsclkrate = i;
        *adcprescale = 0;
        break;
      } else {
        clock_prescaled = clock_prescaled / (SLI_HAL_ADC_ADCPRESCALE_MAX + 1);
        if ( clock_prescaled >= SLI_HAL_ADC_CLK_CORE_MIN
             && clock_prescaled <= SLI_HAL_ADC_CLK_CORE_MAX ) {
          *hsclkrate = i;
          *adcprescale = 1;
          break;
        }
      }
    }
  }

  if ( *hsclkrate == 127 || *adcprescale == 127 ) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

#if defined(_ADC_OFFSETSE_MASK)
/***************************************************************************//**
 * @brief
 *   Calculate the offset calibration to apply from the 6 bit trim value in
 *   DEVINFO.
 *
 * @param[in] trim
 *   The offset trim retrieved from DEVINFO.
 ******************************************************************************/
static uint32_t adc_calculate_offset(uint8_t trim)
{
  // Given that the trim is 6-bits wide and represented by [b5, b4, b3, b2, b1, b0],
  // The calculated offset should be 12-bits wide and translated from the trim
  // as [b5, b5!, b5!, b5!, b5!, b5!, b5!, b4, b3, b2, b1, b0].
  uint32_t b5 = (uint32_t)((trim & (1U << 5)) >> 5);
  return (b5 << 11)
         | ((b5 ^ 0x1U) << 10)
         | ((b5 ^ 0x1U) << 9)
         | ((b5 ^ 0x1U) << 8)
         | ((b5 ^ 0x1U) << 7)
         | ((b5 ^ 0x1U) << 6)
         | ((b5 ^ 0x1U) << 5)
         | (trim & 0x1F);
}
#endif

/***************************************************************************//**
 * @brief
 *   Decodes the data and id of an adc result based on the register format
 *   of a given alignment configuration.
 *
 * @param[in] data
 *   Result data to decode.
 *
 * @param[in] alignment
 *   Alignment of the result data.
 *
 * return
 *   Decoded convertion of the ADC.
 ******************************************************************************/
static sl_hal_adc_result_t adc_decode_result(uint32_t data, sl_hal_adc_alignment_t alignment)
{
  sl_hal_adc_result_t result;

  switch (alignment) {
#if defined(_ADC_SCANFIFOCFG_ALIGNMENT_RIGHT20)
    case SL_HAL_ADC_ALIGNMENT_RIGHT_20:
      //< ID[7:0], SIGN_EXT, DATA[19:0].
      result.data = (data & (uint32_t)0x000FFFFF) >> 0;
      result.id = (data & (uint32_t)0xFF000000) >> 24;
      break;
#endif
    case SL_HAL_ADC_ALIGNMENT_RIGHT_16:
      //< ID[7:0], SIGN_EXT, DATA[15:0].
      result.data = (data & (uint32_t)0x0000FFFF) >> 0;
      result.id = (data & (uint32_t)0xFF000000) >> 24;
      break;
    case SL_HAL_ADC_ALIGNMENT_RIGHT_12:
      //< ID[7:0], SIGN_EXT, DATA[11:0].
      result.data = (data & (uint32_t)0x00000FFF) >> 0;
      result.id = (data & (uint32_t)0xFF000000) >> 24;
      break;
#if defined(_ADC_SCANFIFOCFG_ALIGNMENT_RIGHT8)
    case SL_HAL_ADC_ALIGNMENT_RIGHT_8:
      //< ID[7:0], SIGN_EXT, DATA[7:0].
      result.data = (data & (uint32_t)0x000000FF) >> 0;
      result.id = (data & (uint32_t)0xFF000000) >> 24;
      break;
#endif
#if defined(_ADC_SCANFIFOCFG_ALIGNMENT_LEFT20)
    case  SL_HAL_ADC_ALIGNMENT_LEFT_20:
      //< DATA[19:0], 0000, ID[7:0].
      result.data = (data & (uint32_t)0xFFFFF000) >> 12;
      result.id = (data & (uint32_t)0x000000FF) >> 0;
      break;
#endif
    case  SL_HAL_ADC_ALIGNMENT_LEFT_16:
      //< DATA[15:0], 00000000, ID[7:0].
      result.data = (data & (uint32_t)0xFFFF0000) >> 16;
      result.id = (data & (uint32_t)0x000000FF) >> 0;
      break;
    case  SL_HAL_ADC_ALIGNMENT_LEFT_12:
      //< DATA[11:0], 000000000000, ID[7:0].
      result.data = (data & (uint32_t)0xFFF00000) >> 20;
      result.id = (data & (uint32_t)0x000000FF) >> 0;
      break;
#if defined(_ADC_SCANFIFOCFG_ALIGNMENT_LEFT8)
    case  SL_HAL_ADC_ALIGNMENT_LEFT_8:
      //< DATA[7:0], 0000000000000000, ID[7:0].
      result.data = (data & (uint32_t)0xFF000000) >> 24;
      result.id = (data & (uint32_t)0x000000FF) >> 0;
      break;
#endif
    default:
      EFM_ASSERT(false);
      result.data = 0x0;
      result.id = 0x0;
      break;
  }

  return result;
}

#ifdef __cplusplus
}
#endif
