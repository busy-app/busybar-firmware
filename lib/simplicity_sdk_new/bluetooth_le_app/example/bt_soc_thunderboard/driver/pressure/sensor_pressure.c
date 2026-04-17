/***************************************************************************//**
 * @file
 * @brief Air Pressure Sensor
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

#include <stddef.h>
#include "sl_board_control.h"
#include "sl_pressure.h"
#include "sl_i2cspm_instances.h"
#include "app_assert.h"
#include "sensor_pressure.h"

// -----------------------------------------------------------------------------
// Private variables

static bool initialized = false;

// -----------------------------------------------------------------------------
// Public function definitions

sl_status_t sensor_pressure_init(void)
{
  sl_status_t sc;
  sc = sl_board_enable_sensor(SL_BOARD_SENSOR_RHT);
  app_assert((SL_STATUS_OK == sc),
             "[E: %#04lx] Pressure sensor not available" APP_LOG_NL,
             sc);
  sc = sl_pressure_init(sl_i2cspm_sensor);
  if (SL_STATUS_OK == sc) {
    initialized = true;
  } else {
    initialized = false;
  }
  return sc;
}

void sensor_pressure_deinit(void)
{
  (void)sl_board_disable_sensor(SL_BOARD_SENSOR_PRESSURE);
  initialized = false;
}

sl_status_t sensor_pressure_get(float *pressure)
{
  sl_status_t sc;

  if (initialized && pressure != NULL) {
    sc = sl_pressure_measure_pressure(sl_i2cspm_sensor, pressure);
    // Convert from Pa to mbar (hPa)
    *pressure = *pressure / 100;
  } else {
    sc = SL_STATUS_NOT_INITIALIZED;
  }

  return sc;
}
