/***************************************************************************//**
 * @file
 * @brief Successive Approximation Registers ADC peripheral API
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

#ifndef SL_HAL_ADC_H
#define SL_HAL_ADC_H

#include "em_device.h"

#if defined(ADC_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_enum.h"
#include "sl_assert.h"

/***************************************************************************//**
 * @addtogroup adc ADC - Analog to Digital Converter
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/// Validation of the ADC register block pointer reference for assert statements.
#define SL_HAL_ADC_REF_VALID(ref)    (ADC_NUM(ref) != -1)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Warm-up mode.
SL_ENUM(sl_hal_adc_warmup_mode_t) {
  /// Shut down the ADC after conversions have completed.
  SL_HAL_ADC_WARMUP_NORMAL      = _ADC_CTRL_WARMUPMODE_NORMAL,
  /// Keep the ADC fully powered after conversions have completed.
  SL_HAL_ADC_WARMUP_KEEPWARM    = _ADC_CTRL_WARMUPMODE_KEEPWARM,
  /// Allow the ADC to have 1us warmup instead of 5us after the first conversion.
  SL_HAL_ADC_WARMUP_KEEPSTANDBY = _ADC_CTRL_WARMUPMODE_KEEPSTANDBY
};

/// Voltage reference.
SL_ENUM(sl_hal_adc_voltage_reference_t) {
  SL_HAL_ADC_REFERENCE_VREFINT  = _ADC_CTRL_REFSEL_VREFINT,  ///< Internal voltage reference.
  SL_HAL_ADC_REFERENCE_VREFPL   = _ADC_CTRL_REFSEL_VREFPL,   ///< External reference pin low voltage (1.05-1.25).
  SL_HAL_ADC_REFERENCE_VREFPH   = _ADC_CTRL_REFSEL_VREFPH,   ///< External reference pin high voltage (>1.25).
  SL_HAL_ADC_REFERENCE_VDDA     = _ADC_CTRL_REFSEL_VDDA,     ///< Analog voltage supply.
  SL_HAL_ADC_REFERENCE_VREFPBUF = _ADC_CTRL_REFSEL_VREFPBUF  ///< External reference pin buffered.
};

/// First clock divider.
SL_ENUM(sl_hal_adc_high_speed_clock_rate_t) {
  SL_HAL_ADC_HSCLKRATE_DIV1 = _ADC_CTRL_HSCLKRATE_DIV1,   ///< Use clk_per directly.
  SL_HAL_ADC_HSCLKRATE_DIV2 = _ADC_CTRL_HSCLKRATE_DIV2,   ///< Divide clk_per by 2.
  SL_HAL_ADC_HSCLKRATE_DIV3 = _ADC_CTRL_HSCLKRATE_DIV3,   ///< Divide clk_per by 3.
  SL_HAL_ADC_HSCLKRATE_DIV4 = _ADC_CTRL_HSCLKRATE_DIV4,   ///< Divide clk_per by 4.
  SL_HAL_ADC_HSCLKRATE_DIV5 = _ADC_CTRL_HSCLKRATE_DIV5,   ///< Divide clk_per by 5.
  SL_HAL_ADC_HSCLKRATE_DIV6 = _ADC_CTRL_HSCLKRATE_DIV6,   ///< Divide clk_per by 6.
  SL_HAL_ADC_HSCLKRATE_DIV7 = _ADC_CTRL_HSCLKRATE_DIV7,   ///< Divide clk_per by 7.
  SL_HAL_ADC_HSCLKRATE_DIV8 = _ADC_CTRL_HSCLKRATE_DIV8    ///< Divide clk_per by 8.
};

#if defined(_ADC_CTRL_RESOLUTIONSEL_MASK)
/// ADC conversion resolution.
SL_ENUM(sl_hal_adc_resolution_t) {
  SL_HAL_ADC_RESOLUTION_BIT_12 = _ADC_CTRL_RESOLUTIONSEL_BIT12,  ///< 12 bit resolution.
  SL_HAL_ADC_RESOLUTION_BIT_8  = _ADC_CTRL_RESOLUTIONSEL_BIT8,   ///< 8 bit resolution.
};
#endif

/// ADC scan trigger.
SL_ENUM(sl_hal_adc_trigger_t) {
  SL_HAL_ADC_TRIGGER_IMMEDIATE  = _ADC_TRIGGER_SCANTRIGSEL_IMMEDIATE,  ///< Start scan queue immediately.
  SL_HAL_ADC_TRIGGER_TIMER      = _ADC_TRIGGER_SCANTRIGSEL_TIMER,      ///< Timer starts scan queue.
  SL_HAL_ADC_TRIGGER_PRSCLKGRP  = _ADC_TRIGGER_SCANTRIGSEL_PRSCLKGRP,  ///< PRS0 from timer in same clock group starts scan queue.
  SL_HAL_ADC_TRIGGER_PRSPOS     = _ADC_TRIGGER_SCANTRIGSEL_PRSPOS,     ///< PRS0 positive edge starts scan queue.
  SL_HAL_ADC_TRIGGER_PRSNEG     = _ADC_TRIGGER_SCANTRIGSEL_PRSNEG,     ///< PRS0 negative edge starts scan queue.
#if defined(_ADC_TRIGGER_SCANTRIGSEL_LESENSE)
  SL_HAL_ADC_TRIGGER_LESENSE    = _ADC_TRIGGER_SCANTRIGSEL_LESENSE,    ///< Low Energy Sensor starts scan queue.
#endif
};

/// ADC scan trigger action.
SL_ENUM(sl_hal_adc_trigger_action_t) {
  SL_HAL_ADC_TRIGGER_ACTION_ONCE        = _ADC_TRIGGER_SCANTRIGACTION_ONCE,       ///< Once per trigger.
  SL_HAL_ADC_TRIGGER_ACTION_CONTINUOUS  = _ADC_TRIGGER_SCANTRIGACTION_CONTINUOUS  ///< Continuously after trigger.
};

#if defined(_ADC_TRIGGER_REPDELAY_MASK)
/// Delay value between one sequence of the table and the next start of sequence.
SL_ENUM(sl_hal_adc_repetition_delay_t) {
  SL_HAL_ADC_DELAY_NO          = _ADC_TRIGGER_REPDELAY_NODELAY,    ///< No delay.
  SL_HAL_ADC_DELAY_CYCLES_2    = _ADC_TRIGGER_REPDELAY_CYCLE2,     ///< 2 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_4    = _ADC_TRIGGER_REPDELAY_CYCLE4,     ///< 4 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_8    = _ADC_TRIGGER_REPDELAY_CYCLE8,     ///< 8 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_16   = _ADC_TRIGGER_REPDELAY_CYCLE16,    ///< 16 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_32   = _ADC_TRIGGER_REPDELAY_CYCLE32,    ///< 32 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_64   = _ADC_TRIGGER_REPDELAY_CYCLE64,    ///< 64 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_128  = _ADC_TRIGGER_REPDELAY_CYCLE128,   ///< 128 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_256  = _ADC_TRIGGER_REPDELAY_CYCLE256,   ///< 256 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_512  = _ADC_TRIGGER_REPDELAY_CYCLE512,   ///< 512 conversion clock cycles.
  SL_HAL_ADC_DELAY_CYCLES_1024 = _ADC_TRIGGER_REPDELAY_CYCLE1024   ///< 1024 conversion clock cycles.
};
#endif

/// Analog gain.
SL_ENUM(sl_hal_adc_analog_gain_t) {
#if defined(_ADC_CFG_ANALOGGAIN_GAIN_0_3125)
  SL_HAL_ADC_ANALOG_GAIN_0_3125 = _ADC_CFG_ANALOGGAIN_GAIN_0_3125,  ///< Gain for sampling cap of 0.3125x.
#endif
#if defined(_ADC_CFG_ANALOGGAIN_GAIN_0_5)
  SL_HAL_ADC_ANALOG_GAIN_0_5    = _ADC_CFG_ANALOGGAIN_GAIN_0_5,     ///< Gain for sampling cap of 0.5x.
#endif
#if defined(_ADC_CFG_ANALOGGAIN_GAIN_1)
  SL_HAL_ADC_ANALOG_GAIN_1      = _ADC_CFG_ANALOGGAIN_GAIN_1,       ///< Gain for sampling cap of 1x.
#endif
#if defined(_ADC_CFG_ANALOGGAIN_GAIN_2)
  SL_HAL_ADC_ANALOG_GAIN_2      = _ADC_CFG_ANALOGGAIN_GAIN_2,       ///< Gain for sampling cap of 2x.
#endif
#if defined(_ADC_CFG_ANALOGGAIN_GAIN_4)
  SL_HAL_ADC_ANALOG_GAIN_4      = _ADC_CFG_ANALOGGAIN_GAIN_4        ///< Gain for sampling cap of 4x.
#endif
};

#if defined(_ADC_CFG_OSMODE_MASK)
/// Oversampling mode. DNS = Digital Noise Shaping, MES = Mismatch error Shaping, DWA = Data weighted averaging.
SL_ENUM(sl_hal_adc_oversampling_mode_t) {
  SL_HAL_ADC_OS_MODE_0 = _ADC_CFG_OSMODE_MODE0,  ///< Nyquist mode (DNS, MES and DWA disabled).
  SL_HAL_ADC_OS_MODE_1 = _ADC_CFG_OSMODE_MODE1,  ///< Enhanced Linearization (MES and DWA enabled).
  SL_HAL_ADC_OS_MODE_2 = _ADC_CFG_OSMODE_MODE2   ///< Noise shaping and Enhanced Linearization (DNS, MES and DWA enabled).
};
#endif

#if defined(_ADC_CFG_OSR_MASK)
/// Number of samples used for each conversion.
SL_ENUM(sl_hal_adc_oversampling_rate_t) {
  SL_HAL_ADC_OS_RATE_X1     = _ADC_CFG_OSR_X1,           ///< 1 sample for each conversion result.
  SL_HAL_ADC_OS_RATE_X2     = _ADC_CFG_OSR_X2,           ///< 2 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X4     = _ADC_CFG_OSR_X4,           ///< 4 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X8     = _ADC_CFG_OSR_X8,           ///< 8 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X16    = _ADC_CFG_OSR_X16,          ///< 16 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X32    = _ADC_CFG_OSR_X32,          ///< 32 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X64    = _ADC_CFG_OSR_X64,          ///< 64 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X128   = _ADC_CFG_OSR_X128,         ///< 128 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X256   = _ADC_CFG_OSR_X256,         ///< 256 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X512   = _ADC_CFG_OSR_X512,         ///< 512 samples for each conversion result.
  SL_HAL_ADC_OS_RATE_X1024  = _ADC_CFG_OSR_X1024         ///< 1024 samples for each conversion result.
};
#endif

#if defined(_ADC_CFG_AVERAGESEL_MASK)
/// Number of samples used for averaging.
SL_ENUM(sl_hal_adc_samples_t) {
  SL_HAL_ADC_AVERAGE_X1     = _ADC_CFG_AVERAGESEL_X1,    ///< 1 sample for each conversion result.
  SL_HAL_ADC_AVERAGE_X2     = _ADC_CFG_AVERAGESEL_X2,    ///< 2 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X4     = _ADC_CFG_AVERAGESEL_X4,    ///< 4 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X8     = _ADC_CFG_AVERAGESEL_X8,    ///< 8 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X16    = _ADC_CFG_AVERAGESEL_X16,   ///< 16 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X32    = _ADC_CFG_AVERAGESEL_X32,   ///< 32 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X64    = _ADC_CFG_AVERAGESEL_X64,   ///< 64 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X128   = _ADC_CFG_AVERAGESEL_X128,  ///< 128 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X256   = _ADC_CFG_AVERAGESEL_X256,  ///< 256 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X512   = _ADC_CFG_AVERAGESEL_X512,  ///< 512 samples for each conversion result.
  SL_HAL_ADC_AVERAGE_X1024  = _ADC_CFG_AVERAGESEL_X1024  ///< 1024 samples for each conversion result.
};
#endif

#if defined(_ADC_CFG_TWOSCMPL_MASK)
/// Output data polarity. Determines if the conversion result can be negative or not.
SL_ENUM(sl_hal_adc_output_polarity_t) {
  SL_HAL_ADC_OUTPUT_POLARITY_AUTO     = _ADC_CFG_TWOSCMPL_AUTO,           ///< Single-ended measurements are unipolar and differential measurements are bipolar.
  SL_HAL_ADC_OUTPUT_POLARITY_UNIPOLAR = _ADC_CFG_TWOSCMPL_FORCEUNIPOLAR,  ///< Negative differential numbers will saturate to 0.
  SL_HAL_ADC_OUTPUT_POLARITY_BIPOLAR  = _ADC_CFG_TWOSCMPL_FORCEBIPOLAR    ///< Single-ended measurements are half the range but allow for small negative measurements.
};
#endif

/// Alignment of output data written into FIFO.
SL_ENUM(sl_hal_adc_alignment_t) {
#if defined(_ADC_SCANFIFOCFG_ALIGNMENT_RIGHT20)
  SL_HAL_ADC_ALIGNMENT_RIGHT_20  = _ADC_SCANFIFOCFG_ALIGNMENT_RIGHT20,  ///< ID[7:0], SIGN_EXT, DATA[19:0].
#endif
  SL_HAL_ADC_ALIGNMENT_RIGHT_16  = _ADC_SCANFIFOCFG_ALIGNMENT_RIGHT16,  ///< ID[7:0], SIGN_EXT, DATA[15:0].
  SL_HAL_ADC_ALIGNMENT_RIGHT_12  = _ADC_SCANFIFOCFG_ALIGNMENT_RIGHT12,  ///< ID[7:0], SIGN_EXT, DATA[11:0].
#if defined(_ADC_SCANFIFOCFG_ALIGNMENT_RIGHT8)
  SL_HAL_ADC_ALIGNMENT_RIGHT_8   = _ADC_SCANFIFOCFG_ALIGNMENT_RIGHT8,   ///< ID[7:0], SIGN_EXT, DATA[7:0].
#endif
#if defined(_ADC_SCANFIFOCFG_ALIGNMENT_LEFT20)
  SL_HAL_ADC_ALIGNMENT_LEFT_20   = _ADC_SCANFIFOCFG_ALIGNMENT_LEFT20,   ///< DATA[19:0], 0000, ID[7:0].
#endif
  SL_HAL_ADC_ALIGNMENT_LEFT_16   = _ADC_SCANFIFOCFG_ALIGNMENT_LEFT16,   ///< DATA[15:0], 00000000, ID[7:0].
  SL_HAL_ADC_ALIGNMENT_LEFT_12   = _ADC_SCANFIFOCFG_ALIGNMENT_LEFT12,   ///< DATA[11:0], 000000000000, ID[7:0].
#if defined(_ADC_SCANFIFOCFG_ALIGNMENT_LEFT8)
  SL_HAL_ADC_ALIGNMENT_LEFT_8    = _ADC_SCANFIFOCFG_ALIGNMENT_LEFT8,    ///< DATA[7:0], 0000000000000000, ID[7:0].
#endif
};

/// Data valid level before requesting DMA transfer.
SL_ENUM(sl_hal_adc_data_valid_t) {
  SL_HAL_ADC_DATA_VALID_1   = _ADC_SCANFIFOCFG_DVL_VALID1,    ///< 1 valid entry to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_2   = _ADC_SCANFIFOCFG_DVL_VALID2,    ///< 2 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_3   = _ADC_SCANFIFOCFG_DVL_VALID3,    ///< 3 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_4   = _ADC_SCANFIFOCFG_DVL_VALID4,    ///< 4 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_5   = _ADC_SCANFIFOCFG_DVL_VALID5,    ///< 5 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_6   = _ADC_SCANFIFOCFG_DVL_VALID6,    ///< 6 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_7   = _ADC_SCANFIFOCFG_DVL_VALID7,    ///< 7 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_8   = _ADC_SCANFIFOCFG_DVL_VALID8,    ///< 8 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_9   = _ADC_SCANFIFOCFG_DVL_VALID9,    ///< 9 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_10  = _ADC_SCANFIFOCFG_DVL_VALID10,   ///< 10 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_11  = _ADC_SCANFIFOCFG_DVL_VALID11,   ///< 11 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_12  = _ADC_SCANFIFOCFG_DVL_VALID12,   ///< 12 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_13  = _ADC_SCANFIFOCFG_DVL_VALID13,   ///< 13 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_14  = _ADC_SCANFIFOCFG_DVL_VALID14,   ///< 14 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_15  = _ADC_SCANFIFOCFG_DVL_VALID15,   ///< 15 valid entries to set SCANFIFODVL interrupt.
  SL_HAL_ADC_DATA_VALID_16  = _ADC_SCANFIFOCFG_DVL_VALID16    ///< 16 valid entries to set SCANFIFODVL interrupt.
};

/// Port (A, B, C, D) or special signal assigned to the negative input of the ADC.
SL_ENUM(sl_hal_adc_port_negative_t) {
  SL_HAL_ADC_PORT_NEG_GND     = _ADC_SCAN_PORTNEG_GND,       ///< Ground.
  SL_HAL_ADC_PORT_NEG_PORTA   = _ADC_SCAN_PORTNEG_PORTA,     ///< Port A.
  SL_HAL_ADC_PORT_NEG_PORTB   = _ADC_SCAN_PORTNEG_PORTB,     ///< Port B.
  SL_HAL_ADC_PORT_NEG_PORTC   = _ADC_SCAN_PORTNEG_PORTC,     ///< Port C.
  SL_HAL_ADC_PORT_NEG_PORTD   = _ADC_SCAN_PORTNEG_PORTD      ///< Port D.
};

/// Port (A, B, C, D) or special signal assigned to the positive input of the ADC.
SL_ENUM(sl_hal_adc_port_positive_t) {
  SL_HAL_ADC_PORT_POS_GND     = _ADC_SCAN_PORTPOS_GND,       ///< Ground.
  SL_HAL_ADC_PORT_POS_SUPPLY  = _ADC_SCAN_PORTPOS_SUPPLY,    ///< Supply.
  SL_HAL_ADC_PORT_POS_PORTA   = _ADC_SCAN_PORTPOS_PORTA,     ///< Port A.
  SL_HAL_ADC_PORT_POS_PORTB   = _ADC_SCAN_PORTPOS_PORTB,     ///< Port B.
  SL_HAL_ADC_PORT_POS_PORTC   = _ADC_SCAN_PORTPOS_PORTC,     ///< Port C.
  SL_HAL_ADC_PORT_POS_PORTD   = _ADC_SCAN_PORTPOS_PORTD,     ///< Port D.
};

/// Configuration identifier.
SL_ENUM(sl_hal_adc_config_id_t) {
  SL_HAL_ADC_CONFIG_ID_0 = 0U,
  SL_HAL_ADC_CONFIG_ID_1,
  SL_HAL_ADC_CONFIG_ID_MAX,
};

/// Channel identifier.
SL_ENUM(sl_hal_adc_channel_id_t) {
  SL_HAL_ADC_CHANNEL_ID_0 = 0U,
  SL_HAL_ADC_CHANNEL_ID_1,
  SL_HAL_ADC_CHANNEL_ID_2,
  SL_HAL_ADC_CHANNEL_ID_3,
  SL_HAL_ADC_CHANNEL_ID_4,
  SL_HAL_ADC_CHANNEL_ID_5,
  SL_HAL_ADC_CHANNEL_ID_6,
  SL_HAL_ADC_CHANNEL_ID_7,
  SL_HAL_ADC_CHANNEL_ID_8,
  SL_HAL_ADC_CHANNEL_ID_9,
  SL_HAL_ADC_CHANNEL_ID_10,
  SL_HAL_ADC_CHANNEL_ID_11,
  SL_HAL_ADC_CHANNEL_ID_12,
  SL_HAL_ADC_CHANNEL_ID_13,
  SL_HAL_ADC_CHANNEL_ID_14,
  SL_HAL_ADC_CHANNEL_ID_15,
  SL_HAL_ADC_CHANNEL_ID_MAX,
};
/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// ADC config structure.
typedef struct {
  /// Selects the ADC analog gain.
  sl_hal_adc_analog_gain_t        gain;

#if defined(_ADC_CFG_OSMODE_MASK)
  /// Selects the oversampling mode.
  sl_hal_adc_oversampling_mode_t  oversampling_mode;
#endif

#if defined(_ADC_CFG_OSR_MASK)
  /// Selects the oversampling rate.
  sl_hal_adc_oversampling_rate_t  oversampling_rate;
#endif

#if defined(_ADC_CFG_AVERAGESEL_MASK)
  /// Selects the number of samples for averaging.
  sl_hal_adc_samples_t            average;
#endif

#if defined(_ADC_CFG_TWOSCMPL_MASK)
  /// Selects the output polarity.
  sl_hal_adc_output_polarity_t    output_polarity;
#endif

  /// Selects the acquisition time for scan samples in terms of ADC clock cycles.
  uint16_t                        acquisition_time;

#if defined(_ADC_CFG_FREQDOUBLE_MASK)
  /// Enables double frequency mode. Doubles the conversion clock frequency.
  bool                            double_frequency;
#endif
} sl_hal_adc_config_t;

/// ADC scan table entry structure.
typedef struct {
  /// Perform digital window comparison on the result from this entry.
  bool                            compare;

  /// Configuration ID.
  uint8_t                         config_id;

  /// Selects the input port for the negative side of the ADC.
  sl_hal_adc_port_negative_t      neg_port;

  /// Selects the input pin for the negative side of the ADC.
  uint8_t                         neg_pin;

  /// Selects the input port for the positive side of the ADC.
  sl_hal_adc_port_positive_t      pos_port;

  /// Selects the input pin for the positive side of the ADC.
  uint8_t                         pos_pin;
} sl_hal_adc_scan_entry_t;

/// Initialization structure.
typedef struct {
#if defined(_ADC_CTRL_ADCCLKSUSPEND_MASK)
  /// Enables Clock Suspend Mode. The ADC clock is only enabled with a PRS trigger.
  bool                            clk_suspend;
#endif

  /// Adds a scan entry's ID to each FIFO result.
  bool                            show_id;

  /// Enables halting the ADC in debug mode.
  bool                            debug_halt;

  /// Controls the max threshold.
  uint16_t                        greater_than;

  /// Controls the min threshold.
  uint16_t                        less_than;

  /// Controls the warmup mode.
  sl_hal_adc_warmup_mode_t        warmup_mode;

  /// Selects voltage reference.
  sl_hal_adc_voltage_reference_t  voltage_reference;

#if defined(_ADC_CTRL_RESOLUTIONSEL_MASK)
  /// Select scan sequence conversion resolution.
  sl_hal_adc_resolution_t         scan_resolution;
#endif

  /// Selects trigger to start a scan sequence.
  sl_hal_adc_trigger_t            scan_trigger;

  /// Selects the action mode after a trigger (Single or Continuous).
  sl_hal_adc_trigger_action_t     scan_trigger_action;

#if defined(_ADC_TRIGGER_REPDELAY_MASK)
  /// Selects number of cycles between every scan in continuous action mode.
  sl_hal_adc_repetition_delay_t   repetition_delay;
#endif

  /// Selects the ADC result alignment.
  sl_hal_adc_alignment_t          alignment;

  /// Controls the data valid level before requesting DMA transfer.
  sl_hal_adc_data_valid_t         data_valid;

  /// Configurations entries.
  sl_hal_adc_config_t             config[SL_HAL_ADC_CONFIG_ID_MAX];

  /// ADC scan table entries.
  sl_hal_adc_scan_entry_t         entries[SL_HAL_ADC_CHANNEL_ID_MAX];
} sl_hal_adc_init_t;

/// Structure holding ADC result, including data and ID.
typedef struct {
  /// ADC sample data.
  uint32_t  data;

  /// ID of FIFO entry.
  uint32_t  id;
} sl_hal_adc_result_t;

/*******************************************************************************
 **************************   STRUCT INITIALIZERS   ****************************
 ******************************************************************************/
/// Default value for Clock Suspend Mode for sl_hal_ldma_transfer_init_t struct initializer.
#if defined(_ADC_CTRL_ADCCLKSUSPEND_MASK)
#define SLI_HAL_ADC_CLK_SUSPEND_DEFAULT .clk_suspend = false,
#else
#define SLI_HAL_ADC_CLK_SUSPEND_DEFAULT
#endif

/// Default value for Scan Resolution for sl_hal_ldma_transfer_init_t struct initializer.
#if defined(_ADC_CTRL_RESOLUTIONSEL_MASK)
#define SLI_HAL_ADC_SCAN_RESOLUTION_DEFAULT .scan_resolution = SL_HAL_ADC_RESOLUTION_BIT_12,
#else
#define SLI_HAL_ADC_SCAN_RESOLUTION_DEFAULT
#endif

/// Default value for Repetition Delay for sl_hal_ldma_transfer_init_t struct initializer.
#if defined(_ADC_TRIGGER_REPDELAY_MASK)
#define SLI_HAL_ADC_REPETITION_DELAY_DEFAULT .repetition_delay = SL_HAL_ADC_DELAY_NO,
#else
#define SLI_HAL_ADC_REPETITION_DELAY_DEFAULT
#endif

/// Default ADC initialization structure.
#define SL_HAL_ADC_INIT_DEFAULT                            \
  {                                                        \
    SLI_HAL_ADC_CLK_SUSPEND_DEFAULT                        \
    .show_id = false,                                      \
    .debug_halt = false,                                   \
    .greater_than = 0,                                     \
    .less_than = 0,                                        \
    .warmup_mode = SL_HAL_ADC_WARMUP_KEEPWARM,             \
    .voltage_reference = SL_HAL_ADC_REFERENCE_VREFINT,     \
    SLI_HAL_ADC_SCAN_RESOLUTION_DEFAULT                    \
    .scan_trigger = SL_HAL_ADC_TRIGGER_IMMEDIATE,          \
    .scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_ONCE, \
    SLI_HAL_ADC_REPETITION_DELAY_DEFAULT                   \
    .alignment = SL_HAL_ADC_ALIGNMENT_RIGHT_16,            \
    .data_valid = SL_HAL_ADC_DATA_VALID_1,                 \
  }

/// Default ADC config structure.
#if defined(_ADC_CFG_AVERAGESEL_MASK)
#define SL_HAL_ADC_CONFIG_DEFAULT     \
  {                                   \
    .gain = SL_HAL_ADC_ANALOG_GAIN_1, \
    .average = SL_HAL_ADC_AVERAGE_X1, \
    .acquisition_time = 4,            \
  }
#else
#define SL_HAL_ADC_CONFIG_DEFAULT                       \
  {                                                     \
    .gain = SL_HAL_ADC_ANALOG_GAIN_1,                   \
    .oversampling_mode = SL_HAL_ADC_OS_MODE_0,          \
    .oversampling_rate = SL_HAL_ADC_OS_RATE_X1,         \
    .output_polarity = SL_HAL_ADC_OUTPUT_POLARITY_AUTO, \
    .acquisition_time = 4,                              \
    .double_frequency = false,                          \
  }
#endif

/// Default ADC scan entry structure.
#define SL_HAL_ADC_SCAN_ENTRY_DEFAULT    \
  {                                      \
    .compare = false,                    \
    .config_id = SL_HAL_ADC_CONFIG_ID_0, \
    .neg_port = SL_HAL_ADC_PORT_NEG_GND, \
    .neg_pin = 0,                        \
    .pos_port = SL_HAL_ADC_PORT_POS_GND, \
    .pos_pin = 0,                        \
  }
/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize the ADC.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @param[in] init
 *   A pointer to the ADC initialization structure.
 *
 * @param[in] branch_clock_freq
 *   Incoming peripheral frequency. Should be
 *   retrieved using the Clock Manager service.
 *
 * @note
 *   The ADC peripheral's internal clock prescalers and timebase will be set to
 *   sensible defaults according to the input parameter branch_clock_freq
 *   provided. These default prescalers can be modified using
 *   @ref sl_hal_adc_set_clock_prescalers.
 *   The branch_clock_freq can be obtained for adc0 using the Device Manager and
 *   Clock Manager services as such:
 *   @code{.c}
 *   {
 *     #include "sl_clock_manager.h"
 *     #include "sl_device_peripheral.h"
 *
 *     sl_clock_branch_t clock_branch;
 *     clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_ADC0);
 *
 *     uint32_t branch_clock_freq;
 *     sl_clock_manager_get_clock_branch_frequency(clock_branch, &branch_clock_freq);
 *   }
 *   @endcode
 *
 * @note
 *  For SiXG301, due to hardware limitations, the ADC cannot be configured for
 *  hardware averaging if the adcprescale is 0 (divide by 1). If hardware averaging
 *  is desired, then an adc0 branch_clock_freq of 34 - 40Mhz, 68 - 80Mhz, 102 - 120Mhz
 *  or 136 - 160Mhz can be used to guarantee that the calculated adcprescale is 1.
 ******************************************************************************/
void sl_hal_adc_init(ADC_TypeDef *adc,
                     const sl_hal_adc_init_t *init,
                     uint32_t branch_clock_freq);

/***************************************************************************//**
 * @brief
 *   Reset the ADC.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
void sl_hal_adc_reset(ADC_TypeDef *adc);

/***************************************************************************//**
 * @brief
 *   Set a new Scan Table mask.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @param[in] mask
 *   New scan table mask.
 *
 * @note
 *   The working scan table is updated when it's not being scanned.
 *   Each bit in the mask corresponds to a given ADC channel, and setting a
 *   given bit to 1 will enable the conversion of the respective ADC channel.
 ******************************************************************************/
void sl_hal_adc_set_scan_mask(ADC_TypeDef *adc,
                              uint32_t mask);

/***************************************************************************//**
 * @brief
 *   Get the working scan table mask.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @return
 *   The working scan table mask.
 ******************************************************************************/
uint32_t sl_hal_adc_get_scan_mask(ADC_TypeDef *adc);

/***************************************************************************//**
 * @brief
 *   Update an entry in the scan table.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @param[in] id
 *   ID of scan table entry to add.
 *
 * @param[in] entry
 *   A pointer to a Scan Table entry structure.
 ******************************************************************************/
void sl_hal_adc_update_scan_entry(ADC_TypeDef *adc,
                                  uint8_t id,
                                  const sl_hal_adc_scan_entry_t *entry);

/***************************************************************************//**
 * @brief
 *   Pull the value at the top of the FIFO. FIFO count reduced.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @return
 *   The data at the top of the FIFO.
 ******************************************************************************/
sl_hal_adc_result_t sl_hal_adc_pull(const ADC_TypeDef *adc);

/***************************************************************************//**
 * @brief
 *   Peek at the top of the FIFO without removing the data.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @return
 *   The data at the top of the FIFO.
 *
 * @note The FIFO's count remains unchanged.
 ******************************************************************************/
sl_hal_adc_result_t sl_hal_adc_peek(const ADC_TypeDef *adc);

/***************************************************************************//**
 * @brief
 *   Get the ADC's current FIFO count.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @return
 *   The ADC's current FIFO count.
 ******************************************************************************/
uint32_t sl_hal_adc_get_fifo_count(const ADC_TypeDef *adc);

/***************************************************************************//**
 * @brief
 *   Get the ADC's internal reference voltage in millivolts.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @return
 *   The ADC's internal reference voltage in millivolts.
 ******************************************************************************/
uint32_t sl_hal_adc_get_internal_reference_voltage(const ADC_TypeDef *adc);

/***************************************************************************//**
 * @brief
 *   Calculate and return the clock frequency of the ADC's local timer.
 *
 * @details
 *   Returned value is based on the clock branch selected for the ADC
 *   peripheral and the current hsclkrate prescaler.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @param[in] branch_clock_freq
 *   Incoming peripheral frequency. Should be retrieved using the Clock Manager
 *   service.
 *
 * @return
 *   The clock frequency of the ADC's local timer.
 *
 * @note
 *   The local timer can be used to trigger conversions periodically by setting
 *   the timer period with @ref sl_hal_adc_set_timer_period.
 *
 *   The branch_clock_freq can be obtained for adc0 using the Device Manager and
 *   Clock Manager services as such:
 *
 *       #include "sl_clock_manager.h"
 *       #include "sl_device_peripheral.h"
 *
 *       sl_clock_branch_t clock_branch;
 *       clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_ADC0);
 *
 *       uint32_t branch_clock_freq;
 *       sl_clock_manager_get_clock_branch_frequency(clock_branch, &branch_clock_freq);
 *
 ******************************************************************************/
uint32_t sl_hal_adc_get_timer_frequency(const ADC_TypeDef *adc,
                                        uint32_t branch_clock_freq);

/***************************************************************************//**
 * @brief
 *   Set the timer period to periodically trigger a timer event.
 *
 * @details
 *   Timer events can be used to trigger an ADC scan.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @param[in] timer_period
 *   The number of clock cycles between ADC timer events.
 *
 * @note
 *   The ADC's local timer must be disabled before calling this function.
 *
 ******************************************************************************/
void sl_hal_adc_set_timer_period(ADC_TypeDef *adc,
                                 uint16_t timer_period);

/***************************************************************************//**
 * @brief
 *   Set both ADC prescalers.
 *
 * @details
 *   Re-calculates and updates the timebase.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @param[in] branch_clock_freq
 *   Incoming peripheral frequency. Should be retrieved using the Clock Manager
 *   service.
 *
 * @param[in] hsclkrate
 *   This prescaler divides the branch clock frequency and the resulting clock
 *   is ADC_SRC_CLK. This clocks the ADC peripheral's local timer.
 *   The value can be set to an integer value from 0 to 5. The branch clock
 *   frequency is divided by (hsclkrate + 1).
 *   ADC_SRC_CLK must be less than 40MHz.
 *
 * @param[in] adcprescale
 *   This prescaler divides the ADC_SRC_CLK and the resulting clock is
 *   ADC_CORE_CLK. This clocks the core functionality of the ADC peripheral.
 *   The value can be set to 0 (no divider) or 1 (divide by 2).
 *   The operating frequency of ADC_CORE_CLK is between 17MHz and 22MHz.
 *
 * @note
 *   The ADC must be disabled before making changes to the clock prescalers.
 *   The branch_clock_freq can be obtained for adc0 using the Device Manager and
 *   Clock Manager services as such:
 * @code{.c}
 *   {
 *     #include "sl_clock_manager.h"
 *     #include "sl_device_peripheral.h"
 *
 *     sl_clock_branch_t clock_branch;
 *     clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_ADC0);
 *
 *     uint32_t branch_clock_freq;
 *     sl_clock_manager_get_clock_branch_frequency(clock_branch, &branch_clock_freq);
 *   }
 * @endcode
 ******************************************************************************/
void sl_hal_adc_set_clock_prescalers(ADC_TypeDef *adc,
                                     uint32_t branch_clock_freq,
                                     uint8_t hsclkrate,
                                     uint8_t adcprescale);

/***************************************************************************//**
 * @brief
 *   Enable the ADC.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_enable(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  adc->EN_SET = ADC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Disable the ADC.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_disable(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  if (adc->EN & _ADC_EN_EN_MASK) {
    adc->EN_CLR = ADC_EN_EN;
  }
}

/***************************************************************************//**
 * @brief
 *   Start the ADC.
 *
 * @note
 *   Enables the scan sequence and will start a scan if using the immediate
 *   trigger mode.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_start(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  // Make sure the module is enabled.
  EFM_ASSERT((adc->EN & _ADC_EN_EN_MASK) == _ADC_EN_EN_MASK);

  adc->CMD_SET = ADC_CMD_SCANSTART;
}

/***************************************************************************//**
 * @brief
 *   Stop the ADC.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_stop(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  // Make sure the module is enabled.
  EFM_ASSERT((adc->EN & _ADC_EN_EN_MASK) == _ADC_EN_EN_MASK);

  adc->CMD_SET = ADC_CMD_SCANSTOP;
}

/***************************************************************************//**
 * @brief
 *   Enable the ADC's local timer.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_enable_timer(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  // Make sure the module is enabled.
  EFM_ASSERT((adc->EN & _ADC_EN_EN_MASK) == _ADC_EN_EN_MASK);

  adc->CMD_SET = ADC_CMD_TIMEREN;
}

/***************************************************************************//**
 * @brief
 *   Disable the ADC's local timer and resets the counter.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_disable_timer(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  // Make sure the module is enabled.
  EFM_ASSERT((adc->EN & _ADC_EN_EN_MASK) == _ADC_EN_EN_MASK);

  adc->CMD_SET = ADC_CMD_TIMERDIS;
}

/***************************************************************************//**
 * @brief
 *   Flush the scan FIFO.
 *
 * @note
 *   The ADC must be enabled.
 *   The operation is completed when SCANFIFOFLUSHING STATUS bit is low.
 *   The Scan queue should be disabled.
 *   Any incoming scan queue data will be discarded during the flush.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_flush_fifo(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));
  // Make sure the module is enabled.
  EFM_ASSERT((adc->EN & _ADC_EN_EN_MASK) == _ADC_EN_EN_MASK);

  adc->CMD_SET = ADC_CMD_SCANFIFOFLUSH;
}

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_wait_sync(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  while ((adc->EN & _ADC_EN_EN_MASK) && (adc->STATUS & _ADC_STATUS_SYNCBUSY_MASK)) {
    // Wait for any pending previous write operation to have been completed
    // in the low-frequency domain.
  }
}

/***************************************************************************//**
 * @brief
 *   Wait for disabling to finish.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_adc_wait_ready(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  while (adc->EN & _ADC_EN_DISABLING_MASK) {
    // Wait for disabling to finish
  }
}

/***************************************************************************//**
 * @brief
 *   Return the content of the status register.
 *
 * @param[in] adc
 *   A pointer to the ADC peripheral register block.
 *
 * @return
 *   Status register.
 ******************************************************************************/
__INLINE uint32_t sl_hal_adc_get_status(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  return adc->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more ADC interrupts.
 *
 * @param[in] adc
 *   Pointer to the ADC peripheral register block.
 *
 * @param[in] flags
 *   Pending ADC interrupt source to enable. Use a bitwise logic OR combination
 *   of valid interrupt flags for ADC module (ADC_IEN_nnn).
 ******************************************************************************/
__INLINE void sl_hal_adc_enable_interrupts(ADC_TypeDef *adc,
                                           uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  adc->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more ADC interrupts.
 *
 * @param[in] adc
 *   Pointer to the ADC peripheral register block.
 *
 * @param[in] flags
 *   Pending ADC interrupt source to disable. Use a bitwise logic OR combination
 *   of valid interrupt flags for ADC module (ADC_IEN_nnn).
 ******************************************************************************/
__INLINE void sl_hal_adc_disable_interrupts(ADC_TypeDef *adc,
                                            uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  adc->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending ADC interrupts.
 *
 * @param[in] adc
 *   Pointer to the ADC peripheral register block.
 *
 * @param[in] flags
 *   Pending ADC interrupt source to clear. Use a bitwise logic OR combination
 *   of valid interrupt flags for ADC module (ADC_IEN_nnn).
 ******************************************************************************/
__INLINE void sl_hal_adc_clear_interrupts(ADC_TypeDef *adc,
                                          uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  adc->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending ADC interrupts from software.
 *
 * @param[in] adc
 *   Pointer to the ADC peripheral register block.
 *
 * @param[in] flags
 *   Interrupt source(s) to set to pending.  Use a bitwise logic OR combination
 *   of valid interrupt flags for ADC module (ADC_IEN_nnn).
 ******************************************************************************/
__INLINE void sl_hal_adc_set_interrupts(ADC_TypeDef *adc,
                                        uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  adc->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending ADC interrupt flags.
 *
 * @param[in] adc
 *   Pointer to the ADC peripheral register block.
 *
 * @return
 *   Pending ADC interrupt sources.
 ******************************************************************************/
__INLINE uint32_t sl_hal_adc_get_pending_interrupts(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  return adc->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending ADC interrupt flags.
 *
 * @details
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] adc
 *   Pointer to the ADC peripheral register block.
 *
 * @return
 *   Pending and enabled ADC interrupt sources.
 ******************************************************************************/
__INLINE uint32_t sl_hal_adc_get_enabled_pending_interrupts(ADC_TypeDef *adc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_ADC_REF_VALID(adc));

  uint32_t tmp;

  // Store ADCx->IEN in temporary variable in order to define explicit order
  // of volatile accesses.
  tmp = adc->IEN;

  // Bitwise AND of pending and enabled interrupts
  return adc->IF & tmp;
}

/** @} (end addtogroup adc) */

#ifdef __cplusplus
}
#endif

/* *************************************************************************//**
 * @addtogroup adc ADC - Analog to Digital Converter
 * @{
 *
 * @n @section adc_intro Introduction
 *
 *   This module provides a hardware abstraction layer (HAL) interface
 *   to the ADC peripheral. It allows the user to initialize and
 *   configure the ADC to perform one or more conversions triggered
 *   immediately or continuously. The ADC stores its conversion results
 *   as a FIFO. Additionally, the ADC can be triggered by an external
 *   event or by a timer event.
 *
 *   For more details on the ADC initialization and configuration options
 *   available, please refer to the following structures and their
 *   corresponding attributes:
 *   - @ref sl_hal_adc_init_t
 *   - @ref sl_hal_adc_config_t
 *   - @ref sl_hal_adc_scan_entry_t
 *
 * @n @section adc_example Example
 *
 *   The following code snippet is an example of how to configure the ADC
 *   peripheral to perform a single immediate conversion:
 *   @code{.c}
 *   {
 *     // Enable ADC bus clock.
 *     sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_ADC0);
 *
 *     // Get clock branch frequency to configure the ADC.
 *     sl_clock_branch_t clock_branch;
 *     uint32_t branch_clock_freq;
 *     clock_branch = sl_device_peripheral_get_clock_branch(SL_PERIPHERAL_ADC0);
 *     sl_clock_manager_get_clock_branch_frequency(clock_branch, &branch_clock_freq);
 *
 *     sl_hal_adc_scan_entry_t entry = SL_HAL_ADC_SCAN_ENTRY_DEFAULT;
 *     entry.pos_port = SL_HAL_ADC_PORT_POS_PORTA;
 *     entry.pos_pin = 0;
 *
 *     sl_hal_adc_init_t init = SL_HAL_ADC_INIT_DEFAULT;
 *     init.scan_trigger = SL_HAL_ADC_TRIGGER_IMMEDIATE;
 *     init.scan_trigger_action = SL_HAL_ADC_TRIGGER_ACTION_ONCE;
 *
 *     sl_hal_adc_config_t config = SL_HAL_ADC_CONFIG_DEFAULT;
 *     config.acquisition_time = 16;
 *
 *     init.entries[SL_HAL_ADC_CHANNEL_ID_0] = entry;
 *
 *     sl_hal_adc_reset(ADC0);
 *     sl_hal_adc_enable_interrupts(ADC0, ADC_IF_SCANENTRYDONE);
 *
 *     init.config[SL_HAL_ADC_CONFIG_ID_0] = config;
 *
 *     sl_hal_adc_init(ADC0, &init, branch_clock_freq);
 *     sl_hal_adc_enable(ADC0);
 *
 *     sl_hal_adc_flush_fifo(ADC0);
 *     while (sl_hal_adc_get_status(ADC0) & ADC_STATUS_SCANFIFOFLUSHING) ;
 *
 *     uint32_t scan_mask = 1U << SL_HAL_ADC_CONFIG_ID_0;
 *     sl_hal_adc_set_scan_mask(ADC0, scan_mask);
 *
 *     sl_hal_adc_result_t result;
 *
 *     // Trigger scan and pull results from fifo.
 *     sl_hal_adc_start(ADC0);
 *     while (!sl_hal_adc_get_enabled_pending_interrupts(ADC0)) ;
 *
 *     result = sl_hal_adc_pull(ADC0);
 *
 *     sl_hal_adc_clear_interrupts(ADC0, ADC_IF_SCANENTRYDONE);
 *   }
 *   @endcode
 *
 * # Hardware Limitations and Workarounds
 * ## Continuous Single Channel Conversion Generates Invalid Accumulation Results
 * ### Description
 * When the ADC continuously scans a single channel under certain conditions,
 * the conversion results are incorrectly accumulated.
 *
 * The conversion result is invalid if all of the following conditions are met:
 * - One channel is being converted in the scan sequence.
 * - ADC is performing continuous conversions
 *   - ADC init structure's `scan_trigger_action` is set to
 *    `SL_HAL_ADC_TRIGGER_ACTION_CONTINUOUS`.
 * - ADC is accumulating 4 or more conversions per output
 *   - ADC configuration structure's `average` is set to `SL_HAL_ADC_AVERAGE_X4`
 *    or higher.
 * - Scan repetition delay is disabled
 *   - ADC init structure's `repetition_delay` is set to `SL_HAL_ADC_DELAY_NO`.
 * - The internal ADC clocks CLK_CORE_ADC and CLK_SRC_ADC are equal
 *   - The prescaler `adcprescale` is set to 0 (divide by 1).
 *   - If @ref sl_hal_adc_set_clock_prescalers is not set explicitly after
 *     initializing the ADC with @ref sl_hal_adc_init, the default prescaler
 *     values will be calculated by @ref sl_hal_adc_init based on the branch
 *     clock frequency.
 *
 * ### Workaround
 * There are two workarounds:
 * - Enable scan repetition delay by setting the ADC init structure's
 *   `repetition_delay` to any option other than `SL_HAL_ADC_DELAY_NO`.
 * - Configure CLK_SRC_ADC to be twice the frequency of CLK_CORE_ADC
 *   - By setting `adcprescale` to 1 explicitly with the
 *     @ref sl_hal_adc_set_clock_prescalers function.
 *   - Or by using a branch clock frequency of 34 - 40Mhz, 68 - 80Mhz,
 *     102 - 120Mhz or 136 - 160Mhz, when calling @ref sl_hal_adc_init to guarantee
 *     that the calculated `adcprescale` is 1 (See the note in
 *     @ref sl_hal_adc_init).
 *
 * @} (end addtogroup adc)
 ******************************************************************************/

#endif // ADC_PRESENT
#endif // SL_HAL_ADC_H
