/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_pulse_counter.c
 * @brief Wireless M-Bus pulse counter sensor implementation.
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_rail_sdk_wmbus_sensor_core.h"
#include "sl_rail_sdk_wmbus_sensor_pulse_counter.h"
#include "sl_rail_sdk_wmbus_sensor_pulse_counter_config.h"
#include "sl_rail_sdk_wmbus_support.h"
#include "sl_simple_button_instances.h"
#include "sl_sleeptimer.h"
#include "app_assert.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_SEGMENT_LCD_DRIVER_PRESENT)
#include "sl_segmentlcd.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/*******************************************************************************
 * @brief Value of the VIF (Value Information Field) for the Wireless M-Bus sensor pulse counter.
 *
 * This macro defines the value of the VIF for the Wireless M-Bus sensor pulse counter.
 * The VIF is a field in the Wireless M-Bus protocol that provides information about the data being transmitted.
 * In this case, the VIF value is set to 0x07 what means Watt per hour (Wh).
 ******************************************************************************/
#define SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_VIF_VALUE 0x07

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void sl_rail_sdk_wmbus_sensor_pulse_counter_sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// Counter for the previous pulse count
static uint32_t all_pulse_counter;

// Counter for the actual pulse count
static int32_t actual_pulse_counter;

// Timer handle for the pulse counter
static sl_sleeptimer_timer_handle_t timer_handle;

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

SL_CODE_RAM static void sl_rail_sdk_wmbus_sensor_pulse_counter_sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  sl_rail_sdk_wmbus_sensor_pulse_counter_measure();
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Wireless M-Bus Pulse counter initialization function.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_init(void)
{
  actual_pulse_counter = 0;
  all_pulse_counter = 0;

  sl_status_t status = SL_STATUS_FAIL;

  status = sl_sleeptimer_start_periodic_timer_ms(&timer_handle,
                                                 SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_SLEEPTIMER_TIMEOUT,
                                                 sl_rail_sdk_wmbus_sensor_pulse_counter_sleeptimer_callback,
                                                 NULL,
                                                 SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_SLEEPTIMER_PRIORITY,
                                                 0);
  app_assert_status_f(status,
                      "[E: 0x%04x]: Failed to start periodic sleeptimer\n",
                      (int)status);
  return status;
}

/*******************************************************************************
 * Wireless M-Bus Pulse counter deinitialization function.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_deinit(void)
{
  return sl_sleeptimer_stop_timer(&timer_handle);
}

/*******************************************************************************
 * Wireless M-Bus Pulse counter measure function function. Read sensor data.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_measure(void)
{
  all_pulse_counter += actual_pulse_counter;
  actual_pulse_counter = 0;
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Wireless M-Bus Pulse counter packet assembler function.
 ******************************************************************************/
sl_rail_sdk_wmbus_sensor_data_t* sl_rail_sdk_wmbus_sensor_pulse_counter_get_data(void)
{
  // Sensor data for the pulse counter
  static sl_rail_sdk_wmbus_sensor_data_t sensor_data_pulse_counter = {
    .data = 0,
    .vif = SL_RAIL_SDK_WMBUS_SENSOR_PULSE_COUNTER_VIF_VALUE
  };

  sensor_data_pulse_counter.data = all_pulse_counter;
  return &sensor_data_pulse_counter;
}

/*******************************************************************************
 * Wireless M-Bus Pulse counter 7 segment LCD print function.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_print(void)
{
#if defined(SL_CATALOG_SEGMENT_LCD_DRIVER_PRESENT)
  sl_segment_lcd_number(all_pulse_counter);
#endif
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief Callback function for handling button state changes.
 *
 * If the button is pressed, it increments the counter variable.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_pulse_counter_button_pressed(void)
{
  actual_pulse_counter++;
  return SL_STATUS_OK;
}
