/***************************************************************************/ /**
 * @file
 * @brief Application Over-the-Air Device Firmware Update
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
#include <stdbool.h>
#include <stddef.h>
#include "sl_common.h"
#include "sl_bt_api.h"
#include "gatt_db.h"
#include "btl_interface.h"
#include "btl_interface_storage.h"
#include "app_rta.h"
#include "sli_bt_app_ota_dfu.h"
#include "sl_bt_app_ota_dfu.h"
#include "sl_bt_app_ota_dfu_config.h"

// -----------------------------------------------------------------------------
// Definitions

/// Possible control words of OTA control characteristic
#define SL_BT_APP_OTA_DFU_BEGIN_PROGRESS       0u
#define SL_BT_APP_OTA_DFU_END_PROGRESS         3u
#define SL_BT_APP_OTA_DFU_CLOSE_CONNECTION     4u

#define ATT_ERR_WRITE_REQUEST_REJECTED         0x80u
#define ATT_ERR_NOT_READY_FOR_WRITE_REQUEST    0x81u
#define ATT_ERR_STORAGE_FULL                   0x82u
#define ATT_ERR_PACKAGE_LOST                   0x83u

// Resizing to fit the 32 bit target architecture.
#define SL_BT_APP_OTA_DFU_FLASH_VERIFICATION_CONTEXT_SIZE \
  (uint16_t)(BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE / sizeof(uint32_t))

#ifndef SL_BT_INVALID_CONNECTION_HANDLE
#define SL_BT_INVALID_CONNECTION_HANDLE ((uint8_t) 0xFF)
#endif // SL_BT_INVALID_CONNECTION_HANDLE

/// App OTA structure
typedef struct app_ota_s {
  uint8_t                        connection;
  struct {
    sl_bt_app_ota_dfu_status_t   current;
    sl_bt_app_ota_dfu_status_t   previous;
  }                              status;
  uint32_t                       image_position;
  BootloaderInformation_t        bootloader_info;
  BootloaderStorageSlot_t        slot_info;
  uint32_t                       buffer[SL_BT_APP_OTA_DFU_FLASH_VERIFICATION_CONTEXT_SIZE];
  sl_bt_app_ota_dfu_status_evt_t event;
  app_rta_context_t              ctx;
} app_ota_t;

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static void app_ota_dfu_step(void);
static void set_main_status(sl_bt_app_ota_dfu_status_t new_status,
                            sl_bt_app_ota_dfu_error_t ota_error,
                            int32_t btl_ret_val);
static void on_runtime_error(app_rta_error_t error, sl_status_t result);
static void on_error(sl_bt_app_ota_dfu_error_t ota_error, int32_t btl_api_retval);
static bool init_state(bool start);

// -----------------------------------------------------------------------------
// Private variables

/// App OTA instance
static app_ota_t app_ota;

// -----------------------------------------------------------------------------
// Internal functions

// Application OTA DFU initialization.
void sli_bt_app_ota_dfu_init(void)
{
  // Create context
  sl_status_t sc;
  app_rta_config_t config = {
    .requirement.runtime = true,
    .requirement.guard   = true,
    .requirement.signal  = true,
    .step                = app_ota_dfu_step,
    .priority            = SL_BT_APP_OTA_DFU_CONFIG_PRIORITY,
    .stack_size          = SL_BT_APP_OTA_DFU_CONFIG_STACK,
    .error               = on_runtime_error,
    .wait_for_guard      = SL_BT_APP_OTA_DFU_CONFIG_WAIT
  };
  sc = app_rta_create_context(&config, &app_ota.ctx);
  if (sc != SL_STATUS_OK) {
    on_runtime_error(APP_RTA_ERROR_RUNTIME_INIT_FAILED, sc);
    // Stop execution and wait for application intervention.
    app_ota.status.current = SL_BT_APP_OTA_DFU_ERROR;
  }
}

// Finalize initialization. (App RTA calls that requires scheduler.)
void sli_bt_app_ota_dfu_rta_ready(void)
{
  // Initialize state
  if (!init_state(true)) {
    on_runtime_error(APP_RTA_ERROR_RUNTIME_INIT_FAILED, SL_STATUS_FAIL);
    return;
  }

  sl_status_t sc;
  sc = app_rta_acquire(app_ota.ctx);
  if (sc == SL_STATUS_OK) {
    if (app_ota.status.current == SL_BT_APP_OTA_DFU_UNINIT) {
      set_main_status(SL_BT_APP_OTA_DFU_INIT, SL_BT_APP_OTA_DFU_NO_ERROR, sc);
    }
    (void)app_rta_release(app_ota.ctx);
  } else {
    on_runtime_error(APP_RTA_ERROR_ACQUIRE_FAILED, sc);
  }
}

void sli_bt_app_ota_dfu_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint8_t attr_status = SL_STATUS_OK;
  uint8_t connection;
  uint32_t characteristic;

  int32_t btl_ret_val = BOOTLOADER_OK;
  sl_bt_app_ota_dfu_error_t ota_error = SL_BT_APP_OTA_DFU_NO_ERROR;
  sl_bt_app_ota_dfu_status_t req_sts = SL_BT_APP_OTA_DFU_UNINIT;

  // Handle stack events
  switch (SL_BT_MSG_ID(evt->header)) {
    ///////////////////////////////////////////////////////////////////////////
    // OTA package write request event                                       //
    ///////////////////////////////////////////////////////////////////////////
    case sl_bt_evt_gatt_server_user_write_request_id:
      sc = app_rta_acquire(app_ota.ctx);
      if (sc != SL_STATUS_OK) {
        on_runtime_error(APP_RTA_ERROR_ACQUIRE_FAILED, sc);
        break;
      }
      // Get characteristic and connection handle
      characteristic = evt->data.evt_gatt_server_user_write_request.characteristic;
      connection = evt->data.evt_gatt_server_user_write_request.connection;
      attr_status = SL_STATUS_OK;

      if (characteristic == gattdb_ota_control) {
        // OTA control characteristic detected.
        switch (evt->data.evt_gatt_server_user_write_request.value.data[0]) {
          case SL_BT_APP_OTA_DFU_BEGIN_PROGRESS:
            // Accept OTA begin request from single connection source, only!
            if (app_ota.status.current == SL_BT_APP_OTA_DFU_READY
                && (app_ota.connection == SL_BT_INVALID_CONNECTION_HANDLE
                    || app_ota.connection == connection)) {
              // Application ready for the OTA DFU process.
              req_sts = SL_BT_APP_OTA_DFU_DOWNLOAD_BEGIN;
              app_ota.image_position = 0;
              // save connection handle for later error checking / handling
              app_ota.connection = connection;
            } else {
              // Application not ready for the OTA DFU process.
              req_sts = SL_BT_APP_OTA_DFU_ERROR;
              ota_error = SL_BT_APP_OTA_DFU_ERR_UNEXPECTED_TRANSFER;
            }
            break;

          case SL_BT_APP_OTA_DFU_END_PROGRESS:
            // Wait for connection close and then reboot.
            req_sts = SL_BT_APP_OTA_DFU_DOWNLOAD_END;
            break;

          case SL_BT_APP_OTA_DFU_CLOSE_CONNECTION:
            // Requested to close connection.
            req_sts = SL_BT_APP_OTA_DFU_DISCONNECT;
            break;

          default:
            break;
        }
      } else if (characteristic == gattdb_ota_data
                 && app_ota.connection == connection) {
        // OTA data characteristic detected - handle only if the connection is the initiator one.
        if (app_ota.status.current == SL_BT_APP_OTA_DFU_DOWNLOAD_BEGIN) {
          if (app_ota.image_position >= (app_ota.slot_info.length)) {
            req_sts = SL_BT_APP_OTA_DFU_ERROR;
            ota_error = SL_BT_APP_OTA_DFU_ERR_STORAGE_FULL;
            attr_status = ATT_ERR_STORAGE_FULL;
          } else {
            //Additional buffer required to avoid passing unaligned data to the bootloader API.
            //The data element can be unaligned.
            memcpy(app_ota.buffer,
                   evt->data.evt_gatt_server_user_write_request.value.data,
                   evt->data.evt_gatt_server_user_write_request.value.len);
            btl_ret_val =
              bootloader_writeStorage(SL_BT_APP_OTA_DFU_USED_SLOT,
                                      app_ota.image_position,
                                      (uint8_t *)app_ota.buffer,
                                      evt->data.evt_gatt_server_user_write_request.value.len);
            // Check write storage results
            if (btl_ret_val != BOOTLOADER_OK) {
              // Write failed.
              req_sts = SL_BT_APP_OTA_DFU_ERROR;
              ota_error = SL_BT_APP_OTA_DFU_ERR_BOOTLOADER_API;
              attr_status = ATT_ERR_PACKAGE_LOST;
            } else {
              // Stay in download state until process finished.
              req_sts = SL_BT_APP_OTA_DFU_DOWNLOAD_BEGIN;
              // Write successful. Update image position.
              app_ota.image_position +=
                evt->data.evt_gatt_server_user_write_request.value.len;
              attr_status = SL_STATUS_OK;
            }
          }
        } else {
          // Unexpected download has to be aborted.
          // Application OTA not ready or error occurred during
          // ongoing download process.
          req_sts = SL_BT_APP_OTA_DFU_ERROR;
          attr_status = ATT_ERR_NOT_READY_FOR_WRITE_REQUEST;
        }
      } else {
        // The write request was not Application OTA DFU relevant.
        // Avoid doing anything with it.
        (void)app_rta_release(app_ota.ctx);
        break;
      }

      // Forward download information to application.
      app_ota.event.event_id = SL_BT_APP_OTA_DFU_EVT_DOWNLOAD_PACKET_ID;
      app_ota.event.ota_error_code = ota_error;
      app_ota.event.btl_api_retval = btl_ret_val;
      app_ota.event.evt_info.download_packet.connection_handle = connection;
      app_ota.event.evt_info.download_packet.write_image_position = app_ota.image_position;
      sl_bt_app_ota_dfu_on_status_event(&app_ota.event);

      // Send write-response to GATT device.
      sl_bt_gatt_server_send_user_write_response(connection,
                                                 characteristic,
                                                 attr_status);
      // Update state if necessary.
      if (app_ota.status.current != req_sts) {
        set_main_status(req_sts, ota_error, btl_ret_val);
      }
      (void)app_rta_release(app_ota.ctx);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Bluetooth connection closed event                                     //
    ///////////////////////////////////////////////////////////////////////////
    case sl_bt_evt_connection_closed_id:
      sc = app_rta_acquire(app_ota.ctx);
      if (sc != SL_STATUS_OK) {
        on_runtime_error(APP_RTA_ERROR_ACQUIRE_FAILED, sc);
        break;
      }
      connection = evt->data.evt_connection_closed.connection;
      // Ignore all close events other than the connection that did the update.
      if (app_ota.connection == connection) {
        if (app_ota.status.current == SL_BT_APP_OTA_DFU_DOWNLOAD_END) {
          // Only If the download finished successfully, the image verification
          // will start automatically.

          // Try to prepare for verification.
          btl_ret_val = bootloader_initVerifyImage(SL_BT_APP_OTA_DFU_USED_SLOT,
                                                   app_ota.buffer,
                                                   sizeof(app_ota.buffer));
          if (btl_ret_val != BOOTLOADER_OK) {
            // Verification init failed.
            req_sts = SL_BT_APP_OTA_DFU_ERROR;
            ota_error = SL_BT_APP_OTA_DFU_ERR_BOOTLOADER_API;
          } else {
            req_sts = SL_BT_APP_OTA_DFU_VERIFY;
            ota_error = SL_BT_APP_OTA_DFU_NO_ERROR;
            // Reuse image position to store verification position.
            app_ota.image_position = 0u;
            app_ota.event.ota_error_code = SL_BT_APP_OTA_DFU_NO_ERROR;
          }

          set_main_status(req_sts, ota_error, btl_ret_val);
        } else if (app_ota.status.current == SL_BT_APP_OTA_DFU_DOWNLOAD_BEGIN) {
          // Connection should not be closed during an update!
          req_sts = SL_BT_APP_OTA_DFU_ERROR;
          app_ota.event.ota_error_code = SL_BT_APP_OTA_DFU_ERR_UNEXPECTED_CLOSE;
        } else {
          // Restart handler just in case.
          req_sts = SL_BT_APP_OTA_DFU_INIT;
          app_ota.event.ota_error_code = SL_BT_APP_OTA_DFU_NO_ERROR;
        }

        // Don't call status update again if verify init was already successful.
        if (app_ota.status.current != req_sts) {
          set_main_status(req_sts, ota_error, btl_ret_val);
        }
        // Forget the OTA connection handle
        app_ota.connection = SL_BT_INVALID_CONNECTION_HANDLE;
        // Continue execution.
        (void)app_rta_proceed(app_ota.ctx);
      }
      (void)app_rta_release(app_ota.ctx);
      break;

    default:
      break;
  }
}

// -----------------------------------------------------------------------------
// Private functions

static void app_ota_dfu_step(void)
{
  int32_t btl_ret_val = BOOTLOADER_OK;
  sl_status_t sc;
  sl_bt_app_ota_dfu_error_t ota_error = SL_BT_APP_OTA_DFU_NO_ERROR;
  sl_bt_app_ota_dfu_status_t req_sts = SL_BT_APP_OTA_DFU_UNINIT;
  bool dirty = false;
  uint32_t offset = 0, num_blocks = 0, i = 0;
  // Resizing to fit the 32 bit target architecture.
  uint16_t data_fragments = \
    (uint16_t)(SL_BT_APP_OTA_DFU_READ_STORAGE_CONTEXT_SIZE / sizeof(uint32_t));

  sc = app_rta_acquire(app_ota.ctx);
  if (sc != SL_STATUS_OK) {
    on_runtime_error(APP_RTA_ERROR_ACQUIRE_FAILED, sc);
    return;
  }

  switch (app_ota.status.current) {
    ///////////////////////////////////////////////////////////////////////////
    // Pre initialization state.                                             //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_UNINIT:
      // Intentionally left blank.
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Bootloader API initialized, check storage content.                    //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_INIT:
      // Get bootloader information.
      bootloader_getInfo(&app_ota.bootloader_info);
      // Get storage slot information.
      btl_ret_val =
        bootloader_getStorageSlotInfo(SL_BT_APP_OTA_DFU_USED_SLOT, &app_ota.slot_info);

      if (btl_ret_val == BOOTLOADER_OK) {
        // Forward bootloader and storage information to application.
        app_ota.event.event_id = SL_BT_APP_OTA_DFU_EVT_BTL_STORAGE_INFO_ID;
        app_ota.event.ota_error_code = SL_BT_APP_OTA_DFU_NO_ERROR;
        app_ota.event.btl_api_retval = btl_ret_val;
        app_ota.event.evt_info.btl_storage.bootloader_type = app_ota.bootloader_info.type;
        app_ota.event.evt_info.btl_storage.bootloader_ver = app_ota.bootloader_info.version;
        app_ota.event.evt_info.btl_storage.storage_size_bytes = app_ota.slot_info.length;
        app_ota.event.evt_info.btl_storage.storage_start_addr = app_ota.slot_info.address;
        sl_bt_app_ota_dfu_on_status_event(&app_ota.event);
        // Proceed to read flash state to determine if erase is necessary or not.
        req_sts = SL_BT_APP_OTA_DFU_READ_FLASH;
      } else {
        // Getting bootloader and storage information failed.
        req_sts = SL_BT_APP_OTA_DFU_ERROR;
        ota_error = SL_BT_APP_OTA_DFU_ERR_BOOTLOADER_API;
      }
      // Change to next state.
      set_main_status(req_sts, ota_error, btl_ret_val);
      break;
    ///////////////////////////////////////////////////////////////////////////
    // Check flash storage slot.                                             //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_READ_FLASH:
      // Check the download area content by reading it in blocks.
      num_blocks = app_ota.slot_info.length / SL_BT_APP_OTA_DFU_READ_STORAGE_CONTEXT_SIZE;
      // Run through the full storage and read the context in chunks.
      while ((dirty == false)
             && (offset < SL_BT_APP_OTA_DFU_READ_STORAGE_CONTEXT_SIZE * num_blocks)
             && (btl_ret_val == BOOTLOADER_OK)) {
        // Actual storage content read.
        btl_ret_val =
          bootloader_readStorage(SL_BT_APP_OTA_DFU_USED_SLOT,
                                 offset,
                                 (uint8_t *)app_ota.buffer,
                                 SL_BT_APP_OTA_DFU_READ_STORAGE_CONTEXT_SIZE);

        if (btl_ret_val == BOOTLOADER_OK) {
          // Run through the chunk in 32 bit fragments and check if its empty or not.
          for (i = 0; i < data_fragments && !dirty; i++) {
            if (app_ota.buffer[i] != SL_BT_APP_OTA_DFU_EMPTY_FLASH_CONTENT) {
              dirty = true;
            }
          }
          // Proceed with next data chunk.
          offset += SL_BT_APP_OTA_DFU_READ_STORAGE_CONTEXT_SIZE;
        }
      }
      // Check results.
      if (btl_ret_val != BOOTLOADER_OK) {
        // Bootloader API issue during reading flash content.
        ota_error = SL_BT_APP_OTA_DFU_ERR_BOOTLOADER_API;
        req_sts = SL_BT_APP_OTA_DFU_ERROR;
      } else if (dirty) {
        // Storage space is not empty proceed to erase.
        req_sts = SL_BT_APP_OTA_DFU_ERASE;
      } else {
        // Storage space was originally empty, proceed to next
        req_sts = SL_BT_APP_OTA_DFU_READY;
      }
      // Change to next state.
      set_main_status(req_sts, ota_error, btl_ret_val);
      break;
    ///////////////////////////////////////////////////////////////////////////
    // Erase storage slot.                                                   //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_ERASE:
      btl_ret_val = bootloader_eraseStorageSlot(SL_BT_APP_OTA_DFU_USED_SLOT);
      if (btl_ret_val != BOOTLOADER_OK) {
        // Failed to erase storage slot.
        ota_error = SL_BT_APP_OTA_DFU_ERR_BOOTLOADER_API;
        req_sts = SL_BT_APP_OTA_DFU_ERROR;
      } else {
        // Storage slot erased successfully.
        // Application OTA DFU component is ready for an OTA process.
        req_sts = SL_BT_APP_OTA_DFU_READY;
      }
      // Change to next state.
      set_main_status(req_sts, ota_error, btl_ret_val);
      break;
    ///////////////////////////////////////////////////////////////////////////
    // Ready for OTA DFU process.                                            //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_READY:
      // In ready state the usual business can happen on the application side.
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Disconnected state.                                                   //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_DISCONNECT:
      // Intentionally left blank.
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Download started state.                                               //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_DOWNLOAD_BEGIN:
      // Covered in the event handler.
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Download finished state.                                               //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_DOWNLOAD_END:
      // Covered in the event handler.
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Verify downloaded image.                                              //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_VERIFY:
      // Verify downloaded bytes of the image.
      btl_ret_val = bootloader_continueVerifyImage(app_ota.buffer, NULL);
      if (btl_ret_val == BOOTLOADER_ERROR_PARSE_CONTINUE) {
        // Verification may continue.
        app_ota.image_position += SL_BT_APP_OTA_DFU_VERIFICATION_BLOCK_SIZE;
        // Avoid state change in this case.
        req_sts = app_ota.status.current;
      } else if (btl_ret_val == BOOTLOADER_ERROR_PARSE_SUCCESS) {
        // Last chunk of bytes checked.
        app_ota.image_position += SL_BT_APP_OTA_DFU_VERIFICATION_BLOCK_SIZE;
        // Verification finished successfully.
        // This means state change as well.
        req_sts = SL_BT_APP_OTA_DFU_FINALIZE;
        ota_error = SL_BT_APP_OTA_DFU_NO_ERROR;
      } else {
        // Verification process aborted.
        req_sts = SL_BT_APP_OTA_DFU_ERROR;
        ota_error = SL_BT_APP_OTA_DFU_ERR_BOOTLOADER_API;
      }

      // Update mandatory event fields anyways.
      app_ota.event.event_id = SL_BT_APP_OTA_DFU_EVT_VERIFY_IMAGE_ID;
      app_ota.event.ota_error_code = ota_error;
      app_ota.event.btl_api_retval = btl_ret_val;
      // Forward verification status to application.
      app_ota.event.evt_info.verified_bytes = app_ota.image_position;
      sl_bt_app_ota_dfu_on_status_event(&app_ota.event);

      if (app_ota.status.current != req_sts) {
        // State change is also necessary because
        // error occured or verification finished successfully.
        set_main_status(req_sts, ota_error, btl_ret_val);
      }
      // Continue execution.
      (void)app_rta_proceed(app_ota.ctx);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Finalize Application OTA DFU process and reboot.                      //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_FINALIZE:
      // Set image for next boot.
      btl_ret_val = bootloader_setImageToBootload(SL_BT_APP_OTA_DFU_USED_SLOT);
      if (btl_ret_val != BOOTLOADER_OK) {
        // Setting Image to boot failed.
        req_sts = SL_BT_APP_OTA_DFU_ERROR;
        ota_error = SL_BT_APP_OTA_DFU_ERR_BOOTLOADER_API;
      } else {
        // Get ready for reboot trigger from user code.
        req_sts = SL_BT_APP_OTA_DFU_WAIT_FOR_REBOOT;
      }
      // Change to next state.
      set_main_status(req_sts, ota_error, btl_ret_val);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Wait for reboot triggering state.                                     //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_WAIT_FOR_REBOOT:
      // Intentionally left blank.
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Error state.                                                          //
    ///////////////////////////////////////////////////////////////////////////
    case SL_BT_APP_OTA_DFU_ERROR:
      // In error state the device awaits for application intervention.
      break;
  }

  (void)app_rta_release(app_ota.ctx);

  // Check signal
  sc = app_rta_signal_check_and_acquire(app_ota.ctx);
  if (sc == SL_STATUS_OK) {
    // Re-run init function.
    if (init_state(false)) {
      set_main_status(SL_BT_APP_OTA_DFU_INIT, ota_error, BOOTLOADER_OK);
    }
    (void)app_rta_release(app_ota.ctx);
  } else if (sc == SL_STATUS_EMPTY) {
    // No signal set
  } else {
    on_runtime_error(APP_RTA_ERROR_ACQUIRE_FAILED, sc);
  }
}

// -----------------------------------------------------------------------------
// Public functions

/*****************************************************************************
* Restart application OTA DFU progress without rebooting in case of any error.
*****************************************************************************/
sl_status_t sl_bt_app_ota_dfu_restart_progress(void)
{
  // Send a signal
  sl_status_t sc;
  sc = app_rta_signal(app_ota.ctx);
  return sc;
}

/*****************************************************************************
* Function to reboot only when the application OTA DFU process finished
* completely.
*****************************************************************************/
void sl_bt_app_ota_dfu_reboot(void)
{
  if (app_ota.status.current == SL_BT_APP_OTA_DFU_WAIT_FOR_REBOOT) {
    bootloader_rebootAndInstall();
  }
}

/*****************************************************************************
* Weak function to indicate Application OTA DFU status and in case of error
* the error codes for assertion.
* @param[in] evt Actual app ota dfu event.
* @note To be implemented in user code.
*****************************************************************************/
SL_WEAK void sl_bt_app_ota_dfu_on_status_event(sl_bt_app_ota_dfu_status_evt_t *evt)
{
  // Default error handling: trying to recover.
  if (evt->evt_info.sts.status == SL_BT_APP_OTA_DFU_ERROR) {
    // Restart the system, install the image.
    (void)sl_bt_app_ota_dfu_restart_progress();
  } else if (evt->evt_info.sts.status == SL_BT_APP_OTA_DFU_WAIT_FOR_REBOOT) {
    // Default finish action: auto-reboot.
    sl_bt_app_ota_dfu_reboot();
  }
}

// -----------------------------------------------------------------------------
// Private functions

static bool init_state(bool start)
{
  // Initialize variables
  app_ota.connection      = SL_BT_INVALID_CONNECTION_HANDLE;
  app_ota.status.previous = start ? SL_BT_APP_OTA_DFU_UNINIT : app_ota.status.current;
  app_ota.status.current  = SL_BT_APP_OTA_DFU_UNINIT;

  app_ota.image_position  = 0u;

  // Bootloader init must be called before using bootloader_* API calls!
  int32_t boot_retv;
  boot_retv = bootloader_init();
  if (boot_retv != BOOTLOADER_OK) {
    on_error(SL_BT_APP_OTA_DFU_ERR_BOOTLOADER_API, boot_retv);
  }
  return (boot_retv == BOOTLOADER_OK);
}

/*****************************************************************************
* Set Application OTA DFU main status.
*****************************************************************************/
static void set_main_status(sl_bt_app_ota_dfu_status_t new_status,
                            sl_bt_app_ota_dfu_error_t ota_error,
                            int32_t btl_ret_val)
{
  app_ota.status.previous = app_ota.status.current;
  app_ota.status.current = new_status;
  app_ota.event.event_id = SL_BT_APP_OTA_DFU_EVT_STATE_CHANGE_ID;
  app_ota.event.evt_info.sts.status = app_ota.status.current;
  app_ota.event.evt_info.sts.prev_status = app_ota.status.previous;
  app_ota.event.ota_error_code = ota_error;
  app_ota.event.btl_api_retval = btl_ret_val;
  // Continue execution.
  (void)app_rta_proceed(app_ota.ctx);
  // Callback.
  sl_bt_app_ota_dfu_on_status_event(&app_ota.event);
}

/*****************************************************************************
* App RTA error handler.
*****************************************************************************/
static void on_runtime_error(app_rta_error_t error, sl_status_t result)
{
  sl_bt_app_ota_dfu_error_t ota_error;

  switch (error) {
    case APP_RTA_ERROR_RUNTIME_INIT_FAILED:
      ota_error = SL_BT_APP_OTA_DFU_ERR_INIT;
      break;

    case APP_RTA_ERROR_ACQUIRE_FAILED:
      ota_error = SL_BT_APP_OTA_DFU_ERR_ACQUIRE;
      break;

    case APP_RTA_ERROR_RELEASE_FAILED:
      ota_error = SL_BT_APP_OTA_DFU_ERR_RELEASE;
      break;

    default:
      ota_error = SL_BT_APP_OTA_DFU_ERR_GENERIC;
      break;
  }
  on_error(ota_error, (int32_t)result);
}

/*****************************************************************************
* Generic error handler.
*****************************************************************************/
static void on_error(sl_bt_app_ota_dfu_error_t ota_error, int32_t btl_api_retval)
{
  app_ota.event.event_id = SL_BT_APP_OTA_DFU_EVT_STATE_CHANGE_ID;
  app_ota.event.ota_error_code = ota_error;
  app_ota.event.btl_api_retval = btl_api_retval;
  app_ota.event.evt_info.sts.status = app_ota.status.current;
  app_ota.event.evt_info.sts.prev_status = app_ota.status.previous;
  // Callback.
  sl_bt_app_ota_dfu_on_status_event(&app_ota.event);
}
