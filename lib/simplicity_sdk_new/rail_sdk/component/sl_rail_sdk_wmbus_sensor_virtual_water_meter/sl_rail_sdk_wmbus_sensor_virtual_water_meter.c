/***************************************************************************//**
 * @file sl_rail_sdk_wmbus_sensor_virtual_water_meter.c
 * @brief Wireless M-Bus virtual water meter sensor.
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
#include "sl_rail_sdk_wmbus_sensor_virtual_water_meter.h"
#include "sl_rail_sdk_wmbus_sensor_virtual_water_meter_config.h"
#include "sl_rail_sdk_wmbus_support.h"
#include "sl_sleeptimer.h"
#include "app_assert.h"
#include "sl_rail_sdk_wmbus_packet_assembler.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_SEGMENT_LCD_DRIVER_PRESENT)
#include "sl_segmentlcd.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/*******************************************************************************
 * @brief Value of the VIF (Value Information Field) for the virtual water meter sensor.
 *
 * Defines the value of the VIF for the Wireless M-Bus sensor virtual water meter. The VIF is a field
 * in the Wireless M-Bus frame that provides information about the type of measurement being transmitted.
 * In this case, the VIF value is set to 0x16 what means cubic meter (m3).
 ******************************************************************************/
#define SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_VIF_VALUE 0x16

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void sl_rail_sdk_wmbus_sensor_virtual_water_meter_sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Water meter
static uint32_t water_meter;

/// Periodic Timer Handle
static sl_sleeptimer_timer_handle_t timer_handle;

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * Sleeptimer callback for the thermometer functionality. Periodically:
 * - Measure temperature and humidity
 * - Update 7 segment LCD
 *
 * @param[in] handle Not used
 * @param[in] data Not used
 *****************************************************************************/
SL_CODE_RAM static void sl_rail_sdk_wmbus_sensor_virtual_water_meter_sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;

  sl_rail_sdk_wmbus_sensor_virtual_water_meter_measure();
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Wireless M-Bus Virtual Water Meter initialization function.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_init(void)
{
  water_meter = 0;
  sl_status_t status = SL_STATUS_FAIL;

  status = sl_sleeptimer_start_periodic_timer_ms(&timer_handle,
                                                 SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_SLEEPTIMER_TIMEOUT,
                                                 sl_rail_sdk_wmbus_sensor_virtual_water_meter_sleeptimer_callback,
                                                 NULL,
                                                 SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_SLEEPTIMER_PRIORITY,
                                                 0);
  app_assert_status_f(status,
                      "[E: 0x%04x]: Failed to start periodic sleeptimer\n",
                      (int)status);

  return status;
};

/*******************************************************************************
 * Wireless M-Bus Virtual Water Meter deinitialization function.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_deinit(void)
{
  return sl_sleeptimer_stop_timer(&timer_handle);
}

/*******************************************************************************
 * Wireless M-Bus Virtual Water Meter measure function function. Read sensor data.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_measure(void)
{
  water_meter++;
  return SL_STATUS_OK;
};

/*******************************************************************************
 * Wireless M-Bus Virtual Water Meter packet assembler function.
 ******************************************************************************/
sl_rail_sdk_wmbus_sensor_data_t* sl_rail_sdk_wmbus_sensor_virtual_water_meter_get_data(void)
{
  // Virtual water meter sensor data
  static sl_rail_sdk_wmbus_sensor_data_t sensor_data_virtual_water_meter = {
    .data = 0,
    .vif = SL_RAIL_SDK_WMBUS_SENSOR_VIRTUAL_WATER_METER_VIF_VALUE
  };
  sensor_data_virtual_water_meter.data = water_meter;
  return &sensor_data_virtual_water_meter;
};

/*******************************************************************************
 * Wireless M-Bus Virtual Water Meter 7 segment LCD print function.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_print(void)
{
#if defined(SL_CATALOG_SEGMENT_LCD_DRIVER_PRESENT)
  sl_segment_lcd_number(water_meter);
#endif
  return SL_STATUS_OK;
};

/*******************************************************************************
 * Wireless M-Bus Virtual Water Meter button pressed function.
 ******************************************************************************/
sl_status_t sl_rail_sdk_wmbus_sensor_virtual_water_meter_button_pressed(void)
{
  water_meter = 0;
  return SL_STATUS_OK;
};
