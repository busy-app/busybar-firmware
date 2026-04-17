/***************************************************************************//**
 * @file
 * @brief Thunderboard demo application
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

#include <stdbool.h>
#include "em_emu.h"
#include "em_gpio.h"
#include "sl_status.h"
#include "sl_simple_button_instances.h"
#include "app_log.h"
#include "app_assert.h"
#include "sl_main_init.h"
#include "sl_bluetooth.h"
#include "app_timer.h"
#include "advertise.h"
#include "sl_power_supply.h"
#include "board.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_GATT_SERVICE_AIO_PRESENT
#include "sl_gatt_service_aio.h"
#endif // SL_CATALOG_GATT_SERVICE_AIO_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_BATTERY_PRESENT
#include "sl_gatt_service_battery.h"
#endif // SL_CATALOG_GATT_SERVICE_BATTERY_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_HALL_PRESENT
#include "sl_gatt_service_hall.h"
#include "sensor_hall.h"
#endif // SL_CATALOG_GATT_SERVICE_HALL_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_LIGHT_PRESENT
#include "sl_gatt_service_light.h"
#endif // SL_CATALOG_GATT_SERVICE_LIGHT_PRESENT
#ifdef SL_CATALOG_SENSOR_LIGHT_PRESENT
#include "sl_sensor_light.h"
#endif // SL_CATALOG_SENSOR_LIGHT_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_LUX_PRESENT
#include "sl_gatt_service_lux.h"
#endif // SL_CATALOG_GATT_SERVICE_LUX_PRESENT
#ifdef SL_CATALOG_SENSOR_LUX_PRESENT
#include "sl_sensor_lux.h"
#endif // SL_CATALOG_SENSOR_LUX_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_RHT_PRESENT
#include "sl_gatt_service_rht.h"
#endif // SL_CATALOG_GATT_SERVICE_RHT_PRESENT
#ifdef SL_CATALOG_SENSOR_RHT_PRESENT
#include "sl_sensor_rht.h"
#endif // SL_CATALOG_SENSOR_RHT_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_IMU_PRESENT
#include "sl_gatt_service_imu.h"
#include "sensor_imu.h"
#endif // SL_CATALOG_GATT_SERVICE_IMU_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_RGB_PRESENT
#include "sl_gatt_service_rgb.h"
#endif // SL_CATALOG_GATT_SERVICE_RGB_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT
#include "sl_gatt_service_pressure.h"
#include "sensor_pressure.h"
#endif // SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_SOUND_PRESENT
#include "sl_gatt_service_sound.h"
#include "sensor_sound.h"
#endif // SL_CATALOG_GATT_SERVICE_SOUND_PRESENT

// -----------------------------------------------------------------------------
// Configuration
#define SHUTDOWN_TIMEOUT_MS             60000

// -----------------------------------------------------------------------------
// Private variables
// Timer
static app_timer_t shutdown_timer;

// -----------------------------------------------------------------------------
// Private function declarations
static void shutdown_start_timer(void);
static void shutdown_stop_timer(void);
static void shutdown(app_timer_t *timer, void *data);
static void sensor_init(void);
static void sensor_deinit(void);

// -----------------------------------------------------------------------------
// Public function definitions
void app_init(void)
{
  app_log_info("Silicon Labs Thunderboard / DevKit demo" APP_LOG_NL);
  sl_power_supply_probe();
  shutdown_start_timer();
}

void app_process_action(void)
{
  #ifdef SL_CATALOG_GATT_SERVICE_SOUND_PRESENT
  sensor_sound_step();
  #endif // SL_CATALOG_GATT_SERVICE_SOUND_PRESENT

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

// -----------------------------------------------------------------------------
// Bluetooth event handler
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  uint32_t unique_id;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    case sl_bt_evt_system_boot_id:
      // Print boot message.
      app_log_info("Bluetooth stack booted: v%d.%d.%d+%08lx" APP_LOG_NL,
                   evt->data.evt_system_boot.major,
                   evt->data.evt_system_boot.minor,
                   evt->data.evt_system_boot.patch,
                   evt->data.evt_system_boot.hash);
      sc = sl_bt_gap_get_identity_address(&address, &address_type);
      app_assert_status(sc);
      app_log_info("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X" APP_LOG_NL,
                   address_type ? "static random" : "public device",
                   address.addr[5],
                   address.addr[4],
                   address.addr[3],
                   address.addr[2],
                   address.addr[1],
                   address.addr[0]);
      unique_id = 0xFFFFFF & *((uint32_t*) address.addr);
      advertise_init(unique_id);
      break;

    // -------------------------------
    case sl_bt_evt_connection_opened_id:
      app_log_info("Connection opened" APP_LOG_NL);
      advertise_stop();
      shutdown_stop_timer();
      sensor_init();
      break;

    // -------------------------------
    case sl_bt_evt_connection_closed_id:
      app_log_info("Connection closed" APP_LOG_NL);
      shutdown_start_timer();
      sensor_deinit();
      advertise_start();
      break;

    // -------------------------------
    default:
      break;
  }
}

// -----------------------------------------------------------------------------
// Push button event handler
void sl_button_on_change(const sl_button_t *handle)
{
  (void)handle;
#ifdef SL_CATALOG_GATT_SERVICE_AIO_PRESENT
  sl_gatt_service_aio_on_change();
#endif // SL_CATALOG_GATT_SERVICE_AIO_PRESENT
}

// -----------------------------------------------------------------------------
// Shutdown feature function definitions
static void shutdown(app_timer_t *timer, void *data)
{
  (void)timer;
  (void)data;

  advertise_stop();
  EMU_EnterEM4();
}

static void shutdown_start_timer(void)
{
  sl_status_t sc;
  if (sl_power_supply_is_low_power()) {
    sc = app_timer_start(&shutdown_timer, SHUTDOWN_TIMEOUT_MS, shutdown, NULL, false);
    app_assert_status(sc);
  }
}

static void shutdown_stop_timer(void)
{
  sl_status_t sc;
  sc = app_timer_stop(&shutdown_timer);
  app_assert_status(sc);
}

// -----------------------------------------------------------------------------
// Sensor batch init/deinit
static void sensor_init(void)
{
  sl_status_t sc;
#ifdef SL_CATALOG_GATT_SERVICE_HALL_PRESENT
  sc = sensor_hall_init();
  if (sc != SL_STATUS_OK) {
    app_log_warning("Hall sensor initialization failed" APP_LOG_NL);
  }
#endif // SL_CATALOG_GATT_SERVICE_HALL_PRESENT
#ifdef SL_CATALOG_SENSOR_LIGHT_PRESENT
  sc = sl_sensor_light_init();
  if (sc != SL_STATUS_OK) {
    app_log_warning("Ambient light and UV index sensor initialization failed" APP_LOG_NL);
  }
#endif // SL_CATALOG_SENSOR_LIGHT_PRESENT
#ifdef SL_CATALOG_SENSOR_LUX_PRESENT
  sc = sl_sensor_lux_init();
  if (sc != SL_STATUS_OK) {
    app_log_warning("Ambient light sensor initialization failed" APP_LOG_NL);
  }
#endif // SL_CATALOG_SENSOR_LUX_PRESENT
#ifdef SL_CATALOG_SENSOR_RHT_PRESENT
  sc = sl_sensor_rht_init();
  if (sc != SL_STATUS_OK) {
    app_log_warning("Relative Humidity and Temperature sensor initialization failed" APP_LOG_NL);
  }
#endif // SL_CATALOG_SENSOR_RHT_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_IMU_PRESENT
  sensor_imu_init();
#endif // SL_CATALOG_GATT_SERVICE_IMU_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT
  sc = sensor_pressure_init();
  if (sc != SL_STATUS_OK) {
    app_log_warning("Air Pressure sensor initialization failed" APP_LOG_NL);
  }
#endif // SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_SOUND_PRESENT
  sc = sensor_sound_init();
  if (sc != SL_STATUS_OK) {
    app_log_warning("Sound level sensor initialization failed" APP_LOG_NL);
  }
#endif // SL_CATALOG_GATT_SERVICE_SOUND_PRESENT
}

static void sensor_deinit(void)
{
#ifdef SL_CATALOG_GATT_SERVICE_HALL_PRESENT
  sensor_hall_deinit();
#endif // SL_CATALOG_GATT_SERVICE_HALL_PRESENT
#ifdef SL_CATALOG_SENSOR_LIGHT_PRESENT
  sl_sensor_light_deinit();
#endif // SL_CATALOG_SENSOR_LIGHT_PRESENT
#ifdef SL_CATALOG_SENSOR_LUX_PRESENT
  sl_sensor_lux_deinit();
#endif // SL_CATALOG_SENSOR_LUX_PRESENT
#ifdef SL_CATALOG_SENSOR_RHT_PRESENT
  sl_sensor_rht_deinit();
#endif // SL_CATALOG_SENSOR_RHT_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_IMU_PRESENT
  sensor_imu_deinit();
#endif // SL_CATALOG_GATT_SERVICE_IMU_PRESENT
#if defined(BOARD_RGBLED_COUNT) && (BOARD_RGBLED_COUNT > 0)
  // Turn off RGBLED
  rgb_led_set(0u, 0u, 0u, 0u);
#endif // BOARD_RGBLED_COUNT
#ifdef SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT
  sensor_pressure_deinit();
#endif // SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_SOUND_PRESENT
  sensor_sound_deinit();
#endif // SL_CATALOG_GATT_SERVICE_SOUND_PRESENT
}

// -----------------------------------------------------------------------------
// Connect GATT services with sensors by overriding weak functions

#ifdef SL_CATALOG_GATT_SERVICE_BATTERY_PRESENT
uint8_t sl_gatt_service_battery_get_level(void)
{
  uint8_t bat_level;
  bat_level = sl_power_supply_get_battery_level();
  app_log_info("Battery level = %d %%" APP_LOG_NL, bat_level);
  return bat_level;
}

uint8_t sl_gatt_service_battery_get_type(void)
{
  return sl_power_supply_get_type();
}
#endif // SL_CATALOG_GATT_SERVICE_BATTERY_PRESENT

#if defined(SL_CATALOG_GATT_SERVICE_HALL_PRESENT)
sl_status_t sl_gatt_service_hall_get(float *field_strength, bool *alert, bool *tamper)
{
  sl_status_t sc;
  sc = sensor_hall_get(field_strength, alert, tamper);
  if (SL_STATUS_OK == sc) {
    app_log_info("Magnetic flux = %4.3f mT" APP_LOG_NL, (double)*field_strength);
  } else if (SL_STATUS_NOT_INITIALIZED == sc) {
    app_log_info("Hall sensor is not initialized" APP_LOG_NL);
  } else {
    app_log_status_error_f(sc, "Hall sensor measurement failed" APP_LOG_NL);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_LIGHT_PRESENT) && defined(SL_CATALOG_SENSOR_LIGHT_PRESENT)
sl_status_t sl_gatt_service_light_get(float *lux, float *uvi)
{
  sl_status_t sc;
  sc = sl_sensor_light_get(lux, uvi);
  if (SL_STATUS_OK == sc) {
    app_log_info("Ambient light = %f lux" APP_LOG_NL, (double)*lux);
    app_log_info("UV Index = %u" APP_LOG_NL, (unsigned int)*uvi);
  } else if (SL_STATUS_NOT_INITIALIZED == sc) {
    app_log_info("Ambient light and UV index sensor is not initialized" APP_LOG_NL);
  } else {
    app_log_status_error_f(sc, "Light sensor measurement failed" APP_LOG_NL);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_LUX_PRESENT) && defined(SL_CATALOG_SENSOR_LUX_PRESENT)
sl_status_t sl_gatt_service_lux_get(float *lux)
{
  sl_status_t sc;
  sc = sl_sensor_lux_get(lux);
  if (SL_STATUS_OK == sc) {
    app_log_info("Ambient light = %f lux" APP_LOG_NL, (double)*lux);
  } else if (SL_STATUS_NOT_INITIALIZED == sc) {
    app_log_info("Ambient light sensor is not initialized" APP_LOG_NL);
  } else {
    app_log_status_error_f(sc, "Light sensor measurement failed" APP_LOG_NL);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_RHT_PRESENT) && defined(SL_CATALOG_SENSOR_RHT_PRESENT)
sl_status_t sl_gatt_service_rht_get(uint32_t *rh, int32_t *t)
{
  sl_status_t sc;
  sc = sl_sensor_rht_get(rh, t);
  if (SL_STATUS_OK == sc) {
    app_log_info("Humidity = %3.2f %%RH" APP_LOG_NL, (double)*rh / 1000.0);
    app_log_info("Temperature = %3.2f C" APP_LOG_NL, (double)*t / 1000.0);
  } else if (SL_STATUS_NOT_INITIALIZED == sc) {
    app_log_info("Relative Humidity and Temperature sensor is not initialized" APP_LOG_NL);
  } else {
    app_log_status_error_f(sc, "RHT sensor measurement failed" APP_LOG_NL);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_IMU_PRESENT)
sl_status_t sl_gatt_service_imu_get(int16_t ovec[3], int16_t avec[3])
{
  sl_status_t sc;
  sc = sensor_imu_get(ovec, avec);
  if (SL_STATUS_OK == sc) {
    app_log_info("IMU: ORI : %04d,%04d,%04d" APP_LOG_NL, ovec[0], ovec[1], ovec[2]);
    app_log_info("IMU: ACC : %04d,%04d,%04d" APP_LOG_NL, avec[0], avec[1], avec[2]);
  } else if (SL_STATUS_NOT_INITIALIZED == sc) {
    app_log_info("Inertial Measurement Unit is not initialized" APP_LOG_NL);
  }
  return sc;
}

sl_status_t sl_gatt_service_imu_calibrate(void)
{
  sl_status_t sc;
  sc = sensor_imu_calibrate();
  if (SL_STATUS_NOT_INITIALIZED == sc) {
    app_log_info("Inertial Measurement Unit is not initialized" APP_LOG_NL);
  } else {
    app_log_info("IMU calibration status: %ld" APP_LOG_NL, sc);
  }
  return sc;
}

void sl_gatt_service_imu_enable(bool enable)
{
  sl_status_t sc;
  app_log_info("IMU %sable" APP_LOG_NL, enable ? "en" : "dis");
  sc = sensor_imu_enable(enable);
  if (enable && SL_STATUS_OK != sc) {
    app_log_warning("Inertial Measurement Unit initialization failed" APP_LOG_NL);
  }
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_RGB_PRESENT) && defined(BOARD_RGBLED_COUNT) && (BOARD_RGBLED_COUNT > 0)
void sl_gatt_service_rgb_set_led(uint8_t m, uint8_t r, uint8_t g, uint8_t b)
{
  if (!sl_power_supply_is_low_power()) {
    rgb_led_set(m, r, g, b);
    app_log_info("RGBLED write: m:%02x r:%02x g:%02x b:%02x" APP_LOG_NL, m, r, g, b);
  }
}

uint8_t sl_gatt_service_rgb_get_led_mask(void)
{
  return BOARD_RGBLED_MASK;
}
#endif // BOARD_RGBLED_COUNT

#if defined(SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT)
sl_status_t sl_gatt_service_pressure_get(float *pressure)
{
  sl_status_t sc;
  sc = sensor_pressure_get(pressure);
  if (SL_STATUS_OK == sc) {
    app_log_info("Pressure = %0.3f mbar" APP_LOG_NL, (double)*pressure);
  } else if (SL_STATUS_NOT_INITIALIZED == sc) {
    app_log_info("Air pressure sensor is not initialized" APP_LOG_NL);
  } else {
    app_log_status_error_f(sc, "Pressure sensor measurement failed" APP_LOG_NL);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_SOUND_PRESENT)
sl_status_t sl_gatt_service_sound_get(float *sound_level)
{
  sl_status_t sc;
  sc = sensor_sound_get(sound_level);
  if (SL_STATUS_OK == sc) {
    app_log_info("Sound level = %3.2f dBA" APP_LOG_NL, (double)*sound_level);
  } else if (SL_STATUS_NOT_INITIALIZED == sc) {
    app_log_info("Sound level sensor is not initialized" APP_LOG_NL);
  } else {
    app_log_status_error_f(sc, "Sound level measurement failed" APP_LOG_NL);
  }
  return sc;
}
#endif
