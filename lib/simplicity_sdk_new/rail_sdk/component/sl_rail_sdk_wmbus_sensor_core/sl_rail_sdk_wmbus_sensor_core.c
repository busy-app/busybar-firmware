/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_core.c
 * @brief Wireless M-Bus sensor core module implementation.
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
#include "sl_component_catalog.h"
#include "sl_rail_sdk_wmbus_support.h"
#include "sl_rail_sdk_wmbus_sensor_core.h"
#include "sl_rail_sdk_wmbus_sensor_core_config.h"
#include "sl_rail_sdk_simple_assistance.h"
#include "sl_rail_util_init.h"
#include "sl_simple_button_instances.h"
#include "sl_sleeptimer.h"
#include "app_assert.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_SEGMENT_LCD_DRIVER_PRESENT)
#include "sl_segmentlcd.h"
#endif

#if defined(SL_CATALOG_WMBUS_SENSOR_VIRTUAL_WATER_METER_PRESENT)
  #include "sl_rail_sdk_wmbus_sensor_virtual_water_meter.h"
#endif
#if defined(SL_CATALOG_WMBUS_SENSOR_PULSE_COUNTER_PRESENT)
  #include "sl_rail_sdk_wmbus_sensor_pulse_counter.h"
#endif
#if defined(SL_CATALOG_WMBUS_SENSOR_THERMOMETER_PRESENT)
  #include "sl_rail_sdk_wmbus_sensor_thermometer.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void sl_rail_sdk_wmbus_sensor_core_sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static uint8_t active_sensor_id = 0;

static sl_rail_sdk_wmbus_sensor_t* active_sensor = NULL;

/**
 * @brief Array of @ref sl_rail_sdk_wmbus_sensor_t structures representing the list of sensors.
 *
 * This array contains information about different sensors supported by the
 * sl_rail_sdk_wmbus_sensor_core module.
 * Each element of the array represents a sensor and includes the sensor's name,
 * initialization function pointer,
 * measurement function pointer, packet assembler function pointer, and
 * 7 segment LCD print function pointer.
 *
 * The array is conditionally compiled based on the presence of specific sensor modules.
 *
 * @note This array should be modified to include additional sensors as needed.
 */
static sl_rail_sdk_wmbus_sensor_t sensor_list[] = {
#if defined(SL_CATALOG_WMBUS_SENSOR_VIRTUAL_WATER_METER_PRESENT)
  {
    .id = 0,
    .name = "Water Meter Sensor",
    .init_fnc_ptr = &sl_rail_sdk_wmbus_sensor_virtual_water_meter_init,
    .deinit_fnc_ptr = &sl_rail_sdk_wmbus_sensor_virtual_water_meter_deinit,
    .measure_fnc_ptr = &sl_rail_sdk_wmbus_sensor_virtual_water_meter_measure,
    .get_data_fnc_ptr = &sl_rail_sdk_wmbus_sensor_virtual_water_meter_get_data,
    .print_fnc_ptr = &sl_rail_sdk_wmbus_sensor_virtual_water_meter_print,
    .button_pressed_fnc_ptr = &sl_rail_sdk_wmbus_sensor_virtual_water_meter_button_pressed
  },
#endif
#if defined(SL_CATALOG_WMBUS_SENSOR_PULSE_COUNTER_PRESENT)
  {
    .id = 1,
    .name = "Pulse counter",
    .init_fnc_ptr = &sl_rail_sdk_wmbus_sensor_pulse_counter_init,
    .deinit_fnc_ptr = &sl_rail_sdk_wmbus_sensor_pulse_counter_deinit,
    .measure_fnc_ptr = &sl_rail_sdk_wmbus_sensor_pulse_counter_measure,
    .get_data_fnc_ptr = &sl_rail_sdk_wmbus_sensor_pulse_counter_get_data,
    .print_fnc_ptr = &sl_rail_sdk_wmbus_sensor_pulse_counter_print,
    .button_pressed_fnc_ptr = &sl_rail_sdk_wmbus_sensor_pulse_counter_button_pressed
  },
#endif
#if defined(SL_CATALOG_WMBUS_SENSOR_THERMOMETER_PRESENT)
  {
    .id = 2,
    .name = "Thermometer",
    .init_fnc_ptr = &sl_rail_sdk_wmbus_sensor_thermometer_init,
    .deinit_fnc_ptr = &sl_rail_sdk_wmbus_sensor_thermometer_deinit,
    .measure_fnc_ptr = &sl_rail_sdk_wmbus_sensor_thermometer_measure,
    .get_data_fnc_ptr = &sl_rail_sdk_wmbus_sensor_thermometer_get_data,
    .print_fnc_ptr = &sl_rail_sdk_wmbus_sensor_thermometer_print,
    .button_pressed_fnc_ptr = &sl_rail_sdk_wmbus_sensor_thermometer_button_pressed
  }
#endif
};
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void sl_rail_sdk_wmbus_sensor_core_sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  active_sensor->print_fnc_ptr();
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Initialize the Wireless M-Bus sensor core.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_core_init(void)
{
  static sl_sleeptimer_timer_handle_t timer_handle;
  sl_status_t status = SL_STATUS_FAIL;
  active_sensor_id = SL_RAIL_SDK_WMBUS_SENSOR_ACTIVE_SENSOR_ID;
  if (sl_rail_sdk_wmbus_sensor_core_set_active_sensor(active_sensor_id) != SL_STATUS_OK) {
    app_log_info("Failed to set active sensor with ID: %d", active_sensor_id);
    return SL_STATUS_FAIL;
  }

  // Periodic timer (1s), that calls the active sensor print function
  status = sl_sleeptimer_start_periodic_timer_ms(&timer_handle,
                                                 1000,
                                                 sl_rail_sdk_wmbus_sensor_core_sleeptimer_callback,
                                                 NULL,
                                                 10,
                                                 0);
  app_assert_status_f(status,
                      "[E: 0x%04x]: Failed to start periodic sleeptimer\n",
                      (int)status);

  return status;
}

/*******************************************************************************
 * Set the active sensor.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_core_set_active_sensor(uint8_t id)
{
  for (size_t i = 0; i < sl_rail_sdk_wmbus_sensor_core_get_sensor_count(); i++) {
    if (sensor_list[i].id == id) {
      active_sensor_id = id;
      active_sensor = &sensor_list[i];
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_INVALID_PARAMETER;
}
/*******************************************************************************
 * Get the active sensor.
 ******************************************************************************/
sl_rail_sdk_wmbus_sensor_t* sl_rail_sdk_wmbus_sensor_core_get_active_sensor(void)
{
  return active_sensor;
}

/*******************************************************************************
 * Get the list of available sensors.
 ******************************************************************************/
sl_rail_sdk_wmbus_sensor_t* sl_rail_sdk_wmbus_sensor_core_list_available_sensors(void)
{
  return sensor_list;
}

/*******************************************************************************
 * Get the number of available sensors.
 ******************************************************************************/
size_t sl_rail_sdk_wmbus_sensor_core_get_sensor_count(void)
{
  return sizeof(sensor_list) / sizeof(sl_rail_sdk_wmbus_sensor_t);
}

/*******************************************************************************
 * Initialize the Wireless M-Bus sensor core 7 segment LCD.
 ******************************************************************************/
void sl_rail_sdk_wmbus_sensor_core_lcd_init(void)
{
#if defined(SL_CATALOG_SEGMENT_LCD_DRIVER_PRESENT)
  sl_segment_lcd_init(false);
#endif
}

/*******************************************************************************
 * Wireless M-Bus sensor core process function.
 ******************************************************************************/
uint16_t sl_rail_sdk_wmbus_sensor_core_process(uint8_t* tx_buffer, uint8_t* access_number)
{
  sl_rail_sdk_wmbus_sensor_data_t* sensors_data = NULL;
  sl_rail_sdk_wmbus_sensor_data_t* iter = NULL;

  // Call all sensor assembler function
  for (size_t i = 0; i < sl_rail_sdk_wmbus_sensor_core_get_sensor_count(); i++) {
    if (i == 0) {
      sensors_data = sensor_list[i].get_data_fnc_ptr();
      iter = sensors_data;
    } else {
      iter->next = sensor_list[i].get_data_fnc_ptr();
      iter = iter->next;
    }
  }
  return sl_rail_sdk_wmbus_setup_frame(tx_buffer, *access_number, sl_rail_sdk_wmbus_get_accessibility(), WMBUS_DEVICE_WATER_METER, sensors_data, true, true);;
}

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
/*******************************************************************************
 * Wireless M-Bus sensor core button callback implementation.
 ******************************************************************************/
SL_CODE_RAM void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    #if defined(SL_CATALOG_SIMPLE_BUTTON_BTN1_PRESENT)
    if (&sl_button_btn1 == handle) {
      active_sensor_id++;

      if (active_sensor_id >= sl_rail_sdk_wmbus_sensor_core_get_sensor_count()) {
        active_sensor_id = 0;
      }

      sl_rail_sdk_wmbus_sensor_core_set_active_sensor(active_sensor_id);
    }
    #endif
    #if defined(SL_CATALOG_SIMPLE_BUTTON_BTN0_PRESENT)
    if (&sl_button_btn0 == handle) {
      active_sensor->button_pressed_fnc_ptr();
    }
    #endif
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}
#endif
