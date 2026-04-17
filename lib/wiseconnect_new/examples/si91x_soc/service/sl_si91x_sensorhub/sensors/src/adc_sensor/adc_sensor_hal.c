/***************************************************************************/ /**
 * @file adc_sensor_hal.c
 * @brief adc sensor hal handles ADC based sensors
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rsi_debug.h"
#include "sensor_hub.h"
#include "sensor_type.h"
#include "sl_si91x_adc.h"
#include "adc_sensor_driver.h"

#pragma GCC diagnostic ignored "-Wunused-function"

// For unused parameter
#define UNUSED_PARAM __attribute__((unused))

/* Rx buffer to store ADC data in ram*/
#ifdef SL_SH_ADC_CHANNEL0
uint16_t adc_data_ram_ch0[SL_SH_ADC_SENSOR0_NUM_OF_SAMPLES * SL_SH_ADC_CH0_NUM_SAMPLES] __attribute__((aligned(2)));
#endif
#ifdef SL_SH_ADC_CHANNEL1
uint16_t adc_data_ram_ch1[SL_SH_ADC_SENSOR2_NUM_OF_SAMPLES * SL_SH_ADC_CH1_NUM_SAMPLES] __attribute__((aligned(2)));
#endif
#ifdef SL_SH_ADC_CHANNEL2
uint16_t adc_data_ram_ch2[SL_SH_ADC_SENSOR2_NUM_OF_SAMPLES * SL_SH_ADC_CH2_NUM_SAMPLES] __attribute__((aligned(2)));
#endif
#ifdef SL_SH_ADC_CHANNEL3
uint16_t adc_data_ram_ch3[SL_SH_ADC_SENSOR2_NUM_OF_SAMPLES * SL_SH_ADC_CH3_NUM_SAMPLES] __attribute__((aligned(2)));
#endif

#ifdef SL_SH_ADC_CHANNEL0
uint16_t *adc_data_ptrs[] = {
  adc_data_ram_ch0,
#ifdef SL_SH_ADC_CHANNEL1
  adc_data_ram_ch1,
#endif
#ifdef SL_SH_ADC_CHANNEL2
  adc_data_ram_ch2,
#endif
#ifdef SL_SH_ADC_CHANNEL3
  adc_data_ram_ch3,
#endif
};
#endif
void update_adc_sensor_multi_channel_data(sl_sensor_data_group_t *data_group, uint8_t ch_no);
bool is_sdc = false;
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static bool is_channel_init = false;
static sl_adc_cfg_t *adc_config;

/*******************************************************************************
 * @fn        static sl_status_t adc_stop(sl_adc_config_t *adc_cfg)
 * @brief     ADC stop
 *            This function stops ADC peripheral
 *
 * @param[in] adc_cfg: ADC configuration
 *
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
static sl_status_t adc_stop(sl_adc_config_t *adc_cfg)
{
  sl_status_t sl_status;
  sl_status = sl_si91x_adc_stop(*adc_cfg);

  return sl_status;
}

/*******************************************************************************
 * @fn        sl_sensor_adc_handle_t sl_si91x_adc_sensor_create(sl_adc_bus_handle_t bus, int channel)
 * @brief     ADC sensor create
 *            This function will create ADC sensor by initializing respective sensor and
 *            then starts the ADC peripheral if not started already. We will keep is_init
 *            flag to know if particular sensor is initialized.
 *
 * @param[in] bus: ADC bus value
 * @param[in] channel: ADC channel (used to find respective sensor implementations)
 *
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_sensor_adc_handle_t sl_si91x_adc_sensor_create(UNUSED_PARAM sl_sensor_bus_t bus, int channel)
{
#ifdef SH_SDC_ENABLE
  is_channel_init = true;
  is_sdc          = true;
#endif
  if (!is_channel_init) {
    /* All other adc sensor handles will be called only after sensor_create, so we are storing the pointer to adc bus interface info here */
    adc_config = sl_si91x_fetch_adc_bus_intf_info();
  }
  sl_adc_sensor_data_t *p_sensor = (sl_adc_sensor_data_t *)pvPortMalloc(sizeof(sl_adc_sensor_data_t));
  if (p_sensor == NULL) {
    DEBUGOUT("\r\n ADC sensor create failed while memory allocation:%u \r\n", sizeof(sl_adc_sensor_data_t));
    return NULL;
  }
  p_sensor->channel = (uint8_t)channel;

  if (!is_channel_init) {
    sl_status_t ret = sl_si91x_adc_channel_init(&adc_config->adc_ch_cfg, &adc_config->adc_cfg);
    if (ret != SL_STATUS_OK) {
      DEBUGOUT("\r\n ADC sensor channels init failed, channel \r\n");
      return NULL;
    }
    is_channel_init = true;
  }
  if (IS_MULTI_CHANNEL(channel)) {
    DEBUGOUT("\r\n ADC - sensor created \r\n");
    for (uint8_t ch_no = 0; ch_no < MAX_CHNL_NO; ch_no++) {
      if (BIT(ch_no) & channel) {
        DEBUGOUT(" channel %d\r\n", ch_no);
      }
      channel &= ~(BIT(ch_no));
      if (channel == 0) { // Printed all the channels
        break;
      }
    }
  } else {
    // Channel is set as BIT(channel_number) as per design, so to get the actual channel number use log2
    uint8_t actual_channel = log2(channel);
    DEBUGOUT("\r\n ADC - sensor created, channel %d\r\n", actual_channel);
  }
  p_sensor->is_init = true;

  return (sl_sensor_adc_handle_t)p_sensor;
}

/*******************************************************************************
 * @fn        inline sl_status_t sl_si91x_adc_sensor_enable(sl_adc_sensor_data_t *p_sensor, UNUSED_PARAM int channel)
 * @brief     ADC sensor enable
 *            This function will enable the sensor by enabling respective ADC channel. Note
 *            that ADC peripheral is started as part of sensor init. And this should be 
 *            used only if you are using multiple channels
 *
 * @param[in] p_sensor: ADC sensor pointer which contains implementations, bus, etc.
 * @param[in] channel: ADC channel number
 *
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
inline sl_status_t sl_si91x_adc_sensor_enable(sl_adc_sensor_data_t *p_sensor, UNUSED_PARAM int channel)
{
  if (p_sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  for (uint8_t ch_no = 0; ch_no < MAX_CHNL_NO; ch_no++) {
    if (p_sensor->channel & BIT(ch_no)) {
      sl_status_t ret = sl_si91x_adc_chnl_enable(ch_no);
      if (ret != SL_STATUS_OK) {
        DEBUGOUT("\r\n ADC sensor enable failed \r\n");
        return ret;
      }
    }
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @fn        inline sl_status_t sl_si91x_adc_sensor_disable(sl_adc_sensor_data_t *p_sensor, UNUSED_PARAM int channel)
 * @brief     ADC sensor disable
 *            This function will disables the sensor by disabling respective ADC channel
 *            Note that ADC peripheral will not be deinitialized here. And this should be 
 *            used only if you are using multiple channels
 *
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 * @param[in] channel: ADC channel number
 *
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
inline sl_status_t sl_si91x_adc_sensor_disable(sl_adc_sensor_data_t *p_sensor, UNUSED_PARAM int channel)
{
  if (p_sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  for (uint8_t ch_no = 0; ch_no < MAX_CHNL_NO; ch_no++) {
    if (p_sensor->channel & BIT(ch_no)) {
      sl_status_t ret = sl_si91x_adc_chnl_disable(ch_no);
      if (ret != SL_STATUS_OK) {
        DEBUGOUT("\r\n ADC sensor disable failed \r\n");
        return ret;
      }
    }
  }
  return SL_STATUS_OK;
}

/******************************************************************************
 * @fn        sl_status_t sl_si91x_adc_sensor_delete(sl_sensor_adc_handle_t *sensor)
 * @brief     ADC sensor delete
 *            This function will delete the sensor by freeing the adc sensor configurations
 *            and ADC peripheral will be stopped if no sensor is running.
 *
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 *
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_adc_error_t sl_si91x_adc_sensor_delete(sl_sensor_adc_handle_t *sensor)
{
  if (sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_adc_sensor_data_t *p_sensor = (sl_adc_sensor_data_t *)(*sensor);

#if SH_ADC_ENABLE
  if (!p_sensor->is_init) {
    free(p_sensor);
    return SL_STATUS_OK;
  }

  p_sensor->is_init = false;
  sl_status_t ret   = sl_si91x_adc_de_init(&adc_config->adc_cfg);
  if (ret != SL_STATUS_OK) {
    DEBUGOUT("\r\n ADC sensor deinitialization failed\r\n");
    return ret;
  }

  // TODO: Create a variable to know how many ADC sensors are created
  // Stop ADC only when no ADC sensors are present
  ret = adc_stop(&adc_config->adc_cfg);
  if (ret != SL_STATUS_OK) {
    DEBUGOUT("\r\n ADC sensor stop failed while deinitialization \r\n");
    return ret;
  }
#endif
  free(p_sensor);
  sensor = NULL;
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_adc_sensor_sample_static(sl_sensor_adc_handle_t *sensor, uint16_t *adc_value)
 * @brief     ADC sensor read static data
 *            This function will sample static ADC data from the ADC_DATA register.
 *
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 * @param[in] adc_value: data pointer to store the ADC static value
 * 
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_status_t sl_si91x_adc_sensor_sample_static(uint16_t *adc_value)
{
  sl_status_t ret = sl_si91x_adc_read_static_sample(&adc_config->adc_ch_cfg, &adc_config->adc_cfg, adc_value);
  return ret;
}

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_adc_sensor_sleep(sl_sensor_adc_handle_t sensor)
 * @brief     ADC sensor sleep
 *            This function will put ADC sensor into sleep
 *
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 * 
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_status_t sl_si91x_adc_sensor_sleep(sl_sensor_adc_handle_t sensor)
{
  if (sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_adc_sensor_data_t *p_sensor = (sl_adc_sensor_data_t *)(sensor);
  sl_status_t ret                = p_sensor->sleep();
  return ret;
}

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_adc_sensor_wakeup(sl_sensor_adc_handle_t sensor)
 * @brief     ADC sensor wakeup
 *            This function will wakeup ADC sensor from sleep
 *
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 * 
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_status_t sl_si91x_adc_sensor_wakeup(sl_sensor_adc_handle_t sensor)
{
  if (sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_adc_sensor_data_t *p_sensor = (sl_adc_sensor_data_t *)(sensor);
  sl_status_t ret                = p_sensor->wakeup();
  return ret;
}

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_adc_sensor_set_power(sl_sensor_adc_handle_t sensor, sl_sensor_power_mode_t power_mode)
 * @brief     ADC sensor set power mode
 *            This function will put sensor into requested power mode
 * 
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 * @param[in] power_mode: power mode for sensor
 * 
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_status_t sl_si91x_adc_sensor_set_power(sl_sensor_adc_handle_t sensor, sl_sensor_power_mode_t power_mode)
{
  sl_adc_sensor_data_t *p_sensor = (sl_adc_sensor_data_t *)(sensor);
  sl_status_t ret                = 0;

  if (sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  } else if (power_mode > SL_SENSOR_POWER_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  switch (power_mode) {
    case SL_SENSOR_POWER_MODE_WAKEUP:
      ret = p_sensor->wakeup();
      break;
    case SL_SENSOR_POWER_MODE_SLEEP:
      ret = p_sensor->sleep();
      break;
    default:
      ret = SL_STATUS_INVALID_PARAMETER;
      break;
  }
  return ret;
}

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_sensor_sample(uint8_t channel)
 * @brief     ADC read the sample
 *            This function will read sample data from a particular sample
 *
 * @param[in] sensor: ADC channel number from which to sample
 *
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_adc_error_t sl_si91x_sensor_channel_sample(uint8_t channel)
{
#ifdef SH_ADC_ENABLE
  sl_status_t status;
  status = sl_si91x_adc_channel_read_sample(&adc_config->adc_ch_cfg, channel);

  if (status != SL_STATUS_OK) {
    return status;
  }
#else
  (void)channel;
#endif
  return SL_STATUS_OK;
}

void update_adc_sensor_multi_channel_data(sl_sensor_data_group_t *data_group, uint8_t ch_no)
{
  (void)ch_no;
  (void)data_group;
#ifdef GY61_ADC_SENSOR
  if (ch_no == GY61_ADC_SENSOR_CHANNELS) {
#ifdef GY61_X_AXIS_ADC_CHANNEL
    data_group->sensor_data[0].gy61.x = adc_data_ptrs[GY61_X_AXIS_ADC_CHANNEL];
#endif
#ifdef GY61_Y_AXIS_ADC_CHANNEL
    data_group->sensor_data[0].gy61.y = adc_data_ptrs[GY61_Y_AXIS_ADC_CHANNEL];
#endif
#ifdef GY61_Z_AXIS_ADC_CHANNEL
    data_group->sensor_data[0].gy61.z = adc_data_ptrs[GY61_Z_AXIS_ADC_CHANNEL];
#endif
  }
#endif
}

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_adc_sensor_sample(sl_sensor_adc_handle_t *sensor, sl_sensor_data_group_t *data_group)
 * @brief     ADC sensor sample
 *            This function will sample the respective channel ADC data.
 *
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 * @param[in] data_group: contains memory to store ADC data and handle
 *            number of samples
 * 
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_adc_error_t sl_si91x_adc_sensor_sample(sl_sensor_adc_handle_t sensor, sl_sensor_data_group_t *data_group)
{
  sl_adc_sensor_data_t *p_sensor = (sl_adc_sensor_data_t *)(sensor);
  sl_status_t ret                = 0;
  uint8_t ch_no                  = p_sensor->channel;
  if (p_sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  } else if (p_sensor->is_init == false) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (is_sdc || adc_config->adc_cfg.operation_mode == SL_ADC_STATIC_MODE) {
    //   if ( adc_config->adc_cfg.operation_mode == SL_ADC_STATIC_MODE || is_sdc ) {
#ifdef SH_ADC_ENABLE
    ret = sl_si91x_adc_sensor_sample_static((uint16_t *)&adc_data_ptrs[0][data_group->number]);
#endif

    data_group->sensor_data[0].adc = (uint16_t *)adc_data_ptrs[0];
    if (SL_STATUS_OK != ret) {
      return ret;
    }
    data_group->number++;
  } else if (IS_MULTI_CHANNEL(ch_no)) {
    update_adc_sensor_multi_channel_data(data_group, ch_no);
    for (uint8_t channel = 0; channel < MAX_CHNL_NO; channel++) {
      if (BIT(channel) & ch_no) {
        sl_si91x_sensor_channel_sample(channel);
        for (uint16_t sample_length = 0; sample_length < adc_config->adc_ch_cfg.num_of_samples[channel];
             sample_length++) {
          if (adc_config->adc_ch_cfg.rx_buf[channel][sample_length] & BIT(11)) {
            adc_config->adc_ch_cfg.rx_buf[channel][sample_length] =
              (adc_config->adc_ch_cfg.rx_buf[channel][sample_length] & (ADC_MASK_VALUE));
          } else {
            adc_config->adc_ch_cfg.rx_buf[channel][sample_length] =
              adc_config->adc_ch_cfg.rx_buf[channel][sample_length] | BIT(11);
          }
        }
        ch_no &= ~(BIT(channel));
      }
      if (ch_no == 0) { // sampled all the channels
        break;
      }
    }
  } else {
    // Channel is set as BIT(channel_number) as per design, so to get the actual channel number check which bit is set
    uint8_t channel = log2(ch_no);
    sl_si91x_sensor_channel_sample(channel);
    for (uint16_t sample_length = 0; sample_length < adc_config->adc_ch_cfg.num_of_samples[channel]; sample_length++) {
      if (adc_config->adc_ch_cfg.rx_buf[channel][sample_length] & BIT(11)) {
        adc_config->adc_ch_cfg.rx_buf[channel][sample_length] =
          (adc_config->adc_ch_cfg.rx_buf[channel][sample_length] & (ADC_MASK_VALUE));
      } else {
        adc_config->adc_ch_cfg.rx_buf[channel][sample_length] = adc_config->adc_ch_cfg.rx_buf[channel][sample_length]
                                                                | BIT(11);
      }
    }
    data_group->sensor_data[0].adc = adc_data_ptrs[channel];
    data_group->number++;
  }

  return SL_STATUS_OK;
}

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_adc_sensor_sample(sl_sensor_adc_handle_t *sensor, sl_sensor_data_group_t *data_group)
 * @brief     ADC sensor sample
 *            This function will sample the respective channel ADC data.
 *
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 * @param[in] data_group: contains memory to store ADC data and handle
 *            number of samples
 *
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_adc_error_t sl_si91x_sdc_sensor_sample(sl_sensor_adc_handle_t sensor, sl_sensor_data_group_t *data_group)
{
  sl_adc_sensor_data_t *p_sensor = (sl_adc_sensor_data_t *)(sensor);
  sl_status_t ret                = 0;
  uint8_t ch_no                  = p_sensor->channel;
  (void)ch_no;
  if (p_sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  } else if (p_sensor->is_init == false) {
    return SL_STATUS_NOT_INITIALIZED;
  }

//  if (is_sdc || adc_config->adc_cfg.operation_mode == SL_ADC_STATIC_MODE) {
//   if ( adc_config->adc_cfg.operation_mode == SL_ADC_STATIC_MODE || is_sdc ) {
#ifdef SH_ADC_ENABLE
  ret = sl_si91x_adc_sensor_sample_static((uint16_t *)&adc_data_ptrs[0][data_group->number]);
#endif

#ifdef SH_SDC_ENABLE
#ifdef SDC_MUTI_CHANNEL_ENABLE
  ret = sl_si91x_read_sdc_multi_channel_data(data_group->sensor_data[0].sh_sdc_data);
#else
  ret = sl_si91x_read_sdc_data(data_group->sensor_data[0].sh_sdc_data);
#endif
#endif
  //data_group->sensor_data[0].adc = (uint16_t *)adc_data_ptrs[0];
  if (SL_STATUS_OK != ret) {
    return ret;
  }
  data_group->number++;

#if 0
  } else if (IS_MULTI_CHANNEL(ch_no)) {
    update_adc_sensor_multi_channel_data(data_group, ch_no);
    for (uint8_t channel = 0; channel < MAX_CHNL_NO; channel++) {
      if (BIT(channel) & ch_no) {
        sl_si91x_sensor_channel_sample(channel);
        for (uint16_t sample_length = 0; sample_length < adc_config->adc_ch_cfg.num_of_samples[channel];
             sample_length++) {
          if (adc_config->adc_ch_cfg.rx_buf[channel][sample_length] & BIT(11)) {
            adc_config->adc_ch_cfg.rx_buf[channel][sample_length] =
              (adc_config->adc_ch_cfg.rx_buf[channel][sample_length] & (ADC_MASK_VALUE));
          } else {
            adc_config->adc_ch_cfg.rx_buf[channel][sample_length] =
              adc_config->adc_ch_cfg.rx_buf[channel][sample_length] | BIT(11);
          }
        }
        ch_no &= ~(BIT(channel));
      }
      if (ch_no == 0) { // sampled all the channels
        break;
      }
    }
  } else {
    // Channel is set as BIT(channel_number) as per design, so to get the actual channel number check which bit is set
    uint8_t channel = log2(ch_no);
    sl_si91x_sensor_channel_sample(channel);
    for (uint16_t sample_length = 0; sample_length < adc_config->adc_ch_cfg.num_of_samples[channel]; sample_length++) {
      if (adc_config->adc_ch_cfg.rx_buf[channel][sample_length] & BIT(11)) {
        adc_config->adc_ch_cfg.rx_buf[channel][sample_length] =
          (adc_config->adc_ch_cfg.rx_buf[channel][sample_length] & (ADC_MASK_VALUE));
      } else {
        adc_config->adc_ch_cfg.rx_buf[channel][sample_length] = adc_config->adc_ch_cfg.rx_buf[channel][sample_length]
                                                                | BIT(11);
      }
    }
    data_group->sensor_data[0].adc = adc_data_ptrs[channel];
  }
#endif
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @fn        sl_status_t sl_si91x_adc_sensor_control(sl_sensor_adc_handle_t sensor, sl_sensor_command_t cmd, UNUSED_PARAM void *args)
 * @brief     ADC sensor control
 *            This function will control the different sensor modes
 *
 * @param[in] sensor: ADC sensor pointer which contains implementations, bus, etc.
 * @param[in] cmd: put the sensor into respective mode
 * 
 * @return    respective sl error code (SL_STATUS_OK, SL_STATUS_FAIL, ETC)
*******************************************************************************/
sl_adc_error_t sl_si91x_adc_sensor_control(sl_sensor_adc_handle_t sensor,
                                           sl_sensor_command_t cmd,
                                           UNUSED_PARAM void *args)
{
  sl_status_t ret = SL_STATUS_OK;

  if (sensor == NULL) {
    return SL_STATUS_NULL_POINTER;
  } else if (cmd > SL_COMMAND_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  switch (cmd) {
    case SL_COMMAND_SET_MODE:
      ret = SL_STATUS_OK;
      break;
    case SL_COMMAND_SET_RANGE:
      ret = SL_STATUS_OK;
      break;
    case SL_COMMAND_SET_ODR:
      ret = SL_STATUS_OK;
      break;
    case SL_COMMAND_SET_POWER:
      ret = SL_STATUS_OK;
      break;
    case SL_COMMAND_SELF_TEST:
      ret = SL_STATUS_OK;
      break;
    default:
      ret = SL_STATUS_OK;
      break;
  }
  return ret;
}
