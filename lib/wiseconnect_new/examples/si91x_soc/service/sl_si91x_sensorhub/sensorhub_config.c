/***************************************************************************/ /**
* @file sensorhub_config.c
* @brief sensorhub_config example
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
 *    claim that you wrote the original software.
 *    If you use this software In in a product,
 *    an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
/***************************************************************************
 * @brief : This file contain the sensor configuration structure
 * and peripherals configuration structures
 ******************************************************************************/
#include "sensor_hub.h"
#include "Driver_SPI.h"
#include "Driver_I2C.h"

#define SPI_BAUD        1000000 //speed at which data transmitted through SPI
#define SPI_BIT_WIDTH   8       //SPI bus width
#define SPI_CHIP_SELECT 0       //SPI Chip Select number

sl_sensor_info_t sensor_hub_info_t[SL_MAX_NUM_SENSORS] = {

  {
    .sensor_name               = "TEMPERATURE_SENSOR_LM75",
    .sensor_bus                = SL_SH_I2C,
    .sensor_id                 = SL_SENSOR_LM75_ID,
    .address                   = SL_LM75_ID,
    .sensor_mode               = SL_SH_POLLING_MODE,
    .sampling_interval         = 100,
    .data_deliver.data_mode    = SL_SH_NUM_OF_SAMPLES,
    .data_deliver.numofsamples = 5,
  },
  /* {
    .sensor_name               = "ADC_JOYSTICK",
    .sensor_id                 = SL_SENSOR_ADC_JOYSTICK_ID,
    .channel                   = BIT(SL_SH_ADC_CH0_CHANNEL),
    .sensor_bus                = SL_SH_ADC,
    .sensor_mode               = SL_SH_POLLING_MODE,
    .sampling_interval         = 100,
    .data_deliver.data_mode    = SL_SH_NUM_OF_SAMPLES,
    .data_deliver.numofsamples = SL_SH_ADC_SENSOR1_NUM_OF_SAMPLES,
  },*/
  /*  {
    .sensor_name               = "ACCELEROMETER_SENSOR",
    .sensor_bus                = SL_SH_SPI,
    .sensor_id                 = SL_SENSOR_ADXL345_ID,
    .sensor_mode               = SL_SH_POLLING_MODE,
    .sampling_interval         = 100,
    .address                   = SL_ADXL345_ID,
    .data_deliver.data_mode    = SL_SH_NUM_OF_SAMPLES,
    .data_deliver.numofsamples = 5,
  },*/

  /*{
    .sensor_name               = "GUVA_12D_UV",
    .sensor_id                 = SL_SENSOR_ADC_GUVA_S12D_ID,
    .channel                   = BIT(SL_SH_ADC_CH0_CHANNEL),
    .sensor_bus                = SL_SH_ADC,
    .sensor_mode               = SL_SH_POLLING_MODE,
    .sampling_interval         = 100,
    .data_deliver.data_mode    = SL_SH_NUM_OF_SAMPLES,
    .data_deliver.numofsamples = SL_SH_ADC_SENSOR0_NUM_OF_SAMPLES,
  },*/

  {
    .sensor_name               = "LIGHT_SENSOR",
    .sensor_bus                = SL_SH_I2C,
    .sensor_id                 = SL_SENSOR_BH1750_ID,
    .sensor_mode               = SL_SH_POLLING_MODE,
    .sampling_interval         = 100,
    .address                   = SL_BH1750_ID,
    .data_deliver.data_mode    = SL_SH_NUM_OF_SAMPLES,
    .data_deliver.numofsamples = 5,
  },

  {
    .sensor_name               = "GESTURE_PROXIMITY_RGB_SENSOR",
    .sensor_bus                = SL_SH_I2C,
    .sensor_id                 = SL_SENSOR_APDS9960_ID,
    .sensor_mode               = SL_SH_POLLING_MODE,
    .sampling_interval         = 100,
    .address                   = SL_APDS9960_ID,
    .data_deliver.data_mode    = SL_SH_NUM_OF_SAMPLES,
    .data_deliver.numofsamples = 5,

  },

  {
    .sensor_name            = "Button",
    .sensor_bus             = SL_SH_GPIO,
    .sensor_id              = SL_GPIO_SENSE_BUTTON_ID,
    .sensor_mode            = SL_SH_INTERRUPT_MODE,
    .sampling_intr_req_pin  = SH_BUTTON_PIN,
    .data_deliver.data_mode = SL_SH_NO_DATA_MODE,
  }
};

sl_bus_intf_config_t bus_intf_info = {
  .i2c_config.i2c_bus_speed    = ARM_I2C_BUS_SPEED_FAST,
  .i2c_config.i2c_power_state  = ARM_POWER_FULL,
  .i2c_config.i2c_control_mode = ARM_I2C_BUS_SPEED,

  .spi_config.spi_bit_width    = SPI_BIT_WIDTH,
  .spi_config.spi_baud         = SPI_BAUD,
  .spi_config.spi_control_mode = ARM_SPI_CPOL1_CPHA1,
  .spi_config.spi_mode         = ARM_SPI_MODE_MASTER,
  .spi_config.spi_cs_mode      = ARM_SPI_SS_MASTER_HW_OUTPUT,
  .spi_config.spi_power_state  = ARM_POWER_FULL,
  .spi_config.spi_cs_number    = SPI_CHIP_SELECT,
  .spi_config.spi_cs_misc_mode = ARM_SPI_CONTROL_SS,
  .spi_config.spi_sec_sel_sig  = ARM_SPI_SS_ACTIVE,
#ifdef SH_ADC_ENABLE
#ifdef SL_SH_ADC_CHANNEL0
  .adc_config.adc_cfg.operation_mode        = SL_ADC_FIFO_MODE,
  .adc_config.adc_cfg.num_of_channel_enable = SL_SH_ADC_NUM_CHANNELS_ENABLE,

  .adc_config.adc_ch_cfg.input_type[0]        = SL_ADC_SINGLE_ENDED,
  .adc_config.adc_ch_cfg.sampling_rate[0]     = SL_SH_ADC_SAMPLING_RATE,
  .adc_config.adc_ch_cfg.pos_inp_sel[0]       = SL_SH_ADC_CH0_P_INPUT,
  .adc_config.adc_ch_cfg.opamp_gain[0]        = SL_SH_ADC_CH0_OPAMP_GAIN,
  .adc_config.adc_ch_cfg.num_of_samples[0]    = SL_SH_ADC_CH0_NUM_SAMPLES,
  .adc_config.adc_ch_cfg.chnl_ping_address[0] = ADC_PING_BUFFER0,
  .adc_config.adc_ch_cfg.chnl_pong_address[0] = ADC_PING_BUFFER0 + SL_SH_ADC_CH0_NUM_SAMPLES,
#endif
#ifdef SL_SH_ADC_CHANNEL1
  .adc_config.adc_ch_cfg.input_type[1]        = SL_ADC_SINGLE_ENDED,
  .adc_config.adc_ch_cfg.sampling_rate[1]     = SL_SH_ADC_SAMPLING_RATE,
  .adc_config.adc_ch_cfg.pos_inp_sel[1]       = SL_SH_ADC_CH1_P_INPUT,
  .adc_config.adc_ch_cfg.opamp_gain[1]        = SL_SH_ADC_CH1_OPAMP_GAIN,
  .adc_config.adc_ch_cfg.num_of_samples[1]    = SL_SH_ADC_CH1_NUM_SAMPLES,
  .adc_config.adc_ch_cfg.chnl_ping_address[1] = ADC_PING_BUFFER1,
  .adc_config.adc_ch_cfg.chnl_pong_address[1] = ADC_PING_BUFFER1 + SL_SH_ADC_CH1_NUM_SAMPLES,
#endif
#ifdef SL_SH_ADC_CHANNEL2
  .adc_config.adc_ch_cfg.input_type[2]        = SL_ADC_SINGLE_ENDED,
  .adc_config.adc_ch_cfg.sampling_rate[2]     = SL_SH_ADC_SAMPLING_RATE,
  .adc_config.adc_ch_cfg.pos_inp_sel[2]       = SL_SH_ADC_CH2_P_INPUT,
  .adc_config.adc_ch_cfg.opamp_gain[2]        = SL_SH_ADC_CH2_OPAMP_GAIN,
  .adc_config.adc_ch_cfg.num_of_samples[2]    = SL_SH_ADC_CH2_NUM_SAMPLES,
  .adc_config.adc_ch_cfg.chnl_ping_address[2] = ADC_PING_BUFFER2,
  .adc_config.adc_ch_cfg.chnl_pong_address[2] = ADC_PING_BUFFER2 + SL_SH_ADC_CH2_NUM_SAMPLES,
#endif
#ifdef SL_SH_ADC_CHANNEL3
  .adc_config.adc_ch_cfg.input_type[3]        = SL_ADC_SINGLE_ENDED,
  .adc_config.adc_ch_cfg.sampling_rate[3]     = SL_SH_ADC_SAMPLING_RATE,
  .adc_config.adc_ch_cfg.pos_inp_sel[3]       = SL_SH_ADC_CH3_P_INPUT,
  .adc_config.adc_ch_cfg.opamp_gain[3]        = SL_SH_ADC_CH3_OPAMP_GAIN,
  .adc_config.adc_ch_cfg.num_of_samples[3]    = SL_SH_ADC_CH3_NUM_SAMPLES,
  .adc_config.adc_ch_cfg.chnl_ping_address[3] = ADC_PING_BUFFER3,
  .adc_config.adc_ch_cfg.chnl_pong_address[3] = ADC_PING_BUFFER3 + SL_SH_ADC_CH3_NUM_SAMPLES,
#endif
#endif
#ifdef SH_SDC_ENABLE

  .sh_sdc_config.sh_sdc_p_channel_sel[0]       = 0x04,
  .sh_sdc_config.sh_sdc_n_channel_sel[0]       = 0x00,
  .sh_sdc_config.sh_sdc_auxadc_diff_mode_ch[0] = 0,

#ifdef SDC_MUTI_CHANNEL_ENABLE
#ifdef SDC_CHANNEL_2
  .sh_sdc_config.sh_sdc_p_channel_sel[1]       = 0x05,
  .sh_sdc_config.sh_sdc_n_channel_sel[1]       = 0x00,
  .sh_sdc_config.sh_sdc_auxadc_diff_mode_ch[1] = 0,
#endif
#ifdef SDC_CHANNEL_3
  .sh_sdc_config.sh_sdc_p_channel_sel[2]       = 0x05,
  .sh_sdc_config.sh_sdc_n_channel_sel[2]       = 0x00,
  .sh_sdc_config.sh_sdc_auxadc_diff_mode_ch[2] = 0,
#endif
#ifdef SDC_CHANNEL_4
  .sh_sdc_config.sh_sdc_p_channel_sel[3]       = 0x05,
  .sh_sdc_config.sh_sdc_n_channel_sel[3]       = 0x00,
  .sh_sdc_config.sh_sdc_auxadc_diff_mode_ch[3] = 0,
#endif
#endif
  .sh_sdc_config.sh_sdc_sample_ther        = SDC_SAMP_THRESH,   // Number of samples to read from SDC register
  .sh_sdc_config.sh_sdc_no_channel_sel     = SDC_NUM_CH_SEL,    // Number of channel sel
  .sh_sdc_config.sh_sdc_sample_trigger_sel = SDC_SAMP_TRIG_SEL, // RTC trigger Sel(1-1ms 0-1sec)
  .sh_sdc_config.sh_sdc_cnt_trig_evnt      = SDC_CNT_TRIG_EVNT, // in which trigger event AUX-ADC Data will sampled
#endif
};
