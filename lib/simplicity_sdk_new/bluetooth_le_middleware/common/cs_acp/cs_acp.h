/***************************************************************************//**
 * @file
 * @brief CS Application Co-Processor types
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
#ifndef CS_ACP_H
#define CS_ACP_H

/***********************************************************************************************//**
 * @addtogroup cs_acp
 * @{
 **************************************************************************************************/

// -----------------------------------------------------------------------------
// Includes

#include "sl_bgapi.h"
#include "sl_common.h"
#include "sl_component_catalog.h"
#include "sl_enum.h"
#include "cs_result_config.h"

#ifdef SL_CATALOG_CS_INITIATOR_CLIENT_PRESENT
#include "cs_initiator_client.h"
#endif // SL_CATALOG_CS_INITIATOR_CLIENT_PRESENT

#ifdef SL_CATALOG_CS_REFLECTOR_CONFIG_PRESENT
#include "cs_reflector.h"
#endif // SL_CATALOG_CS_REFLECTOR_CONFIG_PRESENT

// -----------------------------------------------------------------------------
// Macros

/// The MSB of the fragments_left field indicates the start of a fragmented message
#define CS_ACP_FIRST_FRAGMENT_MASK 0x80
/// The remaining bits of the fragments_left field indicate the number of fragments left
#define CS_ACP_FRAGMENTS_LEFT_MASK 0x7F
/// Bitmask for the RAS mode bit in the target config
#define CS_ACP_TARGET_CONFIG_RAS_MODE_BIT_POS 0x00

// -----------------------------------------------------------------------------
// Enums, structs, typedefs

/// @name CS ACP commands
/// @brief Command enumerator defines the available ACP commands
///        between the host application and target device.
SL_ENUM(cs_acp_cmd_id_t) {
  CS_ACP_CMD_CREATE_INITIATOR = 0,  ///< Create initiator instance
  CS_ACP_CMD_CREATE_REFLECTOR = 1,  ///< Create reflector instance
  CS_ACP_CMD_INITIATOR_ACTION = 2,  ///< Initiator action (eg. delete instance)
  CS_ACP_CMD_REFLECTOR_ACTION = 3,  ///< Reflector action (eg. delete instance)
  CS_ACP_CMD_ANTENNA_CONFIGURE = 4, ///< Configure antenna
  CS_ACP_CMD_ENABLE_TRACE = 5,      ///< Enable BGAPI trace feature
  CS_ACP_CMD_GET_TARGET_CONFIG = 6  ///< Get ACP target configuration
};

/// @name ACP initiator actions
/// @brief Initiator role related action enumerator.
SL_ENUM(cs_acp_initiator_action_t) {
  CS_ACP_ACTION_DELETE_INITIATOR = 0 ///< Delete initiator instance
};

/// @name ACP reflector actions
/// @brief Reflector role related action enumerator.
SL_ENUM(cs_acp_reflector_action_t) {
  CS_ACP_ACTION_DELETE_REFLECTOR = 0 ///< Delete reflector instance
};

#ifdef SL_CATALOG_CS_INITIATOR_CLIENT_PRESENT
/// @name Create initiator command data
/// @struct cs_acp_create_initiator_cmd_data_t
/// @brief Data structure that contains the necessary details to create
///        an initiator instance on the target device.
SL_PACK_START(1)
typedef struct {
  uint8_t connection_id;                  ///< Connection ID
  cs_initiator_config_t initiator_config; ///< Initiator config
  rtl_config_t rtl_config;                ///< RTL handle config
  uint8_t extended_result;                ///< Extended result
} SL_ATTRIBUTE_PACKED cs_acp_create_initiator_cmd_data_t;
SL_PACK_END()

SL_PACK_START(1)
/// @name Initiator action data
/// @struct cs_acp_initiator_action_cmd_data_t
/// @brief Data structure that contains the necessary details to handle
///        initiator actions.
typedef struct {
  uint8_t connection_id;                      ///< Connection ID
  cs_acp_initiator_action_t initiator_action; ///< ACP initiator action enumerator
} SL_ATTRIBUTE_PACKED cs_acp_initiator_action_cmd_data_t;
SL_PACK_END()

#endif // SL_CATALOG_CS_INITIATOR_CLIENT_PRESENT

#ifdef SL_CATALOG_CS_REFLECTOR_CONFIG_PRESENT
SL_PACK_START(1)
/// @name Create reflector command data
/// @struct cs_acp_create_reflector_cmd_data_t
/// @brief Data structure that contains the information necessary to create
///        a reflector instance on the target device.
typedef struct create_reflector_cmd_data {
  uint8_t connection_id;                      ///< Connection ID
  cs_reflector_config_t reflector_config;     ///< Reflector config
} SL_ATTRIBUTE_PACKED cs_acp_create_reflector_cmd_data_t;
SL_PACK_END()

SL_PACK_START(1)
/// @name Reflector action data
/// @struct cs_acp_reflector_action_cmd_data_t
/// @brief Data structure that contains the necessary details to handle
///        reflector actions.
typedef struct {
  uint8_t connection_id;                      ///< Connection ID
  cs_acp_reflector_action_t reflector_action; ///< ACP reflector action enumerator
} SL_ATTRIBUTE_PACKED cs_acp_reflector_action_cmd_data_t;
SL_PACK_END()

#endif // SL_CATALOG_CS_REFLECTOR_CONFIG_PRESENT

SL_PACK_START(1)
/// @name ACP target config response data
/// @struct cs_acp_get_target_config_rsp_t
/// @brief Data structure that contains the necessary details about the actual
///        configuration of the target device.
typedef struct {
  uint8_t target_config_bitfield;       ///< Target config bitfield
  uint8_t max_initiator_instance_count; ///< Maximum initiator instance count
  uint8_t max_bluetooth_connections;    ///< Maximum BLE connections
} SL_ATTRIBUTE_PACKED cs_acp_get_target_config_rsp_t;
SL_PACK_END()

SL_PACK_START(1)
/// @name ACP command data
/// @struct cs_acp_cmd_t
/// @brief Command data structure that contains all necessary details for every
///        command sent from the host application to the target device.
typedef struct {
  cs_acp_cmd_id_t cmd_id; ///< CS ACP command enumerator
  union {
#ifdef SL_CATALOG_CS_INITIATOR_CLIENT_PRESENT
    cs_acp_create_initiator_cmd_data_t initiator_cmd_data;    ///< Create initiator command data
    cs_acp_initiator_action_cmd_data_t initiator_action_data; ///< Initiator action command data
#endif // SL_CATALOG_CS_INITIATOR_CLIENT_PRESENT
#ifdef SL_CATALOG_CS_REFLECTOR_CONFIG_PRESENT
    cs_acp_create_reflector_cmd_data_t reflector_cmd_data;    ///< Create reflector command data
    cs_acp_reflector_action_cmd_data_t reflector_action_data; ///< Reflector action command data
#endif // SL_CATALOG_CS_REFLECTOR_CONFIG_PRESENT
    uint8_t antenna_config_wired;                             ///< Antenna configuration for wired offset
    uint8_t enable_trace;                                     ///< Enable BGAPI trace feature
  } data;
} SL_ATTRIBUTE_PACKED cs_acp_cmd_t;
SL_PACK_END()

/// @name CS ACP event enumerator
/// @brief Event enumerator defines the available ACP events coming from
///        target device to the host application.
SL_ENUM(cs_acp_event_id_t) {
  CS_ACP_EVT_RESULT_ID = 0,              ///< Result event (includes distance, likeliness etc.)
  CS_ACP_EVT_STATUS_ID = 1,              ///< Status change event (error, success etc.)
  CS_ACP_EVT_INTERMEDIATE_RESULT_ID = 2, ///< Intermediate result event (progress percentage)
  CS_ACP_EVT_EXTENDED_RESULT_ID = 3      ///< Extended result event (fragments)
};

SL_PACK_START(1)
/// @name Result event data
/// @struct cs_acp_result_evt_t
/// @brief Data structure that contains the high level results
///        (e.g., distance, likeliness, etc.) in type-value pairs.
typedef struct {
  uint8_t type_value_list[CS_RESULT_MAX_BUFFER_SIZE]; ///< Result data in type-value pairs
} SL_ATTRIBUTE_PACKED cs_acp_result_evt_t;
SL_PACK_END()

SL_PACK_START(1)
/// @name Intermediate result event data
/// @struct cs_acp_intermediate_result_evt_t
/// @brief Data structure that contains the progress percentage
///        of the high level results. This feature is used for a specialized
///        algorithm mode in which multiple measurements happens before estimation.
///
/// @note Check the different Algorithm modes in the CS initiator component
///       for more!
typedef struct {
  float progress_percentage;                          ///< Progress percentage
} SL_ATTRIBUTE_PACKED cs_acp_intermediate_result_evt_t;
SL_PACK_END()

SL_PACK_START(1)
/// @name Result event data extended with CS procedure (fragmented)
/// @struct cs_acp_extended_result_evt_t
/// @brief Data structure containing high-level results along with CS procedure data
///        and ranging data in a serialized and fragmented format.
/// @details The content elements placed in the following order:
///          - result size (1 byte)
///          - result buffer content (variable size; type is 1 byte, value is typically 4 bytes)
///          - number of steps (1 byte)
///          - step channel array (variable size, 1 byte per step)
///          - initiator ranging data size (4 bytes)
///          - initiator RAS data (variable size)
///          - reflector ranging data size (4 bytes)
///          - reflector RAS data (variable size)
///
///          This entire data content will be fragmented and sent to the host application
typedef struct {
  uint8_t fragments_left; ///< Number of fragments left
  uint8array fragment;    ///< Content of the fragment
} SL_ATTRIBUTE_PACKED cs_acp_extended_result_evt_t;
SL_PACK_END()

SL_PACK_START(1)
/// @name Status change event data
/// @struct cs_acp_status_t
/// @brief Data structure that contains the status change event.
typedef struct {
  uint32_t sc;                                            ///< Status code
  uint8_t error;                                          ///< Error code
} SL_ATTRIBUTE_PACKED cs_acp_status_t;
SL_PACK_END()

SL_PACK_START(1)
/// @name CS ACP event data
/// @struct cs_acp_event_t
/// @brief Data structure that contains all necessary details for every ACP event.
typedef struct {
  uint8_t connection_id;                                  ///< Connection ID
  cs_acp_event_id_t acp_evt_id;                           ///< CS ACP event enumerator
  union {
    cs_acp_result_evt_t result;                           ///< Result event data
    cs_acp_intermediate_result_evt_t intermediate_result; ///< Intermediate result event data
    cs_acp_extended_result_evt_t ext_result;              ///< Extended result event data
    cs_acp_status_t stat;                                 ///< Status change event data
  } data;
} SL_ATTRIBUTE_PACKED cs_acp_event_t;
SL_PACK_END()

/** @} (end addtogroup cs_acp) */
#endif // CS_ACP_H
