/***************************************************************************//**
 * @file
 * @brief Low Energy Sensor (LESENSE) peripheral API
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

#ifndef SL_HAL_LESENSE_H
#define SL_HAL_LESENSE_H

#include "em_device.h"
#include "sl_enum.h"
#include "sl_assert.h"
#if defined(LESENSE_COUNT) && (LESENSE_COUNT > 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* *INDENT-OFF* */
/***************************************************************************//**
 * @addtogroup lesense LESENSE -  Low Energy Sensor Interface
 * @brief Low Energy Sensor Interface (LESENSE) Peripheral API
 * @details
 *  This module contains functions to control the LESENSE peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. LESENSE is a low energy sensor interface which utilizes
 *  on-chip peripherals to perform measurement of a configurable set of sensors
 * @{
 ******************************************************************************/
/* *INDENT-ON* */

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Number of decoder states supported by current device.
#define SLI_HAL_LESENSE_NUM_DECODER_STATES   (_LESENSE_DECSTATE_DECSTATE_MASK + 1)

// Enable PRS output.
#define SLI_HAL_LESENSE_PRS_OUTPUT_ENABLE 1
/** @endcond */

/// Number of ARCs supported by current device. Number of ARCS is the number of states times 2.
#define SL_HAL_LESENSE_NUM_ARCS   (SLI_HAL_LESENSE_NUM_DECODER_STATES << 1)

/// Number of LESENSE channels.
#define SL_HAL_LESENSE_NUM_CHANNELS         16

/// Default value for the FIFO trigger level.
#define SL_HAL_LESENSE_DEFAULT_TRIGGER_LEVEL 15

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Clock divisors for controlling the prescaling factor of the period counter.
/// @note These enumeration values are used for different clock division
///       related configuration parameters.
SL_ENUM_GENERIC(sl_hal_lesense_clock_prescale_t, uint8_t)
{
  SL_HAL_LESENSE_CLOCK_DIV1   = 0,   ///< Divide clock by 1.
  SL_HAL_LESENSE_CLOCK_DIV2   = 1,   ///< Divide clock by 2.
  SL_HAL_LESENSE_CLOCK_DIV4   = 2,   ///< Divide clock by 4.
  SL_HAL_LESENSE_CLOCK_DIV8   = 3,   ///< Divide clock by 8.
  SL_HAL_LESENSE_CLOCK_DIV16  = 4,   ///< Divide clock by 16.
  SL_HAL_LESENSE_CLOCK_DIV32  = 5,   ///< Divide clock by 32.
  SL_HAL_LESENSE_CLOCK_DIV64  = 6,   ///< Divide clock by 64.
  SL_HAL_LESENSE_CLOCK_DIV128 = 7,   ///< Divide clock by 128.
};

/// Scan modes.
SL_ENUM(sl_hal_lesense_scan_mode_t)
{
  SL_HAL_LESENSE_SCAN_START_PERIODIC = _LESENSE_CFG_SCANMODE_PERIODIC,   ///< New scan is started each time the period counter overflows.
  SL_HAL_LESENSE_SCAN_START_ONESHOT  = _LESENSE_CFG_SCANMODE_ONESHOT,    ///< Single scan is performed when sl_hal_lesense_start_scan() is called.
  SL_HAL_LESENSE_SCAN_START_PRS      = _LESENSE_CFG_SCANMODE_PRS         ///< New scan is triggered by pulse on PRS channel.
};

/// PRS sources.
/// @note These enumeration values are being used for different PRS related.
///       configuration parameters.
SL_ENUM_GENERIC(sl_hal_lesense_prs_source_t, uint8_t)
{
  SL_HAL_LESENSE_PRS_CHANNEL_0  = 0,    ///< PRS channel 0.
  SL_HAL_LESENSE_PRS_CHANNEL_1  = 1,    ///< PRS channel 1.
  SL_HAL_LESENSE_PRS_CHANNEL_2  = 2,    ///< PRS channel 2.
  SL_HAL_LESENSE_PRS_CHANNEL_3  = 3,    ///< PRS channel 3.
  SL_HAL_LESENSE_PRS_CHANNEL_4  = 4,    ///< PRS channel 4.
  SL_HAL_LESENSE_PRS_CHANNEL_5  = 5,    ///< PRS channel 5.
  SL_HAL_LESENSE_PRS_CHANNEL_6  = 6,    ///< PRS channel 6.
  SL_HAL_LESENSE_PRS_CHANNEL_7  = 7,    ///< PRS channel 7.
  SL_HAL_LESENSE_PRS_CHANNEL_8  = 8,    ///< PRS channel 8.
  SL_HAL_LESENSE_PRS_CHANNEL_9  = 9,    ///< PRS channel 9.
  SL_HAL_LESENSE_PRS_CHANNEL_10 = 10,   ///< PRS channel 10.
  SL_HAL_LESENSE_PRS_CHANNEL_11 = 11,   ///< PRS channel 11.
};

/// Scan configuration.
SL_ENUM(sl_hal_lesense_scan_config_t)
{
  SL_HAL_LESENSE_SCAN_CONFIG_DIRECTLY_MAP    = _LESENSE_CFG_SCANCONF_DIRMAP,   ///< Channel configuration registers (CHx_CONF) used are directly mapped to the channel number.
  SL_HAL_LESENSE_SCAN_CONFIG_INV_MAP         = _LESENSE_CFG_SCANCONF_INVMAP,   ///< Channel configuration registers used are CHx+8_CONF for channels 0-7 and CHx-8_CONF for channels 8-15.
  SL_HAL_LESENSE_SCAN_CONFIG_TOGGLE          = _LESENSE_CFG_SCANCONF_TOGGLE,   ///< Channel configuration registers used toggles between CHX_SCANCONF and CHX+8_SCANCONF when channel x triggers.
  SL_HAL_LESENSE_SCAN_CONFIG_DECODER_DEFINES = _LESENSE_CFG_SCANCONF_DECDEF    ///< Decoder state defines the channel configuration register (CHx_CONF) to be used.
};

/// DAC CHx data control configuration.
/// @note This value could be used for both DAC Ch0 and Ch1.
SL_ENUM(sl_hal_lesense_control_dac_data_t)
{
  SL_HAL_LESENSE_DAC_CH_DATA = _LESENSE_PERCTRL_DACCH0DATA_DACDATA,   ///< DAC channel x data is defined by the DAC_CH0DATA register.
  SL_HAL_LESENSE_DAC_THRES   = _LESENSE_PERCTRL_DACCH0DATA_THRES      ///< DAC channel x data is defined by the THRES in LESENSE_CHx_INTERACT.
};

/// ACMPx control configuration.
/// @note This value could be used for both ACMP0 and ACMP1.
SL_ENUM(sl_hal_lesense_control_acmp_t)
{
  SL_HAL_LESENSE_ACMP_MODE_MUX      = _LESENSE_PERCTRL_ACMP0MODE_MUX,       ///< ACMPx is controlled by LESENSE input mux.
  SL_HAL_LESENSE_ACMP_MODE_MUXTHRES = _LESENSE_PERCTRL_ACMP0MODE_MUXTHRES   ///< ACMPx is controlled by LESENSE input mux and threshold value
};

/// Compare source selection for sensor sampling.
SL_ENUM(sl_hal_lesense_channel_sample_t)
{
  SL_HAL_LESENSE_SAMPLE_MODE_ACMP_COUNT = _LESENSE_CH_INTERACT_SAMPLE_ACMPCOUNT,   ///< ACMP Counter output will be used in comparison.
  SL_HAL_LESENSE_SAMPLE_MODE_ACMP       = _LESENSE_CH_INTERACT_SAMPLE_ACMP,        ///< ACMP output will be used in comparison.
  SL_HAL_LESENSE_SAMPLE_MODE_ADC        = _LESENSE_CH_INTERACT_SAMPLE_ADC,         ///< ADC output will be used in comparison.
  SL_HAL_LESENSE_SAMPLE_MODE_ADC_DIFF   = _LESENSE_CH_INTERACT_SAMPLE_ADCDIFF      ///< Differential ADC output will be used in comparison.
};

/// Interrupt generation setup for CHx interrupt flag.
SL_ENUM(sl_hal_lesense_channel_interrupt_t)
{
  SL_HAL_LESENSE_SET_INTERRUPT_NONE          = _LESENSE_CH_INTERACT_SETIF_NONE,        ///< No interrupt is generated.
  SL_HAL_LESENSE_SET_INTERRUPT_LEVEL         = _LESENSE_CH_INTERACT_SETIF_LEVEL,       ///< Set interrupt flag if the sensor triggers.
  SL_HAL_LESENSE_SET_INTERRUPT_POSITIVE_EDGE = _LESENSE_CH_INTERACT_SETIF_POSEDGE,     ///< Set interrupt flag on positive edge of the sensor state.
  SL_HAL_LESENSE_SET_INTERRUPT_NEGATIVE_EDGE = _LESENSE_CH_INTERACT_SETIF_NEGEDGE,     ///< Set interrupt flag on negative edge of the sensor state.
  SL_HAL_LESENSE_SET_INTERRUPT_BOTH_EDGE     = _LESENSE_CH_INTERACT_SETIF_BOTHEDGES    ///< Set interrupt flag on both edges of the sensor state.
};

/// Channel pin mode for the excitation phase of the scan sequence.
SL_ENUM(sl_hal_lesense_pin_excitation_phase_t)
{
  SL_HAL_LESENSE_CHANNEL_PIN_EXCITATION_DISABLE     = _LESENSE_CH_INTERACT_EXMODE_DISABLE,   ///< Channel pin is disabled.
  SL_HAL_LESENSE_CHANNEL_PIN_EXCITATION_HIGH        = _LESENSE_CH_INTERACT_EXMODE_HIGH,      ///< Channel pin is configured as push-pull, driven HIGH.
  SL_HAL_LESENSE_CHANNEL_PIN_EXCITATION_LOW         = _LESENSE_CH_INTERACT_EXMODE_LOW,       ///< Channel pin is configured as push-pull, driven LOW.
  SL_HAL_LESENSE_CHANNEL_PIN_EXCITATION_DAC_OUTPUT  = _LESENSE_CH_INTERACT_EXMODE_DACOUT     ///< DAC output (only available on channel 0, 1, 2, 3, 12, 13, 14 and 15).
};

/// Channel pin mode for the idle phase of scan sequence.
/// @note This value could be used for all channels.
SL_ENUM(sl_hal_lesense_pin_idle_mode_t)
{
  SL_HAL_LESENSE_CHANNEL_PIN_IDLE_DISABLE = _LESENSE_IDLECONF_CHIDLE0_DISABLE,   ///< Channel pin is disabled in idle phase.
  SL_HAL_LESENSE_CHANNEL_PIN_IDLE_HIGH    = _LESENSE_IDLECONF_CHIDLE0_HIGH,      ///< Channel pin is configured as push-pull, driven HIGH in idle phase.
  SL_HAL_LESENSE_CHANNEL_PIN_IDLE_LOW     = _LESENSE_IDLECONF_CHIDLE0_LOW,       ///< Channel pin is configured as push-pull, driven LOW in idle phase.
  SL_HAL_LESENSE_CHANNEL_PIN_IDLE_DAC     = _LESENSE_IDLECONF_CHIDLE0_DAC        ///< Channel pin is connected to DAC output in idle phase.
};

/// Clock used for excitation and sample delay timing.
SL_ENUM(sl_hal_lesense_excitation_clock_t)
{
  SL_HAL_LESENSE_EXCITATION_TIMING_LFACLK   = _LESENSE_CH_INTERACT_EXCLK_LFACLK,    ///< LFACLK (LF clock) is used.
  SL_HAL_LESENSE_EXCITATION_TIMING_AUXHFRCO = _LESENSE_CH_INTERACT_EXCLK_AUXHFRCO   ///< AUXHFRCO (HF clock) is used.
};

/// Compare modes for counter comparison.
SL_ENUM(sl_hal_lesense_compare_counter_t)
{
  SL_HAL_LESENSE_COMPARE_MODE_LESS             = _LESENSE_CH_EVALCFG_COMP_LESS,   ///< Evaluates to 1 if sensor data is less than counter threshold, or if ACMP output is 0.
  SL_HAL_LESENSE_COMPARE_MODE_GREATER_OR_EQUAL = _LESENSE_CH_EVALCFG_COMP_GE      ///< Evaluates to 1 if sensor data is greater than, or equal to counter threshold, or if the ACMP output is 1.
};

/// Mode of Storing of Sensor Sample in Result Buffer.
SL_ENUM(sl_hal_lesense_store_sample_data_t)
{
  SL_HAL_LESENSE_STORE_SAMPLE_DISABLE     = _LESENSE_CH_EVALCFG_STRSAMPLE_DISABLE,   ///< Nothing will be stored in the result buffer.
  SL_HAL_LESENSE_STORE_SAMPLE_DATA        = _LESENSE_CH_EVALCFG_STRSAMPLE_DATA,      ///< The sensor sample data will be stored in the result buffer.
  SL_HAL_LESENSE_STORE_SAMPLE_DATA_SOURCE = _LESENSE_CH_EVALCFG_STRSAMPLE_DATASRC    ///< The data source (i.e. the channel) will be stored alongside the sensor sample data.
};

/// Sensor evaluation modes.
SL_ENUM(sl_hal_lesense_sensor_evaluation_t)
{
  SL_HAL_LESENSE_EVALUATION_MODE_THRESHOLD      = _LESENSE_CH_EVALCFG_MODE_THRES,        ///< Threshold comparison evaluation mode. In this mode, sensor data
                                                                                         ///< is compared to the configured threshold value. Two possible comparison
                                                                                         ///< operators can be used on sensor data, either >= (GE) or < (LT).
                                                                                         ///< Which operator to use is given using the
                                                                                         ///< @ref sl_hal_lesense_channel_descriptor_t::compare_mode member.
  SL_HAL_LESENSE_EVALUATION_MODE_SLIDING_WINDOW = _LESENSE_CH_EVALCFG_MODE_SLIDINGWIN,   ///< Sliding window evaluation mode. In this mode, sensor data is
                                                                                         ///< evaluated against upper and lower limits of a window range. The
                                                                                         ///< windows range is defined by a base value and a window size.
  SL_HAL_LESENSE_EVALUATION_MODE_STEP_DETECTION = _LESENSE_CH_EVALCFG_MODE_STEPDET       ///< Step detection evaluation mode. In this mode, the sensor data is compared
                                                                                         ///< to the sensor data from the previous measurement. Sensor evaluation
                                                                                         ///< will result in a "1" if the difference between current measurement
                                                                                         ///< and previous one is greater than a configurable "step size". If the
                                                                                         ///< difference is less than the configured step size then sensor
                                                                                         ///< evaluation will result in a "0".
};

/// Transition action modes.
SL_ENUM(sl_hal_lesense_transition_action_t)
{
  SL_HAL_LESENSE_TRANSITION_ACTION_NONE            = _LESENSE_ST_ARC_PRSACT_NONE,         ///< No PRS pulses generated (if PRSCOUNT == 0). Do not count (if PRSCOUNT == 1).
  SL_HAL_LESENSE_TRANSITION_ACTION_PRS0            = _LESENSE_ST_ARC_PRSACT_PRS0,         ///< Generate pulse on LESPRS0 (if PRSCOUNT == 0).
  SL_HAL_LESENSE_TRANSITION_ACTION_PRS1            = _LESENSE_ST_ARC_PRSACT_PRS1,         ///< Generate pulse on LESPRS1 (if PRSCOUNT == 0).
  SL_HAL_LESENSE_TRANSITION_ACTION_PRS01           = _LESENSE_ST_ARC_PRSACT_PRS01,        ///< Generate pulse on LESPRS0 and LESPRS1 (if PRSCOUNT == 0).
  SL_HAL_LESENSE_TRANSITION_ACTION_PRS2            = _LESENSE_ST_ARC_PRSACT_PRS2,         ///< Generate pulse on LESPRS2 (for both PRSCOUNT == 0 and PRSCOUNT == 1).
  SL_HAL_LESENSE_TRANSITION_ACTION_PRS02           = _LESENSE_ST_ARC_PRSACT_PRS02,        ///< Generate pulse on LESPRS0 and LESPRS2 (if PRSCOUNT == 0).
  SL_HAL_LESENSE_TRANSITION_ACTION_PRS12           = _LESENSE_ST_ARC_PRSACT_PRS12,        ///< Generate pulse on LESPRS1 and LESPRS2 (if PRSCOUNT == 0).
  SL_HAL_LESENSE_TRANSITION_ACTION_PRS012          = _LESENSE_ST_ARC_PRSACT_PRS012,       ///< Generate pulse on LESPRS0, LESPRS1, and LESPRS2 (if PRSCOUNT == 0).
  SL_HAL_LESENSE_TRANSITION_ACTION_COUNT_UP        = _LESENSE_ST_ARC_PRSACT_UP,           ///< Count up (if PRSCOUNT == 1).
  SL_HAL_LESENSE_TRANSITION_ACTION_COUNT_DOWN      = _LESENSE_ST_ARC_PRSACT_DOWN,         ///< Count down (if PRSCOUNT == 1).
  SL_HAL_LESENSE_TRANSITION_ACTION_COUNT_UP_PRS2   = _LESENSE_ST_ARC_PRSACT_UPANDPRS2,    ///< Count up and generate pulse on LESPRS2 (if PRSCOUNT == 1).
  SL_HAL_LESENSE_TRANSITION_ACTION_COUNT_DOWN_PRS2 = _LESENSE_ST_ARC_PRSACT_DOWNANDPRS2   ///< Count down and generate pulse on LESPRS2 (if PRSCOUNT == 1).
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// Core control (LESENSE_CTRL/CFG) descriptor structure.
typedef struct {
  sl_hal_lesense_scan_mode_t   scan_start;           ///< Select scan start mode to control how the scan start is being triggered.
  sl_hal_lesense_prs_source_t  prs_source;           ///< Select PRS source for scan start if scan_mode is set to LESENSE PRS pulse.
  sl_hal_lesense_scan_config_t scan_config_select;   ///< Select scan configuration register usage strategy.
  bool                         invert_acmp0;         ///< Set to true to invert ACMP0 output.
  bool                         invert_acmp1;         ///< Set to true to invert ACMP1 output.
  bool                         dual_sample;          ///< Set to true to sample both ACMPs simultaneously.
  bool                         store_scanres;        ///< Set to true in order to store SCANRES in the RAM (accessible via RESDATA) after each scan.
  uint8_t                      fifo_trigger_level;   ///< Select result FIFO interrupt and DMA trigger level.
  bool                         wakeup_on_dma;        ///< Configure trigger condition for the DMA wakeup from EM2.
  bool                         debug_run;            ///< Set to true to keep LESENSE running in the debug mode.
} sl_hal_lesense_core_descriptor_t;

/// LESENSE timing control descriptor structure.
typedef struct {
  uint8_t start_delay;         ///< Set number of LFACLK cycles to delay sensor interaction on each channel. Valid range: 0-3 (2 bit).
  bool    delay_aux_startup;   ///< Set to true to delay startup of AUXHFRCO until the system enters
                               ///< excite phase. This will reduce the time AUXHFRCO is enabled and
                               ///< reduce power usage.
} sl_hal_lesense_timing_descriptor_t;

/// LESENSE peripheral control descriptor structure.
typedef struct {
  sl_hal_lesense_control_dac_data_t dac_channel0_data;   ///< Configure DAC channel 0 data control.
  sl_hal_lesense_control_acmp_t     acmp0_mode;          ///< Configure how LESENSE controls ACMP 0.
  sl_hal_lesense_control_acmp_t     acmp1_mode;          ///< Configure how LESENSE controls ACMP 1.
  bool                              dac_scan;            ///< When set to true the DAC is only enabled once for each scan. When
                                                         ///< set to false the DAC is enabled before every channel measurement.
  bool                              dac_startup_half;    ///< When set to true the DAC is started a half clock cycle before sensor
                                                         ///< interaction starts. When set to false, a full clock cycle is used.
} sl_hal_lesense_peripheral_descriptor_t;

/// LESENSE decoder control descriptor structure.
typedef struct {
  bool   check_state;       ///< Set to enable decoder to check the present state in addition to the states defined in TCONF.
  bool   interrupt_map;     ///< When set, a transition from state x in decoder will set the interrupt flag CHx.
  bool   hysteresis_prs0;   ///< Set to enable hysteresis in decoder for suppressing the changes on PRS channel 0.
  bool   hysteresis_prs1;   ///< Set to enable hysteresis in decoder for suppressing the changes on PRS channel 1.
  bool   hysteresis_prs2;   ///< Set to enable hysteresis in decoder for suppressing the changes on PRS channel 2.
  bool   hysteresis_irq;    ///< Set to enable hysteresis in decoder for suppressing the interrupt requests.
  bool   prs_count;         ///< Set to enable count mode on decoder PRS channels 0 and 1 to produce
                            ///< outputs which can be used by a PCNT to count up or down.
} sl_hal_lesense_decoder_descriptor_t;

/// LESENSE module initialization structure.
typedef struct {
  sl_hal_lesense_core_descriptor_t       core_control;       ///< LESENSE core configuration parameters.
  sl_hal_lesense_timing_descriptor_t     time_control;       ///< LESENSE timing configuration parameters.
  sl_hal_lesense_peripheral_descriptor_t per_control;        ///< LESENSE peripheral configuration parameters.
  sl_hal_lesense_decoder_descriptor_t    decoder_control;    ///< LESENSE decoder configuration parameters.
} sl_hal_lesense_init_t;

/// Channel descriptor structure.
typedef struct {
  bool                                  enable_scan;          ///< Set to enable scan channel CHx.
  bool                                  enable_pin;           ///< Set to enable CHx pin.
  bool                                  enable_interrupt;     ///< Enable/disable channel interrupts after configuring all the sensor channel parameters.
  sl_hal_lesense_pin_excitation_phase_t excitation_mode;      ///< Configure channel pin mode for the excitation phase of the scan sequence.
                                                              ///< Note: OPAOUT is only available on channels 2, 3, 4, and 5.
  sl_hal_lesense_pin_idle_mode_t        idle_mode;            ///< Configure channel pin idle setup in LESENSE idle phase.
  bool                                  pin_alternate_excite; ///< Set to use alternate excite pin for excitation.
  bool                                  shift_register;       ///< Set to enable result from this channel being shifted into the decoder register.
  bool                                  invert_register;      ///< Set to invert result bit stored in the SCANRES register.
  sl_hal_lesense_store_sample_data_t    store_counter;        ///< Set to store counter value in the RAM (accessible via RESDATA) and make
                                                              ///< the comparison result available in the SCANRES register.
  sl_hal_lesense_excitation_clock_t     excitation_clock;     ///< Select clock used for the excitation timing.
  sl_hal_lesense_excitation_clock_t     sample_clock;         ///< Select clock used for the sample delay timing.
  uint8_t                               excitation_time;      ///< Configure the excitation time. Excitation will last excitation_time+1 excitation clock.
                                                              ///< cycles. Valid range: 0-63 (6 bits).
  uint8_t                               sample_delay;         ///< Configure the sample delay. Sampling will occur after sample_delay+1 sample
                                                              ///< clock cycles. Valid range: 0-127 (7 bits) or 0-255 (8 bits) depending on device.
  uint16_t                              measure_delay;        ///< Configure the measure delay. Sensor measuring is delayed for measDelay
                                                              ///< excitation clock cycles. Valid range: 0-127 (7 bits) or 0-1023 (10 bits)
                                                              ///< depending on device.
  uint16_t                              acmp_threshold;       ///< Configure the ACMP threshold or the DAC data.
                                                              ///< If dac_channel0_data is set to SL_HAL_LESENSE_DAC_CH_DATA,
                                                              ///< acmp_threshold defines the 12-bit DAC data in the data register
                                                              ///< of DAC interface (DACn_CH0DATA).
                                                              ///< In this case, the valid range is: 0-4095 (12 bits).
                                                              ///< If dac_channel0_data is set to SL_HAL_LESENSE_DAC_THRES,
                                                              ///< acmp_threshold defines the 6-bit Vdd scaling factor of ACMP negative input
                                                              ///< (VDDLEVEL in ACMP_INPUTSEL register).
                                                              ///< In this case, the valid range is: 0-63 (6 bits).
  sl_hal_lesense_channel_sample_t       sample_mode;          ///< Select if the ACMP output, the ADC output or the counter output should be used in comparison.
  sl_hal_lesense_channel_interrupt_t    interrupt_mode;       ///< Configure the interrupt generation mode for the CHx interrupt flag.
  uint16_t                              count_threshold;      ///< Configure the decision threshold for the sensor data comparison. Valid range: 0-65535 (16 bits).
  sl_hal_lesense_compare_counter_t      compare_mode;         ///< Select the mode for counter comparison.
  sl_hal_lesense_sensor_evaluation_t    evaluation_mode;      ///< Select the sensor evaluation mode.
  uint8_t                               offset;               ///< Offset for IADC/ACMP interaction.
                                                              ///< ACMP: determines which port I/O pins on the external override interface to access.
                                                              ///< IADC: determines which IADC scanner channels is sampled.
} sl_hal_lesense_channel_descriptor_t;

/// Configuration structure for all the scan channels.
typedef struct {
  sl_hal_lesense_channel_descriptor_t channel[SL_HAL_LESENSE_NUM_CHANNELS];   ///< Channel descriptor for all the LESENSE channels.
} sl_hal_lesense_channel_all_t;

/// Configuration structure for the alternate excitation.
typedef struct {
  bool alt_ex[SL_HAL_LESENSE_NUM_CHANNELS];   ///< Configure alternate excitation pins. If set, the corresponding alternate
                                              ///< excitation pin/signal is enabled.
} sl_hal_lesense_channel_alternate_excitation_t;

/// Decoder state condition descriptor structure.
typedef struct {
  uint8_t                            compare_value;   ///< Configure compare value. State transition is triggered when the sensor state
                                                      ///< equals to this value. Valid range: 0-15 (4 bits).
  uint8_t                            compare_mask;    ///< Configure compare mask. Set bit X to exclude sensor X from evaluation.
                                                      ///< Note: decoder can handle sensor inputs from up to 4 sensors; therefore,
                                                      ///< this mask is 4 bit long.
  uint8_t                            current_state;   ///< Configure index of the current state when evaluation is done.
                                                      ///< Valid range: 0-15 (4 bits).
  uint8_t                            next_state;      ///< Configure index of state to be entered if the sensor state equals to
                                                      ///< compare_value. Valid range: 0-15 (4 bits).
  sl_hal_lesense_transition_action_t prs_action;      ///< Configure which PRS action to perform when the sensor state equals to
                                                      ///< compare_value.
  bool                               set_interrupt;   ///< If enabled, interrupt flag is set when sensor state equals to compare_value.
} sl_hal_lesense_transition_arc_t;

/// Configuration structure for decoder.
typedef struct {
  sl_hal_lesense_transition_arc_t state[SL_HAL_LESENSE_NUM_ARCS];   ///< Descriptor of the 64 Arcs on series 2 devices.
} sl_hal_lesense_all_transition_arcs_t;

/// Default configuration for sl_hal_lesense_core_descriptor_t structure.
#define SL_HAL_LESENSE_CORE_DESCRIPTOR_DEFAULT                                                            \
  {                                                                                                       \
    SL_HAL_LESENSE_SCAN_START_PERIODIC,      /* Start new scan each time the period counter overflows. */ \
    SL_HAL_LESENSE_PRS_CHANNEL_0,            /* Default PRS channel is selected. */                       \
    SL_HAL_LESENSE_SCAN_CONFIG_DIRECTLY_MAP, /* Direct mapping SCANCONF register usage strategy. */       \
    false,                                   /* Do not invert ACMP0 output. */                            \
    false,                                   /* Do not invert ACMP1 output. */                            \
    false,                                   /* Disable dual sampling. */                                 \
    true,                                    /* Store scan result after each scan. */                     \
    SL_HAL_LESENSE_DEFAULT_TRIGGER_LEVEL,    /* Default value for the FIFO trigger level */               \
    false,                                   /* Do not wake up on DMA from EM2. */                        \
    true                                     /* Keep LESENSE running in debug mode. */                    \
  }

/// Default configuration for sl_hal_lesense_peripheral_descriptor_t structure.
#define SL_HAL_LESENSE_PERIPHERAL_DESCRIPTOR_DEFAULT                                                      \
  {                                                                                                       \
    SL_HAL_LESENSE_DAC_CH_DATA,        /* DAC channel 0 data is defined by DAC_CH0DATA register. */       \
    SL_HAL_LESENSE_ACMP_MODE_MUXTHRES, /* LESENSE controls input mux and threshold value of ACMP0. */     \
    SL_HAL_LESENSE_ACMP_MODE_MUXTHRES, /* LESENSE controls input mux and threshold value of ACMP1. */     \
    false,                             /* DAC is enabled for before every channel measurement. */         \
    false                              /* DAC is enabled a full clock cycle before sensor interaction. */ \
  }

/// Default configuration for sl_hal_lesense_timing_descriptor_t structure.
#define SL_HAL_LESENSE_TIMING_DESCRIPTOR_DEFAULT   \
  {                                                \
    0U,   /* No sensor interaction delay. */       \
    false /* Do not delay the AUXHFRCO startup. */ \
  }

/// Default configuration for sl_hal_lesense_decoder_descriptor_t structure.
#define SL_HAL_LESENSE_DECODER_DESCRIPTOR_DEFAULT                   \
  {                                                                 \
    false, /* Disable check of current state. */                    \
    true,  /* Enable channel x % 16 interrupt on state x change. */ \
    true,  /* Enable decoder hysteresis on PRS0 output. */          \
    true,  /* Enable decoder hysteresis on PRS1 output. */          \
    true,  /* Enable decoder hysteresis on PRS2 output. */          \
    true,  /* Enable decoder hysteresis on interrupt requests. */   \
    false  /* Disable count mode on decoder PRS channels 0 and 1.*/ \
  }

/// Default configuration for sl_hal_lesense_init_t structure.
#define SL_HAL_LESENSE_INIT_DEFAULT                                                            \
  {                                                                                            \
    SL_HAL_LESENSE_CORE_DESCRIPTOR_DEFAULT,       /* Default core control parameters. */       \
    SL_HAL_LESENSE_TIMING_DESCRIPTOR_DEFAULT,     /* Default time control parameters. */       \
    SL_HAL_LESENSE_PERIPHERAL_DESCRIPTOR_DEFAULT, /* Default peripheral control parameters. */ \
    SL_HAL_LESENSE_DECODER_DESCRIPTOR_DEFAULT     /* Default decoder control parameters. */    \
  }

/// Default configuration for the scan channel.
#define SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT                                                                            \
  {                                                                                                                      \
    false,                                         /* Disable scan channel. */                                           \
    false,                                         /* Disable assigned pin on scan channel. */                           \
    false,                                         /* Disable interrupts on channel. */                                  \
    SL_HAL_LESENSE_CHANNEL_PIN_EXCITATION_DISABLE, /* Channel pin is disabled during excitation period. */               \
    SL_HAL_LESENSE_CHANNEL_PIN_IDLE_DISABLE,       /* Channel pin is disabled during idle period. */                     \
    false,                                         /* Do not use alternate excitation pins for excitation. */            \
    false,                                         /* Disabled to shift results from this channel to decoder register.*/ \
    false,                                         /* Disabled to invert scan result bit. */                             \
    SL_HAL_LESENSE_STORE_SAMPLE_DISABLE,           /* Disabled to store counter value in result buffer. */               \
    SL_HAL_LESENSE_EXCITATION_TIMING_LFACLK,       /* Use LF clock for excitation timing. */                             \
    SL_HAL_LESENSE_EXCITATION_TIMING_LFACLK,       /* Use LF clock for sample timing. */                                 \
    0x00U,                                         /* Excitation time is set to 0(+1) excitation clock cycles. */        \
    0x00U,                                         /* Sample delay is set to 0(+1) sample clock cycles. */               \
    0x00U,                                         /* Measure delay is set to 0 excitation clock cycles.*/               \
    0x00U,                                         /* ACMP threshold has been set to 0. */                               \
    SL_HAL_LESENSE_SAMPLE_MODE_ACMP,               /* ACMP output will be used in comparison. */                         \
    SL_HAL_LESENSE_SET_INTERRUPT_NONE,             /* No interrupt is generated by the channel. */                       \
    0x00U,                                         /* Counter threshold has been set to 0x00. */                         \
    SL_HAL_LESENSE_COMPARE_MODE_LESS,              /* Compare mode has been set to trigger interrupt on "less". */       \
    SL_HAL_LESENSE_EVALUATION_MODE_THRESHOLD,      /* Evaluation mode has been set to trigger interrupt on threshold. */ \
    0x00U                                          /* No offset for IADC or ACMP interaction. */                         \
  }

/// Default configuration for all the sensor channels.
#define SL_HAL_LESENSE_SCAN_CONFIG_DEFAULT                          \
  {                                                                 \
    {                                                               \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 0. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 1. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 2. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 3. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 4. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 5. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 6. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 7. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 8. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 9. */  \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 10. */ \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 11. */ \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 12. */ \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 13. */ \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 14. */ \
      SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT, /* Scan channel 15. */ \
    }                                                               \
  }

/// Default configuration for all the alternate excitation channels.
#define SL_HAL_LESENSE_ALTERNATE_EXCITATION_CONFIG_DEFAULT   \
  {                                                          \
    {                                                        \
      false, /* Alternate excitation channel 0 disabled. */  \
      false, /* Alternate excitation channel 1 disabled. */  \
      false, /* Alternate excitation channel 2 disabled. */  \
      false, /* Alternate excitation channel 3 disabled. */  \
      false, /* Alternate excitation channel 4 disabled. */  \
      false, /* Alternate excitation channel 5 disabled. */  \
      false, /* Alternate excitation channel 6 disabled. */  \
      false, /* Alternate excitation channel 7 disabled. */  \
      false, /* Alternate excitation channel 8 disabled. */  \
      false, /* Alternate excitation channel 9 disabled. */  \
      false, /* Alternate excitation channel 10 disabled. */ \
      false, /* Alternate excitation channel 11 disabled. */ \
      false, /* Alternate excitation channel 12 disabled. */ \
      false, /* Alternate excitation channel 13 disabled. */ \
      false, /* Alternate excitation channel 14 disabled. */ \
      false  /* Alternate excitation channel 15 disabled. */ \
    }                                                        \
  }

/// Default configuration for the decoder state condition.
#define SL_HAL_LESENSE_STATE_CONFIG_DEFAULT                                                \
  {                                                                                        \
    0x0FU,                                 /* Compare value set to 0x0F. */                \
    0x00U,                                 /* All decoder inputs masked. */                \
    0U,                                    /* Current state must be state 0. */            \
    0U,                                    /* Next state is state 0. */                    \
    SL_HAL_LESENSE_TRANSITION_ACTION_NONE, /* No PRS action performed on compare match. */ \
    false                                  /* No interrupt triggered on compare match. */  \
  }

/// Default configuration for all decoder states.
#define SL_HAL_LESENSE_DECODER_CONFIG_DEFAULT                      \
  {                                                                \
    {                                                              \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 0. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 1. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 2. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 3. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 4. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 5. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 6. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 7. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 8. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 9. */  \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 10. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 11. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 12. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 13. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 14. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 15. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 16. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 17. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 18. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 19. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 20. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 21. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 22. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 23. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 24. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 25. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 26. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 27. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 28. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 29. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT, /* Decoder state 30. */ \
      SL_HAL_LESENSE_STATE_CONFIG_DEFAULT  /* Decoder state 31. */ \
    }                                                              \
  }

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize the LESENSE module.
 *
 * @details
 *   This function configures the main parameters of the LESENSE interface.
 *   See the initialization parameter type definition
 *   @ref (sl_hal_lesense_init_t) for more details.
 *
 * @note
 *   Function @ref sl_hal_lesense_init() is designed to initialize LESENSE once
 *   in an operation cycle.
 *   Be aware of the effects of reconfiguration if using this function from
 *   multiple sources in your code. This function has not been designed to be
 *   re-entrant.
 *   Notice that GPIO pins used by the LESENSE module must be properly
 *   configured by the user explicitly for the LESENSE to work as intended.
 *   (When configuring pins, one should remember to consider the sequence of
 *   configuration to avoid unintended pulses/glitches on output pins.)
 *   LESENSE needs to be disabled before configuring any register.
 *
 * @param[in] init
 *   The LESENSE initialization structure.
 ******************************************************************************/
void sl_hal_lesense_init(const sl_hal_lesense_init_t *init);

/***************************************************************************//**
 * @brief
 *   Set the scan frequency for periodic scanning.
 *
 * @details
 *   This function only applies to LESENSE if a period counter is used as a
 *   trigger for scan start. The calculation is based on the following
 *   formula: Fscan = LFACLKles / ((1+PCTOP)*2^PCPRESC)
 *
 * @note
 *   Note that the calculation does not necessarily result in the requested scan
 *   frequency due to integer division. Check the return value for the resulted
 *   scan frequency.
 *   LESENSE needs to be disabled before configuring any register.
 *
 * @param[in] reference_frequency
 *   Select reference LFACLK clock frequency in Hz.
 *
 * @param[in] scan_frequency
 *   Set the desired scan frequency in Hz.
 *
 * @return
 *   Frequency in Hz calculated and set by this function. Users can use this to
 *   compare the requested and set values.
 ******************************************************************************/
uint32_t sl_hal_lesense_set_scan_frequency(uint32_t reference_frequency,
                                           uint32_t scan_frequency);

/***************************************************************************//**
 * @brief
 *   Set the clock division for LESENSE timers.
 *
 * @details
 *   Use this function to configure the clock division for the LESENSE timers used
 *   for excitation timing. The division setting is global but the clock source can
 *   be selected for each channel using @ref sl_hal_lesense_channel_config() function.
 *   See documentation for more details.
 *
 * @note
 *   If AUXHFRCO is used for excitation timing, LFACLK can't exceed 500 kHz.
 *   LFACLK can't exceed 50 kHz if the ACMP threshold level (ACMPTHRES) is not equal
 *   for all channels.
 *   LESENSE needs to be disabled before configuring any register.
 *
 * @param[in] clock
 *   Select the clock to prescale.
 *
 * @param[in] clock_divisor
 *   The clock divisor value. A valid range depends on the @p channel_id value.
 *
 * @li SL_HAL_LESENSE_EXCITATION_TIMING_AUXHFRCO - set AUXHFRCO clock divisor for HF timer.
 * @li SL_HAL_LESENSE_EXCITATION_TIMING_LFACLK - set LFACLKles clock divisor for LF timer.
 ******************************************************************************/

void sl_hal_lesense_set_clock_division(sl_hal_lesense_excitation_clock_t clock,
                                       sl_hal_lesense_clock_prescale_t clock_divisor);

/***************************************************************************//**
 * @brief
 *   Configure all (16) LESENSE sensor channels.
 *
 * @details
 *   This function configures all sensor channels of the LESENSE interface.
 *   See the configuration parameter type definition @ref (sl_hal_lesense_channel_all_t)
 *   for more details.
 *
 * @note
 *   Channels can be configured individually using @ref sl_hal_lesense_channel_config()
 *   function. Notice that pins used by the LESENSE module must be properly
 *   configured by the user explicitly for LESENSE to work as intended.
 *   (When configuring pins, consider the sequence of the configuration to
 *   avoid unintended pulses/glitches on output pins.)
 *
 * @param[in] config_all_channel
 *   A configuration structure for all (16) LESENSE sensor channels.
 ******************************************************************************/
void sl_hal_lesense_channel_config_all(const sl_hal_lesense_channel_all_t *config_all_channel);

/***************************************************************************//**
 * @brief
 *   Configure a single LESENSE sensor channel.
 *
 * @details
 *   This function configures a single sensor channel of the LESENSE interface.
 *   See the configuration parameter type definition @ref (sl_hal_lesense_channel_descriptor_t)
 *   for more details.
 *
 * @note
 *   This function has been designed to minimize the effects of sensor channel
 *   reconfiguration while LESENSE is in operation. However, be aware of these
 *   effects and the right timing to call this function. Parameter @p pin_alternate_excite
 *   must be true in the channel configuration to use alternate excitation pins.
 *   LESENSE needs to be disabled before configuring any register.
 *
 * @param[in] config_channel
 *   A configuration structure for a single LESENSE sensor channel.
 *
 * @param[in] channel_index
 *   A channel index to configure (0-15).
 ******************************************************************************/
void sl_hal_lesense_channel_config(const sl_hal_lesense_channel_descriptor_t *config_channel,
                                   uint32_t channel_index);

/***************************************************************************//**
 * @brief
 *   Enable/disable LESENSE scan channel and the pin assigned to it.
 *
 * @details
 *   Use this function to enable/disable a selected LESENSE scan channel and
 *   the pin assigned to it.
 *
 * @note
 *   Users can enable/disable scan channels and the channel pin with the @ref
 *   sl_hal_lesense_channel_config() function, but only with a significant
 *   overhead. This simple function controls these parameters after the channel
 *   has been configured.
 *
 * @param[in] channel_index
 *   An identifier of the scan channel. A valid range: 0-15.
 *
 * @param[in] enable_scan_channel
 *   Enable/disable the selected scan channel by setting this parameter to
 *   true/false respectively.
 *
 * @param[in] enable_pin
 *   Enable/disable the pin assigned to the channel selected by @p channel_index.
 ******************************************************************************/
void sl_hal_lesense_channel_enable(uint8_t channel_index,
                                   bool enable_scan_channel,
                                   bool enable_pin);

/***************************************************************************//**
 * @brief
 *   Enable/disable LESENSE scan channel and the pin assigned to it.
 *
 * @details
 *   Use this function to enable/disable LESENSE scan channels and the pins
 *   assigned to them using a mask.
 *
 * @note
 *   Users can enable/disable scan channels and channel pins by using the
 *   @ref sl_hal_lesense_channel_config_all() function, but only with a
 *   significant overhead.
 *   This function controls these parameters after the channel has been configured.
 *
 * @param[in] channel_mask
 *   Set the corresponding bit to 1 to enable, 0 to disable the selected scan channel.
 *
 * @param[in] pin_mask
 *   Set the corresponding bit to 1 to enable, 0 to disable the pin on selected channel.
 ******************************************************************************/
void sl_hal_lesense_channel_enable_mask(uint16_t channel_mask,
                                        uint16_t pin_mask);

/***************************************************************************//**
 * @brief
 *   Set LESENSE channel threshold parameters.
 *
 * @details
 *   Use this function to set threshold parameters on a selected LESENSE channel.
 *
 * @note
 *   Users can configure the channel threshold parameters with the
 *   @ref sl_hal_lesense_channel_config() function, but only with a significant
 *   overhead. This function serves controls these parameters after the channel
 *   has been configured.
 *   LESENSE needs to be disabled before configuring INTERACT register and needs to be
 *   enabled when configuring EVALTHRES register.
 *
 * @param[in] channel_index
 *   An identifier of the scan channel. A valid range is 0-15.
 *
 * @param[in] acmp_threshold
 *   ACMP threshold.
 *
 * @li If per_control.dac_channel0_data is set to @ref SL_HAL_LESENSE_DAC_CH_DATA,
 *     acmp_threshold defines the 12-bit DAC data in the corresponding data register
 *     of the DAC interface (DACn_CH0DATA and DACn_CH1DATA).
 *     In this case, the valid range is 0-4095 (12 bits).
 *
 * @param[in] count_threshold
 *   A decision threshold for counter comparison. A valid range is 0-65535 (16 bits).
 ******************************************************************************/
void sl_hal_lesense_channel_set_threshold(uint8_t channel_index,
                                          uint16_t acmp_threshold,
                                          uint16_t count_threshold);

/***************************************************************************//**
 * @brief
 *   Configure a Sliding Window evaluation mode for a specific channel.
 *
 * @details
 *   This function will configure the evaluation mode, the initial sensor
 *   measurement (COMPTHRES), and the window size. For other channel-related
 *   configuration, see the @ref sl_hal_lesense_channel_config() function.
 *
 * @note
 *   LESENSE needs to be disabled before configuring EVALCFG register and needs to be
 *   enabled when configuring EVALTHRES register.
 *
 * @warning
 *   Note that the step size and window size configuration are global to all LESENSE
 *   channels and use the same register field in the hardware.
 *   This means that any window_size configuration passed to this function will
 *   apply for all channels and override all other step_size/window_size configurations.
 *
 * @param[in] channel_index
 *   An identifier of the scan channel. A valid range is 0-15.
 *
 * @param[in] window_size
 *   A window size to be used on all channels.
 *
 * @param[in] init_value
 *   The initial sensor value for the channel.
 ******************************************************************************/
void sl_hal_lesense_channel_config_sliding_window(uint8_t channel_index,
                                                  uint32_t window_size,
                                                  uint32_t init_value);

/***************************************************************************//**
 * @brief
 *   Configure the step detection evaluation mode for a specific channel.
 *
 * @details
 *   This function will configure the evaluation mode, the initial sensor
 *   measurement (COMPTHRES) and the window size. For other channel-related
 *   configuration, see the @ref sl_hal_lesense_channel_config() function.
 *
 * @warning
 *   Note that the step size and window size configuration are global to all
 *   LESENSE channels and use the same register field in the hardware.
 *   This means that any step_size configuration passed to this function will
 *   apply for all channels and override all other step_size/window_size
 *   configurations.
 *   LESENSE needs to be disabled before configuring EVALCFG register and needs to be
 *   enabled when configuring EVALTHRES register.
 *
 * @param[in] channel_index
 *   An identifier of the scan channel. A valid range is 0-15.
 *
 * @param[in] step_size
 *   A step size to be used on all channels.
 *
 * @param[in] init_value
 *   The initial sensor value for the channel.
 ******************************************************************************/
void sl_hal_lesense_channel_config_step_detection(uint8_t channel_index,
                                                  uint32_t step_size,
                                                  uint32_t init_value);

/***************************************************************************//**
 * @brief
 *   Configure the LESENSE alternate excitation modes.
 *
 * @details
 *   This function enable/disable the alternate excitation channels of the
 *   LESENSE interface. See the configuration parameter type definition
 *   @ref (sl_hal_lesense_channel_alternate_excitation_t) for more details.
 *
 * @note
 *   The @p config_alternate_excitation parameter must be true in the channel
 *   configuration structure @ref (sl_hal_lesense_channel_descriptor_t) to use
 *   alternate excitation pins on the channel.
 *
 * @param[in] config_alternate_excitation
 *   A configuration structure for LESENSE alternate excitation pins.
 ******************************************************************************/
void sl_hal_lesense_config_alternate_excitation(const sl_hal_lesense_channel_alternate_excitation_t *config_alternate_excitation);

/***************************************************************************//**
 * @brief
 *   Configure all LESENSE transition ARCs.
 *
 * @details
 *   This function configures all the transition ARC of the LESENSE interface.
 *   See the configuration parameter type definition
 *   @ref (sl_hal_lesense_all_transition_arcs_t) for more details.
 *
 * @param[in] config_all_arcs
 *   A configuration structure for all 64 LESENSE decoder arcs.
 ******************************************************************************/
void sl_hal_lesense_decoder_config_all_arcs(const sl_hal_lesense_all_transition_arcs_t *config_all_arcs);

/***************************************************************************//**
 * @brief
 *   Enable the LESENSE module.
 ******************************************************************************/
__INLINE void sl_hal_lesense_enable(void)
{
  LESENSE->EN_SET = LESENSE_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to complete before writing to any other
 *   register.
 ******************************************************************************/
__INLINE void sl_hal_lesense_wait_sync(void)
{
  while (LESENSE_SYNCBUSY_CMD & LESENSE->SYNCBUSY) {
    // Wait for all synchronizations to finish.
  }
}

/***************************************************************************//**
 * @brief
 *   Wait for disabling to finish.
 ******************************************************************************/
__INLINE void sl_hal_lesense_wait_ready(void)
{
  while (LESENSE->EN & _LESENSE_EN_DISABLING_MASK) {
    // Wait for disabling to finish.
  }
}
/***************************************************************************//**
 * @brief
 *   Disable the LESENSE module.
 ******************************************************************************/
__INLINE void sl_hal_lesense_disable(void)
{
  // Wait for synchronization before writing to EN register.
  sl_hal_lesense_wait_sync();
  LESENSE->EN_CLR = LESENSE_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Start the LESENSE decoder.
 *
 * @note
 *   This function will wait for any pending previous write operation to the CMD
 *   register to complete before accessing the CMD register. It will also wait
 *   for the write operation to the CMD register to complete before returning.
 *   Each write operation to the CMD register may take up to 3 LF clock cycles,
 *   so expect some delay. The user may implement a separate function to write
 *   multiple command bits in the CMD register in one single operation to optimize
 *   an application.
 ******************************************************************************/
__INLINE void sl_hal_lesense_decoder_start(void)
{
  sl_hal_lesense_wait_sync();

  // Start the decoder.
  LESENSE->CMD = LESENSE_CMD_DECODE;
}

/***************************************************************************//**
 * @brief
 *   Stop LESENSE decoder.
 *
 * @details
 *   Disables LESENSE decoder by setting the command to LESENSE_DECCTRL register.
 ******************************************************************************/
__INLINE void sl_hal_lesense_decoder_stop(void)
{
  // Stop decoder.
  LESENSE->DECCTRL_SET = LESENSE_DECCTRL_DECDIS;
}

/***************************************************************************//**
 * @brief
 *   Set scan mode of the LESENSE channels.
 *
 * @details
 *   This function configures how the scan start is triggered. It can be used for
 *   re-configuring the scan mode while running the application but it is also
 *   used by @ref sl_hal_lesense_init() for initialization.
 *
 * @note
 *   Users can configure the scan mode by @ref sl_hal_lesense_init() function, but
 *   only with a significant overhead. This simple function serves the purpose of
 *   controlling this parameter after the channel has been configured.
 *   Be aware of the effects of the non-atomic Read-Modify-Write cycle.
 *   LESENSE needs to be disabled before configuring any register.
 *
 * @param[in] scan_mode
 *   Select the location to map LESENSE alternate excitation channels.
 *
 * @li SL_HAL_LESENSE_SCAN_START_PERIODIC - A new scan is started each time the
 *     period counter overflows.
 * @li SL_HAL_LESENSE_SCAN_START_ONESHOT - A single scan is performed when
 *     @ref sl_hal_lesense_start_scan() function is called.
 * @li SL_HAL_LESENSE_SCAN_START_PRS - A new scan is triggered by pulse on the
 *     PRS channel.
 ******************************************************************************/
__INLINE void sl_hal_lesense_set_scan_mode(sl_hal_lesense_scan_mode_t scan_mode)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);

  // Setting the requested scan_mode to the CTRL register.
  LESENSE->CFG = (LESENSE->CFG & ~(_LESENSE_CFG_SCANMODE_MASK))
                 | (uint32_t)scan_mode;
}
/***************************************************************************//**
 * @brief
 *   Start scanning sensors.
 *
 * @note
 *   This function will wait for any pending previous write operation to the CMD
 *   register to complete before accessing the CMD register. It will also wait for
 *   the write operation to the CMD register to complete before returning.
 *   Each write operation to the CMD register may take up to 3 LF clock cycles,
 *   so expect some delay. The user may implement a separate function to write
 *   multiple command bits in the CMD register in one single operation to optimize
 *   an application.
 ******************************************************************************/
__INLINE void sl_hal_lesense_start_scan(void)
{
  sl_hal_lesense_wait_sync();

  // Start scanning sensors.
  LESENSE->CMD = LESENSE_CMD_START;
}

/***************************************************************************//**
 * @brief
 *   Stop scanning sensors.
 *
 * @note
 *   This function will wait for any pending previous write operation to the CMD
 *   register to complete before accessing the CMD register. It will also wait
 *   for the write operation to the CMD register to complete before returning.
 *   Each write operation to the CMD register may take up to 3 LF clock cycles,
 *   so the user should expect some delay. The user may implement a separate
 *   function to write multiple command bits in the CMD register in one single
 *   operation in order to optimize an application.
 *   If issued during a scan, the command takes effect after scan completion.
 ******************************************************************************/
__INLINE void sl_hal_lesense_stop_scan(void)
{
  sl_hal_lesense_wait_sync();

  // Stop scanning sensors.
  LESENSE->CMD = LESENSE_CMD_STOP;
}

/***************************************************************************//**
 * @brief
 *   Set the start delay of the sensor interaction on each channel.
 *
 * @details
 *   This function sets the start delay of the sensor interaction on each channel.
 *   It can be used for adjusting the start delay while running the application
 *   but it is also used by @ref sl_hal_lesense_init() for initialization.
 *
 * @note
 *   Users can configure the start delay by @ref sl_hal_lesense_init() function,
 *   but only with a significant overhead.
 *   This simple function serves the purpose of controlling this parameter after
 *   the channel has been configured.
 *   Be aware of the effects of the non-atomic Read-Modify-Write cycle.
 *   LESENSE needs to be disabled before configuring any register.
 *
 * @param[in] start_delay
 *   A number of LFACLK cycles to delay. A valid range: 0-3 (2 bit).
 ******************************************************************************/
__INLINE void sl_hal_lesense_set_start_delay(uint8_t start_delay)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);
  // Sanity check of the start_delay.
  EFM_ASSERT(start_delay < 4U);

  // Setting the requested start_delay to the TIMCTRL register.
  LESENSE->TIMCTRL = (LESENSE->TIMCTRL & ~(_LESENSE_TIMCTRL_STARTDLY_MASK))
                     | (uint32_t)start_delay << _LESENSE_TIMCTRL_STARTDLY_SHIFT;
}

/***************************************************************************//**
 * @brief
 *   Set the window size for all LESENSE channels.
 *
 * @details
 *   The window size is used by all channels that are configured as
 *   @ref SL_HAL_LESENSE_EVALUATION_MODE_SLIDING_WINDOW.
 *
 * @note
 *   LESENSE needs to be disabled before configuring EVALCTRL register.
 *
 * @warning
 *   The window size configuration is using the same register field as the step
 *   detection size. As a result, the window size configuration will have an effect
 *   on channels configured with the @ref SL_HAL_LESENSE_EVALUATION_MODE_STEP_DETECTION
 *   evaluation mode as well.
 *
 * @param[in] window_size
 *   The window size to use for all channels.
 ******************************************************************************/
__INLINE void sl_hal_lesense_set_window_size(uint32_t window_size)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);

  LESENSE->EVALCTRL = (LESENSE->EVALCTRL & ~_LESENSE_EVALCTRL_WINSIZE_MASK)
                      | window_size;
}

/***************************************************************************//**
 * @brief
 *   Set the step size for all LESENSE channels.
 *
 * @details
 *   The step size is configured using the same register field as used to configure
 *   window size. Therefore, calling this function will overwrite any previously
 *   configured window size as done by the
 *   @ref sl_hal_lesense_set_window_size() function.
 *
 * @param[in] step_size
 *   The step size to use for all channels.
 ******************************************************************************/
__INLINE void sl_hal_lesense_set_step_size(uint32_t step_size)
{
  sl_hal_lesense_set_window_size(step_size);
}

/***************************************************************************//**
 * @brief
 *   Configure a single LESENSE transition ARC.
 *
 * @details
 *   This function configures a transition ARC of the LESENSE interface.
 *   See the configuration parameter type definition
 *   @ref sl_hal_lesense_transition_arc_t for more details.
 *
 * @note
 *   LESENSE needs to be disabled before configuring STx_ARC register.
 *
 * @param[in] config_arc
 *   A configuration structure for a single LESENSE transition ARC.
 *
 * @param[in] arc_index
 *   An ARC index to configure.
 ******************************************************************************/
__INLINE void sl_hal_lesense_decoder_config_arc(const sl_hal_lesense_transition_arc_t *config_arc,
                                                uint32_t arc_index)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);
  // Sanity check of configuration parameters.
  EFM_ASSERT(config_arc != NULL);
  EFM_ASSERT(arc_index < SL_HAL_LESENSE_NUM_ARCS);
  EFM_ASSERT((uint32_t)config_arc->compare_mask < 16U);
  EFM_ASSERT((uint32_t)config_arc->compare_value < 16U);
  EFM_ASSERT((uint32_t)config_arc->current_state < SLI_HAL_LESENSE_NUM_DECODER_STATES);
  EFM_ASSERT((uint32_t)config_arc->next_state < SLI_HAL_LESENSE_NUM_DECODER_STATES);

  LESENSE->ST[arc_index].ARC = (LESENSE->ST[arc_index].ARC & ~(_LESENSE_ST_ARC_PRSACT_MASK
                                                               | _LESENSE_ST_ARC_SMASK_MASK
                                                               | _LESENSE_ST_ARC_SCOMP_MASK
                                                               | _LESENSE_ST_ARC_CURSTATE_MASK
                                                               | _LESENSE_ST_ARC_NEXTSTATE_MASK
                                                               | _LESENSE_ST_ARC_SETIF_MASK))
                               | ((uint32_t)config_arc->prs_action << _LESENSE_ST_ARC_PRSACT_SHIFT)
                               | ((uint32_t)config_arc->compare_mask << _LESENSE_ST_ARC_SMASK_SHIFT)
                               | ((uint32_t)config_arc->compare_value << _LESENSE_ST_ARC_SCOMP_SHIFT)
                               | ((uint32_t)config_arc->current_state << _LESENSE_ST_ARC_CURSTATE_SHIFT)
                               | ((uint32_t)config_arc->next_state << _LESENSE_ST_ARC_NEXTSTATE_SHIFT)
                               | ((uint32_t)config_arc->set_interrupt << _LESENSE_ST_ARC_SETIF_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Enable the PRS output from the LESENSE decoder.
 *
 * @param[in] decoder_mask
 *   A decoder state compare value mask.
 *
 * @param[in] decoder_compare
 *   A decoder state comparison value.
 ******************************************************************************/
__INLINE void sl_hal_lesense_decoder_enable_prs_output(uint32_t decoder_mask,
                                                       uint32_t decoder_compare)
{
  LESENSE->PRSCTRL = (LESENSE->PRSCTRL & ~(_LESENSE_PRSCTRL_DECCMPEN_MASK
                                           | _LESENSE_PRSCTRL_DECCMPMASK_MASK
                                           | _LESENSE_PRSCTRL_DECCMPVAL_MASK))
                     | (SLI_HAL_LESENSE_PRS_OUTPUT_ENABLE << _LESENSE_PRSCTRL_DECCMPEN_SHIFT)
                     | (decoder_mask << _LESENSE_PRSCTRL_DECCMPMASK_SHIFT)
                     | (decoder_compare << _LESENSE_PRSCTRL_DECCMPVAL_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Disable the PRS output from the LESENSE decoder.
 ******************************************************************************/
__INLINE void sl_hal_lesense_decoder_disable_prs_output(void)
{
  LESENSE->PRSCTRL = (LESENSE->PRSCTRL & ~(_LESENSE_PRSCTRL_DECCMPEN_MASK
                                           | _LESENSE_PRSCTRL_DECCMPMASK_MASK
                                           | _LESENSE_PRSCTRL_DECCMPVAL_MASK));
}

/***************************************************************************//**
 * @brief
 *   Get the current status of LESENSE.
 *
 * @return
 *   Returns the value of the LESENSE_STATUS register that contains the OR
 *   combination of the following status bits for EFR series 2:
 * @li LESENSE_STATUS_RESFIFOV - Result Fifo valid. Set when data is available
 *     in result Fifo. Cleared when Fifo is empty.
 * @li LESENSE_STATUS_RESFIFOFULL - Result Fifo full. Set when result Fifo is full.
 * @li LESENSE_STATUS_RUNNING - LESENSE is active.
 * @li LESENSE_STATUS_SCANACTIVE - LESENSE is currently interfacing sensors.
 * @li LESENSE_STATUS_FLUSHING - Fifo flushing
 * @li LESENSE_STATUS_READBUSY - Fifo Read busy
 ******************************************************************************/
__INLINE uint32_t sl_hal_lesense_get_status(void)
{
  return LESENSE->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Set LESENSE channel timing parameters.
 *
 * @details
 *   Use this function to set timing parameters on a selected LESENSE channel.
 *
 * @note
 *   Users can configure the channel timing parameters with the
 *   @ref sl_hal_lesense_channel_config() function, but only with a significant
 *   overhead. This function controls these parameters after the channel has been
 *   configured.
 *   LESENSE needs to be disabled before configuring any register.
 *
 * @param[in] channel_index
 *   An identifier of the scan channel. A valid range is 0-15.
 *
 * @param[in] excitation_time
 *   An excitation time on channel_index. The excitation will last excitation_time+1
 *   excitation clock cycles. A valid range is 0-63 (6 bits).
 *
 * @param[in] sample_delay
 *   Sample delay on channel_index. Sampling will occur after sample_delay+1 sample
 *   clock cycles. A valid range is 0-127 (7 bits).
 *
 * @param[in] measure_delay
 *   A measure delay on channel_index. Sensor measuring is delayed for measure_delay+1
 *   excitation clock cycles. A valid range is 0-127 (7 bits).
 ******************************************************************************/
__INLINE void sl_hal_lesense_channel_set_timing(uint8_t channel_index,
                                                uint8_t excitation_time,
                                                uint8_t sample_delay,
                                                uint16_t measure_delay)
{
  // Make sure the module is disabled.
  EFM_ASSERT(LESENSE->EN == 0);
  // A sanity check of parameters.
  EFM_ASSERT(excitation_time <= (_LESENSE_CH_TIMING_EXTIME_MASK >> _LESENSE_CH_TIMING_EXTIME_SHIFT));
  EFM_ASSERT(measure_delay <= (_LESENSE_CH_TIMING_MEASUREDLY_MASK >> _LESENSE_CH_TIMING_MEASUREDLY_SHIFT));
  EFM_ASSERT(channel_index < SL_HAL_LESENSE_NUM_CHANNELS);

  // A channel-specific timing configuration on the scan channel channel_index.
  // Setting excitation time, sampling delay, and measurement delay.
  LESENSE->CH[channel_index].TIMING = (LESENSE->CH[channel_index].TIMING & ~(_LESENSE_CH_TIMING_EXTIME_MASK
                                                                             | _LESENSE_CH_TIMING_SAMPLEDLY_MASK
                                                                             | _LESENSE_CH_TIMING_MEASUREDLY_MASK))
                                      | ((uint32_t)excitation_time << _LESENSE_CH_TIMING_EXTIME_SHIFT)
                                      | ((uint32_t)sample_delay << _LESENSE_CH_TIMING_SAMPLEDLY_SHIFT)
                                      | ((uint32_t)measure_delay << _LESENSE_CH_TIMING_MEASUREDLY_SHIFT);
}
/***************************************************************************//**
 * @brief
 *   Wait until status of LESENSE is equal to what was requested.
 *
 * @details
 *   Polls LESENSE_STATUS register and waits until requested combination of flags
 *   are set.
 *
 * @param[in] flag
 *   The OR combination of the following status bits for EFR series 2:
 * @li LESENSE_STATUS_RESFIFOV - Result FIFO valid. Set when data is available
 *     in result FIFO. Cleared when FIFO is empty.
 * @li LESENSE_STATUS_RESFIFOFULL - Result FIFO full. Set when result FIFO is full.
 * @li LESENSE_STATUS_RUNNING - LESENSE is active.
 * @li LESENSE_STATUS_SCANACTIVE - LESENSE is currently interfacing sensors.
 * @li LESENSE_STATUS_FLUSHING - FIFO flushing
 * @li LESENSE_STATUS_READBUSY - FIFO Read busy
 ******************************************************************************/
__INLINE void sl_hal_lesense_wait_status(uint32_t flag)
{
  while (!(LESENSE->STATUS & flag)) {
    // Wait until requested flag is set.
  }
}

/***************************************************************************//**
 * @brief
 *   Get the currently active channel index.
 *
 * @return
 *   Returns the value of the LESENSE_CHINDEX register that contains the index
 *   of currently active channel (0-15).
 ******************************************************************************/
__INLINE uint32_t sl_hal_lesense_channel_get_active(void)
{
  return LESENSE->CURCH;
}

/***************************************************************************//**
 * @brief
 *   Get the latest scan comparison result (1 bit / channel).
 *
 * @return
 *   Returns the value of the LESENSE_SCANRES register that contains the comparison
 *   result of last scan on all channels.
 *   Bit x is set if a comparison triggered on channel x, which means that
 *   LESENSE counter met the comparison criteria set in LESENSE_CHx_EVAL by
 *   COMPMODE and CNTTHRES.
 ******************************************************************************/
__INLINE uint32_t sl_hal_lesense_get_scan_result(void)
{
  return LESENSE->SCANRES & _LESENSE_SCANRES_SCANRES_MASK;
}

/***************************************************************************//**
 * @brief
 *   Get the oldest unread data from the result buffer.
 *
 * @note
 *   Make sure that the STORERES bit is set in LESENSE_CHx_EVAL, or the STRSCANRES
 *   bit is set in LESENSE_CTRL; otherwise, returns the undefined value.
 *
 * @return
 *   Returns the value of LESENSE_RESDATA register that contains the oldest unread
 *   counter result from result buffer.
 ******************************************************************************/
__INLINE uint32_t sl_hal_lesense_get_scan_result_data(void)
{
  return LESENSE->RESFIFO;
}

/***************************************************************************//**
 * @brief
 *   Clear the result buffer.
 *
 * @note
 *   This function will wait for any pending previous write operation to the CMD
 *   register to complete before accessing the CMD register. It will also wait
 *   for the write operation to the CMD register to complete before returning.
 *   Each write operation to the CMD register may take up to 3 LF clock cycles,
 *   so expect some delay. The user may implement a separate function to write
 *   multiple command bits in the CMD register in one single operation to optimize
 *   an application.
 ******************************************************************************/
__INLINE void sl_hal_lesense_clear_result_buffer(void)
{
  sl_hal_lesense_wait_sync();

  LESENSE->CMD = LESENSE_CMD_CLEARBUF;
}

/***************************************************************************//**
 * @brief
 *   Get the current state of the LESENSE decoder.
 *
 * @return
 *   This function returns the value of the LESENSE_DECSTATE register that represents
 *   the current state of the LESENSE decoder.
 ******************************************************************************/
__INLINE uint32_t sl_hal_lesense_decoder_get_state(void)
{
  return LESENSE->DECSTATE & _LESENSE_DECSTATE_DECSTATE_MASK;
}

/***************************************************************************//**
 * @brief
 *   Get the current state of the LESENSE sensor.
 *
 * @return
 *   Returns the value of LESENSE_SENSORSTATE register that represents the current
 *   state of the LESENSE sensor.
 ******************************************************************************/
__INLINE uint32_t sl_hal_lesense_get_sensor_state(void)
{
  return LESENSE->SENSORSTATE;
}

/***************************************************************************//**
 * @brief
 *   Reset the LESENSE module.
 *
 * @details
 *   Use this function to reset LESENSE registers.
 *
 * @note
 *   Resetting LESENSE registers is required in each reset or power-on cycle to
 *   configure the default values of the RAM mapped LESENSE registers.
 *   @ref sl_hal_lesense_reset() can be called on initialization by setting the
 *   @p req_reset parameter to true in @ref sl_hal_lesense_init().
 *   Starting from Series 2 Config 3 (xG23 and higher), this function leaves
 *   LESENSE in the disabled state.
 ******************************************************************************/
__INLINE void sl_hal_lesense_reset(void)
{
  LESENSE->SWRST_SET = LESENSE_SWRST_SWRST;
  while (LESENSE->SWRST & _LESENSE_SWRST_RESETTING_MASK) {
    // Wait for resetting to finish.
  }
}
/***************************************************************************//**
 * @brief
 *   Clear one or more pending LESENSE interrupts.
 *
 * @param[in] flags
 *   Pending LESENSE interrupt sources to clear. Use a set of interrupt flags
 *   OR-ed together to clear multiple interrupt sources of LESENSE module
 *   (LESENSE_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_lesense_clear_interrupts(uint32_t flags)
{
  LESENSE->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more LESENSE interrupts.
 *
 * @param[in] flags
 *   LESENSE interrupt sources to enable. Use a set of interrupt flags OR-ed
 *   together to enable multiple interrupt sources of LESENSE module (LESENSE_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_lesense_enable_interrupts(uint32_t flags)
{
  LESENSE->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more LESENSE interrupts.
 *
 * @param[in] flags
 *   LESENSE interrupt sources to disable. Use a set of interrupt flags OR-ed
 *   together to disable multiple interrupt sources of LESENSE module (LESENSE_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_lesense_disable_interrupts(uint32_t flags)
{
  LESENSE->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending LESENSE interrupts from SW.
 *
 * @param[in] flags
 *   LESENSE interrupt sources to set to pending. Use a set of interrupt flags
 *   OR-ed together to set multiple interrupt sources of LESENSE module
 *   (LESENSE_IFS_nnn).
 ******************************************************************************/
__INLINE void sl_hal_lesense_set_interrupts(uint32_t flags)
{
  LESENSE->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending LESENSE interrupt flags.
 *
 * @note
 *   Event bits are not cleared by the use of this function.
 *
 * @return
 *   Pending LESENSE interrupt sources. The OR combination of valid interrupt
 *   flags of the LESENSE module (LESENSE_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_lesense_get_pending_interrupts(void)
{
  return LESENSE->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending LESENSE interrupt flags.
 *
 * @details
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @note
 *   Event bits are not cleared by the use of this function.
 *
 * @return
 *   Pending and enabled LESENSE interrupt sources. Return value is the bitwise
 *   AND combination of
 *   - the OR combination of enabled interrupt sources in LESENSE_IEN_nnn register
 *   (LESENSE_IEN_nnn).
 *   - the OR combination of valid interrupt flags of LESENSE module (LESENSE_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_lesense_get_enabled_pending_interrupts(void)
{
  uint32_t tmp;

  // Store LESENSE->IEN in temporary variable in order to define explicit order
  // of volatile accesses.
  tmp = LESENSE->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return LESENSE->IF & tmp;
}

/** @} (end addtogroup lesense) */

#ifdef __cplusplus
}
#endif

/***************************************************************************//**
 * @addtogroup lesense LESENSE - Low Energy Sensor Interface
 * @{
 *
 * @n @section lesense_example Example
 *  This example demonstrates initialization, basic configuration, and usage of the
 *  LESENSE peripheral. It shows how to:
 *  - Initialize the LESENSE with default settings to sample a single channel. LESENSE
 *  uses ACMP to sample input state of a button 0 and will trigger an interrupt to toggle
 *  LED0.
 *
 * @code{.c}
 * #include "sl_hal_lesense.h"
 * #include "sl_hal_acmp.h"
 * #include "sl_hal_gpio.h"
 * #include "sl_clock_manager.h"
 * 
 * sl_gpio_t led0 = {LED0_PORT, LED0_PIN};
 * void init_gpio(void)
 * {
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
 *   sl_hal_gpio_set_pin_mode(&led0, SL_GPIO_MODE_PUSH_PULL, 0);
 * }
 * 
 * void init_ACMP(void)
 * {
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ACMP0);
 *   sl_hal_acmp_init_t amcp_init = SL_HAL_ACMP_INIT_DEFAULT;
 *   
 *   sl_hal_acmp_init(ACMP0, &amcp_init);
 *   if (BTN0_PIN % 2 == 0) {
 *      GPIO->BBUSALLOC_SET = GPIO_BBUSALLOC_BEVEN0_ACMP0;
 *   } else {
 *      GPIO->BBUSALLOC_SET = GPIO_BBUSALLOC_BODD0_ACMP0;
 *   }
 *   sl_hal_acmp_enable(ACMP0);
 *   // Set 1.25V internal as reference voltage for ACMP negative input and port B external interface override.
 *   sl_hal_acmp_set_input(ACMP0, SL_HAL_ACMP_INPUT_VREFDIV1V25, SL_HAL_ACMP_INPUT_EXTPB);
 *   // Wait for warm-up.
 *   while (!(ACMP0->STATUS & ACMP_IF_ACMPRDY));
 * }
 * 
 * void lesense_acmp_example(void)
 * {
 *   init_gpio();
 *   init_ACMP();
 *   // Initialize configuration structure with default settings
 *   sl_hal_lesense_init_t init = SL_HAL_LESENSE_INIT_DEFAULT;
 *   sl_hal_lesense_channel_descriptor_t lesense_channel = SL_HAL_LESENSE_CHANNEL_CONFIG_DEFAULT;
 *   uint32_t reference_freq;
 *
 *   // Enable clock.
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LFRCO);
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_LESENSE);
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_PRS);
 *
 *   // Get the frequency of LFRCO for LESENSE reference.
 *   sl_clock_manager_get_oscillator_frequency(SL_OSCILLATOR_LFRCO, &reference_freq);
 *
 *   // Do not store scan result and invert ACMP0 to accommodate PB0 pull-up.
 *   init.core_control.store_scanres = false;
 *   init.core_control.invert_acmp0 = true;
 *   // Enable LESENSE control of ACMP0 positive input mux.
 *   init.per_control.acmp0_mode = SL_HAL_LESENSE_ACMP_MODE_MUX;
 *
 *   // Config lesense channel.
 *   lesense_channel.enable_scan = true;
 *   lesense_channel.enable_interrupt = true;
 *   lesense_channel.sample_mode = SL_HAL_LESENSE_SAMPLE_MODE_ACMP;
 *   lesense_channel.interrupt_mode = SL_HAL_LESENSE_SET_INTERRUPT_POSITIVE_EDGE;
 *   lesense_channel.offset = PB0_PIN; // push button pin number.
 *
 *   // Initialize and enable LESENSE with default settings.
 *   sl_hal_lesense_init(&init);
 *   // Config channel 0.
 *   sl_hal_lesense_channel_config(&lesense_channel, 0);
 *   // Get 32 samples per second.
 *   sl_hal_lesense_disable();
 *   sl_hal_lesense_set_scan_frequency(reference_freq, 32);
 *
 *   // Start LESENSE scanning.
 *   sl_hal_lesense_enable();
 *   sl_hal_lesense_start_scan();
 *   
 *   // Enable interrupt in NVIC.
 *   NVIC_ClearPendingIRQ(LESENSE_IRQn);
 *   NVIC_EnableIRQ(LESENSE_IRQn);
 * }
 * 
 * void LESENSE_IRQHandler(void)
 * {
 *   // Clear all LESENSE interrupt flag
 *   uint32_t flags = sl_hal_lesense_get_pending_interrupts();
 *   sl_hal_lesense_clear_interrupts(flags);
 *   // Toggle LED0.
 *   if (flags & LESENSE_IF_CH0) {
 *     sl_hal_gpio_toggle_pin(&led0);
 *   }
 * }
 * @endcode
 *
 * @} (end addtogroup lesense)
 ******************************************************************************/

#endif /* defined(LESENSE_COUNT) && (LESENSE_COUNT > 0) */

#endif /* SL_HAL_LESENSE_H */
