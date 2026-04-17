/***************************************************************************//**
 * @file
 * @brief Application Output LCD code
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
// Includes

#include <stdbool.h>
#include <stdio.h>
#include "sl_status.h"

#include "app.h"
#include "app_log.h"

#include "sl_btmesh_api.h"

#include "sl_component_catalog.h"

#ifdef SL_CATALOG_BTMESH_FACTORY_RESET_PRESENT
#include "sl_btmesh_factory_reset.h"
#endif // SL_CATALOG_BTMESH_FACTORY_RESET_PRESENT

#ifdef SL_CATALOG_BTMESH_LPN_PRESENT
#include "sl_btmesh_lpn.h"
#endif // SL_CATALOG_BTMESH_LPN_PRESENT

#include "sl_btmesh_sensor_server.h"
#include "sl_btmesh_sensor_power_consumption.h"
#include "sl_btmesh_sensor_power_consumption_config.h"

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#include "sl_btmesh_wstk_lcd.h"

#define lcd_print(...) sl_btmesh_LCD_write(__VA_ARGS__)
#else
#define lcd_print(...) SL_STATUS_OK
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/// Integer part of power consumption
#define INT_ENERGY32(x)   (x / 1000)
/// Fractional part of power consumption
#define FRAC_ENERGY32(x)  (x % 1000)

// -----------------------------------------------------------------------------
// Event / callback definitions

/*******************************************************************************
 * Called when the Low Power Node is initialized.
 ******************************************************************************/
void sl_btmesh_lpn_on_init(void)
{
  app_log("BT mesh LPN on" APP_LOG_NL);
}

/*******************************************************************************
 * Called when the Low Power Node is deinitialized.
 ******************************************************************************/
void sl_btmesh_lpn_on_deinit(void)
{
  app_log("BT mesh LPN off" APP_LOG_NL);
}

/*******************************************************************************
 * Called when the Low Power Node establishes friendship with another node
 ******************************************************************************/
void sl_btmesh_lpn_on_friendship_established(uint16_t node_address)
{
  app_log("BT mesh LPN with friend (node address: 0x%04x)" APP_LOG_NL, node_address);
}

/*******************************************************************************
 * Called when the friendship establishment attempt of Low Power Node fails
 ******************************************************************************/
void sl_btmesh_lpn_on_friendship_failed(uint16_t reason)
{
  app_log("BT mesh No friend (reason: 0x%04x)" APP_LOG_NL, reason);
}

/*******************************************************************************
 * Called when friendship that was successfully established has been terminated
 ******************************************************************************/
void sl_btmesh_lpn_on_friendship_terminated(uint16_t reason)
{
  app_log("BT mesh Friend lost (reason: 0x%04x)" APP_LOG_NL, reason);
}

/*******************************************************************************
 * Called when full reset is established, before system reset
 ******************************************************************************/
void sl_btmesh_factory_reset_on_full_reset(void)
{
  app_log("Factory reset" APP_LOG_NL);
}

/*******************************************************************************
 * Called when node reset is established, before system reset
 ******************************************************************************/
void sl_btmesh_factory_reset_on_node_reset(void)
{
  app_log("Node reset" APP_LOG_NL);
}

/*******************************************************************************
 *  Called when a power consumption measurement is done
 ******************************************************************************/
void sl_btmesh_sensor_server_on_energy_monitor_measurement(energy32_t pwr_consumption)
{
  static energy32_t prev_pwr = 0;
  if (prev_pwr == pwr_consumption) {
    return;
  }
  prev_pwr = pwr_consumption;
  if (pwr_consumption == SL_BTMESH_SENSOR_POWER_CONSUMPTION_VALUE_UNKNOWN) {
    app_log("Power consumption unknown" APP_LOG_NL);
  } else if (pwr_consumption == SL_BTMESH_SENSOR_POWER_CONSUMPTION_VALUE_INVALID) {
    app_log("Power consumption invalid" APP_LOG_NL);
  } else {
    app_log("Power consumption: %5lu.%3lukWh" APP_LOG_NL,
            INT_ENERGY32(pwr_consumption),
            FRAC_ENERGY32(pwr_consumption));
  }
}

/*******************************************************************************
 *  Called at node initialization time to provide provisioning information
 ******************************************************************************/
void sl_btmesh_on_provision_init_status(bool provisioned,
                                        uint16_t address,
                                        uint32_t iv_index)
{
  if (provisioned) {
    app_show_btmesh_node_provisioned(address, iv_index);
  } else {
    app_log("BT mesh node is unprovisioned, started unprovisioned beaconing..." APP_LOG_NL);
  }
}

/*******************************************************************************
 *  Called from sl_btmesh_on_node_provisioning_started callback in app.c
 ******************************************************************************/
void app_show_btmesh_node_provisioning_started(uint16_t result)
{
  app_log("BT mesh node provisioning is started (result: 0x%04x)" APP_LOG_NL, result);
  (void)result;
}

/*******************************************************************************
 *  Called from sl_btmesh_on_node_provisioned callback in app.c
 ******************************************************************************/
void app_show_btmesh_node_provisioned(uint16_t address,
                                      uint32_t iv_index)
{
  app_log("BT mesh node is provisioned (address: 0x%04x, iv_index: 0x%lx)" APP_LOG_NL,
          address,
          iv_index);
  (void)address;
  (void)iv_index;
}

/*******************************************************************************
 *  Called from sl_btmesh_on_node_provisioning_failed callback in app.c
 ******************************************************************************/
void app_show_btmesh_node_provisioning_failed(uint16_t result)
{
  app_log("BT mesh node provisioning failed (result: 0x%04x)" APP_LOG_NL, result);
  (void)result;
}
