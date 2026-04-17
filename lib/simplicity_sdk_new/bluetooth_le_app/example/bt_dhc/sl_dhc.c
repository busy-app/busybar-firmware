/***************************************************************************//**
 * @file
 * @brief Dynamic Hardware Configuration logic.
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
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "nvm3.h"
#include "nvm3_default_config.h"
#include "sl_rail.h"
#include "sl_rail_util_rssi.h"
#include "sl_rail_util_pa_nvm_configs.h"
#include "rail.h"
#include "rail_types.h"
#include "pa_conversions_efr32.h"
#include "sl_memory_manager.h"
#include "sl_clock_manager.h"
#include "sl_clock_manager_oscillator_calibration_override.h"
#include "sl_component_catalog.h"
#include "sl_dhc.h"

// Message fragmentation. Message should fit into SL_BTCTRL_MAX_HCI_RESPONSE_LENGTH.
#define MSG_LEN_MAX                       16u
#define PA_CURVE_FRAGMENT_COUNT           ((uint8_t)ceilf(sizeof(sl_rail_nvm_pa_config_t) / (float)MSG_LEN_MAX) + 1u)

// Indicates which part of the message is next.
static uint8_t pa_curve_fragment_index    = 0u;

// Serialized PA Curves structure used for sending to host
static sl_rail_nvm_pa_config_t *pa_config;

void sl_dhc_handle_read(dhc_setting_t setting, void *msg)
{
  switch (setting) {
    case DHC_SETTING_VERSION:
    {
      uint8_t version_response = DHC_VERSION;
      sl_dhc_send_response(SL_STATUS_OK, &version_response, sizeof(version_response), msg);
      break;
    }

    case DHC_SETTING_RSSI_OFFSET:
    {
      int8_t rssi_offset = sl_rail_get_rssi_offset(SL_RAIL_EFR32_HANDLE);
      sl_dhc_send_response(SL_STATUS_OK, (uint8_t*)&rssi_offset, sizeof(rssi_offset), msg);
      break;
    }

    case DHC_SETTING_PA_CURVE:
    {
      if (pa_curve_fragment_index == 0) {
        // First fragment
        // Expose internal RAIL variable to access the current state of the PA curves
        extern RAIL_TxPowerCurvesConfigAlt_t powerCurvesState;

        // Prepare serialized data
        pa_config = sl_malloc(sizeof(sl_rail_nvm_pa_config_t));
        if (pa_config == NULL) {
          sl_dhc_send_response(SL_STATUS_NULL_POINTER, NULL, 0, msg);
          break;
        }
        sl_status_t sc = sli_rail_util_pa_nvm_serialize_config(&powerCurvesState, pa_config);
        if (sc != SL_STATUS_OK) {
          sl_free(pa_config);
          sl_dhc_send_response(sc, NULL, 0, msg);
          break;
        }

        // Only send the number of fragments in the first fragment.
        uint8_t fragments_cnt = PA_CURVE_FRAGMENT_COUNT;
        sl_dhc_send_response(sc, &fragments_cnt, sizeof(fragments_cnt), msg);
        pa_curve_fragment_index++;
      } else {
        if (pa_config == NULL) {
          // pa_config should be set by now
          sl_dhc_send_response(SL_STATUS_NULL_POINTER, NULL, 0, msg);
          break;
        }
        // Send the next fragment of PA curve
        size_t byte_index = (pa_curve_fragment_index - 1) * MSG_LEN_MAX;
        size_t bytes_remaining = sizeof(sl_rail_nvm_pa_config_t) - byte_index;

        if (bytes_remaining > MSG_LEN_MAX) {
          sl_dhc_send_response(SL_STATUS_OK, (uint8_t *)pa_config + byte_index, MSG_LEN_MAX, msg);
          pa_curve_fragment_index++;
        } else {
          // Last fragment
          sl_dhc_send_response(SL_STATUS_OK, (uint8_t *)pa_config + byte_index, bytes_remaining, msg);
          pa_curve_fragment_index = 0u;
          sl_free(pa_config);
        }
      }

      break;
    }

    case DHC_SETTING_PA_MODE:
    {
      RAIL_TxPowerConfig_t *rail_tx_power_config;
      rail_tx_power_config = (RAIL_TxPowerConfig_t *)sl_rail_util_pa_get_tx_power_config_2p4ghz();
      if (rail_tx_power_config == NULL) {
        sl_dhc_send_response(SL_STATUS_NULL_POINTER, NULL, 0, msg);
        break;
      }

      RAIL_TxPowerMode_t pa_mode = rail_tx_power_config->mode;
      sl_dhc_send_response(SL_STATUS_OK, (uint8_t *)&pa_mode, sizeof(pa_mode), msg);
      break;
    }

    case DHC_SETTING_CTUNE:
    {
      uint32_t ctune = 0u;
      sl_status_t sc = slx_clock_manager_hfxo_get_ctune(&ctune);
      sl_dhc_send_response(sc, (uint8_t *)&ctune, sizeof(ctune), msg);
      break;
    }

    default:
      sl_dhc_send_response(SL_STATUS_INVALID_PARAMETER, NULL, 0, msg);
      break;
  }
}

void sl_dhc_handle_write(dhc_setting_t setting, uint8_t *data, uint8_t data_len, void *msg)
{
  // Check message size
  if (data_len < DHC_HEADER_LEN || data_len > DHC_HEADER_LEN + MSG_LEN_MAX) {
    sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, msg);
    return;
  }
  if (data == NULL) {
    sl_dhc_send_response(SL_STATUS_NULL_POINTER, NULL, 0, msg);
    return;
  }

  switch (setting) {
    case DHC_SETTING_VERSION:
      // Defines the configuration structure. Read-only.
      sl_dhc_send_response(SL_STATUS_NOT_SUPPORTED, NULL, 0, msg);
      break;

    case DHC_SETTING_RSSI_OFFSET:
    {
      sl_status_t sc;
      int8_t rssi_offset;

      // Check message size
      if (data_len != DHC_HEADER_LEN + sizeof(rssi_offset)) {
        sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, msg);
        break;
      }

      rssi_offset = (int8_t)data[DHC_HEADER_LEN];
      sc = sl_rail_util_write_nvm_rssi(rssi_offset);
      sl_dhc_send_response(sc, (uint8_t *)&rssi_offset, sizeof(rssi_offset), msg);
      break;
    }

    case DHC_SETTING_PA_CURVE:
    {
      if (pa_curve_fragment_index == 0) {
        // Prepare memory
        pa_config = sl_malloc(sizeof(sl_rail_nvm_pa_config_t));
        if (pa_config == NULL) {
          sl_dhc_send_response(SL_STATUS_NULL_POINTER, NULL, 0, msg);
          break;
        }
        memset(pa_config, 0, sizeof(sl_rail_nvm_pa_config_t));

        // Check message length
        uint8_t fragments_num;
        if (data_len != DHC_HEADER_LEN + sizeof(fragments_num)) {
          sl_free(pa_config);
          sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, msg);
          break;
        }

        // First fragment only contains the number of fragments
        memcpy(&fragments_num, data + DHC_HEADER_LEN, data_len - DHC_HEADER_LEN);
        // Fragments number should match
        if (fragments_num != PA_CURVE_FRAGMENT_COUNT) {
          sl_free(pa_config);
          sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, msg);
          break;
        }

        pa_curve_fragment_index++;
        sl_dhc_send_response(SL_STATUS_OK, NULL, 0, msg);
      } else {
        if (pa_config == NULL) {
          // pa_config should be allocated by now
          sl_dhc_send_response(SL_STATUS_NULL_POINTER, NULL, 0, msg);
          break;
        }

        // Check message length
        if (data_len > DHC_HEADER_LEN + MSG_LEN_MAX) {
          pa_curve_fragment_index = 0u;
          sl_free(pa_config);
          sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, msg);
          break;
        }

        // Store next fragment
        size_t byte_index = (pa_curve_fragment_index - 1) * MSG_LEN_MAX;
        uint8_t *address = (uint8_t *)pa_config + byte_index;
        memcpy(address, data + DHC_HEADER_LEN, data_len - DHC_HEADER_LEN);
        pa_curve_fragment_index++;

        if (pa_curve_fragment_index != PA_CURVE_FRAGMENT_COUNT) {
          sl_dhc_send_response(SL_STATUS_OK, NULL, 0, msg);
        } else {
          // Last fragment received
          // Verify configuration
          RAIL_TxPowerCurvesConfigAlt_t *tx_power_curves_config_alt;
          tx_power_curves_config_alt = sli_rail_util_pa_nvm_deserialize_config(pa_config);
          if (tx_power_curves_config_alt == NULL) {
            pa_curve_fragment_index = 0u;
            sl_free(pa_config);
            sl_dhc_send_response(SL_STATUS_INVALID_CONFIGURATION, NULL, 0, msg);
            break;
          }

          RAIL_Status_t status;
          status = RAIL_VerifyTxPowerCurves(tx_power_curves_config_alt);
          if (status != RAIL_STATUS_NO_ERROR) {
            pa_curve_fragment_index = 0u;
            sl_free(pa_config);
            sl_dhc_send_response(SL_STATUS_INVALID_CONFIGURATION, (void *)&status, sizeof(status), msg);
            break;
          }

          // Write to NVM
          sl_status_t sc = sl_rail_util_pa_nvm_write_config(pa_config);
          sl_dhc_send_response(sc, NULL, 0, msg);

          pa_curve_fragment_index = 0u;
          sl_free(pa_config);
        }
      }

      break;
    }

    case DHC_SETTING_PA_MODE:
    {
      RAIL_TxPowerMode_t pa_mode;

      // Check message size
      if (data_len != DHC_HEADER_LEN + sizeof(pa_mode)) {
        sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, msg);
        break;
      }

      pa_mode = (RAIL_TxPowerMode_t)data[DHC_HEADER_LEN];

      // Check if power mode (PA) is supported
      if (!RAIL_SupportsTxPowerModeAlt(RAIL_EFR32_HANDLE, &pa_mode, NULL, NULL)) {
        sl_dhc_send_response(SL_STATUS_INVALID_MODE, NULL, 0, msg);
        break;
      }

      sl_status_t sc = sl_rail_util_pa_nvm_write_mode(pa_mode);
      sl_dhc_send_response(sc, (uint8_t *)&pa_mode, sizeof(pa_mode), msg);
      break;
    }

    case DHC_SETTING_CTUNE:
    {
      uint32_t ctune;

      // Check message size
      if (data_len != DHC_HEADER_LEN + sizeof(ctune)) {
        sl_dhc_send_response(SL_STATUS_INVALID_COUNT, NULL, 0, msg);
        break;
      }
      // Copy the payload from the message
      memcpy(&ctune, data + DHC_HEADER_LEN, sizeof(ctune));

      sl_status_t sc = sl_clock_manager_write_hfxo_calibration_override(ctune);
      sl_dhc_send_response(sc, (uint8_t *)&ctune, sizeof(ctune), msg);
      break;
    }

    default:
      sl_dhc_send_response(SL_STATUS_INVALID_PARAMETER, NULL, 0, msg);
      break;
  }
}
