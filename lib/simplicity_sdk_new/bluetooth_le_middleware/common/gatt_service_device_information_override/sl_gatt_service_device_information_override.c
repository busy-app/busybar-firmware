/***************************************************************************//**
 * @file
 * @brief Device Information GATT Service Override
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

#include <assert.h>
#include <stdint.h>
#include "sl_status.h"
#include "gatt_db.h"
#include "app_assert.h"
#include "sl_gatt_service_device_information_override.h"

// -----------------------------------------------------------------------------
// Set default values for the characteristics if not provided externally.

#if !defined(FIRMWARE_REVISION_STRING)
// Derive firmware revision string from Bluetooth major, minor and patch versions.
#include "sl_bt_version.h"
#define STR(s)  #s
#define XSTR(s)  STR(s)

#define FIRMWARE_REVISION_STRING \
  XSTR(SL_BT_VERSION_MAJOR)      \
  "." XSTR(SL_BT_VERSION_MINOR)  \
  "." XSTR(SL_BT_VERSION_PATCH)
#endif

#if !defined(MODEL_NUMBER_STRING) && defined(SL_BOARD_NAME)
// Use board name as model number string
#define MODEL_NUMBER_STRING SL_BOARD_NAME
#endif

#if !defined(HARDWARE_REVISION_STRING) && defined(SL_BOARD_REV)
// Use board revision as hardware revision string
#define HARDWARE_REVISION_STRING SL_BOARD_REV
#endif

// -----------------------------------------------------------------------------
// Check if the default values fit into the characteristic value buffers.

#if defined(gattdb_firmware_revision_string) && defined(gattdb_firmware_revision_string_len) && defined(FIRMWARE_REVISION_STRING)
#define FIRMWARE_REVISION_STRING_LEN (sizeof(FIRMWARE_REVISION_STRING) - 1)
static_assert(gattdb_firmware_revision_string_len >= FIRMWARE_REVISION_STRING_LEN,
              "Bluetooth stack version does not fit into firmware revision string characteristic. Please adjust GATT configuration.");
static const uint8_t firmware_revision_string[FIRMWARE_REVISION_STRING_LEN] = FIRMWARE_REVISION_STRING;
#endif

#if defined(gattdb_model_number_string) && defined(gattdb_model_number_string_len) && defined(MODEL_NUMBER_STRING)
#define MODEL_NUMBER_STRING_LEN (sizeof(MODEL_NUMBER_STRING) - 1)
static_assert(gattdb_model_number_string_len >= MODEL_NUMBER_STRING_LEN,
              "Board name does not fit into model number string characteristic. Please adjust GATT configuration.");
static const uint8_t model_number_string[MODEL_NUMBER_STRING_LEN] = MODEL_NUMBER_STRING;
#endif

#if defined(gattdb_hardware_revision_string) && defined(gattdb_hardware_revision_string_len) && defined(HARDWARE_REVISION_STRING)
#define HARDWARE_REVISION_STRING_LEN (sizeof(HARDWARE_REVISION_STRING) - 1)
static_assert(gattdb_hardware_revision_string_len >= HARDWARE_REVISION_STRING_LEN,
              "Board revision does not fit into hardware revision string characteristic. Please adjust GATT configuration.");
static const uint8_t hardware_revision_string[HARDWARE_REVISION_STRING_LEN] = HARDWARE_REVISION_STRING;
#endif

#if defined(gattdb_system_id) && defined(gattdb_system_id_len)
static_assert(gattdb_system_id_len == 8,
              "System ID does not fit into the System ID characteristic. Please adjust GATT configuration.");
#endif

/**************************************************************************//**
 * Bluetooth stack event handler.
 *****************************************************************************/
void sl_gatt_service_device_information_override_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  // Handle stack events
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      // Firmware Revision String
#if defined(gattdb_firmware_revision_string) && defined(gattdb_firmware_revision_string_len) && defined(FIRMWARE_REVISION_STRING)
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_firmware_revision_string,
                                                   0,
                                                   sizeof(firmware_revision_string),
                                                   firmware_revision_string);
      app_assert_status(sc);
#else
// Skip setting Firmware Revision String.
// Check the presence of this characteristic and the ID reference in the GATT
// Configurator.
#endif
      // Model Number String
#if defined(gattdb_model_number_string) && defined(gattdb_model_number_string_len) && defined(MODEL_NUMBER_STRING)
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_model_number_string,
                                                   0,
                                                   sizeof(model_number_string),
                                                   model_number_string);
      app_assert_status(sc);
#else
// Skip setting Model Number String characteristic.
// Check the presence of this characteristic and the ID reference in the GATT
// Configurator. If using a custom board, remove this section and use the
// GATT Configurator to set the value manually.
#endif

      // Hardware Revision String
#if defined(gattdb_hardware_revision_string) && defined(gattdb_hardware_revision_string_len) && defined(HARDWARE_REVISION_STRING)
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_hardware_revision_string,
                                                   0,
                                                   sizeof(hardware_revision_string),
                                                   hardware_revision_string);
      app_assert_status(sc);
#else
// Skip setting Hardware Revision String.
// Check the presence of this characteristic and the ID reference in the GATT
// Configurator. If using a custom board, remove this section and use the
// GATT Configurator to set the value manually.
#endif

      // System ID
#if defined(gattdb_system_id) && defined(gattdb_system_id_len)
      bd_addr address;
      uint8_t address_type;
      uint8_t system_id[gattdb_system_id_len];
      // Extract unique ID from BT Address.
      sc = sl_bt_gap_get_identity_address(&address, &address_type);
      app_assert_status(sc);

      // Pad and reverse unique ID to get System ID.
      system_id[0] = address.addr[5];
      system_id[1] = address.addr[4];
      system_id[2] = address.addr[3];
      system_id[3] = 0xFF;
      system_id[4] = 0xFE;
      system_id[5] = address.addr[2];
      system_id[6] = address.addr[1];
      system_id[7] = address.addr[0];

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   gattdb_system_id_len,
                                                   system_id);
      app_assert_status(sc);
#else
// Skip setting System ID.
// Check the presence of this characteristic and the ID reference in the GATT
// Configurator.
#endif
      break;

    default:
      break;
  }
}
