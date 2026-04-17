/***************************************************************************//**
 * @file
 * @brief Incremental Analog to Digital Converter (IADC) Peripheral API
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

#include "sl_hal_iadc.h"

#if defined(IADC_COUNT) && (IADC_COUNT > 0)

#include "sl_assert.h"
#include "sl_common.h"
#include "sl_hal_system.h"
#include <stddef.h>

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/
// Max IADC clock rates.
#define IADC_CLK_MAX_FREQ                    40000000UL
#define IADC_ANA_CLK_HIGH_SPEED_MAX_FREQ     20000000UL
#define IADC_ANA_CLK_NORMAL_MAX_FREQ         10000000UL
#define IADC_ANA_CLK_HIGH_ACCURACY_MAX_FREQ  5000000UL
#if defined(_IADC_CFG_ADCMODE_HIGHSPEED)
#define IADC_ANA_CLK_MAX_FREQ(adc_mode) ((adc_mode) == SL_HAL_IADC_CFG_ADC_MODE_NORMAL ? IADC_ANA_CLK_NORMAL_MAX_FREQ            \
                                         : ((adc_mode) == SL_HAL_IADC_CFG_ADC_MODE_HIGH_SPEED ? IADC_ANA_CLK_HIGH_SPEED_MAX_FREQ \
                                            : IADC_ANA_CLK_HIGH_ACCURACY_MAX_FREQ))
#else
#define IADC_ANA_CLK_MAX_FREQ(adc_mode) ((adc_mode) == SL_HAL_IADC_CFG_ADC_MODE_NORMAL ? IADC_ANA_CLK_NORMAL_MAX_FREQ \
                                         : IADC_ANA_CLK_HIGH_ACCURACY_MAX_FREQ)
#endif

#define IADC_ROUND_D2I(n) (int)((n) < 0.0f ? ((n) - 0.5f) : ((n) + 0.5f))

#define IADC0_SCANENTRIES IADC0_ENTRIES
#define IADC0_FIFOENTRIES 0x4UL

#define IADC1_SCANENTRIES IADC1_ENTRIES
#define IADC1_FIFOENTRIES 0x4UL

#if defined(IADC_ENTRIES)
#define IADC_SCANENTRIES(iadc) IADC_ENTRIES(IADC_NUM(iadc))
#else
#define IADC_SCANENTRIES(iadc) ((iadc) == IADC0 ? IADC0_SCANENTRIES \
                                : 0UL)
#endif

#if !defined(IADC_CONFIGNUM)
#define IADC_CONFIGNUM(iadc) ((iadc) == 0 ? IADC0_CONFIGNUM \
                              : 0UL)
#endif

#define IADC_FIFOENTRIES(iadc) ((iadc) == IADC0 ? IADC0_FIFOENTRIES \
                                : 0UL)

/*******************************************************************************
 **************************   LOCAL VARIABLES   ********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Convert raw IADC data to a structured result.
 *
 * @details
 *   This function processes the raw data from the IADC and converts
 *   it into a  structured result containing the data and the ID of
 *   the scan table entry or single conversion indicator.
 *
 * @note
 *   The alignment mode determines how the raw data is processed.
 *   Ensure that the alignment parameter matches the configuration of the IADC.
 *
 * @param[in] raw_data
 *   The raw 32-bit data read from the IADC.
 * @param[in] alignment
 *   The alignment mode of the IADC data (e.g., right-aligned or left-aligned).
 *
 * @return
 *   A structured result of type `sl_hal_iadc_result_t` containing:
 *    - `data`: The converted IADC data.
 *    - `id`: The ID of the scan table entry or single conversion indicator.
 ******************************************************************************/
static sl_hal_iadc_result_t iadc_convert_raw_data_to_result(uint32_t raw_data,
                                                            sl_hal_iadc_alignment_t alignment);

/***************************************************************************//**
 * @brief
 *   Get analog gain calibration value from device info.
 *
 * @param[in] analog_gain
 *   The analog gain setting.
 *
 * @return
 *   The calibration value for the specified analog gain.
 ******************************************************************************/
static uint16_t iadc_get_analog_gain_cal_value(unsigned analog_gain);

/***************************************************************************//**
 * @brief
 *   Calculate offset for normal and high-speed modes.
 *
 * @param analog_gain
 *   Analog gain.
 * @param[in] adc_mode
 *   ADC mode (normal or high-speed).
 * @return
 *   The calculated offset.
 ******************************************************************************/
static float iadc_calculate_offset(unsigned analog_gain,
                                   sl_hal_iadc_config_adc_mode_t adc_mode);

/***************************************************************************//**
 * @brief
 *   Calculate gain and offset for normal and high-speed modes.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 * @param[in] init
 *   Pointer to IADC initialization structure.
 * @param[in] adc_mode
 *   ADC mode (normal or high-speed).
 * @param[in] config
 *   Configuration index.
 ******************************************************************************/
static void iadc_calculate_normal_highspeed_gain_offset(IADC_TypeDef *iadc,
                                                        const sl_hal_iadc_init_t *init,
                                                        sl_hal_iadc_config_adc_mode_t adc_mode,
                                                        uint8_t config);

#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
/***************************************************************************//**
 * @brief
 *   Calculate gain and offset for high accuracy mode.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 * @param[in] init
 *   Pointer to IADC initialization structure.
 * @param[in] config
 *   Configuration index.
 ******************************************************************************/
static void iadc_calculate_high_accuracy_gain_offset(IADC_TypeDef *iadc,
                                                     const sl_hal_iadc_init_t *init,
                                                     uint8_t config);
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE void sl_hal_iadc_enable(IADC_TypeDef *iadc);
#if defined(_IADC_STATUS_SYNCBUSY_MASK)
extern __INLINE void sl_hal_iadc_wait_sync(IADC_TypeDef *iadc);
#endif
#if defined(_IADC_EN_DISABLING_MASK)
extern __INLINE void sl_hal_iadc_wait_ready(IADC_TypeDef *iadc);
#endif
extern __INLINE void sl_hal_iadc_disable(IADC_TypeDef *iadc);
extern __INLINE void sl_hal_iadc_start_scan(IADC_TypeDef *iadc);
extern __INLINE void sl_hal_iadc_stop_scan(IADC_TypeDef *iadc);
extern __INLINE void sl_hal_iadc_start_single(IADC_TypeDef *iadc);
extern __INLINE void sl_hal_iadc_stop_single(IADC_TypeDef *iadc);
extern __INLINE void sl_hal_iadc_enable_timer(IADC_TypeDef *iadc);
extern __INLINE void sl_hal_iadc_disable_timer(IADC_TypeDef *iadc);
extern __INLINE uint32_t sl_hal_iadc_get_scan_mask(IADC_TypeDef *iadc);
extern __INLINE uint32_t sl_hal_iadc_get_status(IADC_TypeDef *iadc);
extern __INLINE void sl_hal_iadc_enable_interrupts(IADC_TypeDef *iadc,
                                                   uint32_t flags);
extern __INLINE void sl_hal_iadc_disable_interrupts(IADC_TypeDef *iadc,
                                                    uint32_t flags);
extern __INLINE void sl_hal_iadc_clear_interrupts(IADC_TypeDef *iadc,
                                                  uint32_t flags);
extern __INLINE void sl_hal_iadc_set_pending_interrupts(IADC_TypeDef *iadc,
                                                        uint32_t flags);
extern __INLINE uint32_t sl_hal_iadc_get_pending_interrupts(IADC_TypeDef *iadc);
extern __INLINE uint32_t sl_hal_iadc_get_enable_pending_interrupts(IADC_TypeDef *iadc);
extern __INLINE uint32_t sl_hal_iadc_pull_single_fifo_data(IADC_TypeDef *iadc);
extern __INLINE uint32_t sl_hal_iadc_read_single_data(IADC_TypeDef *iadc);
extern __INLINE uint32_t sl_hal_iadc_pull_scan_fifo_data(IADC_TypeDef *iadc);
extern __INLINE uint32_t sl_hal_iadc_read_scan_data(IADC_TypeDef *iadc);
extern __INLINE void sl_hal_iadc_set_command(IADC_TypeDef *iadc,
                                             sl_hal_iadc_cmd_t cmd);
extern __INLINE uint8_t sl_hal_iadc_get_single_fifo_cnt(IADC_TypeDef *iadc);
extern __INLINE uint8_t sl_hal_iadc_get_scan_fifo_cnt(IADC_TypeDef *iadc);
extern __INLINE sl_hal_iadc_negative_port_input_t sl_hal_iadc_port_pin_to_neg_port(const sl_gpio_t *gpio);
extern __INLINE sl_hal_iadc_positive_port_input_t sl_hal_iadc_port_pin_to_pos_port(const sl_gpio_t *gpio);

/***************************************************************************//**
 * Initialize IADC.
 ******************************************************************************/
void sl_hal_iadc_init(IADC_TypeDef *iadc,
                      const sl_hal_iadc_init_t *init,
                      uint32_t src_clk_freq)
{
  uint32_t tmp;
  uint16_t wanted_prescale;
  uint8_t src_clk_prescale;
  uint32_t adc_clk_prescale;
  uint8_t timebase;
  sl_hal_iadc_config_adc_mode_t adc_mode;

  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(init != NULL);

  // Calculate min allowed SRC_CLK prescaler setting.
  src_clk_prescale = sl_hal_iadc_calculate_src_clk_prescale(iadc, IADC_CLK_MAX_FREQ, src_clk_freq);
  wanted_prescale = init->src_clk_prescale;
  // Use wanted SRC_CLK prescaler setting instead if it is high enough.
  if (wanted_prescale >= src_clk_prescale) {
    src_clk_prescale = (uint8_t)wanted_prescale;
  }

  sl_hal_iadc_disable(iadc);

  timebase = init->timebase;
  if (timebase == 0) {
    // CLK_SRC_ADC is derived from CLK_CMU_ADC, and must be no faster than 40 MHz.
    // If src_clk_freq is greater than 40MHz, then divide by the prescaler HSCLKRATE to obtain valid frequency.
    if (src_clk_freq >= IADC_CLK_MAX_FREQ) {
      src_clk_freq = src_clk_freq / src_clk_prescale;
    }

    // Calculate timebase.
    timebase = sl_hal_iadc_calculate_timebase(iadc, src_clk_freq);
  }

  tmp = (iadc->CTRL & ~(_IADC_CTRL_WARMUPMODE_MASK | _IADC_CTRL_TIMEBASE_MASK
                        | _IADC_CTRL_HSCLKRATE_MASK))
        | (((uint32_t) init->warmup) << _IADC_CTRL_WARMUPMODE_SHIFT)
        | (((uint32_t) timebase) << _IADC_CTRL_TIMEBASE_SHIFT)
        | (((uint32_t) src_clk_prescale) << _IADC_CTRL_HSCLKRATE_SHIFT);

  if (init->iadc_clk_suspend0) {
    tmp |= IADC_CTRL_ADCCLKSUSPEND0;
  }
  if (init->iadc_clk_suspend1) {
    tmp |= IADC_CTRL_ADCCLKSUSPEND1;
  }
  if (init->debug_halt) {
    tmp |= IADC_CTRL_DBGHALT;
  }
  iadc->CTRL = tmp;

  iadc->TIMER = (iadc->TIMER & ~_IADC_TIMER_TIMER_MASK)
                | ((uint32_t) init->timer_cycles) << _IADC_TIMER_TIMER_SHIFT;

  iadc->CMPTHR = (iadc->CMPTHR & ~(_IADC_CMPTHR_ADGT_MASK | _IADC_CMPTHR_ADLT_MASK))
                 | (((uint32_t) init->greater_than_equal_thres) << _IADC_CMPTHR_ADGT_SHIFT)
                 | (((uint32_t) init->less_than_equal_thres) << _IADC_CMPTHR_ADLT_SHIFT);

  // Write configurations.
  for (uint8_t config = 0; config < IADC_CONFIGNUM(IADC_NUM(iadc)); config++) {
    // Find min allowed ADC_CLK prescaler setting for given mode.
    adc_mode = init->configs[config].adc_mode;
    wanted_prescale = (uint16_t)init->configs[config].adc_clk_prescale;
    adc_clk_prescale = sl_hal_iadc_calculate_adc_clk_prescale(iadc,
                                                              IADC_ANA_CLK_MAX_FREQ(adc_mode),
                                                              src_clk_freq,
                                                              adc_mode,
                                                              src_clk_prescale);

    // Use wanted ADC_CLK prescaler setting instead if it is high enough.
    adc_clk_prescale = SL_MAX(adc_clk_prescale, wanted_prescale);

    tmp = iadc->CFG[config].CFG & ~(_IADC_CFG_ADCMODE_MASK | _IADC_CFG_OSRHS_MASK
                                    | _IADC_CFG_ANALOGGAIN_MASK | _IADC_CFG_REFSEL_MASK
#if defined(_IADC_CFG_DIGAVG_MASK)
                                    | _IADC_CFG_DIGAVG_MASK
#endif
                                    | _IADC_CFG_TWOSCOMPL_MASK
#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
                                    | _IADC_CFG_OSRHA_MASK
#endif
                                    );
    iadc->CFG[config].CFG = tmp
                            | (((uint32_t)(adc_mode) << _IADC_CFG_ADCMODE_SHIFT) & _IADC_CFG_ADCMODE_MASK)
                            | (((uint32_t)(init->configs[config].osr_high_speed) << _IADC_CFG_OSRHS_SHIFT)
                               & _IADC_CFG_OSRHS_MASK)
#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
                            | (((uint32_t)(init->configs[config].osr_high_accuracy) << _IADC_CFG_OSRHA_SHIFT)
                               & _IADC_CFG_OSRHA_MASK)
#endif
                            | (((uint32_t)(init->configs[config].analog_gain) << _IADC_CFG_ANALOGGAIN_SHIFT)
                               & _IADC_CFG_ANALOGGAIN_MASK)
                            | (((uint32_t)(init->configs[config].reference) << _IADC_CFG_REFSEL_SHIFT)
                               & _IADC_CFG_REFSEL_MASK)
#if defined(_IADC_CFG_DIGAVG_MASK)
                            | (((uint32_t)(init->configs[config].dig_avg) << _IADC_CFG_DIGAVG_SHIFT)
                               & _IADC_CFG_DIGAVG_MASK)
#endif
                            | (((uint32_t)(init->configs[config].twos_complement) << _IADC_CFG_TWOSCOMPL_SHIFT)
                               & _IADC_CFG_TWOSCOMPL_MASK);

    sl_hal_iadc_calculate_gain_offset(iadc, init, adc_mode, config, adc_clk_prescale);
  }
}

/***************************************************************************//**
 * Calculate gain and offset.
 ******************************************************************************/
void sl_hal_iadc_calculate_gain_offset(IADC_TypeDef *iadc,
                                       const sl_hal_iadc_init_t *init,
                                       sl_hal_iadc_config_adc_mode_t adc_mode,
                                       uint8_t config,
                                       uint32_t adc_clk_prescale)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(init != NULL);

  // Calculate gain and offset based on ADC mode
  switch (adc_mode) {
    case SL_HAL_IADC_CFG_ADC_MODE_NORMAL:
#if defined(_IADC_CFG_ADCMODE_HIGHSPEED)
    case SL_HAL_IADC_CFG_ADC_MODE_HIGH_SPEED:
#endif
      iadc_calculate_normal_highspeed_gain_offset(iadc, init, adc_mode, config);
      break;

#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
    case SL_HAL_IADC_CFG_ADC_MODE_HIGH_ACCURACY:
      iadc_calculate_high_accuracy_gain_offset(iadc, init, config);
      break;
#endif

    default:
      // Mode not supported.
      EFM_ASSERT(false);
      break;
  }

  // Set ADC clock prescaler
  iadc->CFG[config].SCHED = ((adc_clk_prescale << _IADC_SCHED_PRESCALE_SHIFT)
                             & _IADC_SCHED_PRESCALE_MASK);
}

/***************************************************************************//**
 * Reset IADC to same state as after a HW reset.
 ******************************************************************************/
void sl_hal_iadc_reset(IADC_TypeDef *iadc)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  // Write all WSYNC registers to reset value while enabled.
  sl_hal_iadc_enable(iadc);
  EFM_ASSERT((iadc->EN & _IADC_EN_EN_MASK) == _IADC_EN_EN_MASK);

  // Stop conversions and timer, before resetting other registers.
  iadc->CMD = IADC_CMD_SINGLESTOP | IADC_CMD_SCANSTOP | IADC_CMD_TIMERDIS;

  // Wait for all IADC operations to stop.
  while ((iadc->STATUS & (IADC_STATUS_CONVERTING
                          | IADC_STATUS_SCANQUEUEPENDING
                          | IADC_STATUS_SINGLEQUEUEPENDING
                          | IADC_STATUS_TIMERACTIVE))
         != 0UL) {
    // No operation is performed, the loop simply waits for the IADC to stop.
  }

  // Reset all WSYNC registers.
  iadc->MASKREQ = _IADC_MASKREQ_RESETVALUE;
  iadc->SINGLE  = _IADC_SINGLE_RESETVALUE;

  // Wait for SINGLE and MASQREQ writes to propagate to working registers.
  while ((iadc->STATUS & (IADC_STATUS_MASKREQWRITEPENDING
                          | IADC_STATUS_SINGLEWRITEPENDING))
         != 0UL) {
    // No operation is needed here.
  }

  // Pull from FIFOs until they are empty.

  // Errata IADC_E305: Check SINGLEFIFOSTAT to make sure that SINGLEFIFO is getting emptied in case.
  // where STATUS register is incorrect.
  while (((iadc->STATUS & IADC_STATUS_SINGLEFIFODV) != 0UL) || (iadc->SINGLEFIFOSTAT > 0)) {
    (void) sl_hal_iadc_pull_single_fifo_data(iadc);
  }

  // Errata IADC_E305: check SCANFIFOSTAT to make sure that SCANFIFO is getting emptied in case.
  // where STATUS register is incorrect.
  while (((iadc->STATUS & IADC_STATUS_SCANFIFODV) != 0UL) || (iadc->SCANFIFOSTAT > 0)) {
    (void) sl_hal_iadc_pull_scan_fifo_data(iadc);
  }

  // Read data registers to clear data valid flags.
  (void) sl_hal_iadc_read_single_data(iadc);
  (void) sl_hal_iadc_read_scan_data(iadc);

  // Write all WSTATIC registers to reset value while disabled.
  sl_hal_iadc_disable(iadc);

  // Reset all WSTATIC registers.
  iadc->CTRL            = _IADC_CTRL_RESETVALUE;
  iadc->TIMER           = _IADC_TIMER_RESETVALUE;
  iadc->TRIGGER         = _IADC_TRIGGER_RESETVALUE;

  iadc->CMPTHR          = _IADC_CMPTHR_RESETVALUE;
  iadc->SINGLEFIFOCFG   = _IADC_SINGLEFIFOCFG_RESETVALUE;
  iadc->SCANFIFOCFG     = _IADC_SCANFIFOCFG_RESETVALUE;

  for (uint8_t i = 0; i < IADC_CONFIGNUM(IADC_NUM(iadc)); i++) {
    iadc->CFG[i].CFG    = _IADC_CFG_RESETVALUE;
    iadc->CFG[i].SCALE  = _IADC_SCALE_RESETVALUE;
    iadc->CFG[i].SCHED  = _IADC_SCHED_RESETVALUE;
  }

  for (uint8_t i = 0; i < IADC_SCANENTRIES(iadc); i++) {
    iadc->SCANTABLE[i].SCAN = _IADC_SCAN_RESETVALUE;
  }

  // Clear interrupt flags and disable interrupts.
  sl_hal_iadc_clear_interrupts(iadc, _IADC_IF_MASK);
  sl_hal_iadc_disable_interrupts(iadc, _IADC_IEN_MASK);
}

/***************************************************************************//**
 * Initialize IADC scan sequence.
 ******************************************************************************/
void sl_hal_iadc_init_scan(IADC_TypeDef *iadc,
                           const sl_hal_iadc_init_scan_t *init,
                           const sl_hal_iadc_scan_table_t *scan_table)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(init != NULL);
  EFM_ASSERT(scan_table != NULL);

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)
  // Errata IADC_E305. Makes sure that DVL is equal or less than 7 entries.
  EFM_ASSERT(init->data_valid_level <= SL_HAL_IADC_DATA_VALID_7);
#endif

  sl_hal_iadc_disable(iadc);

  iadc->SCANFIFOCFG = (iadc->SCANFIFOCFG & ~(_IADC_SCANFIFOCFG_ALIGNMENT_MASK | _IADC_SCANFIFOCFG_SHOWID_MASK
                                             | _IADC_SCANFIFOCFG_DVL_MASK | _IADC_SCANFIFOCFG_DMAWUFIFOSCAN_MASK))
                      | (((uint32_t) init->alignment) << _IADC_SCANFIFOCFG_ALIGNMENT_SHIFT)
                      | (((uint32_t) init->show_id) <<  _IADC_SCANFIFOCFG_SHOWID_SHIFT)
                      | (((uint32_t) init->data_valid_level) << _IADC_SCANFIFOCFG_DVL_SHIFT)
                      | (((uint32_t) init->fifo_dma_wakeup) << _IADC_SCANFIFOCFG_DMAWUFIFOSCAN_SHIFT);

  // Clear bitfields for scan conversion in IADCn->TRIGGER and set new values.
  iadc->TRIGGER = (iadc->TRIGGER & ~(_IADC_TRIGGER_SCANTRIGSEL_MASK | _IADC_TRIGGER_SCANTRIGACTION_MASK))
                  | (((uint32_t) init->trigger_select) << _IADC_TRIGGER_SCANTRIGSEL_SHIFT)
                  | (((uint32_t) init->trigger_action) << _IADC_TRIGGER_SCANTRIGACTION_SHIFT);

  // Write scan table.
  for (uint8_t entry_num = 0; entry_num < IADC_SCANENTRIES(iadc); entry_num++) {
    iadc->SCANTABLE[entry_num].SCAN = (iadc->SCANTABLE[entry_num].SCAN & ~(_IADC_SCAN_PINNEG_MASK | _IADC_SCAN_PORTNEG_MASK
                                                                           | _IADC_SCAN_PINPOS_MASK | _IADC_SCAN_PORTPOS_MASK
                                                                           | _IADC_SCAN_CFG_MASK | _IADC_SCAN_CMP_MASK))
                                      |  ((uint32_t) scan_table->entries[entry_num].negative_port << _IADC_SCAN_PORTNEG_SHIFT)
                                      |  ((uint32_t) scan_table->entries[entry_num].negative_pin << _IADC_SCAN_PINNEG_SHIFT)
                                      |  ((uint32_t) scan_table->entries[entry_num].positive_port << _IADC_SCAN_PORTPOS_SHIFT)
                                      |  ((uint32_t) scan_table->entries[entry_num].positive_pin << _IADC_SCAN_PINPOS_SHIFT)
                                      |  ((uint32_t) scan_table->entries[entry_num].config_id << _IADC_SCAN_CFG_SHIFT)
                                      |  (((uint32_t) scan_table->entries[entry_num].compare) << _IADC_SINGLE_CMP_SHIFT);
  }
}

/***************************************************************************//**
 * Add/update entry in scan table.
 ******************************************************************************/
void sl_hal_iadc_update_scan_entry(IADC_TypeDef *iadc,
                                   uint8_t id,
                                   sl_hal_iadc_scan_table_entry_t *entry)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(entry != NULL);

  sl_hal_iadc_disable(iadc);

  // Update entry in scan table.
  iadc->SCANTABLE[id].SCAN = (iadc->SCANTABLE[id].SCAN & ~(_IADC_SCAN_PINNEG_MASK | _IADC_SCAN_PORTNEG_MASK
                                                           | _IADC_SCAN_PINPOS_MASK | _IADC_SCAN_PORTPOS_MASK
                                                           | _IADC_SCAN_CFG_MASK | _IADC_SCAN_CMP_MASK))
                             |  ((uint32_t) entry->negative_port << _IADC_SCAN_PORTNEG_SHIFT)
                             |  ((uint32_t) entry->negative_pin << _IADC_SCAN_PINNEG_SHIFT)
                             |  ((uint32_t) entry->positive_port << _IADC_SCAN_PORTPOS_SHIFT)
                             |  ((uint32_t) entry->positive_pin << _IADC_SCAN_PINPOS_SHIFT)
                             |  ((uint32_t) entry->config_id << _IADC_SCAN_CFG_SHIFT)
                             |  (((uint32_t) entry->compare) << _IADC_SINGLE_CMP_SHIFT);

  sl_hal_iadc_enable(iadc);

  if (entry->include_in_scan) {
    iadc->MASKREQ_SET = (1UL << (id & 0x1FUL)) << _IADC_MASKREQ_MASKREQ_SHIFT;
  } else {
    iadc->MASKREQ_CLR = (1UL << (id & 0x1FUL)) << _IADC_MASKREQ_MASKREQ_SHIFT;
  }
}

/***************************************************************************//**
 * Set mask of IADC scan table entries to include in scan.
 ******************************************************************************/
void sl_hal_iadc_set_scan_mask(IADC_TypeDef *iadc, uint32_t mask)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(mask <= ((1UL << IADC_SCANENTRIES(iadc)) - 1UL));

  sl_hal_iadc_enable(iadc);

  iadc->MASKREQ = (iadc->MASKREQ & ~_IADC_MASKREQ_MASKREQ_MASK)
                  | (mask << _IADC_MASKREQ_MASKREQ_SHIFT);
}

/***************************************************************************//**
 * Set mask of IADC scan table entries to include in scan using scan table.
 ******************************************************************************/
void sl_hal_iadc_set_scan_mask_multiple_entries(IADC_TypeDef *iadc,
                                                const sl_hal_iadc_scan_table_t *scan_table)
{
  uint32_t temp = 0;

  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(scan_table != NULL);

  sl_hal_iadc_enable(iadc);

  iadc->MASKREQ &= ~_IADC_MASKREQ_MASKREQ_SHIFT;
  // Set scan mask.
  for (uint8_t entry_num = 0; entry_num < IADC_SCANENTRIES(iadc); entry_num++) {
    if (scan_table->entries[entry_num].include_in_scan) {
      temp |= (1UL << entry_num) << _IADC_MASKREQ_MASKREQ_SHIFT;
    }
  }
  iadc->MASKREQ = temp;
}

/***************************************************************************//**
 * Initialize single IADC conversion.
 ******************************************************************************/
void sl_hal_iadc_init_single(IADC_TypeDef *iadc,
                             const sl_hal_iadc_init_single_t *init,
                             const sl_hal_iadc_single_input_t *input)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(init != NULL);
  EFM_ASSERT(input != NULL);

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)
  // Errata IADC_E305. Makes sure that DVL is equal or less than 7 entries.
  EFM_ASSERT(init->data_valid_level <= SL_HAL_IADC_DATA_VALID_7);
#endif

  sl_hal_iadc_disable(iadc);

  iadc->SINGLEFIFOCFG = (iadc->SINGLEFIFOCFG & ~(_IADC_SINGLEFIFOCFG_ALIGNMENT_MASK | _IADC_SINGLEFIFOCFG_DVL_MASK
                                                 | _IADC_SINGLEFIFOCFG_SHOWID_MASK | _IADC_SINGLEFIFOCFG_DMAWUFIFOSINGLE_MASK))
                        | (((uint32_t) init->alignment) << _IADC_SINGLEFIFOCFG_ALIGNMENT_SHIFT)
                        | (((uint32_t) init->show_id) << _IADC_SINGLEFIFOCFG_SHOWID_SHIFT)
                        | (((uint32_t) init->data_valid_level) << _IADC_SINGLEFIFOCFG_DVL_SHIFT)
                        | (((uint32_t) init->fifo_dma_wakeup) << _IADC_SINGLEFIFOCFG_DMAWUFIFOSINGLE_SHIFT);

  // Clear bitfields for single conversion in IADCn->TRIGGER and set new values.
  iadc->TRIGGER = (iadc->TRIGGER & ~(_IADC_TRIGGER_SINGLETRIGSEL_MASK | _IADC_TRIGGER_SINGLETRIGACTION_MASK
                                     | _IADC_TRIGGER_SINGLETAILGATE_MASK))
                  | (((uint32_t) init->trigger_select) << _IADC_TRIGGER_SINGLETRIGSEL_SHIFT)
                  | (((uint32_t) init->trigger_action) << _IADC_TRIGGER_SINGLETRIGACTION_SHIFT)
                  | (((uint32_t) init->single_tailgate) << _IADC_TRIGGER_SINGLETAILGATE_SHIFT);

  sl_hal_iadc_update_single_input(iadc, input);

  sl_hal_iadc_enable(iadc);
}

/***************************************************************************//**
 * Update IADC single input selection.
 ******************************************************************************/
void sl_hal_iadc_update_single_input(IADC_TypeDef *iadc,
                                     const sl_hal_iadc_single_input_t *input)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(input != NULL);

  // IADCn->SINGLE has WSYNC type and can only be written while enabled.
  sl_hal_iadc_enable(iadc);

  iadc->SINGLE = (iadc->SINGLE & ~(_IADC_SINGLE_PORTNEG_MASK | _IADC_SINGLE_PINNEG_MASK
                                   | _IADC_SINGLE_PORTPOS_MASK | _IADC_SINGLE_PINPOS_MASK
                                   | _IADC_SINGLE_CFG_MASK | _IADC_SINGLE_CMP_MASK))
                 |  ((uint32_t) input->negative_port << _IADC_SINGLE_PORTNEG_SHIFT)
                 |  ((uint32_t) input->negative_pin << _IADC_SINGLE_PINNEG_SHIFT)
                 |  ((uint32_t) input->positive_port << _IADC_SINGLE_PORTPOS_SHIFT)
                 |  ((uint32_t) input->positive_pin << _IADC_SINGLE_PINPOS_SHIFT)
                 |  ((uint32_t) input->single_input_config_id << _IADC_SINGLE_CFG_SHIFT)
                 |  (((uint32_t) input->compare) << _IADC_SINGLE_CMP_SHIFT);
}
/***************************************************************************//**
 * Calculate prescaler for CLK_SRC_ADC high speed clock.
 ******************************************************************************/
uint8_t sl_hal_iadc_calculate_src_clk_prescale(IADC_TypeDef *iadc,
                                               uint32_t src_clk_freq,
                                               uint32_t cmu_clk_freq)
{
  uint32_t ret;

  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(src_clk_freq);

  // Make sure wanted CLK_SRC_ADC clock is below max allowed frequency.
  src_clk_freq = SL_MIN(src_clk_freq, IADC_CLK_MAX_FREQ);

  ret = (cmu_clk_freq + src_clk_freq - 1UL) / src_clk_freq;
  if (ret != 0UL) {
    ret--;
  }

  // Limit to max allowed register setting.
  if (ret > _IADC_CTRL_HSCLKRATE_DIV4) {
    ret = _IADC_CTRL_HSCLKRATE_DIV4;
  }

  return (uint8_t)ret;
}

/***************************************************************************//**
 * Calculate prescaler for ADC_CLK clock.
 ******************************************************************************/
uint32_t sl_hal_iadc_calculate_adc_clk_prescale(IADC_TypeDef *iadc,
                                                uint32_t adc_clk_freq,
                                                uint32_t cmu_clk_freq,
                                                sl_hal_iadc_config_adc_mode_t adc_mode,
                                                uint8_t scr_clk_prescaler)
{
  uint32_t ret;

  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  EFM_ASSERT(adc_clk_freq);

  // Make sure wanted analog clock is below max allowed frequency for the given mode.
  if (adc_clk_freq > IADC_ANA_CLK_MAX_FREQ(adc_mode)) {
    adc_clk_freq = IADC_ANA_CLK_MAX_FREQ(adc_mode);
  }

  // Apply CLK_SRC_ADC prescaler.
  cmu_clk_freq /= scr_clk_prescaler + 1UL;

  ret = (cmu_clk_freq + adc_clk_freq - 1UL) / adc_clk_freq;
  if (ret != 0UL) {
    ret--;
  }

  // Limit to max allowed register setting.
  ret = SL_MIN(ret, (_IADC_SCHED_PRESCALE_MASK >> _IADC_SCHED_PRESCALE_SHIFT));

  return ret;
}

/***************************************************************************//**
 * Calculate timebase value in order to get a timebase providing at least 1us.
 ******************************************************************************/
uint8_t sl_hal_iadc_calculate_timebase(IADC_TypeDef *iadc,
                                       uint32_t src_clk_freq)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  // Make sure we get non-zero frequency for below calculation.
  if (src_clk_freq == 0UL) {
    src_clk_freq = 1;
  }
  // If src_clk_freq is greater than 40MHz, then divide by the prescaler HSCLKRATE.
  if (src_clk_freq > IADC_CLK_MAX_FREQ) {
    uint32_t prescaler = (iadc->CTRL & _IADC_CTRL_HSCLKRATE_MASK) >> _IADC_CTRL_HSCLKRATE_SHIFT;
    src_clk_freq /= (prescaler + 1);
  }

  // Determine number of ADCCLK cycle >= 1us.
  src_clk_freq += 999999UL;
  src_clk_freq /= 1000000UL;

  // Convert to N+1 format.
  src_clk_freq -= 1UL;

  // Limit to max allowed register setting.
  src_clk_freq = SL_MIN(src_clk_freq, (_IADC_CTRL_TIMEBASE_MASK >> _IADC_CTRL_TIMEBASE_SHIFT));

  // Return timebase value.
  return (uint8_t) src_clk_freq;
}

/***************************************************************************//**
 * Read most recent single conversion result. The result struct includes both
 * the data and the ID (0x20) if show_id was set when initializing single mode.
 * Calling this function will not affect the state of the single data FIFO.
 ******************************************************************************/
sl_hal_iadc_result_t sl_hal_iadc_read_single_result(IADC_TypeDef *iadc)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  uint32_t alignment = (iadc->SINGLEFIFOCFG & _IADC_SINGLEFIFOCFG_ALIGNMENT_MASK)
                       >> _IADC_SINGLEFIFOCFG_ALIGNMENT_SHIFT;

  return iadc_convert_raw_data_to_result(iadc->SINGLEDATA,
                                         (sl_hal_iadc_alignment_t) alignment);
}

/***************************************************************************//**
 * Pull result from single data FIFO. The result struct includes both the data
 * and the ID (0x20) if show_id was set when initializing single mode.
 ******************************************************************************/
sl_hal_iadc_result_t sl_hal_iadc_read_single_fifo_result(IADC_TypeDef *iadc)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  uint32_t alignment = (iadc->SINGLEFIFOCFG & _IADC_SINGLEFIFOCFG_ALIGNMENT_MASK)
                       >> _IADC_SINGLEFIFOCFG_ALIGNMENT_SHIFT;

  return iadc_convert_raw_data_to_result(iadc->SINGLEFIFODATA,
                                         (sl_hal_iadc_alignment_t) alignment);
}

/***************************************************************************//**
 * Read most recent scan conversion result. The result struct includes both
 * the data and the ID (0x20) if show_id was set when initializing scan entry.
 * Calling this function will not affect the state of the scan data FIFO.
 ******************************************************************************/
sl_hal_iadc_result_t sl_hal_iadc_read_scan_result(IADC_TypeDef *iadc)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  uint32_t alignment = (iadc->SCANFIFOCFG & _IADC_SCANFIFOCFG_ALIGNMENT_MASK)
                       >> _IADC_SCANFIFOCFG_ALIGNMENT_SHIFT;

  return iadc_convert_raw_data_to_result(iadc->SCANDATA,
                                         (sl_hal_iadc_alignment_t) alignment);
}

/***************************************************************************//**
 * Pull result from scan data FIFO. The result struct includes both the data
 * and the ID (0x20) if show_id was set when initializing scan entry.
 ******************************************************************************/
sl_hal_iadc_result_t sl_hal_iadc_pull_scan_fifo_result(IADC_TypeDef *iadc)
{
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  uint32_t alignment = (iadc->SCANFIFOCFG & _IADC_SCANFIFOCFG_ALIGNMENT_MASK)
                       >> _IADC_SCANFIFOCFG_ALIGNMENT_SHIFT;

  return iadc_convert_raw_data_to_result(iadc->SCANFIFODATA,
                                         (sl_hal_iadc_alignment_t) alignment);
}

/***************************************************************************//**
 * Get reference voltage selection.
 ******************************************************************************/
uint32_t sl_hal_iadc_get_reference_voltage(sl_hal_iadc_voltage_reference_t reference)
{
  uint32_t ref_voltage = 0;
  sl_hal_system_chip_revision_t chip_rev;
  sl_hal_system_get_chip_revision(&chip_rev);

  switch (reference) {
    case SL_HAL_IADC_REFERENCE_VREFINT_1V2:
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
      if (chip_rev.major == 1UL) {
        ref_voltage = 1210;
      } else {
        ref_voltage = 1180;
      }
#else
      ref_voltage = 1210;
#endif
      break;
    case SL_HAL_IADC_VREF_EXT_1V25:
      ref_voltage = 1250;
      break;
#if defined(_IADC_CFG_REFSEL_VREF2P5)
    case SL_HAL_IADC_VREF_EXT_2V5:
      ref_voltage = 2500;
      break;
#endif
    case SL_HAL_IADC_VREF_VDDX:
      ref_voltage = 3000;
      break;
    case SL_HAL_IADC_VREF_VDDX0P8BUF:
      ref_voltage = 2400;
      break;
#if defined(_IADC_CFG_REFSEL_VREFBUF)
    case SL_HAL_IADC_VREF_BUF:
      ref_voltage = 12500;
      break;
#endif
#if defined(_IADC_CFG_REFSEL_VREF0P8BUF)
    case SL_HAL_IADC_VREF_0P8BUF:
      ref_voltage = 1000;
      break;
#endif
    default:
      EFM_ASSERT(false);
      break;
  }

  return ref_voltage;
}

static sl_hal_iadc_result_t iadc_convert_raw_data_to_result(uint32_t raw_data,
                                                            sl_hal_iadc_alignment_t alignment)
{
  sl_hal_iadc_result_t result;

  switch (alignment) {
    case SL_HAL_IADC_ALIGNMENT_RIGHT_12:
#if defined(IADC_SINGLEFIFOCFG_ALIGNMENT_RIGHT16)
    case SL_HAL_IADC_ALIGNMENT_RIGHT_16:
#endif
#if defined(IADC_SINGLEFIFOCFG_ALIGNMENT_RIGHT20)
    case SL_HAL_IADC_ALIGNMENT_RIGHT_20:
#endif
      // Mask out ID and replace with sign extension.
      result.data = (raw_data & 0x00FFFFFFUL)
                    | ((raw_data & 0x00800000UL) != 0x0UL ? 0xFF000000UL : 0x0UL);
      // Mask out data and shift down.
      result.id   = (uint8_t)((raw_data & 0xFF000000UL) >> 24);
      break;

    case SL_HAL_IADC_ALIGNMENT_LEFT_12:
#if defined(IADC_SINGLEFIFOCFG_ALIGNMENT_RIGHT16)
    case SL_HAL_IADC_ALIGNMENT_LEFT_16:
#endif
#if defined(IADC_SINGLEFIFOCFG_ALIGNMENT_RIGHT20)
    case SL_HAL_IADC_ALIGNMENT_LEFT_20:
#endif
      result.data = raw_data & 0xFFFFFF00UL;
      result.id   = (uint8_t)(raw_data & 0x000000FFUL);
      break;
    default:
      break;
  }

  return result;
}

/***************************************************************************//**
 * Get analog gain calibration value from device info.
 ******************************************************************************/
static uint16_t iadc_get_analog_gain_cal_value(unsigned analog_gain)
{
  uint16_t calc_ana_gain;

  switch (analog_gain) {
#if defined(_IADC_CFG_ANALOGGAIN_ANAGAIN0P25)
    case SL_HAL_IADC_ANALOG_GAIN_0P25:
#endif
    case SL_HAL_IADC_ANALOG_GAIN_0P5:
    case SL_HAL_IADC_ANALOG_GAIN_1:
      calc_ana_gain = (uint16_t)((DEVINFO->IADC0GAIN0 & _DEVINFO_IADC0GAIN0_GAINCANA1_MASK) >> _DEVINFO_IADC0GAIN0_GAINCANA1_SHIFT);
      break;
    case SL_HAL_IADC_ANALOG_GAIN_2:
      calc_ana_gain = (uint16_t)((DEVINFO->IADC0GAIN0 & _DEVINFO_IADC0GAIN0_GAINCANA2_MASK) >> _DEVINFO_IADC0GAIN0_GAINCANA2_SHIFT);
      break;
    case SL_HAL_IADC_ANALOG_GAIN_3:
      calc_ana_gain = (uint16_t)((DEVINFO->IADC0GAIN1 & _DEVINFO_IADC0GAIN1_GAINCANA3_MASK) >> _DEVINFO_IADC0GAIN1_GAINCANA3_SHIFT);
      break;
    case SL_HAL_IADC_ANALOG_GAIN_4:
      calc_ana_gain = (uint16_t)((DEVINFO->IADC0GAIN1 & _DEVINFO_IADC0GAIN1_GAINCANA4_MASK) >> _DEVINFO_IADC0GAIN1_GAINCANA4_SHIFT);
      break;
    default:
      calc_ana_gain = (uint16_t)((DEVINFO->IADC0GAIN0 & _DEVINFO_IADC0GAIN0_GAINCANA1_MASK) >> _DEVINFO_IADC0GAIN0_GAINCANA1_SHIFT);
      break;
  }

  return calc_ana_gain;
}

/***************************************************************************//**
 * Calculate offset for normal and high-speed modes.
 ******************************************************************************/
static float iadc_calculate_offset(unsigned analog_gain,
                                   sl_hal_iadc_config_adc_mode_t adc_mode)
{
  float offset = 0.0f;

  if (analog_gain == SL_HAL_IADC_ANALOG_GAIN_2) {
    if (adc_mode == SL_HAL_IADC_CFG_ADC_MODE_NORMAL) {
      offset = (int16_t)(DEVINFO->IADC0NORMALOFFSETCAL0 >> _DEVINFO_IADC0NORMALOFFSETCAL0_OFFSETANA2NORM_SHIFT);
    } else {
      offset = (int16_t)(DEVINFO->IADC0HISPDOFFSETCAL0 >> _DEVINFO_IADC0HISPDOFFSETCAL0_OFFSETANA2HISPD_SHIFT);
    }
  } else if (analog_gain == SL_HAL_IADC_ANALOG_GAIN_3) {
    if (adc_mode == SL_HAL_IADC_CFG_ADC_MODE_NORMAL) {
      offset = (int16_t)(DEVINFO->IADC0NORMALOFFSETCAL0 >> _DEVINFO_IADC0NORMALOFFSETCAL0_OFFSETANA2NORM_SHIFT) * 2;
    } else {
      offset = (int16_t)(DEVINFO->IADC0HISPDOFFSETCAL0 >> _DEVINFO_IADC0HISPDOFFSETCAL0_OFFSETANA2HISPD_SHIFT) * 2;
    }
  } else if (analog_gain == SL_HAL_IADC_ANALOG_GAIN_4) {
    if (adc_mode == SL_HAL_IADC_CFG_ADC_MODE_NORMAL) {
      offset = (int16_t)(DEVINFO->IADC0NORMALOFFSETCAL0 >> _DEVINFO_IADC0NORMALOFFSETCAL0_OFFSETANA2NORM_SHIFT) * 3;
    } else {
      offset = (int16_t)(DEVINFO->IADC0HISPDOFFSETCAL0 >> _DEVINFO_IADC0HISPDOFFSETCAL0_OFFSETANA2HISPD_SHIFT) * 3;
    }
  }

  return offset;
}

/***************************************************************************//**
 * Calculate gain and offset for normal and high-speed modes.
 ******************************************************************************/
static void iadc_calculate_normal_highspeed_gain_offset(IADC_TypeDef *iadc,
                                                        const sl_hal_iadc_init_t *init,
                                                        sl_hal_iadc_config_adc_mode_t adc_mode,
                                                        uint8_t config)
{
  float offset;
  uint32_t scale;
  int32_t i_offset;
  int32_t i_osr;
  uint32_t tmp;
  unsigned analog_gain;
  uint16_t calc_ana_gain;

  // Calculate analog gain.
  analog_gain = (iadc->CFG[config].CFG & _IADC_CFG_ANALOGGAIN_MASK) >> _IADC_CFG_ANALOGGAIN_SHIFT;
  calc_ana_gain = iadc_get_analog_gain_cal_value(analog_gain);

  // Calculate base offset based on analog gain.
  offset = iadc_calculate_offset(analog_gain, adc_mode);

  // Set gain correction bitfields in scale variable.
  tmp = (uint32_t)calc_ana_gain & 0x9FFFU;
  scale = tmp << _IADC_SCALE_GAIN13LSB_SHIFT;
  if ((tmp & 0x8000U) != 0U) {
    scale |= IADC_SCALE_GAIN3MSB;
  }

  // Adjust offset according to selected OSR.
  i_osr = 1U << (((iadc->CFG[config].CFG & _IADC_CFG_OSRHS_MASK) >> _IADC_CFG_OSRHS_SHIFT) + 1U);
  if (i_osr == 2) {
    if (adc_mode == SL_HAL_IADC_CFG_ADC_MODE_NORMAL) {
      offset += (int16_t)(DEVINFO->IADC0NORMALOFFSETCAL0 & _DEVINFO_IADC0NORMALOFFSETCAL0_OFFSETANA1NORM_MASK);
    } else {
      offset += (int16_t)(DEVINFO->IADC0HISPDOFFSETCAL0 & _DEVINFO_IADC0HISPDOFFSETCAL0_OFFSETANA1HISPD_MASK);
    }
  } else {
    if (adc_mode == SL_HAL_IADC_CFG_ADC_MODE_NORMAL) {
      offset = (int16_t)(DEVINFO->IADC0NORMALOFFSETCAL1 & _DEVINFO_IADC0NORMALOFFSETCAL1_OFFSETANA3NORM_MASK) - offset;
    } else {
      offset += (int16_t)(DEVINFO->IADC0HISPDOFFSETCAL1 & _DEVINFO_IADC0HISPDOFFSETCAL1_OFFSETANA3HISPD_MASK) - offset;
    }
    offset /= (float)i_osr / 2.0f;
    offset += (int16_t)(DEVINFO->IADC0OFFSETCAL0 & _DEVINFO_IADC0OFFSETCAL0_OFFSETANABASE_MASK);
  }

  // Compensate offset according to selected reference voltage.
  if (init->configs[config].reference == SL_HAL_IADC_REFERENCE_VREFINT_1V2) {
    // Internal reference voltage (VBGR) depends on the chip revision.
    offset *= 1.25f / ((float)sl_hal_iadc_get_reference_voltage(init->configs[config].reference) / 1000.0f);
  } else {
    offset *= 1.25f / ((float)(init->configs[config].vref) / 1000.0f);
  }

  // Compensate offset for systematic offset.
  offset = (offset * 4.0f) + (640.0f * (256.0f / (float)i_osr));

  // Apply gain error correction.
  if (scale != 0x80000000U) {
    offset = (calc_ana_gain / 32768.0f) * (offset + 524288.0f) - 524288.0f;
  }

  i_offset = IADC_ROUND_D2I(-offset);

  // We only have 18 bits available for OFFSET in SCALE register
  // OFFSET is a 2nd complement number.
  if (i_offset > 131071) {           // Positive overflow at 0x0001FFFF ?
    scale |= 0x1FFFFU;
  } else if (i_offset < -131072) {   // Negative overflow at 0xFFFE0000 ?
    scale |= 0x20000U;
  } else {
    scale |= (uint32_t)i_offset & 0x3FFFFU;
  }

  iadc->CFG[config].SCALE = scale;
}

#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
/***************************************************************************//**
 * Calculate gain and offset for high accuracy mode.
 ******************************************************************************/
static void iadc_calculate_high_accuracy_gain_offset(IADC_TypeDef *iadc,
                                                     const sl_hal_iadc_init_t *init,
                                                     uint8_t config)
{
  float ana_gain;
  int32_t ana_gain_round;
  float offset_ana;
  float offset_2;
  int32_t offset_long;
  int32_t offset_ana1_high_acc_int;
  uint8_t osr_value;
  float offset_ana_base;
  float gain_sys_high_acc;
  float ref_voltage;
  unsigned analog_gain;
  uint16_t calc_ana_gain;

  // Calculate analog gain.
  analog_gain = (iadc->CFG[config].CFG & _IADC_CFG_ANALOGGAIN_MASK) >> _IADC_CFG_ANALOGGAIN_SHIFT;
  calc_ana_gain = iadc_get_analog_gain_cal_value(analog_gain);

  // Over sampling ratio for high accuracy conversions.
  const float osr_high_acc[6] = { 16.0, 32.0, 64.0, 92.0, 128.0, 256.0 };

  // Get reference voltage in volts.
  if (init->configs[config].reference == SL_HAL_IADC_REFERENCE_VREFINT_1V2) {
    // Internal reference voltage (VBGR) depends on the chip revision.
    ref_voltage = (float)sl_hal_iadc_get_reference_voltage(init->configs[config].reference) / 1000.0f;
  } else {
    ref_voltage = (float)init->configs[config].vref / 1000.0f;
  }

  // Get OSR from config register.
  osr_value = (iadc->CFG[config].CFG & _IADC_CFG_OSRHA_MASK) >> _IADC_CFG_OSRHA_SHIFT;

  // 1. Calculate gain correction.
  if ((uint32_t)osr_high_acc[osr_value] == 92U) {
    // for OSR = 92, gain_sys_high_acc = 0.957457.
    gain_sys_high_acc = 0.957457f;
  } else {
    // for OSR != 92, gain_sys_high_acc = OSR/(OSR + 1).
    gain_sys_high_acc = osr_high_acc[osr_value] / (osr_high_acc[osr_value] + 1.0f);
  }

  ana_gain = (float) calc_ana_gain * gain_sys_high_acc;
  ana_gain_round = IADC_ROUND_D2I(ana_gain);
  iadc->CFG[config].SCALE &= ~_IADC_SCALE_MASK;

  // Write GAIN3MSB.
  if ((uint32_t)ana_gain_round & 0x8000) {
    iadc->CFG[config].SCALE |= IADC_SCALE_GAIN3MSB_GAIN100;
  } else {
    iadc->CFG[config].SCALE |= IADC_SCALE_GAIN3MSB_GAIN011;
  }

  // Write GAIN13LSB.
  iadc->CFG[config].SCALE |= ((uint32_t)ana_gain_round & 0x1FFF) << _IADC_SCALE_GAIN13LSB_SHIFT;

  // Get offset value for high accuracy mode from DEVINFO.
  offset_ana1_high_acc_int = (uint16_t)((DEVINFO->IADC0OFFSETCAL0 & _DEVINFO_IADC0OFFSETCAL0_OFFSETANA1HIACC_MASK)
                                        >> _DEVINFO_IADC0OFFSETCAL0_OFFSETANA1HIACC_SHIFT);

  // 2. OSR adjustment.
  // Get offset from DEVINFO.
  offset_ana_base = (int16_t)((DEVINFO->IADC0OFFSETCAL0 & _DEVINFO_IADC0OFFSETCAL0_OFFSETANABASE_MASK)
                              >> _DEVINFO_IADC0OFFSETCAL0_OFFSETANABASE_SHIFT);
  // 1 << osr_value is the same as pow(2, osr_value).
  offset_ana = offset_ana_base + (float)offset_ana1_high_acc_int / (float)(1 << osr_value);

  // 3. Reference voltage adjustment.
  offset_ana = offset_ana * (1.25f / ref_voltage);

  // 4. Calculate final offset.
  offset_2 = 262144.0f / osr_high_acc[osr_value] / (osr_high_acc[osr_value] + 1.0f) + offset_ana * 4.0f + 524288.0f;
  offset_2 = (calc_ana_gain / 32768.0f * (-1.0f)) * offset_2 + 524288.0f;
  offset_long = IADC_ROUND_D2I(offset_2);

  // 5. Write offset to scale register.
  iadc->CFG[config].SCALE |= offset_long & _IADC_SCALE_OFFSET_MASK;
}
#endif

#endif /* defined(IADC_COUNT) && (IADC_COUNT > 0) */
