/*******************************************************************************
 * @file
 * @brief ESL host library core component.
 *
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include "ncp_host.h"
#include "sl_bt_api.h"
#include "sl_bt_ots_client.h"
#include "sl_status.h"
#include "simple_argparse.h"
#include "esl_lib_command_list.h"
#include "esl_lib.h"
#include "esl_lib_core.h"
#include "esl_lib_connection.h"
#include "esl_lib_pawr.h"
#include "esl_lib_ap_control.h"
#include "app_timer.h"
#include "esl_lib_log.h"
#include "esl_lib_memory.h"

// -----------------------------------------------------------------------------
// Definitions

// Connection type values
#define NCP_CONN_TYPE_NOT_INITIALIZED 0
#define NCP_CONN_TYPE_SERIAL          'u'
#define NCP_CONN_TYPE_TCP             't'

// ESL library internal event codes
#define  ESL_LIB_EVT_CONNECTION_REQ_TIMEOUT        20
#define  ESL_LIB_EVT_CONNECTED                     21
#define  ESL_LIB_EVT_PARSING_SERVICES_DONE         22
#define  ESL_LIB_EVT_PARSING_CHARS_DONE            23
#define  ESL_LIB_EVT_SUBSCRIBED_TO_NOTIFICATION    24

// AD types
#define  AD_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS 0x02
#define  AD_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS   0x03

// -----------------------------------------------------------------------------
// Forward declaration of private functions

static void parse_config(char *config);
static sl_status_t ap_init(esl_lib_ap_state_t **handle_out);
static void run_command(esl_lib_command_list_cmd_t *cmd);
static void esl_lib_core_step(void);
static void esl_lib_core_on_bt_event(sl_bt_msg_t *evt);
static sl_status_t send_core_error(esl_lib_status_t     lib_status,
                                   sl_status_t          status,
                                   esl_lib_core_state_t data);
static sl_status_t send_tag_found(uint8_t *addr,
                                  uint8_t address_type,
                                  int8_t  rssi);
static sl_status_t send_connection_mode_event(void);
static sl_status_t send_scan_status(void);
static sl_status_t set_connection_mode(esl_lib_connection_mode_t requested_mode,
                                       esl_lib_status_t *lib_status);
static bool find_service_in_advertisement(uint8_t *data, uint8_t len);
static void esl_lib_core_internal_reset(void);
static void send_shutdown_ready_event(void);

// -----------------------------------------------------------------------------
// Private variables

// Access Point State
esl_lib_ap_state_t *ap_state = NULL;

/// Input argument set for initialization.
static struct argparse_descriptor_s arg_descriptor[] =
{
  { "-connection", "ip,serial" },   // define whatever key-value pairs you like
  { "-device", "" },                // empty string as option allows anything
  { "-baud", "115200,921600" },
  { "-handshake", "no,ctsrts,hw" }, // more options can be given where it's needed
  { "-secure", NULL },              // enable encryption for NCP communication
  { NULL, NULL }
};

static bd_addr identity_address = { { 0 } };
// Set identity address type invalid by default to be able to detect if uninitialized
static uint8_t identity_address_type = (sl_bt_gap_static_address | 0x80);

// -----------------------------------------------------------------------------
// Public functions

void esl_lib_init(char *config)
{
  sl_status_t sc;

  // Allocate and intialize radio data container
  sc = ap_init(&ap_state);
  if (sc != SL_STATUS_OK) {
    esl_lib_log_core_critical("Failed to allocate memory!" APP_LOG_NL);
    exit(EXIT_FAILURE);
  }

  // Parse configuration
  esl_lib_log_core_debug("Parsing host library configuration: %s" APP_LOG_NL, config);
  parse_config(config);
  // Initialize NCP connection.
  esl_lib_log_core_debug("Initializing NCP host" APP_LOG_NL);
  sc = ncp_host_init();
  if (sc == SL_STATUS_INVALID_PARAMETER) {
    esl_lib_log_core_critical("Failed to initialize host library!" APP_LOG_NL);
    exit(EXIT_FAILURE);
  }
  if (sc != SL_STATUS_OK) {
    esl_lib_log_core_critical("Error initializing host library: 0x%04x" APP_LOG_NL, sc);
    exit(EXIT_FAILURE);
  }
  // NCP host initialization implicitly reboots the target.
  esl_lib_log_core_info("Waiting for boot event..." APP_LOG_NL);

  // Initialize L2CAP transfer
  esl_lib_log_core_debug("Init L2CAP transfer layer" APP_LOG_NL);
  sli_bt_l2cap_transfer_init();

  // Initialize OTS Client
  esl_lib_log_core_debug("Prepare OTS client" APP_LOG_NL);
  sli_bt_ots_client_init();
  esl_lib_log_core_debug("AP host library instance ready" APP_LOG_NL);
}

void esl_lib_process_action(void)
{
  esl_lib_core_step();
  esl_lib_pawr_step();
  esl_lib_connection_step();
  esl_lib_image_transfer_step();
}

void esl_lib_deinit(void)
{
  // stop scanning
  (void)sl_bt_scanner_stop();

  esl_lib_core_internal_reset();
  esl_lib_auto_initiator_deinit();

  esl_lib_memory_free(ap_state);

  send_shutdown_ready_event();
}

sl_status_t esl_lib_core_add_command(esl_lib_command_list_cmd_t *cmd)
{
  sl_status_t sc = esl_lib_command_list_put(&ap_state->command_list, cmd);
  esl_lib_log_core_debug("Add command = %u, id: %#04" PRIx32 ", sc: 0x%04x" APP_LOG_NL,
                         cmd->cmd_code,
                         cmd->id,
                         sc);
  return sc;
}

void sl_bt_on_event(sl_bt_msg_t *evt)
{
  // Do not change the order of processing unless there is a good reason to do so.
  esl_lib_pawr_on_bt_event(evt);
  esl_lib_image_transfer_on_bt_event(evt);
  esl_lib_connection_on_bt_event(evt);
  esl_lib_core_on_bt_event(evt);
  esl_lib_ap_control_on_bt_event(evt);
}

void esl_lib_core_connection_complete()
{
  // This function should only be called after the ESL_LIB_CMD_CONNECT request is processed (with or without error)
  if (ap_state->command == NULL) {
    // Since the ESL_LIB_CMD_CONNECT request is the owner of the command, the ap_state->command should be NULL
    ap_state->command_complete = true;
  }
}

sl_status_t esl_lib_core_get_identity_address(bd_addr *address, uint8_t *type)
{
  sl_status_t sc = SL_STATUS_INITIALIZATION;

  if (address == NULL || type == NULL) {
    return SL_STATUS_NULL_POINTER;
  } else if (identity_address_type <= sl_bt_gap_static_address) {
    *type = identity_address_type;
    memcpy(address, &identity_address, sizeof(bd_addr));
    sc = SL_STATUS_OK;
  }

  return sc;
}

// -----------------------------------------------------------------------------
// Private functions

static void esl_lib_core_on_bt_event(sl_bt_msg_t *evt)
{
  sl_status_t      sc                 = SL_STATUS_OK;
  esl_lib_status_t lib_status         = ESL_LIB_STATUS_NO_ERROR;
  bool             lib_critical_error = false;
  esl_lib_evt_t    *new_event         = NULL;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Do internal LIB reset
      esl_lib_core_internal_reset();
      ap_state->scan.enabled     = ESL_LIB_FALSE;
      ap_state->scan.configured  = ESL_LIB_FALSE;
      ap_state->command_complete = true;
      ap_state->core_state       = ESL_LIB_CORE_STATE_IDLE;
      lib_status                 = ESL_LIB_STATUS_INIT_FAILED;
      // Extract unique ID from BT Address.
      sc = sl_bt_gap_get_identity_address(&identity_address, &identity_address_type);
      if (sc != SL_STATUS_OK) {
        lib_critical_error = true;
      }

      esl_lib_log_core_info("Bluetooth " ESL_LIB_LOG_ADDR_FORMAT APP_LOG_NL,
                            identity_address_type,
                            identity_address_type ? "random" : "public",
                            ESL_LIB_LOG_BD_ADDR(identity_address));

      // Configure Security Manager to default
      sc = sl_bt_sm_configure(0,
                              sl_bt_sm_io_capability_noinputnooutput);
      if (sc != SL_STATUS_OK) {
        esl_lib_log_core_critical("Failed to configure SM, sc = 0x%04x" APP_LOG_NL, sc);
        lib_critical_error = true;
      }

      sc = sl_bt_sm_set_oob(false, NULL, NULL);
      if (sc != SL_STATUS_OK) {
        esl_lib_log_core_critical("Failed to clear local OOB, sc = 0x%04x" APP_LOG_NL, sc);
        lib_critical_error = true;
      }

      sc = sl_bt_sm_set_bondable_mode(true);
      if (sc != SL_STATUS_OK) {
        esl_lib_log_core_critical("Failed to set bondable mode, sc = 0x%04x" APP_LOG_NL, sc);
        lib_critical_error = true;
      }

      sc = sl_bt_connection_set_default_parameters(ESL_LIB_CONN_INTERVAL_MIN_DEFAULT,
                                                   ESL_LIB_CONN_INTERVAL_MAX_DEFAULT,
                                                   ESL_LIB_CONN_PERIPHERAL_LATENCY_DEFAULT,
                                                   ESL_LIB_CONN_TIMEOUT_DEFAULT,
                                                   ESL_LIB_CONN_MIN_CE_LENGTH,
                                                   ESL_LIB_CONN_MAX_CE_LENGTH);
      if (sc != SL_STATUS_OK) {
        esl_lib_log_core_critical("Failed to set connection parameters, sc = 0x%04x" APP_LOG_NL, sc);
        lib_critical_error = true;
      } else {
        esl_lib_log_core_debug("Connection parameters successfully set: Interval: %u/%u [* 1.25 ms], Latency: %u, Timeout %u [ms], CE: %u/%u" APP_LOG_NL,
                               ESL_LIB_CONN_INTERVAL_MIN_DEFAULT,
                               ESL_LIB_CONN_INTERVAL_MAX_DEFAULT,
                               ESL_LIB_CONN_PERIPHERAL_LATENCY_DEFAULT,
                               ESL_LIB_CONN_TIMEOUT_DEFAULT * 10,
                               ESL_LIB_CONN_MIN_CE_LENGTH,
                               ESL_LIB_CONN_MAX_CE_LENGTH);
      }

      // Try to set prefferred PHY (not forced, not critical)
      sc = sl_bt_connection_set_default_preferred_phy(sl_bt_gap_phy_2m,
                                                      sl_bt_gap_phy_1m | sl_bt_gap_phy_2m);
      if (sc != SL_STATUS_OK) {
        esl_lib_log_core_warning("Failed to set preferred PHY, sc = 0x%04x, using default" APP_LOG_NL, sc);
      } else {
        esl_lib_log_core_debug("Default PHY successfully set: preferred = 2M, accepted = 1M & 2M" APP_LOG_NL);
      }

      // Allocate and add event to the event list
      sc = esl_lib_event_list_allocate(ESL_LIB_EVT_SYSTEM_BOOT, 0, &new_event);
      if (sc == SL_STATUS_OK) {
        new_event->evt_code = ESL_LIB_EVT_SYSTEM_BOOT;
        new_event->data.evt_boot.status = SL_STATUS_OK;
        memcpy((void *)&new_event->data.evt_boot.address,
               (void *)&identity_address,
               sizeof(bd_addr));
        new_event->data.evt_boot.address.address_type = identity_address_type;
        sc = esl_lib_event_list_push_back(new_event);
      }
      if (sc != SL_STATUS_OK) {
        esl_lib_log_core_critical("Failed to allocate event list memory, sc = 0x%04x" APP_LOG_NL, sc);
        lib_critical_error = true;
      } else {
        lib_status = ESL_LIB_STATUS_NO_ERROR;
      }
      send_connection_mode_event();
      break;
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      if (find_service_in_advertisement(evt->data.evt_scanner_legacy_advertisement_report.data.data,
                                        evt->data.evt_scanner_legacy_advertisement_report.data.len)) {
        esl_lib_core_state_t core_status;
        bool ll_list_busy;
        uint8_t connections;

        (void)esl_lib_get_connection_mode_and_status(&core_status, &connections, &ll_list_busy);
        if (ll_list_busy && core_status == ESL_LIB_CORE_STATE_CONNECTING) {
          // Our internal acceptance list is practically never busy, unlike its LL counterpart, but with hundreds
          // to thousands of nearby advertisers it's still better to fill our list gradually, i.e. while the LL is
          // not busy, to initiate a connection to one of the devices on its filter acceptance list.
          // This helps reduce the otherwise heavy load on the library interface and avoids initial connection
          // losses, especially when the AP is cold started.
          esl_lib_log_core_debug("Defer reporting ESL at " ESL_LIB_LOG_ADDR_FORMAT ", RSSI = %d due to controller busy connecting. Connections: %u." APP_LOG_NL,
                                 ESL_LIB_LOG_ADDR(evt->data.evt_scanner_legacy_advertisement_report),
                                 evt->data.evt_scanner_legacy_advertisement_report.rssi,
                                 connections);
          break; // exit case immediately, do not send report while connecting
        }
        esl_lib_log_core_debug("ESL found at " ESL_LIB_LOG_ADDR_FORMAT ", RSSI = %d" APP_LOG_NL,
                               ESL_LIB_LOG_ADDR(evt->data.evt_scanner_legacy_advertisement_report),
                               evt->data.evt_scanner_legacy_advertisement_report.rssi);
        (void)send_tag_found(evt->data.evt_scanner_legacy_advertisement_report.address.addr,
                             evt->data.evt_scanner_legacy_advertisement_report.address_type,
                             evt->data.evt_scanner_legacy_advertisement_report.rssi);
      }
      break;
    case sl_bt_evt_system_resource_exhausted_id:
      lib_status = ESL_LIB_STATUS_RESOURCE_EXCEEDED;
      esl_lib_log_core_error("BLE stack resource exhausted, data may have been lost!"
                             " [discarded: %u, buf: %u, heap: %u, msg: %u]" APP_LOG_NL,
                             evt->data.evt_system_resource_exhausted.num_buffers_discarded,
                             evt->data.evt_system_resource_exhausted.num_buffer_allocation_failures,
                             evt->data.evt_system_resource_exhausted.num_heap_allocation_failures,
                             evt->data.evt_system_resource_exhausted.num_message_allocation_failures);
      (void)send_core_error(lib_status,
                            SL_STATUS_ALLOCATION_FAILED,
                            ap_state->core_state);
      break;
    case sl_bt_evt_system_error_id: {
      uint32_t data = 0;
      lib_status = ESL_LIB_STATUS_SYSTEM_ERROR;

      // Get a maximum of 4 bytes from the available data. If there's any.
      switch (evt->data.evt_system_error.data.len) {
        default:
        /* FALLTHROUGH */
        case 4:
          data |= (evt->data.evt_system_error.data.data[3] << 24);
        /* FALLTHROUGH */
        case 3:
          data |= (evt->data.evt_system_error.data.data[2] << 16);
        /* FALLTHROUGH */
        case 2:
          data |= (evt->data.evt_system_error.data.data[1] << 8);
        /* FALLTHROUGH */
        case 1:
          data |= evt->data.evt_system_error.data.data[0];
        /* FALLTHROUGH */
        case 0:
          break;
      }
      // Swap DATA endianness for better readability in error message (can be read as BGAPI MSG HDR data if complete)
      data = ((data >> 24) & 0xff)        // move byte 3 to byte 0
             | ((data << 8)  & 0xff0000)  // move byte 1 to byte 2
             | ((data >> 8)  & 0xff00)    // move byte 2 to byte 1
             | ((data << 24) & 0xff000000); // byte 0 to byte 3

      if (evt->data.evt_system_error.reason == SL_STATUS_COMMAND_INCOMPLETE) {
        // Reduce the severity level for incomplete commands (only!)
        lib_critical_error = false;
      } else {
        lib_critical_error = true;
      }
      esl_lib_log_core_error("System error occured, sc = 0x%04x, data=0x%08x" APP_LOG_NL, evt->data.evt_system_error.reason, data);
    } break;

    default:
      break;
  }

  if (lib_critical_error) {
    esl_lib_log_core_critical("Critical error, exiting..." APP_LOG_NL);
    // Send error
    (void)send_core_error(lib_status, sc, ap_state->core_state);
    // Exit on critical error
    exit(EXIT_FAILURE);
  }
}

/*******************************************************************************
 * Parse configuration string.
 *
 * @param[in]  config Library configuration.
 ******************************************************************************/
static void parse_config(char *config)
{
  sl_status_t sc;
  simple_argparse_parameter_pair_p parsed;
  simple_argparse_handle_p handle;
  int parsed_count;
  uint8_t conn_type = NCP_CONN_TYPE_NOT_INITIALIZED;

  sc = simple_argparse_init(arg_descriptor, &handle);
  if (sc != SL_STATUS_OK) {
    esl_lib_log_core_critical("Failed to initialize AP host library! sc = 0x%04x" APP_LOG_NL, sc);
    exit(EXIT_FAILURE);
  }

  int ret = simple_argparse_validate(handle, config, &parsed, &parsed_count);

  if (ret) {
    esl_lib_log_core_critical("Library argument parser error: status=%d, \"%s\" " APP_LOG_NL,
                              ret,
                              &config[parsed_count]);
    simple_argparse_deinit(handle);
    exit(EXIT_FAILURE);
  } else {
    esl_lib_log_core_debug("AP host library parsed commands: %d" APP_LOG_NL, parsed_count);
    for (int i = 0; i < parsed_count; ++i) {
      esl_lib_log_core_debug("%s : %s" APP_LOG_NL, parsed[i].arg, parsed[i].opt);

      if (strcmp(arg_descriptor[0].arg, parsed[i].arg) == 0) {
        // Connection type
        if (strcmp("ip", parsed[i].opt) == 0) {
          // TCP/IP connection
          conn_type = NCP_CONN_TYPE_TCP;
        } else if (strcmp("serial", parsed[i].opt) == 0) {
          // Serial connection
          conn_type = NCP_CONN_TYPE_SERIAL;
        }
      } else if (conn_type != NCP_CONN_TYPE_NOT_INITIALIZED
                 && strcmp(arg_descriptor[1].arg, parsed[i].arg) == 0) {
        // Device
        (void)ncp_host_set_option(conn_type, parsed[i].opt);
      } else if (strcmp(arg_descriptor[2].arg, parsed[i].arg) == 0) {
        // Baud rate
        (void)ncp_host_set_option('b', parsed[i].opt);
      } else if (strcmp(arg_descriptor[3].arg, parsed[i].arg) == 0) {
        // Flow control
        if (strcmp("no", parsed[i].opt) == 0) {
          (void)ncp_host_set_option('f', parsed[i].opt);
        }
      } else if (strcmp(arg_descriptor[4].arg, parsed[i].arg) == 0) {
        int status = ncp_host_set_option('s', NULL);
        esl_lib_log_core_debug("Attempt to enable NCP encryption, result status: 0x%04x" APP_LOG_NL,
                               status);
      }
    }
  }

  sc = simple_argparse_deinit(handle);
  if (sc != SL_STATUS_OK) {
    esl_lib_log_core_critical("Failed to deinit arparse, sc = 0x%04x" APP_LOG_NL, sc);
    exit(EXIT_FAILURE);
  }
  esl_lib_log_core_debug("AP host library configured" APP_LOG_NL);
}

/*******************************************************************************
 * Allocate and initialize radio data.
 *
 * @return Radio data pointer.
 ******************************************************************************/
static sl_status_t ap_init(esl_lib_ap_state_t **handle_out)
{
  sl_status_t sc = SL_STATUS_OK;

  esl_lib_ap_state_t *handle = esl_lib_memory_allocate(sizeof(esl_lib_ap_state_t));
  if (handle == NULL) {
    esl_lib_log_core_critical("Failed to allocate memory for AP library!" APP_LOG_NL);
    return SL_STATUS_ALLOCATION_FAILED;
  }
  // Configure default scanning parameters
  handle->scanner_suspended             = ESL_LIB_FALSE;
  handle->scan.enabled                  = ESL_LIB_FALSE;
  handle->scan.configured               = ESL_LIB_FALSE;
  handle->scan.parameters.scanning_phy  = sl_bt_gap_phy_1m;
  handle->scan.parameters.discover_mode = sl_bt_scanner_discover_generic;
  handle->scan.parameters.mode          = sl_bt_scanner_scan_mode_passive;
  handle->scan.parameters.interval      = 16;
  handle->scan.parameters.window        = 16;
  handle->command_list                  = NULL;
  handle->command                       = NULL;
  handle->command_complete              = true;
  handle->core_state                    = ESL_LIB_CORE_STATE_IDLE;

  // Set output
  *handle_out = handle;

  return sc;
}

static sl_status_t send_connection_mode_event(void)
{
  sl_status_t   sc;
  esl_lib_evt_t *lib_evt;
  esl_lib_core_state_t core_state;
  esl_lib_connection_mode_t mode;
  uint8_t connections;

  mode = esl_lib_get_connection_mode_and_status(&core_state, &connections, NULL);

  esl_lib_log_core_debug("Connection mode: %s, %s." APP_LOG_NL,
                         mode == ESL_LIB_CONNECTION_MODE_SINGLE ? "single" : "accept list",
                         core_state == ESL_LIB_CORE_STATE_IDLE ? "idle" : "initiating");

  sc = esl_lib_event_list_allocate(ESL_LIB_EVT_CONNECTION_MODE,
                                   0,
                                   &lib_evt);
  if (sc == SL_STATUS_OK) {
    // Set status data
    lib_evt->data.evt_connection_mode.mode = mode;
    lib_evt->data.evt_connection_mode.core_state = core_state;

    if (mode == ESL_LIB_CONNECTION_MODE_SINGLE) {
      lib_evt->data.evt_connection_mode.filter_size = (core_state == ESL_LIB_CORE_STATE_IDLE) ? 0 : 1;
    } else {
      lib_evt->data.evt_connection_mode.filter_size = esl_lib_get_initiator_filter_size();
    }

    lib_evt->data.evt_connection_mode.connections = connections;
    sc = esl_lib_event_list_push_back(lib_evt);

    if (sc != SL_STATUS_OK) {
      // Free up memory on failure
      esl_lib_memory_free(lib_evt);
    }
  }
  return sc;
}

static sl_status_t send_scan_status(void)
{
  sl_status_t   sc;
  esl_lib_evt_t *lib_evt;

  esl_lib_log_core_debug("Scanning = %u" APP_LOG_NL, ap_state->scan.enabled);

  sc = esl_lib_event_list_allocate(ESL_LIB_EVT_SCAN_STATUS,
                                   0,
                                   &lib_evt);
  if (sc == SL_STATUS_OK) {
    // Copy status data
    memcpy(&lib_evt->data,
           &ap_state->scan,
           sizeof(esl_lib_scan_status_t));
    sc = esl_lib_event_list_push_back(lib_evt);
    if (sc != SL_STATUS_OK) {
      // Free up memory on failure
      esl_lib_memory_free(lib_evt);
    }
  }
  return sc;
}

static sl_status_t send_tag_found(uint8_t *addr,
                                  uint8_t address_type,
                                  int8_t  rssi)
{
  sl_status_t   sc;
  esl_lib_evt_t *lib_evt;

  sc = esl_lib_event_list_allocate(ESL_LIB_EVT_TAG_FOUND,
                                   0,
                                   &lib_evt);
  if (sc == SL_STATUS_OK) {
    lib_evt->data.evt_tag_found.rssi = rssi;
    lib_evt->data.evt_tag_found.address.address_type = address_type;
    // Copy address
    memcpy(lib_evt->data.evt_tag_found.address.addr,
           addr,
           sizeof(lib_evt->data.evt_tag_found.address.addr));

    sc = esl_lib_event_list_push_back(lib_evt);
    if (sc != SL_STATUS_OK) {
      // Free up memory on failure
      esl_lib_memory_free(lib_evt);
    }
  }
  return sc;
}

static sl_status_t send_core_error(esl_lib_status_t     lib_status,
                                   sl_status_t          status,
                                   esl_lib_core_state_t data)
{
  sl_status_t sc;
  esl_lib_node_id_t node_id;
  node_id.type = ESL_LIB_NODE_ID_TYPE_NONE;
  sc = esl_lib_event_push_error(lib_status,
                                &node_id,
                                status,
                                (esl_lib_status_data_t)data);
  return sc;
}

static sl_status_t set_connection_mode(esl_lib_connection_mode_t requested_mode,
                                       esl_lib_status_t *lib_status)
{
  sl_status_t sc = SL_STATUS_BUSY;

  if (lib_status == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sc = esl_lib_change_connection_mode(requested_mode);
  *lib_status = sc == SL_STATUS_OK ? ESL_LIB_STATUS_NO_ERROR : ESL_LIB_STATUS_CONN_SET_MODE_FAILED;

  return sc;
}

// Process single command.
static void run_command(esl_lib_command_list_cmd_t *cmd)
{
  sl_status_t                sc                = SL_STATUS_FAIL;
  esl_lib_status_t           lib_status        = ESL_LIB_STATUS_UNKNOWN_COMMAND;
  esl_lib_node_id_t node_id;

  node_id.type = ESL_LIB_NODE_ID_TYPE_NONE;

  if (cmd != NULL) {
    switch (cmd->cmd_code) {
      case ESL_LIB_CMD_AP_CONTROL_ADV_ENABLE:
        esl_lib_log_core_debug("Command: Enable AP control advertising" APP_LOG_NL);
        lib_status = ESL_LIB_STATUS_CONTROL_FAILED;
        if (cmd->data.cmd_ap_control.len == 1) {
          sc = esl_lib_ap_control_adv_enable(cmd->data.cmd_ap_control.data[0]);
          if (sc == SL_STATUS_OK) {
            lib_status = ESL_LIB_STATUS_NO_ERROR;
          }
        } else {
          sc = SL_STATUS_INVALID_PARAMETER;
        }
        if (sc != SL_STATUS_OK) {
          esl_lib_log_core_error("Failed to %s AP control adveritsing, sc = 0x%04x" APP_LOG_NL,
                                 (cmd->data.cmd_ap_control.data[0] == ESL_LIB_TRUE) ? "enable" : "disable",
                                 sc);
        }
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_AP_CONTROL_CP_RESPONSE:
        esl_lib_log_core_debug("Command: AP control CP response" APP_LOG_NL);
        lib_status = ESL_LIB_STATUS_CONTROL_FAILED;
        sc = esl_lib_ap_control_response(&cmd->data.cmd_ap_control);
        if (sc == SL_STATUS_OK) {
          lib_status = ESL_LIB_STATUS_NO_ERROR;
        } else {
          esl_lib_log_core_error("Failed to send AP control CP response, sc = 0x%04x" APP_LOG_NL, sc);
        }
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_AP_CONTROL_IT_RESPONSE:
        esl_lib_log_core_debug("Command: AP control IT response" APP_LOG_NL);
        lib_status = ESL_LIB_STATUS_CONTROL_FAILED;
        sc = esl_lib_ap_control_image_transfer_response(&cmd->data.cmd_ap_control);
        if (sc == SL_STATUS_OK) {
          lib_status = ESL_LIB_STATUS_NO_ERROR;
        } else {
          esl_lib_log_core_error("Failed to send AP control IT response, sc = 0x%04x" APP_LOG_NL, sc);
        }
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_GET_SCAN_STATUS:
        send_scan_status();
        lib_status = ESL_LIB_STATUS_NO_ERROR;
        sc = SL_STATUS_OK;
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_SCAN_CONFIG:
        esl_lib_log_core_debug("Command: scan config" APP_LOG_NL);
        lib_status = ESL_LIB_STATUS_SCAN_CONFIG_FAILED;
        sc = sl_bt_scanner_set_parameters(cmd->data.cmd_scan_config.mode,
                                          cmd->data.cmd_scan_config.interval,
                                          cmd->data.cmd_scan_config.window);
        if (sc == SL_STATUS_OK) {
          // Save parameters
          ap_state->scan.parameters.mode          = cmd->data.cmd_scan_config.mode;
          ap_state->scan.parameters.interval      = cmd->data.cmd_scan_config.interval;
          ap_state->scan.parameters.window        = cmd->data.cmd_scan_config.window;
          ap_state->scan.parameters.scanning_phy  = cmd->data.cmd_scan_config.scanning_phy;
          ap_state->scan.parameters.discover_mode = cmd->data.cmd_scan_config.discover_mode;
          // Set configured state
          ap_state->scan.configured               = ESL_LIB_TRUE;
          lib_status = ESL_LIB_STATUS_NO_ERROR;
        } else {
          esl_lib_log_core_error("Failed to configure scanning, sc = 0x%04x" APP_LOG_NL, sc);
        }
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_SCAN_ENABLE:
        esl_lib_log_core_debug("Command: scan enable" APP_LOG_NL);
        if (cmd->data.cmd_scan_enable.enable == ESL_LIB_TRUE) {
          lib_status = ESL_LIB_STATUS_SCAN_START_FAILED;
          if (!(ap_state->scan.enabled == ESL_LIB_TRUE)) {
            sc = sl_bt_scanner_start(ap_state->scan.parameters.scanning_phy,
                                     ap_state->scan.parameters.discover_mode);
            if (sc == SL_STATUS_OK) {
              ap_state->scan.enabled = ESL_LIB_TRUE;
              lib_status = ESL_LIB_STATUS_NO_ERROR;
              send_scan_status();
            }
          } else {
            sc = SL_STATUS_INVALID_STATE;
          }
        } else {
          lib_status = ESL_LIB_STATUS_SCAN_STOP_FAILED;

          if (ap_state->scanner_suspended != ESL_LIB_FALSE) {
            sc = SL_STATUS_OK;
          } else {
            sc = sl_bt_scanner_stop();
          }

          if (sc == SL_STATUS_OK) {
            ap_state->scan.enabled = ESL_LIB_FALSE;
            lib_status = ESL_LIB_STATUS_NO_ERROR;
            send_scan_status();
          }
        }
        if (sc != SL_STATUS_OK) {
          esl_lib_log_core_error("Failed to %s scanning, sc = 0x%04x" APP_LOG_NL,
                                 (cmd->data.cmd_scan_enable.enable == ESL_LIB_TRUE) ? "enable" : "disable",
                                 sc);
        }
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_CONNECT:
        ap_state->command = NULL;
        esl_lib_log_core_debug("Command: connect" APP_LOG_NL);
        // Assume that the connect command is failed
        // and try to initiate the connection.
        lib_status = ESL_LIB_STATUS_CONN_FAILED;
        node_id.type = ESL_LIB_NODE_ID_TYPE_ADDRESS;
        node_id.id.address.address_type = cmd->data.cmd_connect.address.address_type;
        memcpy(node_id.id.address.addr, cmd->data.cmd_connect.address.addr, sizeof(node_id.id.address.addr));
        sc = esl_lib_initiate_connection(cmd);
        if (sc == SL_STATUS_OK) {
          lib_status = ESL_LIB_STATUS_NO_ERROR;
        } else {
          esl_lib_log(((sc == SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED) \
                       ? ESL_LIB_LOG_LEVEL_WARNING : ESL_LIB_LOG_LEVEL_ERROR),
                      ESL_LIB_LOG_MODULE_CORE,
                      "Failed to open connection, sc = 0x%04x" APP_LOG_NL, sc);
          ap_state->command_complete = true;
          esl_lib_memory_free(ap_state->command);
        }
        break;
      case ESL_LIB_CMD_SET_CONNECTION_MODE:
        sc = set_connection_mode(cmd->data.cmd_set_connection_mode.mode, &lib_status);
        send_connection_mode_event();
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_GET_CONNECTION_MODE:
        send_connection_mode_event();
        lib_status = ESL_LIB_STATUS_NO_ERROR;
        sc = SL_STATUS_OK;
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_REMOVE_FILTER_ACCEPT_LIST:
        lib_status = ESL_LIB_STATUS_UNKNOWN_COMMAND;
        sc = SL_STATUS_FAIL;
        ap_state->command_complete = true;
        break;
      case ESL_LIB_CMD_CLEAR_FILTER_ACCEPT_LIST:
        lib_status = ESL_LIB_STATUS_UNKNOWN_COMMAND;
        sc = SL_STATUS_FAIL;
        ap_state->command_complete = true;
        break;

      default:
        break; // default
    }
  }

  if (sc != SL_STATUS_OK) {
    esl_lib_status_data_t status_data;
    status_data = (esl_lib_status_data_t)ap_state->core_state;
    esl_lib_log(((sc == SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED) \
                 ? ESL_LIB_LOG_LEVEL_WARNING : ESL_LIB_LOG_LEVEL_ERROR),
                ESL_LIB_LOG_MODULE_CORE,
                "State machine failure, lib status = %d, sc = 0x%04x, core status = %d" APP_LOG_NL,
                lib_status,
                sc,
                ap_state->core_state);
    // Send available data in the error message
    (void)esl_lib_event_push_error(lib_status,
                                   &node_id,
                                   sc,
                                   status_data);
    // Move to the next command in case of error
    ap_state->command_complete = true;
  }
}

// Process common commands.
static void esl_lib_core_step(void)
{
  esl_lib_command_list_cmd_t *cmd;

  if (ap_state->command_complete) {
    // If there is an ongoing but complete command, remove that.
    if (ap_state->command != NULL) {
      esl_lib_command_list_remove(&ap_state->command_list, ap_state->command);
      ap_state->command = NULL;
    }
    // Move and execute next command.
    cmd = esl_lib_command_list_get(&ap_state->command_list);
    if (cmd != NULL) {
      esl_lib_log_core_debug("Running next command: %d, id = %#04" PRIx32 APP_LOG_NL,
                             cmd->cmd_code,
                             cmd->id);
      ap_state->command = cmd;
      ap_state->command_complete = false;
      run_command(ap_state->command);
    }
  }
}

// Parse advertisements looking for advertised ESL service
static bool find_service_in_advertisement(uint8_t *data, uint8_t len)
{
  uint8_t  ad_field_length;
  uint8_t  ad_field_type;
  uint16_t i               = 0;
  uint8_t  esl_uuid_arr[2] = ESL_SERVICE_UUID;
  uint16_t esl_uuid        = *((uint16_t *)esl_uuid_arr);
  uint16_t *ptr;
  const uint8_t head_len   = 2 * sizeof(uint8_t);
  const uint8_t uuid_len   = sizeof(esl_uuid);

  // Basic sanity check: need at least 2 bytes for length+type
  if (data == NULL || len < (head_len + uuid_len)) {
    return false;
  }

  // Parse advertisement packet
  while (i < (uint16_t)len) {
    ad_field_length = data[i];

    // Validate AD field length to prevent buffer overflow
    // AD length should not be 0, and (i + 1 + ad_field_length) must not exceed len
    if (ad_field_length == 0) {
      // Zero-length field is invalid, stop parsing
      break;
    }

    // Check if we have enough data for the complete AD structure
    // Need: 1 byte (length already read) + 1 byte (type) + ad_field_length-1 bytes (data)
    if (i + 1 + ad_field_length > len) {
      // Malformed packet: advertised length exceeds buffer, stop parsing
      break;
    }

    ad_field_type = data[i + 1];
    // Check for 16-bit Service UUIDs AD types
    if (ad_field_type == AD_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS
        || ad_field_type == AD_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS) {
      // Calculate safe end boundary for UUID iteration
      uint16_t field_end = i + 1 + ad_field_length;

      // Iterate through UUIDs in this AD structure
      // Start at i+2 (skip length and type bytes)
      for (uint16_t j = i + head_len; j + uuid_len <= field_end && j + uuid_len <= len; j += uuid_len) {
        // Safe to read uint16_t at this position
        ptr = ((uint16_t *)&data[j]);
        if (*ptr == esl_uuid) {
          return true;
        }
      }
    }

    // Advance to the next AD struct
    // The next structure starts at: current position + 1 (length byte) + ad_field_length (data)
    i = i + 1 + ad_field_length;
  }
  return false;
}

static void esl_lib_core_internal_reset(void)
{
  esl_lib_evt_t *last_evt;

  // Clean up current command, if any
  if (ap_state->command != NULL) {
    esl_lib_command_list_remove(&ap_state->command_list, ap_state->command);
    ap_state->command = NULL;
  }

  // Clean command list
  esl_lib_command_list_cleanup(&ap_state->command_list);
  esl_lib_log_core_debug("Command list cleanup complete" APP_LOG_NL);

  // Cleanup relationship
  esl_lib_image_transfer_cleanup();
  esl_lib_connection_cleanup();
  esl_lib_pawr_cleanup();
  (void)esl_lib_ap_control_cleanup();
  (void)esl_lib_initiator_filter_cleanup();

  // Cleanup events
  while ((last_evt = esl_lib_event_list_get_first()) != NULL) {
    esl_lib_event_list_remove_first();
  }
  while (esl_lib_event_gc_step(0) != SL_STATUS_IDLE) {
    // Keep calling until all events are freed
  }
  esl_lib_log_core_debug("Event list cleanup complete" APP_LOG_NL);
}

static void send_shutdown_ready_event(void)
{
  sl_status_t   sc;
  esl_lib_evt_t *lib_evt;

  // This event shall be the only one on the list and event_handler_step should be called immediately after!
  if (esl_lib_event_list_get_first() == NULL) {
    sc = esl_lib_event_list_allocate(ESL_LIB_EVT_SHUTDOWN_READY,
                                     0,
                                     &lib_evt);
    if (sc == SL_STATUS_OK) {
      sc = esl_lib_event_list_push_back(lib_evt);
      if (sc != SL_STATUS_OK) {
        // Free up memory on failure
        esl_lib_memory_free(lib_evt);
      }
    }

    esl_lib_log_core_debug("Library deinit complete with status: 0x%04x." APP_LOG_NL, sc);
  }
}
