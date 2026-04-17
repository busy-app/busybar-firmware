/***************************************************************************/ /**
* @file sensorhub_app.c
* @brief sensorhub_app example
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
 * @brief : This file perform the sensor hub application related operations
 * like intilize ,scanning the sensors,and create/start the sensor HUB
 ******************************************************************************/

// Include files
#include "rsi_ccp_user_config.h"
#include "sensor_hub.h"
#include <stdio.h>
#include "rsi_os.h"
#include "rsi_debug.h"
#include "cmsis_os2.h"
#include "rsi_pll.h"
#include "rsi_power_save.h"
#include "rsi_rom_clks.h"
#include "sl_wifi.h"
#include "sl_si91x_driver.h"
/*******************************************************************************
 **************  Sensor app Task Attributes structure for thread   *************
 ******************************************************************************/
#define SL_APP_TASK_STACK_SIZE 4096
const osThreadAttr_t app_thread_attributes = {
  .name       = "SensorHub_App", // Name of thread
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = SL_APP_TASK_STACK_SIZE, // Stack size of sensor_app task
  .priority   = osPriorityLow,          // Priority of Sensor task
  .tz_module  = 0,
  .reserved   = 0,
};

#if SH_AWS_ENABLE
const osThreadAttr_t aws_thread_attributes = {
  .name       = "AWS_App",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow3,
  .tz_module  = 0,
  .reserved   = 0,
};
#endif

rsi_task_handle_t app_task_handle = NULL;

#if SH_AWS_ENABLE
osSemaphoreId_t sl_semaphore_app_task_id_2;

char mqtt_publish_payload[500];

void sl_si91x_aws_task(void);
#endif

/*******************************************************************************
 ********************  Extern variables/structures   ***************************
 ******************************************************************************/
extern sl_sensor_info_t sensor_hub_info_t[SL_MAX_NUM_SENSORS]; // Sensor info structure
extern sl_bus_intf_config_t bus_intf_info;                     //< Bus interface configuration structure
extern osSemaphoreId_t sl_semaphore_aws_task_id;
/*******************************************************************************
 ********************** Local/global variables  *******************************
 ******************************************************************************/
uint32_t event_ack              = 0; // Sensor event acknowledge
static uint32_t sensor_scan_cnt = 0; // Sensor scan count

#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

/*******************************************************************************
 ************************* Local functions  ************************************
 ******************************************************************************/
void sl_si91x_sensorhub_app_task(void);                               // application task
void sensorhub_app_init(void);                                        // application initialization
void sl_si91x_sensor_event_handler(uint8_t sensor_id, uint8_t event); // application event handler
#ifndef SH_AWS_ENABLE
static sl_status_t initialize_wireless(void);
void wireless_sleep(void);
#endif
/**************************************************************************/ /**
 * @fn           void gy61_adc_raw_data_map()
 * @brief        Map the raw input data of adc gy61 to output range
 *
 * @param[in]    x        :Sensor raw data
 * @param[in]    in_min   :Sensor raw min
 * @param[in]    in_max   :Sensor raw max
 * @param[in]    out_min  :Sensor output min
 * @param[in]    out_max  :Sensor output max
 * @param[out]   sensor g output
 *
******************************************************************************/
#ifdef GY61_ADC_SENSOR
static long gy61_adc_raw_data_map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif
/**************************************************************************/ /**
 * @fn           void sl_si91x_sensor_event_handler()
 * @brief        This Sensor event handle to the Sensor HUB.
 *
 * @param[in]    sensor_id        :Id of the Sensor
 * @param[in]    events           :Sensor hub event
 * @param[out]   NULL
 *
******************************************************************************/
void sl_si91x_sensor_event_handler(uint8_t sensor_id, uint8_t event)
{
#if SH_AWS_ENABLE
  osStatus_t sl_sem_status;

  strcpy(mqtt_publish_payload, "");
#endif

  uint8_t sens_ind;
  for (sens_ind = 0; sens_ind < SL_MAX_NUM_SENSORS; sens_ind++) {
    if (sensor_hub_info_t[sens_ind].sensor_id == sensor_id) {
      break;
    }
  }

  if (SL_MAX_NUM_SENSORS == sens_ind) {
    DEBUGOUT("Sensor not Found!");
    return;
  }

  switch (event) {
    case SL_SENSOR_CREATION_FAILED:
      DEBUGOUT("%s Not created\r\n", sensor_hub_info_t[sens_ind].sensor_name);
      break;
    case SL_SENSOR_STARTED:
      DEBUGOUT("Sensor Started:%u \r\n", sensor_id);
      break;
    case SL_SENSOR_STOPPED:
      DEBUGOUT("Sensor Stopped:%u \r\n", sensor_id);
      break;
    case SL_SENSOR_DATA_READY:

      DEBUGOUT("\r\n %s ", sensor_hub_info_t[sens_ind].sensor_name);
      DEBUGOUT("\r\n Sensor data Ready:%u \t", sensor_id);

      if (SL_SENSOR_ADXL345_ID == sensor_id) {
        if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_INTERRUPT_MODE) {
          DEBUGOUT("Axis X = %f, \t",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.x);
          DEBUGOUT("Axis Y = %f, \t",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.y);
          DEBUGOUT("Axis Z = %f \t\n ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.z);
#if SH_AWS_ENABLE
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_ADXL345_ID_x: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.x);
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_ADXL345_ID_y: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.y);
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_ADXL345_ID_z: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.z);
#endif
        } else if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_POLLING_MODE) {
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_THRESHOLD) {
            DEBUGOUT("Axis X = %f, \t",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.x);
            DEBUGOUT("Axis Y = %f, \t",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.y);
            DEBUGOUT("Axis Z = %f \t\r\n ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.z);
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_ADXL345_ID_x: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.x);
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_ADXL345_ID_y: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.y);
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_ADXL345_ID_z: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].accelerometer.z);
#endif
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_TIMEOUT) {
            for (uint32_t i = 0;
                 i < sensor_hub_info_t[sens_ind].data_deliver.timeout / sensor_hub_info_t[sens_ind].sampling_interval;
                 i++) {
              DEBUGOUT("\r\n Axis X = %f, \t",
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.x);
              DEBUGOUT("Axis Y = %f, \t",
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.y);
              DEBUGOUT("Axis Z = %f \r\n ",
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.z);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_ADXL345_ID_%lu_x: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.x);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_ADXL345_ID_%lu_y: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.y);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_ADXL345_ID_%lu_z: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.z);
#endif
            }
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_NUM_OF_SAMPLES) {
            for (uint32_t i = 0; i < sensor_hub_info_t[sens_ind].data_deliver.numofsamples; i++) {
              DEBUGOUT("\r\n Axis:- X = %f, \t",
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.x);
              DEBUGOUT("Y = %f, \t",
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.y);
              DEBUGOUT("Z = %f  \t",
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.z);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_ADXL345_ID_%lu_x: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.x);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_ADXL345_ID_%lu_y: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.y);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_ADXL345_ID_%lu_z: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].accelerometer.z);
#endif
            }
          }
        }
      }
      if (SL_SENSOR_APDS9960_ID == sensor_id) {
        if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_INTERRUPT_MODE) {
          DEBUGOUT("R = %f, \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.r);
          DEBUGOUT("G = %f, \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.g);
          DEBUGOUT("B = %f \t\n ", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.b);
          DEBUGOUT("Proximity = %f ;\t\n",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.proximity);
          //   DEBUGOUT("Gesture = %c \t\n", (char)sensor_hub_info_t[sens_ind].sens_data_ptr->sensor_data[0].gesture);
#if SH_AWS_ENABLE
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_APDS9960_ID_r: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.r);
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_APDS9960_ID_g: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.g);
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_APDS9960_ID_b: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.b);
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_APDS9960_ID_prox: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.proximity);
#endif

        } else if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_POLLING_MODE) {
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_THRESHOLD) {
            DEBUGOUT("R = %f, \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.r);
            DEBUGOUT("G = %f, \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.g);
            DEBUGOUT("B = %f \t\n ", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.b);
            DEBUGOUT("Proximity = %f ;\t\n",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.proximity);
            //  DEBUGOUT("Gesture = %c \t\n", (char)sensor_hub_info_t[sens_ind].sens_data_ptr->sensor_data[0].gesture);
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_APDS9960_ID_r: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.r);
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_APDS9960_ID_g: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.g);
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_APDS9960_ID_b: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.b);
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_APDS9960_ID_prox: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].rgbw.proximity);
#endif
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_TIMEOUT) {
            for (uint32_t i = 0;
                 i < sensor_hub_info_t[sens_ind].data_deliver.timeout / sensor_hub_info_t[sens_ind].sampling_interval;
                 i++) {
              DEBUGOUT("Proximity = %f ;\t",
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.proximity);
              DEBUGOUT(" R = %f, \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.r);
              DEBUGOUT(" G = %f, \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.g);
              DEBUGOUT(" B = %f \t\n ", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.b);
              // DEBUGOUT("Gesture = %c \t\n", (char)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].gesture);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_APDS9960_ID_%lu_r: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.r);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_APDS9960_ID_%lu_g: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.g);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_APDS9960_ID_%lu_b: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.b);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_APDS9960_ID_%lu_prox: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.proximity);
#endif
            }
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_NUM_OF_SAMPLES) {
            for (uint32_t i = 0; i < sensor_hub_info_t[sens_ind].data_deliver.numofsamples; i++) {
              DEBUGOUT("\r\n R = %f, \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.r);
              DEBUGOUT(" G = %f, \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.g);
              DEBUGOUT(" B = %f \t; ", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.b);
              DEBUGOUT(" Proximity = %f ",
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.proximity);
              //  DEBUGOUT("Gesture = %c \t\n", (char)sensor_hub_info_t[sens_ind].sens_data_ptr->sensor_data[i].gesture);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_APDS9960_ID_%lu_r: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.r);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_APDS9960_ID_%lu_g: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.g);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_APDS9960_ID_%lu_b: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.b);
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_APDS9960_ID_%lu_prox: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].rgbw.proximity);
#endif
            }
          }
        }
      }

      if (SL_SENSOR_LM75_ID == sensor_id) {
        if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_INTERRUPT_MODE) {
          DEBUGOUT("%f \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].temperature);
#if SH_AWS_ENABLE
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_LM75_ID: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].temperature);
#endif
        } else if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_POLLING_MODE) {
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_TIMEOUT) {
            for (uint32_t i = 0;
                 i < sensor_hub_info_t[sens_ind].data_deliver.timeout / sensor_hub_info_t[sens_ind].sampling_interval;
                 i++) {
              DEBUGOUT(" %f \t ", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].temperature);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_LM75_ID_%lu: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].temperature);
#endif
            }
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_NUM_OF_SAMPLES) {
            for (uint32_t i = 0; i < sensor_hub_info_t[sens_ind].data_deliver.numofsamples; i++) {
              DEBUGOUT("%f \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].temperature);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_LM75_ID_%lu: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].temperature);
#endif
            }
          }

          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_THRESHOLD) {
            DEBUGOUT("%f \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].temperature);
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_LM75_ID: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].temperature);
#endif
          }
        }
      }

      if (SL_SENSOR_BH1750_ID == sensor_id) {
        if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_INTERRUPT_MODE) {
          DEBUGOUT("%f \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].light);
#if SH_AWS_ENABLE
          snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                   sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                   "SL_SENSOR_BH1750_ID: %f    ",
                   (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].light);
#endif
        } else if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_POLLING_MODE) {
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_TIMEOUT) {
            for (uint32_t i = 0;
                 i < sensor_hub_info_t[sens_ind].data_deliver.timeout / sensor_hub_info_t[sens_ind].sampling_interval;
                 i++) {
              DEBUGOUT("%f \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].light);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_BH1750_ID_%lu: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].light);
#endif
            }
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_NUM_OF_SAMPLES) {
            for (uint32_t i = 0; i < sensor_hub_info_t[sens_ind].data_deliver.numofsamples; i++) {
              DEBUGOUT("%f \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].light);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_BH1750_ID_%lu: %f    ",
                       (i + 1),
                       (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[i].light);
#endif
            }
          }

          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_THRESHOLD) {
            DEBUGOUT("%f \t", (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].light);
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_BH1750_ID: %f    ",
                     (double)sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].light);
#endif
          }
        }
      }

      if (SL_SENSOR_ADC_JOYSTICK_ID == sensor_id) {

#if ((defined SH_ADC_ENABLE) || (defined SH_SDC_ENABLE))
        float vout = 0;
#endif
        if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_INTERRUPT_MODE) {
#ifdef SH_ADC_ENABLE
          for (uint32_t i = 0; i < bus_intf_info.adc_config.adc_ch_cfg.num_of_samples[JS_ADC_CHANNEL]; i++) {
            DEBUGOUT("%dmV \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_JOYSTICK_ID_%lu: %dmV    ",
                     (i + 1),
                     sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#endif
          }
#endif

#ifdef SH_SDC_ENABLE
#if ((defined SDC_MUTI_CHANNEL_ENABLE) || (SH_AWS_ENABLE))
          uint16_t sdc_channel_id = 0;
#endif
#ifdef SDC_MUTI_CHANNEL_ENABLE

          for (uint32_t i = 0; i <= bus_intf_info.sh_sdc_config.sh_sdc_sample_ther; i++) {
            //DEBUGOUT("%dmV \t", (0x0FFF&sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].sh_sdc_data[i]));
            sdc_channel_id = sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].sh_sdc_data[i];
            sdc_channel_id = sdc_channel_id >> 12;
            vout = (((float)((0x0FFF & sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].sh_sdc_data[i]))
                     / (float)SL_SH_ADC_MAX_OP_VALUE)
                    * SL_SH_ADC_VREF_VALUE);

            DEBUGOUT("\n\r SDC Channel_Id:[%d]\tSample: %lfV", sdc_channel_id, (double)vout);
#else
          DEBUGOUT("SDC_Samples:");
          for (uint32_t i = 0; i <= bus_intf_info.sh_sdc_config.sh_sdc_sample_ther; i++) {
            //DEBUGOUT("%dmV \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].sh_sdc_data[i]);
            vout = (((float)(sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].sh_sdc_data[i])
                     / (float)SL_SH_ADC_MAX_OP_VALUE)
                    * SL_SH_ADC_VREF_VALUE);
            DEBUGOUT(" %lfV \t", (double)vout);
#endif
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_JOYSTICK_ID_%lu: C_ID[%d] %dmV    ",
                     (i + 1),
                     sdc_channel_id,
                     sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].sh_sdc_data[i]);
#endif
          }

#endif
        } else if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_POLLING_MODE) {
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_TIMEOUT) {
            for (uint32_t i = 0;
                 i < sensor_hub_info_t[sens_ind].data_deliver.timeout / sensor_hub_info_t[sens_ind].sampling_interval;
                 i++) {
              DEBUGOUT("%dmV \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_JOYSTICK_ID_%lu: %dmV    ",
                       (i + 1),
                       sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#endif
            }
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_NUM_OF_SAMPLES) {
            for (uint32_t i = 0; i < sensor_hub_info_t[sens_ind].data_deliver.numofsamples; i++) {
              DEBUGOUT("%dmV \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_JOYSTICK_ID_%lu: %dmV    ",
                       (i + 1),
                       sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#endif
            }
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_THRESHOLD) {
            DEBUGOUT("%dmV \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[0]);
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_JOYSTICK_ID: %dmV    ",
                     sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[0]);
#endif
          }
        }
#ifdef SH_ADC_ENABLE
        vout =
          (((float)(sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[0]) / (float)SL_SH_ADC_MAX_OP_VALUE)
           * SL_SH_ADC_VREF_VALUE);

#if SH_AWS_ENABLE
        snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                 sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                 "Single-ended output: %lfV    ",
                 (double)vout);
#endif
#endif
#ifdef SH_ADC_ENABLE
        DEBUGOUT("Single ended input: %lfV \t", (double)vout);
#endif
      }

      if (SL_SENSOR_ADC_GUVA_S12D_ID == sensor_id) {
        float vout = 0;
        if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_INTERRUPT_MODE) {
          for (uint32_t i = 0; i < bus_intf_info.adc_config.adc_ch_cfg.num_of_samples[GUVA_ADC_CHANNEL]; i++) {
            DEBUGOUT("%d \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_GUVA_S12D_ID_%lu: %d    ",
                     (i + 1),
                     sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#endif
          }
        } else if (sensor_hub_info_t[sens_ind].sensor_mode == SL_SH_POLLING_MODE) {
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_TIMEOUT) {
            for (uint32_t i = 0;
                 i < sensor_hub_info_t[sens_ind].data_deliver.timeout / sensor_hub_info_t[sens_ind].sampling_interval;
                 i++) {
              DEBUGOUT("%dmV \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_GUVA_S12D_ID_%lu: %d    ",
                       (i + 1),
                       sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#endif
            }
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_NUM_OF_SAMPLES) {
            for (uint32_t i = 0; i < sensor_hub_info_t[sens_ind].data_deliver.numofsamples; i++) {
              DEBUGOUT("%dmV \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#if SH_AWS_ENABLE
              snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                       sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                       "SL_SENSOR_GUVA_S12D_ID_%lu: %d    ",
                       (i + 1),
                       sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[i]);
#endif
            }
          }
          if (sensor_hub_info_t[sens_ind].data_deliver.data_mode == SL_SH_THRESHOLD) {
            DEBUGOUT("%dmV \t", sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[0]);
#if SH_AWS_ENABLE
            snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                     sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                     "SL_SENSOR_GUVA_S12D_ID: %dmV    ",
                     sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[0]);
#endif
          }
        }
        vout =
          (((float)(sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].adc[0]) / (float)SL_SH_ADC_MAX_OP_VALUE)
           * SL_SH_ADC_VREF_VALUE);
#if SH_AWS_ENABLE
        snprintf(mqtt_publish_payload + strlen(mqtt_publish_payload),
                 sizeof(mqtt_publish_payload) - strlen(mqtt_publish_payload),
                 "Single-ended output: %lfV    ",
                 (double)vout);
#endif
        DEBUGOUT("Single ended input: %lfV \t", (double)vout);
      }

      if (SL_SENSOR_ADC_GY_61_ID == sensor_id) {
#ifdef GY61_X_AXIS_ADC_CHANNEL
        for (uint32_t i = 0; i < bus_intf_info.adc_config.adc_ch_cfg.num_of_samples[GY61_X_AXIS_ADC_CHANNEL]; i++) {
          double x_g =
            ((float)gy61_adc_raw_data_map(sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].gy61.x[i],
                                          GY61_X_RAW_MIN,
                                          GY61_X_RAW_MAX,
                                          GY61_G_SCALE_MIN,
                                          GY61_G_SCALE_MAX))
            / -100.0;
          DEBUGOUT("X = %gg, \t", x_g);
        }
#endif
#ifdef GY61_Y_AXIS_ADC_CHANNEL
        for (uint32_t i = 0; i < bus_intf_info.adc_config.adc_ch_cfg.num_of_samples[GY61_Y_AXIS_ADC_CHANNEL]; i++) {
          double y_g =
            ((float)gy61_adc_raw_data_map(sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].gy61.y[0],
                                          GY61_Y_RAW_MIN,
                                          GY61_Y_RAW_MAX,
                                          GY61_G_SCALE_MIN,
                                          GY61_G_SCALE_MAX))
            / -100.0;
          DEBUGOUT("Y = %gg, \t", y_g);
        }
#endif
#ifdef GY61_Z_AXIS_ADC_CHANNEL
        for (uint32_t i = 0; i < bus_intf_info.adc_config.adc_ch_cfg.num_of_samples[GY61_Z_AXIS_ADC_CHANNEL]; i++) {
          double z_g =
            ((float)gy61_adc_raw_data_map(sensor_hub_info_t[sens_ind].sensor_data_ptr->sensor_data[0].gy61.z[0],
                                          GY61_Z_RAW_MIN,
                                          GY61_Z_RAW_MAX,
                                          GY61_G_SCALE_MIN,
                                          GY61_G_SCALE_MAX))
            / -100.0;
          DEBUGOUT("Z = %gg \t", z_g);
        }
#endif
      }

      DEBUGOUT(" data_deliver.mode:%d \r\n", sensor_hub_info_t[sens_ind].data_deliver.data_mode);
      // Acknowledge data reception
      event_ack = sensor_id;

#if SH_AWS_ENABLE
      sl_sem_status = osSemaphoreRelease(sl_semaphore_aws_task_id);
      if (sl_sem_status != osOK) {
        DEBUGOUT("\r\n event post osSemaphoreRelease failed :%d \r\n", sl_sem_status);
      }
      sl_sem_status = osSemaphoreAcquire(sl_semaphore_app_task_id_2, osWaitForever);
      if (sl_sem_status != osOK) {
        DEBUGOUT("\r\n osSemaphoreAcquire failed :%d \r\n", sl_sem_status);
      }
#endif
      break;

    case SL_SENSOR_CNFG_INVALID:
      DEBUGOUT(" SL_SENSOR_CNFG_INVALID:%u \r\n", sensor_id);

      break;
    case SL_SENSOR_START_FAILED:
      DEBUGOUT("Sensor START failed:%u \r\n", sensor_id);

      break;
    case SL_SENSOR_STOP_FAILED:
      DEBUGOUT("Sensor STOP failed:%u \r\n", sensor_id);

      break;
    case SL_SENSOR_DELETED:
      DEBUGOUT("Sensor deleted:%u \r\n", sensor_id);
      break;

    case SL_SENSOR_DELETE_FAILED:
      DEBUGOUT("Sensor deleted failed:%u \r\n", sensor_id);
      break;
    default:
      break;
  }
}
#ifndef SH_AWS_ENABLE
/*******************************************************************************
   * Initialization of wireless APIs.
   * M4-TA handshake is performed.
   * TA is send to standby with RAM retention mode if SWITCH_TO_PS0 is disabled.
   * TA is send to standby without RAM retention mode if SWITCH_TO_PS0 is enabled.
   ******************************************************************************/
static sl_status_t initialize_wireless(void)
{
  // For M4-sleep wakeup, and to achieve minimum current in powersave application,
  // wifi is initialized, handshake is performed between M4 and TA, then
  // TA powersave profile is updated sleep with/without retention as per
  // requirements.
  // Wifi device configuration
  const sl_wifi_device_configuration_t client_init_configuration = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = US,
    .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                     .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                     .feature_bit_map =
                       (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE),
                     .tcp_ip_feature_bit_map =
                       (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT | SL_SI91X_TCP_IP_FEAT_SSL
                        | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                     .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                     .ext_custom_feature_bit_map = 0,
                     .bt_feature_bit_map         = 0,
                     .ext_tcp_ip_feature_bit_map =
                       (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)
                        | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                     .ble_feature_bit_map     = 0,
                     .ble_ext_feature_bit_map = 0,
                     .config_feature_bit_map  = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
  };
  sl_status_t status;
  // Initialize the wifi interface.
  status = sl_wifi_init(&client_init_configuration, NULL, NULL);
  DEBUGINIT();
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    DEBUGOUT("sl_wifi_init failed, Error Code: 0x%lX \n", status);
    return status;
  }
  uint8_t xtal_enable = 1;
  // M4-TA handshake is required for TA communication.
  status = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with error code.
    DEBUGOUT("sl_si91x_m4_ta_secure_handshake failed, Error Code: 0x%lX \n", status);
    return status;
  }
  // Wireless Sleep with ram retention
  wireless_sleep();
  // If reaches here, returns SL_STATUS_OK.
  return SL_STATUS_OK;
}

/*******************************************************************************
   * After PS2 to PS4 transition, flash is initialized and to initialize flash
   * wireless processor is set to active mode.
   * This function sends the wireless processor to sleep with retention.
   ******************************************************************************/
void wireless_sleep(void)
{
  sl_status_t status;
  // Wifi Profile (TA Mode) is set to High Performance.
  sl_wifi_performance_profile_v2_t ta_performance_profile = { .profile = HIGH_PERFORMANCE };

  status = sl_wifi_set_performance_profile_v2(&ta_performance_profile);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with error code.
    DEBUGOUT("sl_wifi_set_performance_profile_v2 failed, Error Code: 0x%lX \n", status);
    return;
  }
  // Wifi Profile (TA Mode) is set to standby power save with RAM retention.
  ta_performance_profile.profile = DEEP_SLEEP_WITH_RAM_RETENTION;

  // Wifi Profile (TA Mode) is set to standby power save with RAM retention.
  status = sl_wifi_set_performance_profile_v2(&ta_performance_profile);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with error code.
    DEBUGOUT("sl_wifi_set_performance_profile_v2 failed, Error Code: 0x%lX \n", status);
    return;
  }
}
#endif
/**************************************************************************/ /**
 * @fn           void sl_si91x_sensorhub_app_task()
 * @brief        This function perform the all sensor related operations.
 *
 * @param[in]    None
 * @param[out]   None
******************************************************************************/
void sl_si91x_sensorhub_app_task(void)
{

  uint32_t status = 0;
  sl_sensor_id_t sl_sensor_scan_info[SL_MAX_NUM_SENSORS];

  osSemaphoreId_t sl_semaphore_app_task_id;
  osStatus_t sl_semapptaskacq_status;

  osSemaphoreAttr_t sl_app_semaphore_attr_st;
  sl_app_semaphore_attr_st.attr_bits = 0U;
  sl_app_semaphore_attr_st.cb_mem    = NULL;
  sl_app_semaphore_attr_st.cb_size   = 0U;
  sl_app_semaphore_attr_st.name      = NULL;

  DEBUGOUT("\r\n Start Sensor HUB APP Task \r\n");
  sl_semaphore_app_task_id = osSemaphoreNew(1U, 0U, &sl_app_semaphore_attr_st);
#ifndef SH_AWS_ENABLE
  // Initialize the wireless interface and put the TA in Standby with RAM retention mode.
  status = initialize_wireless();
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    DEBUGOUT("Wireless API initialization failed, Error Code: 0x%lX \n", status);
    return;
  }
#endif

  // Register callback handler for getting different events from the sensor hub
  status = sl_si91x_sensorhub_notify_cb_register(sl_si91x_sensor_event_handler, (sl_sensor_id_t *)&event_ack);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Unable to create call back info: %lu \r\n", status);
    while (1)
      ;
  }
  // Initialize sensor interface
  status = sl_si91x_sensorhub_init();
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Sensor Hub Init failed \r\n");
    while (1)
      ;
  }

  // sensor hub scan
  sensor_scan_cnt = sl_si91x_sensorhub_detect_sensors((sl_sensor_id_t *)&sl_sensor_scan_info, SL_MAX_NUM_SENSORS);
  if (sensor_scan_cnt == 0) {
    DEBUGOUT("\r\n No sensor is detected \r\n");
    while (1)
      ;
  }

  // create sensors for scanned sensors
  for (uint32_t sensor_cnt = 0; sensor_cnt < sensor_scan_cnt; sensor_cnt++) {
    status = sl_si91x_sensorhub_create_sensor(sl_sensor_scan_info[sensor_cnt]);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n Unable to create sensor %d,Error Code:%lu \r\n", sl_sensor_scan_info[sensor_cnt], status);
    }
  }
  // Start the sensor HUb Tasks
  status = sl_si91x_sensor_hub_start();
  if (status != RSI_OK) {
    DEBUGOUT("\r\n  Sensor HUB start failed \r\n");
    while (1)
      ;
  }
#if SH_AWS_ENABLE
  osThreadNew((osThreadFunc_t)sl_si91x_aws_task, NULL, &aws_thread_attributes);

  sl_semapptaskacq_status = osSemaphoreAcquire(sl_semaphore_app_task_id_2, osWaitForever);
  if (sl_semapptaskacq_status != osOK) {
    DEBUGOUT("\r\n osSemaphoreAcquire failed :%d \r\n", sl_semapptaskacq_status);
  }
#endif
  // Start the sensors
  for (uint32_t sensor_cnt = 0; sensor_cnt < sensor_scan_cnt; sensor_cnt++) {
    // start a sensor, data ready events will be posted once data is acquired successfully
    status = sl_si91x_sensorhub_start_sensor(sl_sensor_scan_info[sensor_cnt]);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n Unable to start sensor %d \r\n", sl_sensor_scan_info[sensor_cnt]);
    }
  }
  while (1) {
    // waiting for the semaphore release
    sl_semapptaskacq_status = osSemaphoreAcquire(sl_semaphore_app_task_id, osWaitForever);
    if (sl_semapptaskacq_status != osOK) {
      DEBUGOUT("\r\n osSemaphoreAcquire failed :%d \r\n", sl_semapptaskacq_status);
    }
  }
}
/**************************************************************************/ /**
 * @fn           void sensorhub_app_init()
 * @brief        This function will update the SCB->VTOR Register with new Vector address.
 *               It will change the core frequency to 20 MHz and create the
 *               thread to the sensor app task.
 * @param[in]    None
 * @param[out]   None
******************************************************************************/

void sensorhub_app_init(void)
{
  // Updating the CPU core clock by 20 MHz to work in PS2 mode
  RSI_IPMU_M20rcOsc_TrimEfuse();
  RSI_PS_FsmHfFreqConfig(20);
  RSI_CLK_M4SocClkConfig(M4CLK, M4_ULPREFCLK, 0);

  // Initializes board UART for Prints
  DEBUGINIT();
#if SH_AWS_ENABLE
  osSemaphoreAttr_t sl_app_semaphore_attr_st;
  sl_app_semaphore_attr_st.attr_bits = 0U;
  sl_app_semaphore_attr_st.cb_mem    = NULL;
  sl_app_semaphore_attr_st.cb_size   = 0U;
  sl_app_semaphore_attr_st.name      = NULL;

  sl_semaphore_app_task_id_2 = osSemaphoreNew(1U, 0U, &sl_app_semaphore_attr_st);
#endif

  // Create the APP task
  osThreadNew((osThreadFunc_t)sl_si91x_sensorhub_app_task, NULL, &app_thread_attributes);
}
