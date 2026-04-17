/***************************************************************************//**
* @file
* @brief BT Mesh Host Provisioner component - Out-of-Band Provisioning
********************************************************************************
* # License
* <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
********************************************************************************
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
*******************************************************************************/

// -----------------------------------------------------------------------------
// Includes

// standard library headers
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// app-related headers
#include "app.h"
#include "app_assert.h"
#include "app_log.h"
#include "btmesh_app_prov.h"
#include "btmesh_app_prov_oob.h"
#include "btmesh_prov.h"
#include "sl_common.h"
#include "sl_bt_api.h"


// -----------------------------------------------------------------------------
// Macros

/// OOB Public key for setting up the OOB requirements, set to 0 if unused.
#define OOB_PUBLIC_KEY                        0
/// Flag indicating OOB authorization method
#define OOB_AUTH_METHOD_FLAG                  sl_btmesh_node_auth_method_flag_input
/// Flag indicating output action, if any
#define OOB_OUTPUT_ACTION_FLAG                0
/// Flag indicating input action, if any
#define OOB_INPUT_ACTION_FLAG                 sl_btmesh_node_oob_input_action_flag_numeric
/// Minimum size of the in/outout action, 0 if not used
#define OOB_MINIMUM_OOB_ACTION_SIZE           1
/// Maximum size of the in/outout action, 0 if not used
#define OOB_MAXIMUM_OOB_ACTION_SIZE           8

// -----------------------------------------------------------------------------
// Enums, structs, typedefs


// -----------------------------------------------------------------------------
// Static function declarations


// -----------------------------------------------------------------------------
// Static variables

/// OOB is to be used in provisioning
bool oob_enabled = false;

/// Command line options
static struct option oob_long_options[OOB_OPTLENGTH] = {
  { "oob", no_argument, 0, 'o' }
};


// -----------------------------------------------------------------------------
// Function definitions

void btmesh_app_prov_set_oob_capability(bool capability)
{
  app_log_debug("OOB set to %s" APP_LOG_NEW_LINE, capability ? "true" : "false");
  oob_enabled = capability;
}

bool btmesh_app_prov_get_oob_status(void)
{
  return oob_enabled;
}

sl_status_t btmesh_oob_init(int cmd_opt, char *cmd_optarg)
{
  (void)cmd_optarg;
  sl_status_t sc = SL_STATUS_OK;

  switch (cmd_opt) {
    case 'o':
      btmesh_app_prov_set_oob_capability(true);
      break;
    default:
      sc = SL_STATUS_NOT_FOUND;
      break;
  }
  return sc;
}


// -----------------------------------------------------------------------------
// Event / callback definitions

/***************************************************************************//**
* Setup out-of-band provisioning
*******************************************************************************/
sl_status_t btmesh_prov_setup_oob(uuid_128 uuid)
{
  sl_status_t sc = SL_STATUS_OK;
  if (btmesh_app_prov_get_oob_status() == true) {
    app_log_debug("Setup OOB" APP_LOG_NEW_LINE);
    sc = sl_btmesh_prov_set_oob_requirements(uuid, 
                                             OOB_PUBLIC_KEY,
                                             OOB_AUTH_METHOD_FLAG,
                                             OOB_OUTPUT_ACTION_FLAG,
                                             OOB_INPUT_ACTION_FLAG,
                                             OOB_MINIMUM_OOB_ACTION_SIZE,
                                             OOB_MAXIMUM_OOB_ACTION_SIZE);

    if (sc != SL_STATUS_OK) {
      app_log_status_error_f(sc, "Failed to set OOB requirements" APP_LOG_NEW_LINE);
      return sc;
    }
  }
  return sc;
}

/***************************************************************************//**
* BT Mesh event handler for OOB
*******************************************************************************/
void btmesh_oob_on_event(sl_btmesh_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_btmesh_evt_prov_start_sent_id:
      app_log_debug("OOB Event: sl_btmesh_evt_prov_start_sent_id" APP_LOG_NEW_LINE);
      sl_btmesh_evt_prov_start_sent_t *prov_start_sent_data;
      prov_start_sent_data = (sl_btmesh_evt_prov_start_sent_t *)&(evt->data);
      app_log_debug("public_key %#x" APP_LOG_NEW_LINE, prov_start_sent_data->public_key);
      app_log_debug("authentication_method %#x" APP_LOG_NEW_LINE, prov_start_sent_data->authentication_method);
      app_log_debug("authentication_action %#x" APP_LOG_NEW_LINE, prov_start_sent_data->authentication_action);
      app_log_debug("authentication_size %#x" APP_LOG_NEW_LINE, prov_start_sent_data->authentication_size);
      break;
    case sl_btmesh_evt_prov_oob_display_input_id:
      app_log_debug("OOB Event: sl_btmesh_evt_prov_oob_display_input_id" APP_LOG_NEW_LINE);
      sl_btmesh_evt_prov_oob_display_input_t *prov_oob_display_input_data = (sl_btmesh_evt_prov_oob_display_input_t *)&(evt->data);
      app_log_debug("input_action %#x" APP_LOG_NEW_LINE, prov_oob_display_input_data->input_action);
      app_log_debug("input_size %#x" APP_LOG_NEW_LINE, prov_oob_display_input_data->input_size);
      uint32_t oob_value = 0;

      // The OOB display input data is encoded according to Mesh Protocol 1.1 specifcation Section 5.4.2.4.
      // The data array consists of either 16 or 32 bytes, padded with zeros from the left.
      // The actual OOB value will be 8 or less decimal digits (at most contained in the last 4 bytes)
      // So this will not overflow the oob_value variable.
      for (uint8_t i = 0; i < prov_oob_display_input_data->data.len; i++) {
        oob_value = (oob_value << 8) | prov_oob_display_input_data->data.data[i];
      }

      char oob_str[OOB_MAXIMUM_OOB_ACTION_SIZE + 1];
      snprintf(oob_str, sizeof(oob_str), "%0*u", prov_oob_display_input_data->input_size, oob_value);
      app_log_info("Input OOB numeric: %s" APP_LOG_NEW_LINE, oob_str);
      break;
    default:
      break;
  }
}

/***************************************************************************//**
* Check if OOB-related options are enabled
*******************************************************************************/
sl_status_t btmesh_oob_on_check_cmd_options(int cmd_opt, char *cmd_optarg)
{
  return btmesh_oob_init(cmd_opt, cmd_optarg);
}

void btmesh_oob_on_build_cmd_options(struct option *long_options)
{
  if (NULL != long_options) {
    memcpy(long_options, oob_long_options, OOB_OPTLENGTH * sizeof(struct option));
  }
}


