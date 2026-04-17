/***************************************************************************//**
 * @file
 * @brief BT Mesh Common Manufacturing Token
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

#include "sl_btmesh_cmt.h"
#include "sl_btmesh_cmt_config.h"

#include "sl_bt_api.h"
#include "sl_btmesh_api.h"

#include "sl_token_manager_api.h"
#include "sl_token_manager_manufacturing.h"
#include "sl_token_manager_defines.h"

#include "sl_status.h"

// Warning! The app_btmesh_util shall be included after the component configuration
// header file in order to provide the component specific logging macro.
#include "app_btmesh_util.h"

/***************************************************************************//**
 * @addtogroup btmesh_ctm
 * @{
 ******************************************************************************/

/// Location of Mesh Configuration Data token
#define SL_BTMESH_CMT_MESH_CONFIG_DATA   (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | SL_BTMESH_CMT_MESH_TOKEN_OFFSET)
/// Signature length in bytes
#define SL_BTMESH_CMT_TOKEN_SIGNATURE_LEN 64

/// Mesh token data
static uint8_t mesh_token_data[SL_BTMESH_CMT_MESH_CONFIG_DATA_SIZE] = { 0 };

/*******************************************************************************
 * Find the first occurrence of a tag in the byte array containing length-tag-value triplets
 *
 * @param[in] token_data Pointer to the byte array
 * @param[in] token_len Length of the byte array
 * @param[in] tag Tag to search for
 * @param[out] out_ptr Pointer to the value field of the found tag, or NULL if not found
 * @param[out] out_len Length of the value field of the found tag, or 0 if not found
 ******************************************************************************/
static void find_tagged_data(const uint8_t *token_data,
                             size_t token_len,
                             uint8_t tag,
                             const uint8_t **out_ptr,
                             size_t *out_len);

/*******************************************************************************
 * Initialize the BT Mesh Common Manufacturing Token component
 ******************************************************************************/
void sl_btmesh_cmt_on_init(void)
{
  log_info("Initializing BT Mesh Common Manufacturing Token" NL);
  sl_token_init();
  // Read Mesh token into our buffer
  sl_status_t sc = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_SECURE_TOKEN(SL_BTMESH_CMT_MESH_CONFIG_DATA),
                                             mesh_token_data,
                                             SL_BTMESH_CMT_MESH_CONFIG_DATA_SIZE);
  log_status_error_f(sc, "Failed to read Mesh token data" NL);

  #if SL_BTMESH_CMT_USE_UUID
  uuid_128 uuid = sl_btmesh_cmt_get_uuid();
  sc = sl_btmesh_node_set_uuid(uuid);
  log_status_error_f(sc, "Failed to set UUID" NL);
  #endif
}

/*******************************************************************************
 * Get the Mesh Manufacturing Token data
 ******************************************************************************/
uint8_t *sl_btmesh_cmt_get_token_data(void)
{
  return mesh_token_data;
}

/*******************************************************************************
 * Get UUID from Common Manufacturing Token
 ******************************************************************************/
uuid_128 sl_btmesh_cmt_get_uuid(void)
{
  uint8_t *uuid = NULL;
  size_t uuid_len = 0;
  uuid_128 uuid_128 = { 0 };

  find_tagged_data(mesh_token_data,
                   SL_BTMESH_CMT_MESH_CONFIG_DATA_SIZE - SL_BTMESH_CMT_TOKEN_SIGNATURE_LEN,
                   SL_BTMESH_CMT_TAG_UUID,
                   (const uint8_t **)&uuid,
                   &uuid_len);
  if (uuid != NULL && uuid_len == 16) {
    // Set device UUID for unprovisioned node
    memcpy(uuid_128.data, uuid, 16);
  }
  return uuid_128;
}

/*******************************************************************************
 * Get OOB Data from Common Manufacturing Token
 ******************************************************************************/
sl_status_t sl_btmesh_cmt_get_oob_data(uint8_t  *output_size,
                                       uint16_t *output_actions,
                                       uint8_t  *input_size,
                                       uint16_t *input_actions)
{
  sl_status_t sc = SL_STATUS_FAIL;
  uint8_t *oob_config = NULL;
  size_t oob_config_len = 0;
  find_tagged_data(mesh_token_data,
                   SL_BTMESH_CMT_MESH_CONFIG_DATA_SIZE - SL_BTMESH_CMT_TOKEN_SIGNATURE_LEN,
                   SL_BTMESH_CMT_TAG_OOB_CONFIG,
                   (const uint8_t **)&oob_config,
                   &oob_config_len);
  if (oob_config == NULL || oob_config_len != 6) {
    sc = SL_STATUS_FAIL;
  } else {
    // Parse OOB config data
    *output_size = oob_config[0];
    *output_actions = (uint16_t)(oob_config[1] | (oob_config[2] << 8));
    *input_size = oob_config[3];
    *input_actions = (uint16_t)(oob_config[4] | (oob_config[5] << 8));
    sc = SL_STATUS_OK;
  }
  return sc;
}

/*******************************************************************************
 * Find the first occurrence of a tag in the byte array containing length-tag-value triplets
 ******************************************************************************/
static void find_tagged_data(const uint8_t *token_data,
                             size_t token_len,
                             uint8_t tag,
                             const uint8_t **out_ptr,
                             size_t *out_len)
{
  *out_ptr = NULL;
  *out_len = 0;
  size_t i = 0;
  while (i + 2 <= token_len) {
    size_t len = token_data[i];
    uint8_t t = token_data[i + 1];
    if (len == 0) {
      break;
    }
    if (t == tag) {
      if (i + 2 + len <= token_len) {
        *out_ptr = &token_data[i + 2];
        *out_len = len;
      }
      break;
    }
    i += 2 + len;
  }
}
/** @} end btmesh_ctm */

#ifdef __cplusplus
}
#endif // __cplusplus
