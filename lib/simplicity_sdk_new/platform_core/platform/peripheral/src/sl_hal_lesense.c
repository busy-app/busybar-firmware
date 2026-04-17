/***************************************************************************//**
 * @file
 * @brief Low Energy Sensor (LESENSE) Peripheral API
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

#include "sl_hal_lesense.h"

#if defined(LESENSE_COUNT) && (LESENSE_COUNT > 0)

#include "sl_hal_bus.h"
#include <stddef.h>

/***************************************************************************//**
 * @addtogroup lesense LESENSE - Low Energy Sensor
 * @brief Low Energy Sensor (LESENSE) Peripheral API
 * @details
 *  This module contains functions to control the LESENSE peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. LESENSE is a low-energy sensor interface capable
 *  of autonomously collecting and processing data from multiple sensors even
 *  when in EM2.
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#if !defined(UINT32_MAX)
#define UINT32_MAX ((uint32_t)(0xFFFFFFFF))
#endif

// LESENSE GPIO route.
#define SLI_HAL_GENERIC_LESENSE_ROUTE    GPIO->LESENSEROUTE.ROUTEEN

// DACOUT mode only available on channel 0, 1, 2.
#define SLI_HAL_LESENSE_DACOUT_SUPPORT  0x7

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
// Period counter top value (max = 255).
#define SLI_HAL_LESENSE_PERIOD_COUNTER_TOP  0xFF
#else
// Period counter top value (max = 63).
#define SLI_HAL_LESENSE_PERIOD_COUNTER_TOP  0x3F
#endif
/** @endcond */

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
extern __INLINE void sl_hal_lesense_enable(void);
extern __INLINE void sl_hal_lesense_wait_sync(void);
extern __INLINE void sl_hal_lesense_wait_ready(void);
extern __INLINE void sl_hal_lesense_disable(void);
extern __INLINE void sl_hal_lesense_decoder_start(void);
extern __INLINE void sl_hal_lesense_decoder_stop(void);
extern __INLINE void sl_hal_lesense_set_scan_mode(sl_hal_lesense_scan_mode_t scan_mode);
extern __INLINE void sl_hal_lesense_start_scan(void);
extern __INLINE void sl_hal_lesense_stop_scan(void);
extern __INLINE void sl_hal_lesense_set_start_delay(uint8_t start_delay);
extern __INLINE void sl_hal_lesense_set_window_size(uint32_t window_size);
extern __INLINE void sl_hal_lesense_set_step_size(uint32_t step_size);
extern __INLINE void sl_hal_lesense_decoder_config_arc(const sl_hal_lesense_transition_arc_t *config_arc,
                                                       uint32_t arc_index);
extern __INLINE void sl_hal_lesense_decoder_enable_prs_output(uint32_t decoder_mask,
                                                              uint32_t decoder_compare);
extern __INLINE void sl_hal_lesense_decoder_disable_prs_output(void);
extern __INLINE void sl_hal_lesense_channel_set_timing(uint8_t channel_index,
                                                       uint8_t excitation_time,
                                                       uint8_t sample_delay,
                                                       uint16_t measure_delay);
extern __INLINE uint32_t sl_hal_lesense_get_status(void);
extern __INLINE void sl_hal_lesense_wait_status(uint32_t flag);
extern __INLINE uint32_t sl_hal_lesense_channel_get_active(void);
extern __INLINE uint32_t sl_hal_lesense_get_scan_result(void);
extern __INLINE uint32_t sl_hal_lesense_get_scan_result_data(void);
extern __INLINE void sl_hal_lesense_clear_result_buffer(void);
extern __INLINE uint32_t sl_hal_lesense_decoder_get_state(void);
extern __INLINE uint32_t sl_hal_lesense_get_sensor_state(void);
extern __INLINE void sl_hal_lesense_reset(void);
extern __INLINE void sl_hal_lesense_clear_interrupts(uint32_t flags);
extern __INLINE void sl_hal_lesense_enable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_lesense_disable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_lesense_set_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_lesense_get_pending_interrupts(void);
extern __INLINE uint32_t sl_hal_lesense_get_enabled_pending_interrupts(void);

/***************************************************************************//**
 * Initialize the LESENSE module.
 ******************************************************************************/
void sl_hal_lesense_init(const sl_hal_lesense_init_t *init)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);
  // Sanity check of initialization values.
  EFM_ASSERT(init != NULL);
  EFM_ASSERT(init->time_control.start_delay < 4U);
  EFM_ASSERT(init->core_control.fifo_trigger_level < 16);

  // Set sensor start delay for each channel.
  sl_hal_lesense_set_start_delay((uint32_t)init->time_control.start_delay);

  // Clear bitfields and configure the AUXHRFCO startup delay.
  LESENSE->TIMCTRL = (LESENSE->TIMCTRL & (~_LESENSE_TIMCTRL_AUXSTARTUP_MASK))
                     | (init->time_control.delay_aux_startup << _LESENSE_TIMCTRL_AUXSTARTUP_SHIFT);

  // Clear LESENSE core control configuration bitfields and configure.
  LESENSE->CFG = (LESENSE->CFG & ~(_LESENSE_CFG_SCANCONF_MASK | _LESENSE_CFG_DMAWU_MASK
                                   | _LESENSE_CFG_RESFIDL_MASK | _LESENSE_CFG_DUALSAMPLE_MASK
                                   | _LESENSE_CFG_STRSCANRES_MASK | _LESENSE_CFG_DEBUGRUN_MASK))
                 | ((uint32_t)init->core_control.scan_config_select << _LESENSE_CFG_SCANCONF_SHIFT)
                 | ((uint32_t)init->core_control.wakeup_on_dma << _LESENSE_CFG_DMAWU_SHIFT)
                 | ((uint32_t)init->core_control.fifo_trigger_level << _LESENSE_CFG_RESFIDL_SHIFT)
                 | ((uint32_t)init->core_control.dual_sample << _LESENSE_CFG_DUALSAMPLE_SHIFT)
                 | ((uint32_t)init->core_control.store_scanres << _LESENSE_CFG_STRSCANRES_SHIFT)
                 | ((uint32_t)init->core_control.debug_run << _LESENSE_CFG_DEBUGRUN_SHIFT);

  // Set PRS input.
  PRS->CONSUMER_LESENSE_START = (PRS->CONSUMER_LESENSE_START & ~(_PRS_CONSUMER_LESENSE_START_MASK))
                                | init->core_control.prs_source;

  // Set scan mode in the CTRL/CFG register using the provided function. Don't
  // start scanning immediately.
  sl_hal_lesense_set_scan_mode((sl_hal_lesense_scan_mode_t)init->core_control.scan_start);

  // The LESENSE peripheral control configuration.
  LESENSE->PERCTRL = (LESENSE->PERCTRL & ~(_LESENSE_PERCTRL_DACCH0DATA_MASK | _LESENSE_PERCTRL_DACSTARTUP_MASK
                                           | _LESENSE_PERCTRL_DACCONVTRIG_MASK | _LESENSE_PERCTRL_ACMP0MODE_MASK
                                           | _LESENSE_PERCTRL_ACMP1MODE_MASK | _LESENSE_PERCTRL_ACMP0INV_MASK
                                           | _LESENSE_PERCTRL_ACMP1INV_MASK))
                     | ((uint32_t)init->per_control.dac_channel0_data << _LESENSE_PERCTRL_DACCH0DATA_SHIFT)
                     | ((uint32_t)init->per_control.dac_startup_half << _LESENSE_PERCTRL_DACSTARTUP_SHIFT)
                     | ((uint32_t)init->per_control.dac_scan << _LESENSE_PERCTRL_DACCONVTRIG_SHIFT)
                     | ((uint32_t)init->per_control.acmp0_mode << _LESENSE_PERCTRL_ACMP0MODE_SHIFT)
                     | ((uint32_t)init->per_control.acmp1_mode << _LESENSE_PERCTRL_ACMP1MODE_SHIFT)
                     | ((uint32_t)init->core_control.invert_acmp0 << _LESENSE_PERCTRL_ACMP0INV_SHIFT)
                     | ((uint32_t)init->core_control.invert_acmp1 << _LESENSE_PERCTRL_ACMP1INV_SHIFT);

  // The LESENSE decoder general control configuration.
  LESENSE->DECCTRL = (LESENSE->DECCTRL & ~(_LESENSE_DECCTRL_INTMAP_MASK | _LESENSE_DECCTRL_HYSTPRS0_MASK
                                           | _LESENSE_DECCTRL_HYSTPRS1_MASK | _LESENSE_DECCTRL_HYSTPRS2_MASK
                                           | _LESENSE_DECCTRL_HYSTIRQ_MASK | _LESENSE_DECCTRL_PRSCNT_MASK))
                     | ((uint32_t)init->decoder_control.interrupt_map << _LESENSE_DECCTRL_INTMAP_SHIFT)
                     | ((uint32_t)init->decoder_control.hysteresis_prs0 << _LESENSE_DECCTRL_HYSTPRS0_SHIFT)
                     | ((uint32_t)init->decoder_control.hysteresis_prs1 << _LESENSE_DECCTRL_HYSTPRS1_SHIFT)
                     | ((uint32_t)init->decoder_control.hysteresis_prs2 << _LESENSE_DECCTRL_HYSTPRS2_SHIFT)
                     | ((uint32_t)init->decoder_control.hysteresis_irq << _LESENSE_DECCTRL_HYSTIRQ_SHIFT)
                     | ((uint32_t)init->decoder_control.prs_count << _LESENSE_DECCTRL_PRSCNT_SHIFT);
}

/***************************************************************************//**
 * Set the scan frequency for periodic scanning.
 ******************************************************************************/
uint32_t sl_hal_lesense_set_scan_frequency(uint32_t reference_frequency,
                                           uint32_t scan_frequency)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);
  // The maximum value of period_counter_prescaler is 128. As a result, using the reference frequency less than
  // 33554431 Hz (33.554431 MHz), the frequency calculation in the while loop
  // below will not overflow.
  EFM_ASSERT(reference_frequency < ((uint32_t)UINT32_MAX / 128UL));

  // A sanity check of scan frequency value.
  EFM_ASSERT((scan_frequency > 0U) && (scan_frequency <= reference_frequency));

  uint32_t period_counter_prescaler = 0UL;
  uint32_t clock_divisor  = 1UL;
  uint32_t calculate_scan_frequency;
  uint32_t period_counter_top = SLI_HAL_LESENSE_PERIOD_COUNTER_TOP;

  // Calculate the minimum necessary prescaler value to provide the
  // biggest possible resolution for setting the scan frequency.
  // The maximum number of calculation cycles is 7 (value of SL_HAL_LESENSE_CLOCK_DIV128).
  while ((reference_frequency / ((uint32_t)scan_frequency * clock_divisor) > (period_counter_top + 1UL))
         && (period_counter_prescaler < SL_HAL_LESENSE_CLOCK_DIV128)) {
    ++period_counter_prescaler;
    clock_divisor = (uint32_t)1UL << period_counter_prescaler;
  }

  // Calculate the period_counter_top value.
  period_counter_top = ((uint32_t)reference_frequency / ((uint32_t)scan_frequency * clock_divisor)) - 1UL;
  EFM_ASSERT(period_counter_top <= (_LESENSE_TIMCTRL_PCTOP_MASK >> _LESENSE_TIMCTRL_PCTOP_SHIFT));

  // Clear current PCPRESC and PCTOP settings and set values in the LESENSE_TIMCTRL register.
  LESENSE->TIMCTRL = (LESENSE->TIMCTRL & ~(_LESENSE_TIMCTRL_PCPRESC_MASK | _LESENSE_TIMCTRL_PCTOP_MASK))
                     | ((uint32_t)period_counter_prescaler << _LESENSE_TIMCTRL_PCPRESC_SHIFT)
                     | ((uint32_t)period_counter_top << _LESENSE_TIMCTRL_PCTOP_SHIFT);

  // For testing the calculation algorithm.
  calculate_scan_frequency = ((uint32_t)reference_frequency / ((uint32_t)(1UL + period_counter_top) * clock_divisor));

  return calculate_scan_frequency;
}

/***************************************************************************//**
 * Set the clock division for LESENSE timers.
 ******************************************************************************/
void sl_hal_lesense_set_clock_division(sl_hal_lesense_excitation_clock_t clock,
                                       sl_hal_lesense_clock_prescale_t clock_divisor)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);

  // Select the clock to prescale.
  switch (clock) {
    case SL_HAL_LESENSE_EXCITATION_TIMING_AUXHFRCO:
      // A sanity check of the clock divisor for the HF clock.
      EFM_ASSERT((uint32_t)clock_divisor <= SL_HAL_LESENSE_CLOCK_DIV8);
      // Clear current AUXPRESC settings and set new values in LESENSE_TIMCTRL register.
      LESENSE->TIMCTRL = (LESENSE->TIMCTRL & ~(_LESENSE_TIMCTRL_AUXPRESC_MASK))
                         | (uint32_t)clock_divisor << _LESENSE_TIMCTRL_AUXPRESC_SHIFT;
      break;

    case SL_HAL_LESENSE_EXCITATION_TIMING_LFACLK:
      // Clear current LFPRESC settings and set new values in the LESENSE_TIMCTRL register.
      LESENSE->TIMCTRL = (LESENSE->TIMCTRL & ~(_LESENSE_TIMCTRL_LFPRESC_MASK))
                         | (uint32_t)clock_divisor << _LESENSE_TIMCTRL_LFPRESC_SHIFT;
      break;

    default:
      EFM_ASSERT(0);
      break;
  }
}

/***************************************************************************//**
 * Configure all (16) LESENSE sensor channels.
 ******************************************************************************/
void sl_hal_lesense_channel_config_all(const sl_hal_lesense_channel_all_t *config_all_channel)
{
  EFM_ASSERT(config_all_channel != NULL);

  // Iterate through all 16 channels.
  for (uint8_t i = 0U; i < SL_HAL_LESENSE_NUM_CHANNELS; ++i) {
    // Configure scan channels.
    sl_hal_lesense_channel_config(&config_all_channel->channel[i], i);
  }
}

/***************************************************************************//**
 * Configure a single LESENSE sensor channel.
 ******************************************************************************/
void sl_hal_lesense_channel_config(const sl_hal_lesense_channel_descriptor_t *config_channel,
                                   uint32_t channel_index)
{
  // A sanity check of configuration parameters.
  EFM_ASSERT(config_channel != NULL);
  EFM_ASSERT(channel_index < SL_HAL_LESENSE_NUM_CHANNELS);
  EFM_ASSERT(config_channel->excitation_time <= (_LESENSE_CH_TIMING_EXTIME_MASK >> _LESENSE_CH_TIMING_EXTIME_SHIFT));
  EFM_ASSERT(config_channel->measure_delay   <= (_LESENSE_CH_TIMING_MEASUREDLY_MASK >> _LESENSE_CH_TIMING_MEASUREDLY_SHIFT));
  EFM_ASSERT(config_channel->offset          <= (_LESENSE_CH_INTERACT_OFFSET_MASK >> _LESENSE_CH_INTERACT_OFFSET_SHIFT));
  // Not a complete assert, as the maximum value of acmp_threshold depends on other
  // configuration parameters. Check the parameter description of acmp_threshold
  // for more details.
  EFM_ASSERT(config_channel->acmp_threshold < (_LESENSE_CH_INTERACT_THRES_MASK + 1U));
  if (config_channel->excitation_mode == SL_HAL_LESENSE_CHANNEL_PIN_EXCITATION_DAC_OUTPUT) {
    EFM_ASSERT((0x1 << channel_index) & SLI_HAL_LESENSE_DACOUT_SUPPORT);
  }

  // Wait for synchronization before writing to EN register.
  sl_hal_lesense_wait_sync();
  // Disable LESENSE module.
  sl_hal_lesense_disable();
  sl_hal_lesense_wait_ready();

  // Configure the channel_index setup in LESENSE idle phase.
  LESENSE->IDLECONF  = (LESENSE->IDLECONF & ~((uint32_t)0x3UL << (channel_index * 2UL)))
                       | ((uint32_t)config_channel->idle_mode << (channel_index * 2UL));

  // A channel-specific timing configuration on scan channel channel_index.
  // Set excitation time, sampling delay, and measurement delay.
  sl_hal_lesense_channel_set_timing(channel_index,
                                    config_channel->excitation_time,
                                    config_channel->sample_delay,
                                    config_channel->measure_delay);

  // A channel-specific configuration of clocks, sample mode, excitation pin mode
  // alternate excitation usage, and interrupt mode on scan channel channel_index in
  // LESENSE_CHx_INTERACT.
  LESENSE->CH[channel_index].INTERACT = (LESENSE->CH[channel_index].INTERACT & ~(_LESENSE_CH_INTERACT_EXCLK_MASK
                                                                                 | _LESENSE_CH_INTERACT_SAMPLECLK_MASK
                                                                                 | _LESENSE_CH_INTERACT_SAMPLE_MASK
                                                                                 | _LESENSE_CH_INTERACT_SETIF_MASK
                                                                                 | _LESENSE_CH_INTERACT_EXMODE_MASK
                                                                                 | _LESENSE_CH_INTERACT_ALTEX_MASK
                                                                                 | _LESENSE_CH_INTERACT_OFFSET_MASK))
                                        | ((uint32_t)config_channel->excitation_clock << _LESENSE_CH_INTERACT_EXCLK_SHIFT)
                                        | ((uint32_t)config_channel->sample_clock << _LESENSE_CH_INTERACT_SAMPLECLK_SHIFT)
                                        | ((uint32_t)config_channel->sample_mode << _LESENSE_CH_INTERACT_SAMPLE_SHIFT)
                                        | ((uint32_t)config_channel->interrupt_mode << _LESENSE_CH_INTERACT_SETIF_SHIFT)
                                        | ((uint32_t)config_channel->excitation_mode << _LESENSE_CH_INTERACT_EXMODE_SHIFT)
                                        | ((uint32_t)config_channel->pin_alternate_excite << _LESENSE_CH_INTERACT_ALTEX_SHIFT)
                                        | ((uint32_t)config_channel->offset << _LESENSE_CH_INTERACT_OFFSET_SHIFT);

  // Configure the channel-specific counter comparison mode, optional result
  // forwarding to decoder, optional counter value storing, and optional result
  // inverting on scan channel channel_index in LESENSE_CHx_EVALCFG.
  LESENSE->CH[channel_index].EVALCFG = (LESENSE->CH[channel_index].EVALCFG & ~(_LESENSE_CH_EVALCFG_COMP_MASK
                                                                               | _LESENSE_CH_EVALCFG_DECODE_MASK
                                                                               | _LESENSE_CH_EVALCFG_STRSAMPLE_MASK
                                                                               | _LESENSE_CH_EVALCFG_SCANRESINV_MASK
                                                                               | _LESENSE_CH_EVALCFG_MODE_MASK))
                                       | ((uint32_t)config_channel->compare_mode << _LESENSE_CH_EVALCFG_COMP_SHIFT)
                                       | ((uint32_t)config_channel->shift_register << _LESENSE_CH_EVALCFG_DECODE_SHIFT)
                                       | ((uint32_t)config_channel->store_counter << _LESENSE_CH_EVALCFG_STRSAMPLE_SHIFT)
                                       | ((uint32_t)config_channel->invert_register << _LESENSE_CH_EVALCFG_SCANRESINV_SHIFT)
                                       | ((uint32_t)config_channel->evaluation_mode << _LESENSE_CH_EVALCFG_MODE_SHIFT);

  // Configure the analog comparator (ACMP) threshold and decision threshold for
  // the counter separately with the function provided for that.
  sl_hal_lesense_channel_set_threshold(channel_index,
                                       config_channel->acmp_threshold,
                                       config_channel->count_threshold);

  // Enable/disable interrupts on channel.
  sl_hal_bus_reg_write_bit(&LESENSE->IEN, channel_index, config_channel->enable_interrupt);

  // Enable/disable CHx pin.
  sl_hal_bus_reg_write_bit(&SLI_HAL_GENERIC_LESENSE_ROUTE, channel_index, config_channel->enable_pin);

  // Enable/disable scan channel channel_index.
  sl_hal_bus_reg_write_bit(&LESENSE->CHEN, channel_index, config_channel->enable_scan);
}

/***************************************************************************//**
 * Configure the LESENSE alternate excitation modes.
 ******************************************************************************/
void sl_hal_lesense_config_alternate_excitation(const sl_hal_lesense_channel_alternate_excitation_t *config_alternate_excitation)
{
  EFM_ASSERT(config_alternate_excitation != NULL);

  // Iterate through all 16 alternate excitation channels.
  for (uint8_t i = 0U; i < 16U; ++i) {
    // Enable/disable the alternate excitation channel pin i.
    sl_hal_bus_reg_write_bit(&SLI_HAL_GENERIC_LESENSE_ROUTE, i, config_alternate_excitation->alt_ex[i]);
  }
}

/***************************************************************************//**
 * Enable/disable LESENSE scan channel and the pin assigned to it.
 ******************************************************************************/
void sl_hal_lesense_channel_enable(uint8_t channel_index,
                                   bool enable_scan_channel,
                                   bool enable_pin)
{
  EFM_ASSERT(channel_index < SL_HAL_LESENSE_NUM_CHANNELS);

  // Enable/disable the assigned pin of scan channel channel_index.
  sl_hal_bus_reg_write_bit(&SLI_HAL_GENERIC_LESENSE_ROUTE, channel_index, enable_pin);

  // Enable/disable scan channel channel_index.
  sl_hal_bus_reg_write_bit(&LESENSE->CHEN, channel_index, enable_scan_channel);
}

/***************************************************************************//**
 * Enable/disable LESENSE scan channel and the pin assigned to it.
 ******************************************************************************/
void sl_hal_lesense_channel_enable_mask(uint16_t channel_mask,
                                        uint16_t pin_mask)
{
  // Enable/disable all channels at once according to the mask.
  LESENSE->CHEN = channel_mask;
  // Enable/disable all channel pins at once according to the mask.
  SLI_HAL_GENERIC_LESENSE_ROUTE = pin_mask;
}

/***************************************************************************//**
 * Set LESENSE channel threshold parameters.
 ******************************************************************************/
void sl_hal_lesense_channel_set_threshold(uint8_t channel_index,
                                          uint16_t acmp_threshold,
                                          uint16_t count_threshold)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);
  // A sanity check for acmp_threshold only, count_threshold is a 16 bit value.
  EFM_ASSERT(acmp_threshold < (_LESENSE_CH_INTERACT_THRES_MASK + 1U));
  // A sanity check for the LESENSE channel ID.
  EFM_ASSERT(channel_index < SL_HAL_LESENSE_NUM_CHANNELS);

  // Set the ACMP threshold value to the INTERACT register of the channel channel_index.
  LESENSE->CH[channel_index].INTERACT = (LESENSE->CH[channel_index].INTERACT & ~(_LESENSE_CH_INTERACT_THRES_MASK))
                                        | (uint32_t)acmp_threshold << _LESENSE_CH_INTERACT_THRES_SHIFT;

  sl_hal_lesense_enable();

  LESENSE->CH[channel_index].EVALTHRES = (LESENSE->CH[channel_index].EVALTHRES & ~_LESENSE_CH_EVALTHRES_EVALTHRES_MASK)
                                         | (uint32_t)count_threshold << _LESENSE_CH_EVALTHRES_EVALTHRES_SHIFT;

  sl_hal_lesense_wait_sync();
}

/***************************************************************************//**
 * Configure a Sliding Window evaluation mode for a specific channel.
 ******************************************************************************/
void sl_hal_lesense_channel_config_sliding_window(uint8_t channel_index,
                                                  uint32_t window_size,
                                                  uint32_t init_value)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);
  EFM_ASSERT(channel_index < SL_HAL_LESENSE_NUM_CHANNELS);

  LESENSE_CH_TypeDef *channel = &LESENSE->CH[channel_index];

  sl_hal_lesense_set_window_size(window_size);

  // Set Channel Sliding Window configuration.
  channel->EVALCFG = (channel->EVALCFG & ~_LESENSE_CH_EVALCFG_MODE_MASK)
                     | LESENSE_CH_EVALCFG_MODE_SLIDINGWIN;

  sl_hal_lesense_enable();
  channel->EVALTHRES = (channel->EVALTHRES & ~_LESENSE_CH_EVALTHRES_EVALTHRES_MASK)
                       | (init_value << _LESENSE_CH_EVALTHRES_EVALTHRES_SHIFT);
  sl_hal_lesense_wait_sync();
}

/***************************************************************************//**
 * Configure the step detection evaluation mode for a specific channel.
 ******************************************************************************/
void sl_hal_lesense_channel_config_step_detection(uint8_t channel_index,
                                                  uint32_t step_size,
                                                  uint32_t init_value)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);
  EFM_ASSERT(channel_index < SL_HAL_LESENSE_NUM_CHANNELS);

  LESENSE_CH_TypeDef *channel = &LESENSE->CH[channel_index];

  sl_hal_lesense_set_step_size(step_size);

  channel->EVALCFG = (channel->EVALCFG & ~_LESENSE_CH_EVALCFG_MODE_MASK)
                     | LESENSE_CH_EVALCFG_MODE_STEPDET;

  sl_hal_lesense_enable();
  channel->EVALTHRES = (channel->EVALTHRES & ~_LESENSE_CH_EVALTHRES_EVALTHRES_MASK)
                       | (init_value << _LESENSE_CH_EVALTHRES_EVALTHRES_SHIFT);
  sl_hal_lesense_wait_sync();
}

/***************************************************************************//**
 * Configure all LESENSE decoder states.
 ******************************************************************************/
void sl_hal_lesense_decoder_config_all_arcs(const sl_hal_lesense_all_transition_arcs_t *config_all_arcs)
{
  EFM_ASSERT(config_all_arcs != NULL);

  // Iterate through all 64 transition arcs.
  for (uint8_t i = 0U; i < SL_HAL_LESENSE_NUM_ARCS; ++i) {
    // A configure decoder for arc i.
    sl_hal_lesense_decoder_config_arc(&config_all_arcs->state[i], i);
  }
}

/** @} (end addtogroup lesense) */

#endif /* defined(LESENSE_COUNT) && (LESENSE_COUNT > 0) */
