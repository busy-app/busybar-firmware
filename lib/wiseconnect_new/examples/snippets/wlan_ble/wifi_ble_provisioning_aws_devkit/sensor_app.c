/***************************************************************************/ /**
 * @file
 * @brief Embedded Sensor Demo
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

#include "sl_net.h"
#include "app.h"
#include "errno.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket_constants.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_socket.h"
#include "sl_net_si91x.h"
#include "provisioning.h"
#include "light_sensor.h"
#include "motion_sensor.h"
#include "humidity_sensor.h"
#include "rgb_led.h"
#include "sl_sleeptimer.h"
#include <stdio.h>
#include <stdint.h>

/******************************************************
   *               Function Declarations
   ******************************************************/

void sensors_init(void);
void get_sensor_data(char *response_data);
void sensors_deinit(void);

/******************************************************
   *               Function Definitions
   ******************************************************/

void sensors_init(void)
{
  // Init sensor when application starts
  light_sensor_init();
  humidity_sensor_init();
  motion_sensor_init();
}

void sensors_deinit(void)
{
  // Deinitialize the sensor before entering sleep mode
  light_sensor_deinit();
  motion_sensor_deinit();
}

void get_sensor_data(char *response_data)
{
  int32_t temperature     = 0;
  float ambient_light_lux = 0.0f;
  float white_light_lux   = 0.0f;
  uint32_t humidity       = 0;

  temperature_read(&temperature);
  ambient_light_read(&ambient_light_lux);
  white_light_read(&white_light_lux);
  humidity_read(&humidity);

  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  accelerometer_read(&x, &y, &z);

  float gyro_x = 0.0f;
  float gyro_y = 0.0f;
  float gyro_z = 0.0f;
  gyro_read(&gyro_x, &gyro_y, &gyro_z);

  // Create JSON payload
  char json_payload[SENSOR_RESPONSE_BUFFER_SIZE];
  snprintf(json_payload,
           sizeof(json_payload),
           "{\"temperature\": %ld, \"ambient_light\": %.2f, \"white_light\": %.2f, \"humidity\": %lu, "
           "\"accelerometer\": {\"x\": %.2f, \"y\": %.2f, \"z\": %.2f}, "
           "\"gyro\": {\"x\": %.2f, \"y\": %.2f, \"z\": %.2f}}",
           temperature,
           ambient_light_lux,
           white_light_lux,
           humidity,
           x,
           y,
           z,
           gyro_x,
           gyro_y,
           gyro_z);

  memcpy(response_data, json_payload, sizeof(json_payload));
}
