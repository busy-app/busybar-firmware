/***************************************************************************//**
 * @file
 * @brief Incremental Analog to Digital Converter (IADC) peripheral API
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

#ifndef SL_HAL_IADC_H
#define SL_HAL_IADC_H

#include "em_device.h"
#include "sl_hal_gpio.h"
#include "sl_enum.h"
#if defined(IADC_COUNT) && (IADC_COUNT > 0)

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup iadc IADC - Incremental ADC
 * @brief Incremental Analog to Digital Converter (IADC) Peripheral API
 * @details
 *  This module contains functions to control the IADC peripheral of Silicon
 *  Labs 32-bit MCUs and SoCs. The IADC is used to convert analog signals into a
 *  digital representation.
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/// Validation of IADC register block pointer reference for assert statements.
#define SL_HAL_IADC_REF_VALID(ref)    (IADC_NUM(ref) != -1)
/// IADC default reference voltage in mV.
#define SL_HAL_IADC_DEFAULT_VREF     1210

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Warm-up mode.
SL_ENUM(sl_hal_iadc_warmup_t){
  SL_HAL_IADC_WARMUP_NORMAL          = _IADC_CTRL_WARMUPMODE_NORMAL,          ///< IADC shutdown after each conversion.
  SL_HAL_IADC_WARMUP_KEEP_IN_STANDBY = _IADC_CTRL_WARMUPMODE_KEEPINSTANDBY,   ///< IADC is kept in standby mode between conversion.
  SL_HAL_IADC_WARMUP_KEEP_WARM       = _IADC_CTRL_WARMUPMODE_KEEPWARM         ///< IADC and reference selected for scan mode kept warmup, allowing continuous conversion.
};

/// IADC result alignment.
SL_ENUM(sl_hal_iadc_alignment_t){
  SL_HAL_IADC_ALIGNMENT_RIGHT_12 = _IADC_SCANFIFOCFG_ALIGNMENT_RIGHT12,   ///< IADC results 12-bit right aligned.
  SL_HAL_IADC_ALIGNMENT_LEFT_12  = _IADC_SCANFIFOCFG_ALIGNMENT_LEFT12,    ///< IADC results 12-bit left aligned.
#if defined(IADC_SINGLEFIFOCFG_ALIGNMENT_RIGHT16)
  SL_HAL_IADC_ALIGNMENT_RIGHT_16 = _IADC_SCANFIFOCFG_ALIGNMENT_RIGHT16,   ///< IADC results 16-bit right aligned.
  SL_HAL_IADC_ALIGNMENT_LEFT_16  = _IADC_SCANFIFOCFG_ALIGNMENT_LEFT16,    ///< IADC results 16-bit left aligned.
#endif

#if defined(IADC_SINGLEFIFOCFG_ALIGNMENT_RIGHT20)
  SL_HAL_IADC_ALIGNMENT_RIGHT_20 = _IADC_SCANFIFOCFG_ALIGNMENT_RIGHT20,   ///< IADC results 20-bit right aligned.
  SL_HAL_IADC_ALIGNMENT_LEFT_20  = _IADC_SCANFIFOCFG_ALIGNMENT_LEFT20     ///< IADC results 20-bit left aligned.
#endif
};

/// IADC Reference.
SL_ENUM(sl_hal_iadc_voltage_reference_t) {
  SL_HAL_IADC_REFERENCE_VREFINT_1V2 = _IADC_CFG_REFSEL_VBGR,         ///< Internal 1.2V Band Gap Reference (buffered) to ground.
  SL_HAL_IADC_VREF_EXT_1V25         = _IADC_CFG_REFSEL_VREF,         ///< External reference (unbuffered) VREFP to VREFN.
                                                                     ///< VEVREF up to AVDD. When inputs are routed to external GPIO pins,
                                                                     ///< the maximum pin voltage is limited to the lower
                                                                     ///< of the IOVDD and AVDD supplies.
                                                                     ///< The internal calibration values correspond to a 1.25V reference,
                                                                     ///< use of other voltages may require recalibration.
                                                                     ///< See AN1189: Incremental Analog to Digital Converter (IADC).
#if defined(_IADC_CFG_REFSEL_VREF2P5)
  SL_HAL_IADC_VREF_EXT_2V5          = _IADC_CFG_REFSEL_VREF2P5,      ///< External reference (unbuffered) VREFP to VREFN. Supports 2.5V in high accuracy mode.
#endif
  SL_HAL_IADC_VREF_VDDX             = _IADC_CFG_REFSEL_VDDX,         ///< VDDX (unbuffered) to ground.
  SL_HAL_IADC_VREF_VDDX0P8BUF       = _IADC_CFG_REFSEL_VDDX0P8BUF,   ///< 0.8 * VDDX (buffered) to ground.
#if defined(_IADC_CFG_REFSEL_VREFBUF)
  SL_HAL_IADC_VREF_BUF              = _IADC_CFG_REFSEL_VREFBUF,      ///< pad_vrefp (buffered) to pad_vrefn (pad_refp < vddx - 0.3).
#endif
#if defined(_IADC_CFG_REFSEL_VREF0P8BUF)
  SL_HAL_IADC_VREF_0P8BUF           = _IADC_CFG_REFSEL_VREF0P8BUF    ///< pad_vref (buffered) * 0.8 to pad_vrefn (LPF pad_refp noise).
#endif
};

/// IADC negative input ground pin selection.
SL_ENUM(sl_hal_iadc_neg_input_port_gnd_pin_selection_t) {
  SL_HAL_IADC_NEG_PIN_INPUT_GND_AUX  = 0,   ///< GND auxiliary.
  SL_HAL_IADC_NEG_PIN_INPUT_GND      = 1    ///< GND.
};

/// IADC positive input supply pin selection.
SL_ENUM(sl_hal_iadc_pos_input_port_supply_pin_selection_t) {
  SL_HAL_IADC_POS_PIN_INPUT_AVDD     = 0,   ///< AVDD / 4.
  SL_HAL_IADC_POS_PIN_INPUT_IOVDD    = 1,   ///< VDDIO / 4.
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
  SL_HAL_IADC_POS_PIN_INPUT_IOVDD1   = 2,   ///< VDDIO1 / 4.
  SL_HAL_IADC_POS_PIN_INPUT_IOVDD2   = 3,   ///< VDDIO2 / 4.
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7) || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9)
  SL_HAL_IADC_POS_PIN_INPUT_VBAT     = 2,   ///< VBAT / 4.
  SL_HAL_IADC_POS_PIN_INPUT_VSS      = 3,   ///< VSS.
#else
  SL_HAL_IADC_POS_PIN_INPUT_VSS      = 2,   ///< VSS.
  SL_HAL_IADC_POS_PIN_INPUT_VSS_AUX  = 3,   ///< VSS auxiliary.
#endif
  SL_HAL_IADC_POS_PIN_INPUT_DVDD     = 4,   ///< DVDD / 4.
  SL_HAL_IADC_POS_PIN_INPUT_DECOUPLE = 7    ///< DECOUPLE / 4.
};

/// IADC negative input port selection.
SL_ENUM(sl_hal_iadc_negative_port_input_t) {
  SL_HAL_IADC_NEG_PORT_INPUT_GND       = _IADC_SCAN_PORTNEG_GND,         ///< Ground.
#if defined(_IADC_SCAN_PORTNEG_DAC1)
  SL_HAL_IADC_NEG_PORT_INPUT_DAC_1     = _IADC_SCAN_PORTNEG_DAC1,        ///< Direct connection to DAC_1 input pin.
#endif
#if defined(_IADC_SCAN_PORTNEG_PADANA1)
  SL_HAL_IADC_NEG_PORT_INPUT_PAD_ANA_1 = _IADC_SCAN_PORTNEG_PADANA1,     ///< Direct connection to Pad_ana_1 input pin.
#endif
#if defined(_IADC_SCAN_PORTNEG_PADANA3)
  SL_HAL_IADC_NEG_PORT_INPUT_PAD_ANA_3 = _IADC_SCAN_PORTNEG_PADANA3,     ///< Direct connection to Pad_ana_3 input pin.
#endif
#if defined(_IADC_SCAN_PORTNEG_PADREFNEG)
  SL_HAL_IADC_NEG_PORT_INPUT_REF       = _IADC_SCAN_PORTNEG_PADREFNEG,   ///< Negative reference pin 0.
#endif
  SL_HAL_IADC_NEG_PORT_INPUT_PORT_A    = _IADC_SCAN_PORTNEG_PORTA,       ///< GPIO port A.
  SL_HAL_IADC_NEG_PORT_INPUT_PORT_B    = _IADC_SCAN_PORTNEG_PORTB,       ///< GPIO port B.
  SL_HAL_IADC_NEG_PORT_INPUT_PORT_C    = _IADC_SCAN_PORTNEG_PORTC,       ///< GPIO port C.
  SL_HAL_IADC_NEG_PORT_INPUT_PORT_D    = _IADC_SCAN_PORTNEG_PORTD        ///< GPIO port D.
};

/// IADC positive input port selection.
SL_ENUM(sl_hal_iadc_positive_port_input_t) {
  SL_HAL_IADC_POS_PORT_INPUT_GND       = _IADC_SCAN_PORTPOS_GND,         ///< Ground.
  SL_HAL_IADC_POS_PORT_INPUT_SUPPLY    = _IADC_SCAN_PORTPOS_SUPPLY,      ///< Supply.
#if defined(_IADC_SCAN_PORTPOS_DAC0)
  SL_HAL_IADC_POS_PORT_INPUT_DAC_0     = _IADC_SCAN_PORTPOS_DAC0,        ///< Direct connection to DAC_0 input pin.
#endif
#if defined(_IADC_SCAN_PORTPOS_PADANA0)
  SL_HAL_IADC_POS_PORT_INPUT_PAD_ANA_0 = _IADC_SCAN_PORTPOS_PADANA0,    ///< Direct connection to Pad_ana_0 input pin.
#endif
#if defined(_IADC_SCAN_PORTPOS_PADANA2)
  SL_HAL_IADC_POS_PORT_INPUT_PAD_ANA_2 = _IADC_SCAN_PORTPOS_PADANA2,    ///< Direct connection to Pad_ana_2 input pin.
#endif
#if defined(_IADC_SCAN_PORTPOS_PADREFPOS)
  SL_HAL_IADC_POS_PORT_INPUT_REF       = _IADC_SCAN_PORTPOS_PADREFPOS,   ///< Positive reference pin 0.
#endif
  SL_HAL_IADC_POS_PORT_INPUT_PORT_A    = _IADC_SCAN_PORTPOS_PORTA,       ///< GPIO port A.
  SL_HAL_IADC_POS_PORT_INPUT_PORT_B    = _IADC_SCAN_PORTPOS_PORTB,       ///< GPIO port B.
  SL_HAL_IADC_POS_PORT_INPUT_PORT_C    = _IADC_SCAN_PORTPOS_PORTC,       ///< GPIO port C.
  SL_HAL_IADC_POS_PORT_INPUT_PORT_D    = _IADC_SCAN_PORTPOS_PORTD        ///< GPIO port D.
};

/// IADC pin input for GPIO port.
SL_ENUM_GENERIC(sl_hal_iadc_gpio_port_input_pin_t, uint8_t)
{
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_0 = 0,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_1 = 1,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_2 = 2,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_3 = 3,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_4 = 4,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_5 = 5,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_6 = 6,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_7 = 7,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_8 = 8,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_9 = 9,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_10 = 10,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_11 = 11,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_12 = 12,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_13 = 13,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_14 = 14,
  SL_HAL_IADC_GPIO_PORT_INPUT_PIN_15 = 15
};

/// Channel identifier.
SL_ENUM(sl_hal_iadc_channel_id_t) {
  SL_HAL_IADC_CHANNEL_ID_0 = 0,
  SL_HAL_IADC_CHANNEL_ID_1 = 1,
  SL_HAL_IADC_CHANNEL_ID_2 = 2,
  SL_HAL_IADC_CHANNEL_ID_3 = 3,
  SL_HAL_IADC_CHANNEL_ID_4 = 4,
  SL_HAL_IADC_CHANNEL_ID_5 = 5,
  SL_HAL_IADC_CHANNEL_ID_6 = 6,
  SL_HAL_IADC_CHANNEL_ID_7 = 7,
  SL_HAL_IADC_CHANNEL_ID_8 = 8,
  SL_HAL_IADC_CHANNEL_ID_9 = 9,
  SL_HAL_IADC_CHANNEL_ID_10 = 10,
  SL_HAL_IADC_CHANNEL_ID_11 = 11,
  SL_HAL_IADC_CHANNEL_ID_12 = 12,
  SL_HAL_IADC_CHANNEL_ID_13 = 13,
  SL_HAL_IADC_CHANNEL_ID_14 = 14,
  SL_HAL_IADC_CHANNEL_ID_15 = 15,
  SL_HAL_IADC_CHANNEL_ID_MAX = 16
};

/// IADC Commands.
SL_ENUM_GENERIC(sl_hal_iadc_cmd_t, uint32_t) {
  SL_HAL_IADC_CMD_START_SINGLE  = IADC_CMD_SINGLESTART,   ///< Start single queue.
  SL_HAL_IADC_CMD_STOP_SINGLE   = IADC_CMD_SINGLESTOP,    ///< Stop single queue.
  SL_HAL_IADC_CMD_START_SCAN    = IADC_CMD_SCANSTART,     ///< Start scan queue.
  SL_HAL_IADC_CMD_STOP_SCAN     = IADC_CMD_SCANSTOP,      ///< Stop scan queue.
  SL_HAL_IADC_CMD_ENABLE_TIMER  = IADC_CMD_TIMEREN,       ///< Enable Timer.
  SL_HAL_IADC_CMD_DISABLE_TIMER = IADC_CMD_TIMERDIS       ///< Disable Timer.
};

/// IADC Configuration.
SL_ENUM(sl_hal_iadc_config_adc_mode_t) {
  SL_HAL_IADC_CFG_ADC_MODE_NORMAL         = _IADC_CFG_ADCMODE_NORMAL,        ///< Normal mode.
#if defined(_IADC_CFG_ADCMODE_HIGHSPEED)
  SL_HAL_IADC_CFG_ADC_MODE_HIGH_SPEED     = _IADC_CFG_ADCMODE_HIGHSPEED,     ///< High Speed mode.
#endif
#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
  SL_HAL_IADC_CFG_ADC_MODE_HIGH_ACCURACY  = _IADC_CFG_ADCMODE_HIGHACCURACY   ///< High Accuracy mode.
#endif
};

/// IADC Over sampling rate for high speed.
SL_ENUM(sl_hal_iadc_config_osr_high_speed_t) {
  SL_HAL_IADC_OSR_HIGH_SPEED_2X   = _IADC_CFG_OSRHS_HISPD2,    ///< High speed oversampling of 4x.
  SL_HAL_IADC_OSR_HIGH_SPEED_4X   = _IADC_CFG_OSRHS_HISPD4,    ///< High speed oversampling of 4x.
  SL_HAL_IADC_OSR_HIGH_SPEED_8X   = _IADC_CFG_OSRHS_HISPD8,    ///< High speed oversampling of 8x.
  SL_HAL_IADC_OSR_HIGH_SPEED_16X  = _IADC_CFG_OSRHS_HISPD16,   ///< High speed oversampling of 16x.
  SL_HAL_IADC_OSR_HIGH_SPEED_32X  = _IADC_CFG_OSRHS_HISPD32,   ///< High speed oversampling of 32x.
  SL_HAL_IADC_OSR_HIGH_SPEED_64X  = _IADC_CFG_OSRHS_HISPD64    ///< High speed oversampling of 64x.
};

#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
/// IADC Over sampling rate for high accuracy.
SL_ENUM(sl_hal_iadc_config_osr_high_accuracy_t) {
  SL_HAL_IADC_OSR_HIGH_ACCURACY_16X   = _IADC_CFG_OSRHA_HIACC16,    ///< High accuracy oversampling of 16x.
  SL_HAL_IADC_OSR_HIGH_ACCURACY_32X   = _IADC_CFG_OSRHA_HIACC32,    ///< High accuracy oversampling of 32x.
  SL_HAL_IADC_OSR_HIGH_ACCURACY_64X   = _IADC_CFG_OSRHA_HIACC64,    ///< High accuracy oversampling of 64x.
  SL_HAL_IADC_OSR_HIGH_ACCURACY_92X   = _IADC_CFG_OSRHA_HIACC92,    ///< High accuracy oversampling of 92x.
  SL_HAL_IADC_OSR_HIGH_ACCURACY_128X  = _IADC_CFG_OSRHA_HIACC128,   ///< High accuracy oversampling of 128x.
  SL_HAL_IADC_OSR_HIGH_ACCURACY_256X  = _IADC_CFG_OSRHA_HIACC256    ///< High accuracy oversampling of 256x.
};
#endif

/// IADC Analog Gain.
SL_ENUM(sl_hal_iadc_analog_gain_t) {
#if defined(_IADC_CFG_ANALOGGAIN_ANAGAIN0P25)
  SL_HAL_IADC_ANALOG_GAIN_0P25 = _IADC_CFG_ANALOGGAIN_ANAGAIN0P25,   ///< Analog gain of 0.25x.
#endif
  SL_HAL_IADC_ANALOG_GAIN_0P5  = _IADC_CFG_ANALOGGAIN_ANAGAIN0P5,    ///< Analog gain of 0.5x.
  SL_HAL_IADC_ANALOG_GAIN_1    = _IADC_CFG_ANALOGGAIN_ANAGAIN1,      ///< Analog gain of 1x.
  SL_HAL_IADC_ANALOG_GAIN_2    = _IADC_CFG_ANALOGGAIN_ANAGAIN2,      ///< Analog gain of 2x.
  SL_HAL_IADC_ANALOG_GAIN_3    = _IADC_CFG_ANALOGGAIN_ANAGAIN3,      ///< Analog gain of 3x.
  SL_HAL_IADC_ANALOG_GAIN_4    = _IADC_CFG_ANALOGGAIN_ANAGAIN4       ///< Analog gain of 4x.
};

/// IADC Two's complement results.
SL_ENUM(sl_hal_iadc_config_twos_comp_t) {
  SL_HAL_IADC_TWOS_COMP_AUTO     = _IADC_CFG_TWOSCOMPL_AUTO,            ///< Automatic. Single ended => Unipolar, Differential => Bipolar.
  SL_HAL_IADC_TWOS_COMP_UNIPOLAR = _IADC_CFG_TWOSCOMPL_FORCEUNIPOLAR,   ///< All results in unipolar format. Negative diff input gives 0 as result.
  SL_HAL_IADC_TWOS_COMP_BIPOLAR  = _IADC_CFG_TWOSCOMPL_FORCEBIPOLAR     ///< All results in bipolar (2's complement) format. Half range for SE.
};

/// IADC trigger.
SL_ENUM(sl_hal_iadc_trigger_t) {
  SL_HAL_IADC_TRIGGER_IMMEDIATE  = _IADC_TRIGGER_SCANTRIGSEL_IMMEDIATE,   ///< Start single/scan queue immediately.
  SL_HAL_IADC_TRIGGER_TIMER      = _IADC_TRIGGER_SCANTRIGSEL_TIMER,       ///< Timer starts single/scan queue.
  SL_HAL_IADC_TRIGGER_PRSCLKGRP  = _IADC_TRIGGER_SCANTRIGSEL_PRSCLKGRP,   ///< PRS0 from timer in same clock group starts single/scan queue.
  SL_HAL_IADC_TRIGGER_PRSPOS     = _IADC_TRIGGER_SCANTRIGSEL_PRSPOS,      ///< PRS0 positive edge starts single/scan queue.
  SL_HAL_IADC_TRIGGER_PRSNEG     = _IADC_TRIGGER_SCANTRIGSEL_PRSNEG,      ///< PRS0 negative edge starts single/scan queue.
#if defined(_IADC_TRIGGER_SCANTRIGSEL_LESENSE)
  SL_HAL_IADC_TRIGGER_LESENSE    = _IADC_TRIGGER_SCANTRIGSEL_LESENSE      ///< LESENSE starts scan queue.
#endif
};

/// IADC trigger action.
SL_ENUM(sl_hal_iadc_trigger_action_t) {
  SL_HAL_IADC_TRIGGER_ACTION_ONCE        = _IADC_TRIGGER_SCANTRIGACTION_ONCE,         ///< Convert single/scan queue once per trigger.
  SL_HAL_IADC_TRIGGER_ACTION_CONTINUOUS  = _IADC_TRIGGER_SCANTRIGACTION_CONTINUOUS    ///< Convert single/scan queue continuously.
};

/// IADC data valid level before requesting DMA transfer.
SL_ENUM(sl_hal_iadc_fifo_config_dvl_t) {
  SL_HAL_IADC_DATA_VALID_1  = _IADC_SCANFIFOCFG_DVL_VALID1,   ///< Data valid level is 1 before requesting DMA transfer.
  SL_HAL_IADC_DATA_VALID_2  = _IADC_SCANFIFOCFG_DVL_VALID2,   ///< Data valid level is 2 before requesting DMA transfer.
  SL_HAL_IADC_DATA_VALID_3  = _IADC_SCANFIFOCFG_DVL_VALID3,   ///< Data valid level is 3 before requesting DMA transfer.
  SL_HAL_IADC_DATA_VALID_4  = _IADC_SCANFIFOCFG_DVL_VALID4,   ///< Data valid level is 4 before requesting DMA transfer.

#if defined(_IADC_SCANFIFOCFG_DVL_VALID7)
  SL_HAL_IADC_DATA_VALID_5  = _IADC_SCANFIFOCFG_DVL_VALID5,   ///< Data valid level is 5 before requesting DMA transfer.
  SL_HAL_IADC_DATA_VALID_6  = _IADC_SCANFIFOCFG_DVL_VALID6,   ///< Data valid level is 6 before requesting DMA transfer.
  SL_HAL_IADC_DATA_VALID_7  = _IADC_SCANFIFOCFG_DVL_VALID7,   ///< Data valid level is 7 before requesting DMA transfer.
#endif
#if defined(_IADC_SCANFIFOCFG_DVL_VALID8)
  SL_HAL_IADC_DATA_VALID_8  = _IADC_SCANFIFOCFG_DVL_VALID8    ///< Data valid level is 8 before requesting DMA transfer.
#endif
};

#if defined(_IADC_CFG_DIGAVG_MASK)
/// IADC digital averaging function.
SL_ENUM(sl_hal_iadc_digital_averaging_t){
  SL_HAL_IADC_AVERAGE_1  = _IADC_CFG_DIGAVG_AVG1,   ///< Average over 1 sample (no averaging).
  SL_HAL_IADC_AVERAGE_2  = _IADC_CFG_DIGAVG_AVG2,   ///< Average over 2 sample.
  SL_HAL_IADC_AVERAGE_4  = _IADC_CFG_DIGAVG_AVG4,   ///< Average over 4 sample.
  SL_HAL_IADC_AVERAGE_8  = _IADC_CFG_DIGAVG_AVG8,   ///< Average over 8 sample.
  SL_HAL_IADC_AVERAGE_16 = _IADC_CFG_DIGAVG_AVG16   ///< Average over 16 sample.
};
#endif

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// IADC scan table entry structure.
typedef struct {
  sl_hal_iadc_negative_port_input_t  negative_port;     ///< Port input for the negative side of the ADC.
  uint8_t                            negative_pin;      ///< Negative pin number.
  sl_hal_iadc_positive_port_input_t  positive_port;     ///< Port input for the positive side of the ADC.
  uint8_t                            positive_pin;      ///< Positive pin number.
  uint8_t                            config_id;         ///< Configuration id.
  bool                               compare;           ///< Perform digital window comparison on the result from this entry.
  bool                               include_in_scan;   ///< Include this scan table entry in scan operation.
} sl_hal_iadc_scan_table_entry_t;

/// IADC config structure.
typedef struct {
  sl_hal_iadc_config_adc_mode_t            adc_mode;            ///< IADC mode; Normal, High speed or High Accuracy.
  sl_hal_iadc_config_osr_high_speed_t      osr_high_speed;      ///< Over sampling ratio for High Speed and Normal modes.
#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
  sl_hal_iadc_config_osr_high_accuracy_t   osr_high_accuracy;   ///< Over sampling ratio for High Accuracy mode.
#endif
  sl_hal_iadc_analog_gain_t                analog_gain;         ///< Analog gain.
  sl_hal_iadc_voltage_reference_t          reference;           ///< Reference selection.
  sl_hal_iadc_config_twos_comp_t           twos_complement;     ///< Two's complement reporting.
  uint32_t                                 adc_clk_prescale;    ///< ADC_CLK divider (prescale+1).
  uint32_t                                 vref;                ///< Vref magnitude expressed in millivolts.
#if defined(_IADC_CFG_DIGAVG_MASK)
  sl_hal_iadc_digital_averaging_t          dig_avg;             ///< Digital average mode.
#endif
} sl_hal_iadc_config_t;

/// IADC scan table structure.
typedef struct {
  sl_hal_iadc_scan_table_entry_t   entries[IADC0_ENTRIES];
} sl_hal_iadc_scan_table_t;

/// IADC init structure, common for single conversion and scan sequence.
typedef struct {
  bool                          iadc_clk_suspend0;          ///< Suspend IADC_CLK when in scan mode until PRS trigger.
  bool                          iadc_clk_suspend1;          ///< Suspend IADC_CLK when in single mode until PRS trigger.
  bool                          debug_halt;                 ///< Halt IADC during debug mode.
  sl_hal_iadc_warmup_t          warmup;                     ///< IADC warmup mode.
  uint8_t                       timebase;                   ///< IADC clock cycles (timebase+1) corresponding to 1us.
                                                            ///< Used as time reference for IADC delays, e.g. warmup.
                                                            ///< If the user sets timebase to 0, then sl_hal_iadc_init() will
                                                            ///< calculate the timebase using the currently defined CMU
                                                            ///< clock setting for the IADC.
  uint8_t                       src_clk_prescale;           ///< User requested source clock divider (prescale+1) which
                                                            ///< will be used if the calculated prescaler value is less.
  uint16_t                      timer_cycles;               ///< Number of ADC_CLK cycles per TIMER event.
  uint16_t                      greater_than_equal_thres;   ///< Digital window comparator greater-than or equal threshold.
  uint16_t                      less_than_equal_thres;      ///< Digital window comparator less-than or equal threshold.
  sl_hal_iadc_config_t          configs[IADC0_CONFIGNUM];   ///< IADC configurations.
} sl_hal_iadc_init_t;

/// IADC scan init structure.
typedef struct {
  sl_hal_iadc_alignment_t            alignment;          ///< Alignment of data in FIFO.
  bool                               show_id;            ///< Tag FIFO entry with scan table entry id.
  sl_hal_iadc_fifo_config_dvl_t      data_valid_level;   ///< Data valid level before requesting DMA transfer.
  bool                               fifo_dma_wakeup;    ///< Wake-up DMA when FIFO reaches data valid level.
  sl_hal_iadc_trigger_t              trigger_select;     ///< Trigger selection.
  sl_hal_iadc_trigger_action_t       trigger_action;     ///< Trigger action.
} sl_hal_iadc_init_scan_t;

/// IADC single init structure.
typedef struct {
  sl_hal_iadc_alignment_t            alignment;          ///< Alignment of data in FIFO.
  bool                               show_id;            ///< Tag FIFO entry with single indicator (0x20).
  sl_hal_iadc_fifo_config_dvl_t      data_valid_level;   ///< Data valid level before requesting DMA transfer.
  bool                               fifo_dma_wakeup;    ///< Wake-up DMA when FIFO reaches data valid level.
  sl_hal_iadc_trigger_t              trigger_select;     ///< Trigger selection.
  sl_hal_iadc_trigger_action_t       trigger_action;     ///< Trigger action.
  bool                               single_tailgate;    ///< If true, wait until end of SCAN queue before single queue warmup and conversion.
} sl_hal_iadc_init_single_t;

/// IADC single input selection structure.
typedef struct {
  sl_hal_iadc_negative_port_input_t    negative_port;            ///< Port input for the negative side of the ADC.
  uint8_t                              negative_pin;             ///< Negative pin input.
  sl_hal_iadc_positive_port_input_t    positive_port;            ///< Port input for the positive side of the ADC.
  uint8_t                              positive_pin;             ///< Positive pin input.
  uint8_t                              single_input_config_id;   ///< Configuration id.
  bool                                 compare;                  ///< Perform digital window comparison on the result from this entry.
} sl_hal_iadc_single_input_t;

/// Structure holding IADC result, including data and ID.
typedef struct {
  uint32_t data;   ///< ADC sample data.
  uint8_t  id;     ///< ID of FIFO entry; Scan table entry id or single indicator (0x20).
} sl_hal_iadc_result_t;

// Default IADC config for scan table.
#define SL_HAL_IADC_SCANTABLE_DEFAULT     \
  {                                       \
    {                                     \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT, \
      SL_HAL_IADC_SCANTABLEENTRY_DEFAULT  \
    }                                     \
  }

/// Default config for IADC scan table entry structure.
#define SL_HAL_IADC_SCANTABLEENTRY_DEFAULT                          \
  {                                                                 \
    SL_HAL_IADC_NEG_PORT_INPUT_GND,  /* Negative input port GND. */ \
    SL_HAL_IADC_NEG_PIN_INPUT_GND,   /* Negative input pin GND. */  \
    SL_HAL_IADC_POS_PORT_INPUT_GND,  /* Positive input port GND. */ \
    0,                               /* Positive input pin GND*/    \
    0,                               /* Config 0. */                \
    false,                           /* Do not compare results. */  \
    false,                           /* Do not include in scan. */  \
  }

/// Default IADC sructure for all configs.
#define SL_HAL_IADC_ALL_CONFIG  \
  {                             \
    SL_HAL_IADC_CONFIG_DEFAULT, \
    SL_HAL_IADC_CONFIG_DEFAULT  \
  }
/// Default config for IADC init structure.
#define SL_HAL_IADC_INIT_DEFAULT                                                 \
  {                                                                              \
    false,                            /* IADC clock not disabled on PRS0. */     \
    false,                            /* IADC clock not disabled on PRS1. */     \
    false,                            /* Do not halt during debug. */            \
    SL_HAL_IADC_WARMUP_NORMAL,        /* IADC shutdown after each conversion. */ \
    0,                                /* Calculate timebase. */                  \
    0,                                /* Max IADC clock rate. */                 \
    _IADC_TIMER_TIMER_DEFAULT,        /* Use HW default value. */                \
    _IADC_CMPTHR_ADGT_DEFAULT,        /* Use HW default value. */                \
    _IADC_CMPTHR_ADLT_DEFAULT,        /* Use HW default value. */                \
    SL_HAL_IADC_ALL_CONFIG            /* IADC default configs. */                \
  }

#if defined(_IADC_CFG_DIGAVG_MASK)
#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
/// Default IADC config structure.
#define SL_HAL_IADC_CONFIG_DEFAULT                                                \
  {                                                                               \
    SL_HAL_IADC_CFG_ADC_MODE_NORMAL,      /* Normal mode for IADC. */             \
    SL_HAL_IADC_OSR_HIGH_SPEED_2X,        /* 2x high speed over sampling. */      \
    SL_HAL_IADC_OSR_HIGH_ACCURACY_92X,    /* 92x high accuracy over sampling. */  \
    SL_HAL_IADC_ANALOG_GAIN_1,            /* 1x analog gain. */                   \
    SL_HAL_IADC_REFERENCE_VREFINT_1V2,    /* Internal 1.2V band gap reference. */ \
    SL_HAL_IADC_TWOS_COMP_AUTO,           /* Automatic Two's Complement. */       \
    0,                                    /* Max IADC analog clock rate. */       \
    SL_HAL_IADC_DEFAULT_VREF,             /* Vref expressed in millivolts. */     \
    SL_HAL_IADC_AVERAGE_1                 /* No averaging. */                     \
  }
#else
/// Default IADC config structure.
#define SL_HAL_IADC_CONFIG_DEFAULT                                                 \
  {                                                                                \
    SL_HAL_IADC_CFG_ADC_MODE_NORMAL,       /* Normal mode for IADC.             */ \
    SL_HAL_IADC_OSR_HIGH_SPEED_2X,         /* 2x high speed over sampling.      */ \
    SL_HAL_IADC_ANALOG_GAIN_1,             /* 1x analog gain.                   */ \
    SL_HAL_IADC_REFERENCE_VREFINT_1V2,     /* Internal 1.2V band gap reference. */ \
    SL_HAL_IADC_TWOS_COMP_AUTO,            /* Automatic Two's Complement.       */ \
    0,                                     /* Max IADC analog clock rate.       */ \
    SL_HAL_IADC_DEFAULT_VREF,              /* Vref expressed in millivolts.     */ \
    SL_HAL_IADC_AVERAGE_1                  /* No averaging.                     */ \
  }
#endif
#else
#if defined(_IADC_CFG_ADCMODE_HIGHACCURACY)
/// Default IADC config structure.
#define SL_HAL_IADC_CONFIG_DEFAULT                                                 \
  {                                                                                \
    SL_HAL_IADC_CFG_ADC_MODE_NORMAL,       /* Normal mode for IADC. */             \
    SL_HAL_IADC_OSR_HIGH_SPEED_2X,         /* 2x high speed over sampling. */      \
    SL_HAL_IADC_OSR_HIGH_ACCURACY_92X,     /* 92x high speed over sampling. */     \
    SL_HAL_IADC_ANALOG_GAIN_1,             /* 1x analog gain. */                   \
    SL_HAL_IADC_REFERENCE_VREFINT_1V2,     /* Internal 1.2V band gap reference. */ \
    SL_HAL_IADC_TWOS_COMP_AUTO,            /* Automatic Two's Complement. */       \
    0,                                     /* Max IADC analog clock rate. */       \
    SL_HAL_IADC_DEFAULT_VREF               /* Vref expressed in millivolts. */     \
  }
#else
/// Default IADC config structure.
#define SL_HAL_IADC_CONFIG_DEFAULT                                                \
  {                                                                               \
    SL_HAL_IADC_CFG_ADC_MODE_NORMAL,      /* Normal mode for IADC. */             \
    SL_HAL_IADC_OSR_HIGH_SPEED_2X,        /* 2x high speed over sampling. */      \
    SL_HAL_IADC_ANALOG_GAIN_1,            /* 1x analog gain. */                   \
    SL_HAL_IADC_REFERENCE_VREFINT_1V2,    /* Internal 1.2V band gap reference. */ \
    SL_HAL_IADC_TWOS_COMP_AUTO,           /* Automatic Two's Complement. */       \
    0,                                    /* Max IADC analog clock rate. */       \
    SL_HAL_IADC_DEFAULT_VREF              /* Vref expressed in millivolts. */     \
  }
#endif
#endif

/// Default config for IADC scan init structure.
#define SL_HAL_IADC_INITSCAN_DEFAULT                                                        \
  {                                                                                         \
    SL_HAL_IADC_ALIGNMENT_RIGHT_12,              /* Results 12-bit right aligned. */        \
    false,                                       /* Do not show ID in result. */            \
    SL_HAL_IADC_DATA_VALID_4,                    /* Use HW default value. */                \
    false,                                       /* Do not wake up DMA on scan FIFO DVL. */ \
    SL_HAL_IADC_TRIGGER_IMMEDIATE,               /* Start scan immediately on trigger. */   \
    SL_HAL_IADC_TRIGGER_ACTION_ONCE,             /* Convert once on scan trigger. */        \
  }

/// Default config for IADC single init structure.
#define SL_HAL_IADC_INITSINGLE_DEFAULT                                                      \
  {                                                                                         \
    SL_HAL_IADC_ALIGNMENT_RIGHT_12,            /* Results 12-bit right aligned. */          \
    false,                                     /* Do not show ID in result. */              \
    SL_HAL_IADC_DATA_VALID_4,                  /* Use HW default value. */                  \
    false,                                     /* Do not wake up DMA on single FIFO DVL. */ \
    SL_HAL_IADC_TRIGGER_IMMEDIATE,             /* Start single immediately on trigger. */   \
    SL_HAL_IADC_TRIGGER_ACTION_ONCE,           /* Convert once on single trigger. */        \
    false,                                     /* No tailgating. */                         \
  }

/// Default config for IADC single input structure.
#define SL_HAL_IADC_SINGLEINPUT_DEFAULT                                 \
  {                                                                     \
    SL_HAL_IADC_NEG_PORT_INPUT_GND,      /* Negative input port GND. */ \
    SL_HAL_IADC_NEG_PIN_INPUT_GND,       /* Negative input pin GND. */  \
    SL_HAL_IADC_POS_PORT_INPUT_GND,      /* Positive input port GND. */ \
    0,                                   /* Positive input port GND. */ \
    0,                                   /* Config 0. */                \
    false                                /* Do not compare results. */  \
  }

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize IADC.
 *
 * @details
 *   Initializes common parts for both single conversion and scan sequence.
 *   In addition, single and/or scan control configuration must be done,
 *   please refer to @ref sl_hal_iadc_init_single() and
 *   @ref sl_hal_iadc_init_scan() respectively.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] init
 *   Pointer to IADC initialization structure.
 *
 * @param[in] src_clk_freq
 *   CLK_SRC_ADC frequency wanted. The frequency will
 *   automatically be adjusted to be within valid range
 *   according to reference manual.
 *
 * @note
 *    This function will stop any ongoing conversions.
 ******************************************************************************/
void sl_hal_iadc_init(IADC_TypeDef *iadc,
                      const sl_hal_iadc_init_t *init,
                      uint32_t src_clk_freq);

/***************************************************************************//**
 * @brief
 *   Calculate gain and offset.
 *
 * @details
 *   This function calculates the gain and offset values for the IADC
 *   based on the provided configuration.
 *
 * @param[in] iadc
 *    Pointer to IADC peripheral register block.
 *
 * @param[in] init
 *    Pointer to IADC initialization structure.
 *
 * @param[in] adc_mode
 *    Mode for IADC config.
 *
 * @param[in] config
 *    IADC config number.
 *
 * @param[in] adc_clk_prescale
 *    IADC clock prescaler.
 ******************************************************************************/
void sl_hal_iadc_calculate_gain_offset(IADC_TypeDef *iadc,
                                       const sl_hal_iadc_init_t *init,
                                       sl_hal_iadc_config_adc_mode_t adc_mode,
                                       uint8_t config,
                                       uint32_t adc_clk_prescale);

/***************************************************************************//**
 * @brief
 *   Reset IADC to same state as after a HW reset.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 ******************************************************************************/
void sl_hal_iadc_reset(IADC_TypeDef *iadc);

/***************************************************************************//**
 * @brief
 *   Initialize IADC scan sequence.
 *
 * @details
 *   This function will configure scan mode and set up entries in the
 *   scan table. The scan table mask can be updated by calling
 *   IADC_updateScanMask.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] init
 *   Pointer to IADC initialization structure.
 *
 * @param[in] scan_table
 *   Pointer to IADC scan table structure.
 *
 * @note
 *   This function will stop any ongoing conversions.
 *   If an even numbered pin is selected for the positive input, the
 *   negative input must use an odd numbered pin and vice versa.
 ******************************************************************************/
void sl_hal_iadc_init_scan(IADC_TypeDef *iadc,
                           const sl_hal_iadc_init_scan_t *init,
                           const sl_hal_iadc_scan_table_t *scan_table);

/***************************************************************************//**
 * @brief
 *   Add/update entry in scan table.
 *
 * @details
 *   This function will update or add an entry in the scan table with
 *   a specific ID.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] id
 *   ID of scan table entry to add.
 *
 * @param[in] entry
 *   Pointer to scan table entry structure.
 *
 * @note This function will stop any ongoing conversions.
 ******************************************************************************/
void sl_hal_iadc_update_scan_entry(IADC_TypeDef *iadc,
                                   uint8_t id,
                                   sl_hal_iadc_scan_table_entry_t *entry);

/***************************************************************************//**
 * @brief
 *   Set mask of IADC scan table entries to include in scan using scan mask.
 *
 * @details
 *   Set mask of scan table entries to include in next scan. This function
 *   can be called while scan conversions are ongoing, but the new scan
 *   mask will take effect once the ongoing scan is completed.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] mask
 *   Mask of scan table entries to include in scan.
 ******************************************************************************/
void sl_hal_iadc_set_scan_mask(IADC_TypeDef *iadc,
                               uint32_t mask);

/***************************************************************************//**
 * @brief
 *   Set mask of IADC scan table entries to include in scan using scan table.
 *
 * @details
 *   Set mask of scan table entries to include in next scan. This function
 *   can be called while scan conversions are ongoing, but the new scan
 *   mask will take effect once the ongoing scan is completed.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] scan_table
 *   Pointer to IADC scan table structure.
 ******************************************************************************/
void sl_hal_iadc_set_scan_mask_multiple_entries(IADC_TypeDef *iadc,
                                                const sl_hal_iadc_scan_table_t *scan_table);

/***************************************************************************//**
 * @brief
 *   Initialize single IADC conversion.
 *
 * @details
 *   This function will initialize the single conversion and configure the
 *   single input selection.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] init
 *   Pointer to IADC single initialization structure.
 *
 * @param[in] input
 *   Pointer to IADC single input selection initialization
 *   structure.
 *
 * @note
 *   This function will stop any ongoing conversions.
 *   If an even numbered pin is selected for the positive input, the
 *   negative input must use an odd numbered pin and vice versa.
 ******************************************************************************/
void sl_hal_iadc_init_single(IADC_TypeDef *iadc,
                             const sl_hal_iadc_init_single_t *init,
                             const sl_hal_iadc_single_input_t *input);

/***************************************************************************//**
 * @brief
 *   Update IADC single input selection.
 *
 * @details This function updates the single input selection. The function can be
 *          called while single and/or scan conversions are ongoing and the new
 *          input configuration will take place on the next single conversion.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] input
 *   Pointer to single input selection structure.
 *
 * @note
 *   If an even numbered pin is selected for the positive input, the
 *   negative input must use an odd numbered pin and vice versa.
 ******************************************************************************/
void sl_hal_iadc_update_single_input(IADC_TypeDef *iadc,
                                     const sl_hal_iadc_single_input_t *input);

/***************************************************************************//**
 * @brief
 *   Calculate prescaler for CLK_SRC_ADC high speed clock.
 *
 * @details
 *   The IADC high speed clock is given by:
 *   CLK_SRC_ADC / (scr_clk_prescaler + 1).
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] src_clk_freq
 *   CLK_SRC_ADC frequency wanted. The frequency will automatically
 *   be adjusted to be within valid range according to reference manual.
 *
 * @param[in] cmu_clk_freq
 *   Frequency in Hz of reference CLK_CMU_ADC.
 *
 * @return Divider value to use for IADC in order to achieve a high speed
 *         clock value.
 ******************************************************************************/
uint8_t sl_hal_iadc_calculate_src_clk_prescale(IADC_TypeDef *iadc,
                                               uint32_t src_clk_freq,
                                               uint32_t cmu_clk_freq);

/***************************************************************************//**
 * @brief
 *   Calculate prescaler for ADC_CLK clock.
 *
 * @details The ADC_CLK is given by: CLK_SRC_ADC / (adcClkprescale + 1).
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] adc_clk_freq
 *   ADC_CLK frequency wanted. The frequency will automatically
 *   be adjusted to be within valid range according to reference manual.
 *
 * @param[in] cmu_clk_freq
 *   Frequency in Hz of CLK_CMU_ADC.
 *
 * @param[in] adc_mode
 *   Mode for IADC config.
 *
 * @param[in] scr_clk_prescaler
 *   Precaler setting for ADC_CLK.
 *
 * @return
 *   Divider value to use for IADC in order to achieve a ADC_CLK frequency.
 ******************************************************************************/
uint32_t sl_hal_iadc_calculate_adc_clk_prescale(IADC_TypeDef *iadc,
                                                uint32_t adc_clk_freq,
                                                uint32_t cmu_clk_freq,
                                                sl_hal_iadc_config_adc_mode_t adc_mode,
                                                uint8_t scr_clk_prescaler);

/***************************************************************************//**
 * @brief
 *   Calculate timebase value in order to get a timebase providing at least 1us.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] src_clk_freq
 *   Frequency in Hz of reference CLK_SRC_ADC clock.
 *
 * @return
 *   Timebase value to use for IADC in order to achieve at least 1 us.
 ******************************************************************************/
uint8_t sl_hal_iadc_calculate_timebase(IADC_TypeDef *iadc,
                                       uint32_t src_clk_freq);

/***************************************************************************//**
 * @brief
 *   Read most recent single conversion result. The result struct includes both
 *   the data and the ID (0x20) if show_id was set when initializing single mode.
 *   Calling this function will not affect the state of the single data FIFO.
 *
 * @note
 *   Check data valid flag before calling this function.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   Single conversion result struct holding data and id.
 ******************************************************************************/
sl_hal_iadc_result_t sl_hal_iadc_read_single_result(IADC_TypeDef *iadc);

/***************************************************************************//**
 * @brief
 *   Pull result from single data FIFO. The result struct includes both the data
 * and the ID (0x20) if show_id was set when initializing single mode.
 *
 * @note
 *   Check data valid flag before calling this function.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   Single conversion result struct holding data and id.
 ******************************************************************************/
sl_hal_iadc_result_t sl_hal_iadc_read_single_fifo_result(IADC_TypeDef *iadc);

/***************************************************************************//**
 * @brief
 *   Read most recent scan conversion result. The result struct includes both
 *   the data and the ID (0x20) if show_id was set when initializing scan entry.
 *   Calling this function will not affect the state of the scan data FIFO.
 *
 * @note
 *   Check data valid flag before calling this function.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   Scan conversion result struct holding data and id.
 ******************************************************************************/
sl_hal_iadc_result_t sl_hal_iadc_read_scan_result(IADC_TypeDef*iadc);

/***************************************************************************//**
 * @brief
 *   Pull result from scan data FIFO. The result struct includes both the data
 *   and the ID (0x20) if show_id was set when initializing scan entry.
 *
 * @note
 *   Check data valid flag before calling this function.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   Scan conversion result struct holding data and id.
 ******************************************************************************/
sl_hal_iadc_result_t sl_hal_iadc_pull_scan_fifo_result(IADC_TypeDef *iadc);

/***************************************************************************//**
 * @brief
 *   Get reference voltage selection.
 *
 * @param[in] reference
 *   IADC Reference selection.
 *
 * @return
 *   IADC reference voltage in millivolts.
 ******************************************************************************/
uint32_t sl_hal_iadc_get_reference_voltage(sl_hal_iadc_voltage_reference_t reference);

/***************************************************************************//**
 * @brief
 *   Enable the IADC.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_iadc_enable(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  iadc->EN_SET = IADC_EN_EN;
}

/***************************************************************************//**
 * @brief
 *   Wait for ongoing sync of register(s) to the low-frequency domain to complete.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 ******************************************************************************/
#if defined(_IADC_STATUS_SYNCBUSY_MASK)
__INLINE void sl_hal_iadc_wait_sync(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  while ((iadc->EN & _IADC_EN_EN_MASK) && (iadc->STATUS & _IADC_STATUS_SYNCBUSY_MASK)) {
    // Wait for any pending previous write operation to have been completed
    // in the low-frequency domain.
  }
}
#endif

#if defined(_IADC_EN_DISABLING_MASK)
/***************************************************************************//**
 * @brief
 *   Wait for disabling to finish.
 *
 * @param[in] iadc
 *   A pointer to the ADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_iadc_wait_ready(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  while (iadc->EN & _IADC_EN_DISABLING_MASK) {
    // Wait for disabling to finish.
  }
}
#endif

/***************************************************************************//**
 * @brief
 *   Disable the IADC.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_iadc_disable(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

#if defined(_IADC_STATUS_SYNCBUSY_MASK)
  sl_hal_iadc_wait_sync(iadc);
#endif
  iadc->EN_CLR = IADC_EN_EN;
#if defined(_IADC_EN_DISABLING_MASK)
  sl_hal_iadc_wait_ready(iadc);
#endif
}

/***************************************************************************//**
 * @brief
 *   Start the IADC scan queue.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 *
 * @note Enables
 *   The scan sequence and will start a scan if using the immediate
 *   trigger mode.
 ******************************************************************************/
__INLINE void sl_hal_iadc_start_scan(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  // Make sure the module is enabled.
  EFM_ASSERT((iadc->EN & _IADC_EN_EN_MASK) == _IADC_EN_EN_MASK);

  iadc->CMD_SET = IADC_CMD_SCANSTART;

#if defined(_IADC_EN_DISABLING_MASK)
  sl_hal_iadc_wait_ready(iadc);
#endif
}

/***************************************************************************//**
 * @brief
 *   Stop the IADC scan queue.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_iadc_stop_scan(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  // Make sure the module is enabled.
  EFM_ASSERT((iadc->EN & _IADC_EN_EN_MASK) == _IADC_EN_EN_MASK);

  iadc->CMD_SET = IADC_CMD_SCANSTOP;

#if defined(_IADC_EN_DISABLING_MASK)
  sl_hal_iadc_wait_ready(iadc);
#endif
}

/***************************************************************************//**
 * @brief
 *   Start the IADC single queue.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 *
 * @note
 *   Enables the scan sequence and will start a scan if using the immediate
 *   trigger mode.
 ******************************************************************************/
__INLINE void sl_hal_iadc_start_single(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  // Make sure the module is enabled.
  EFM_ASSERT((iadc->EN & _IADC_EN_EN_MASK) == _IADC_EN_EN_MASK);

  iadc->CMD_SET = IADC_CMD_SINGLESTART;

#if defined(_IADC_EN_DISABLING_MASK)
  sl_hal_iadc_wait_ready(iadc);
#endif
}

/***************************************************************************//**
 * @brief
 *   Stop the IADC single queue.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_iadc_stop_single(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  // Make sure the module is enabled.
  EFM_ASSERT((iadc->EN & _IADC_EN_EN_MASK) == _IADC_EN_EN_MASK);

  iadc->CMD_SET = IADC_CMD_SINGLESTOP;

#if defined(_IADC_EN_DISABLING_MASK)
  sl_hal_iadc_wait_ready(iadc);
#endif
}

/***************************************************************************//**
 * @brief
 *   Enable the IADC's local timer.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_iadc_enable_timer(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  // Make sure the module is enabled.
  EFM_ASSERT((iadc->EN & _IADC_EN_EN_MASK) == _IADC_EN_EN_MASK);

  iadc->CMD_SET = IADC_CMD_TIMEREN;

#if defined(_IADC_EN_DISABLING_MASK)
  sl_hal_iadc_wait_ready(iadc);
#endif
}

/***************************************************************************//**
 * @brief
 *   Disable the IADC's local timer and resets the counter.
 *
 * @param[in] iadc
 *   A pointer to the IADC peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_iadc_disable_timer(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));
  // Make sure the module is enabled.
  EFM_ASSERT((iadc->EN & _IADC_EN_EN_MASK) == _IADC_EN_EN_MASK);

  iadc->CMD_SET = IADC_CMD_TIMERDIS;

#if defined(_IADC_EN_DISABLING_MASK)
  sl_hal_iadc_wait_ready(iadc);
#endif
}

/***************************************************************************//**
 * @brief
 *   Get status bits of IADC.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   IADC status bits.
 ******************************************************************************/
__INLINE uint32_t sl_hal_iadc_get_status(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return iadc->STATUS;
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending IADC interrupts.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] flags
 *   Pending IADC interrupt source to clear. Use a bitwise logic
 *   OR combination of valid interrupt flags for the IADC
 *   module (IADC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_iadc_clear_interrupts(IADC_TypeDef *iadc,
                                           uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  iadc->IF_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Disable one or more IADC interrupts.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.

 *
 * @param[in] flags
 *   IADC interrupt sources to disable. Use a bitwise logic OR
 *   combination of valid interrupt flags for the IADC module
 *   (IADC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_iadc_disable_interrupts(IADC_TypeDef *iadc,
                                             uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  iadc->IEN_CLR = flags;
}

/***************************************************************************//**
 * @brief
 *   Enable one or more IADC interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. Consider using sl_hal_iadc_clear_interrupts()
 *   prior to enabling if such a pending interrupt should be ignored.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] flags
 *   IADC interrupt sources to enable. Use a bitwise logic OR
 *   combination of valid interrupt flags for the IADC
 *   module (IADC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_iadc_enable_interrupts(IADC_TypeDef *iadc,
                                            uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  iadc->IEN_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Get pending IADC interrupt flags.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   IADC interrupt sources pending. A bitwise logic OR combination of
 *   valid interrupt flags for the IADC module (IADC_IF_nnn).
 *
 * @note
 *   The event bits are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_iadc_get_pending_interrupts(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return iadc->IF;
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending IADC interrupt flags.
 *   Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @note
 *   Interrupt flags are not cleared by the use of this function.
 *
 * @return
 *   Pending and enabled IADC interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - The OR combination of enabled interrupt sources in IADCx_IEN_nnn
 *   register (IADCx_IEN_nnn).
 *   - The OR combination of valid interrupt flags of the IADC module
 *   (IADCx_IF_nnn).
 ******************************************************************************/
__INLINE uint32_t sl_hal_iadc_get_enable_pending_interrupts(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  uint32_t ien;

  // Store IADCx->IEN in temporary variable in order to define explicit order
  // of volatile accesses.
  ien = iadc->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return iadc->IF & ien;
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending IADC interrupts from SW.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] flags
 *   IADC interrupt sources to set to pending. Use a bitwise logic
 *   OR combination of valid interrupt flags for the IADC
 *   module (IADC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_iadc_set_pending_interrupts(IADC_TypeDef *iadc,
                                                 uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  iadc->IF_SET = flags;
}

/***************************************************************************//**
 * @brief
 *   Pull data from single data FIFO. If show_id was set when initializing
 *   single mode, the results will contain the ID (0x20).
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @note
 *   Check data valid flag before calling this function.
 *
 * @return
 *   Single conversion data.
 ******************************************************************************/
__INLINE uint32_t sl_hal_iadc_pull_single_fifo_data(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return iadc->SINGLEFIFODATA;
}

/***************************************************************************//**
 * @brief
 *   Read most recent single conversion data. If show_id was set when
 *   initializing single mode, the data will contain the ID (0x20). Calling
 *   this function will not affect the state of the single data FIFO.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @note
 *   Check data valid flag before calling this function.
 *
 * @return
 *   Single conversion data.
 ******************************************************************************/
__INLINE uint32_t sl_hal_iadc_read_single_data(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return iadc->SINGLEDATA;
}

/***************************************************************************//**
 * @brief
 *   Pull data from scan data FIFO. If show_id was set for the scan entry
 *   initialization, the data will contain the ID of the scan entry.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @note
 *   Check data valid flag before calling this function.
 *
 * @return
 *   Scan conversion data.
 ******************************************************************************/
__INLINE uint32_t sl_hal_iadc_pull_scan_fifo_data(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return iadc->SCANFIFODATA;
}

/***************************************************************************//**
 * @brief
 *   Read most recent scan conversion data. If show_id was set for the scan
 *   entry initialization, the data will contain the ID of the scan entry.
 *   Calling this function will not affect the state of the scan data FIFO.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @note
 *   Check data valid flag before calling this function.
 *
 * @return
 *   Scan conversion data.
 ******************************************************************************/
__INLINE uint32_t sl_hal_iadc_read_scan_data(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return iadc->SCANDATA;
}

/***************************************************************************//**
 * @brief
 *   Start/stop scan sequence, single conversion and/or timer.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @param[in] cmd
 *   Command to be performed.
 ******************************************************************************/
__INLINE void sl_hal_iadc_set_command(IADC_TypeDef *iadc,
                                      sl_hal_iadc_cmd_t cmd)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  iadc->CMD = cmd;
#if defined(_IADC_EN_DISABLING_MASK)
  sl_hal_iadc_wait_ready(iadc);
#endif
}

/***************************************************************************//**
 * @brief
 *   Get the scan mask currently used in the IADC.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   Mask of scan table entries currently included in scan.
 ******************************************************************************/
__INLINE uint32_t sl_hal_iadc_get_scan_mask(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return (iadc->STMASK) >> _IADC_STMASK_STMASK_SHIFT;
}

/***************************************************************************//**
 * @brief
 *   Get the number of elements in the IADC single FIFO.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   Number of elements in single FIFO.
 ******************************************************************************/
__INLINE uint8_t sl_hal_iadc_get_single_fifo_cnt(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return (uint8_t) ((iadc->SINGLEFIFOSTAT & _IADC_SINGLEFIFOSTAT_FIFOREADCNT_MASK)
                    >> _IADC_SINGLEFIFOSTAT_FIFOREADCNT_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Get the number of elements in the IADC scan FIFO.
 *
 * @param[in] iadc
 *   Pointer to IADC peripheral register block.
 *
 * @return
 *   Number of elements in scan FIFO.
 ******************************************************************************/
__INLINE uint8_t sl_hal_iadc_get_scan_fifo_cnt(IADC_TypeDef *iadc)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_IADC_REF_VALID(iadc));

  return (uint8_t) ((iadc->SCANFIFOSTAT & _IADC_SCANFIFOSTAT_FIFOREADCNT_MASK)
                    >> _IADC_SCANFIFOSTAT_FIFOREADCNT_SHIFT);
}

/***************************************************************************//**
 * @brief
 *   Convert the GPIO port/pin to IADC negative port input selection.
 *
 * @param[in] port
 *   GPIO port.
 *
 * @param[in] pin
 *   GPIO in.
 *
 * @return IADC negative port input selection.
 ******************************************************************************/
__INLINE sl_hal_iadc_negative_port_input_t sl_hal_iadc_port_pin_to_neg_port(const sl_gpio_t *gpio)
{
  uint32_t input = (uint32_t)gpio->port + _IADC_SCAN_PORTNEG_PORTA;

  return (sl_hal_iadc_negative_port_input_t) input;
}

/***************************************************************************//**
 * @brief
 *   Convert the GPIO port/pin to IADC positive port input selection.
 *
 * @param[in] port
 *   GPIO port.
 *
 * @param[in] pin
 *   GPIO in.
 *
 * @return IADC positive port input selection.
 ******************************************************************************/
__INLINE sl_hal_iadc_positive_port_input_t sl_hal_iadc_port_pin_to_pos_port(const sl_gpio_t *gpio)
{
  uint32_t input = (uint32_t) gpio->port + _IADC_SCAN_PORTPOS_PORTA;

  return (sl_hal_iadc_positive_port_input_t) input;
}

/** @} (end addtogroup iadc) */
#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/****************************************************************************//**
 * @addtogroup iadc IADC - Incremental ADC
 * @{
 *
 *@n @section iadc_intro Introduction
 *  The IADC provides high-accuracy analog-to-digital conversion with
 *  incremental/sigma-delta operation for improved resolution. It's designed for
 *  precision measurements and offers various operating modes.
 *
 *@n @section iadc_example Example
 *
 *  IADC Basic example:
 *  @code{.c}
 *  {
 *    // Enable bus clock for IADC0
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_IADC0);
 *
 *    // Initialize IADC
 *    sl_hal_iadc_init_t init = {
 *      .iadc_clk_suspend0 = false,
 *      .iadc_clk_suspend1 = false,
 *      .debug_halt = false,
 *      .warmup = SL_HAL_IADC_WARMUP_NORMAL,
 *      .timebase = 0,              // Will be calculated automatically
 *      .src_clk_prescale = 0,
 *      .timer_cycles = 1000        // Timer period in ADC_CLK cycles
 *    };
 *
 *    // IADC scan configuration
 *    sl_hal_iadc_init_scan_t scan_init = {
 *      .alignment = SL_HAL_IADC_ALIGNMENT_RIGHT_12,
 *      .show_id = true,
 *      .data_valid_level = SL_HAL_IADC_DATA_VALID_1,
 *      .fifo_dma_wakeup = false,
 *      .trigger_select = SL_HAL_IADC_TRIGGER_IMMEDIATE,
 *      .trigger_action = SL_HAL_IADC_TRIGGER_ACTION_ONCE
 *    };
 *
 *    // IADC default configuration
 *    sl_hal_iadc_config_t iadc_config = {
 *      .adc_mode = SL_HAL_IADC_CFG_ADC_MODE_NORMAL,
 *      .osr_high_speed = SL_HAL_IADC_OSR_HIGH_SPEED_16X,
 *      .analog_gain = SL_HAL_IADC_ANALOG_GAIN_1,
 *      .reference = SL_HAL_IADC_REFERENCE_VREFINT_1V2,
 *      .twos_complement = SL_HAL_IADC_TWOS_COMP_AUTO,
 *      .adc_clk_prescale = 0,
 *      .vref = SL_HAL_IADC_DEFAULT_VREF
 *    };
 *
 *    // Assign config to the init structure
 *    init.configs[0] = iadc_config;
 *
 *    // Configure scan table entry
 *    sl_hal_iadc_scan_table_entry_t entry = {
 *      .negative_port = SL_HAL_IADC_NEG_PORT_INPUT_GND,
 *      .negative_pin = 0,
 *      .positive_port = SL_HAL_IADC_POS_PORT_INPUT_PORT_A,
 *      .positive_pin = 0,
 *      .config_id = 0,
 *      .compare = false,
 *      .include_in_scan = true
 *    };
 *
 *    // Set the scan table entry
 *    sl_hal_iadc_scan_table_t scan_table = SL_HAL_IADC_SCANTABLE_DEFAULT;
 *    scan_table.entries[0] = entry;
 *
 *    // Initialize hardware
 *    sl_hal_iadc_init(IADC0, &init);
 *    sl_hal_iadc_init_scan(IADC0, &scan_init, &scan_table);
 *    sl_hal_iadc_set_scan_mask_multiple_entries(IADC0, &scan_table);
 *
 *    // Enable IADC
 *    sl_hal_iadc_enable(IADC0);
 *
 *    // Start conversion
 *    sl_hal_iadc_cmd(IADC0, SL_HAL_IADC_CMD_START_SCAN);
 *
 *    // Wait for conversion to complete
 *    while ((IADC0->STATUS & IADC_STATUS_SCANFIFODV) == 0) {
 *      // Wait for data valid flag
 *    }
 *
 *    // Read result
 *    sl_hal_iadc_result_t result = sl_hal_iadc_scan_read_fifo(IADC0);
 *
 *    // Use result.data and result.id here
 *  }
 *  @endcode
 *
 * @} (end addtogroup iadc)
 ******************************************************************************/
/* *INDENT-ON* */

#endif /* defined(IADC_COUNT) && (IADC_COUNT > 0) */
#endif /* SL_HAL_IADC_H */
