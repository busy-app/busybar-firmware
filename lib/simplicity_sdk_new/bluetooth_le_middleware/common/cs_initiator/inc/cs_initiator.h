/***************************************************************************//**
 * @file
 * @brief CS initiator API
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

#ifndef CS_INITIATOR_H
#define CS_INITIATOR_H

/***************************************************************************//**
 * @addtogroup cs_initiator
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Includes

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_bt_api.h"
#include "sl_rtl_clib_api.h"
#include "cs_result.h"
#include "cs_initiator_client.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Definitions

/// Maximum number of steps
#define CS_MAX_STEP_COUNT 256

/// RTL library intermediate result type
typedef struct {
  uint8_t connection;                   ///< Connection handle
  float progress_percentage;            ///< Progress in percentages
} cs_intermediate_result_t;

/// Ranging data array type
typedef struct {
  uint32_t ranging_data_size; ///< Size of the ranging data array
  uint8_t *ranging_data;      ///< Ranging data array
} cs_ranging_data_array_t;

/// Unified ranging data
typedef struct {
  uint8_t num_steps;                 ///< Number of steps reported
  uint8_t *step_channels;            ///< Step channel array
  cs_ranging_data_array_t initiator; ///< Initiator ranging data
  cs_ranging_data_array_t reflector; ///< Reflector ranging data
} cs_ranging_data_t;

// CS Initiator callback function pointer types

/***************************************************************************//**
 * Initiator error callback type
 * Called in case any CS initiator related error occurred.
 *
 * @param[in] conn_handle connection handle.
 * @param[in] evt         error event.
 * @param[in] sc          status code: sl_status_t or rtl_error code depending
 *                        on the event.
 ******************************************************************************/
typedef void (*cs_error_cb_t)(uint8_t conn_handle, cs_error_event_t evt, uint32_t sc);

/***************************************************************************//**
 * Initiator result callback type
 * Called in case a CS procedure extracted successfully and based on the
 * extracted data a distance measurement by the RTL library has been successfully
 * performed.
 *
 * @param[in] conn_handle connection handle
 * @param[in] ranging_counter Ranging counter value.
 * @param[in] result pointer to the result array.
 * @param[in] result_data pointer to the result session data.
 * @param[in] ranging_data ranging data that the result was calculated from.
 * @param[in] user_data pointer to additional user data.
 ******************************************************************************/
typedef void (*cs_result_cb_t)(const uint8_t conn_handle,
                               const uint16_t ranging_counter,
                               const uint8_t *result,
                               const cs_result_session_data_t *result_data,
                               const cs_ranging_data_t *ranging_data,
                               const void *user_data);

/***************************************************************************//**
 * Initiator intermediate result callback type
 * Called only in case the static object tracking mode is active.
 * That means not in every CS procedure the CS initiator gets a distance
 * measurement. When the measurement is still in progress at the end of a
 * successfully extracted CS procedure this callback will be called instead of
 * the result callback.
 *
 * @param[in] result pointer to the intermediate result structure.
 * @param[in] user_data pointer to additional user data.
 ******************************************************************************/
typedef void (*cs_intermediate_result_cb_t)(const cs_intermediate_result_t *result,
                                            const void *user_data);

// -----------------------------------------------------------------------------
// Function declarations

/***************************************************************************//**
 * Create CS Initiator instance for the given connection handle.
 * @param[in] conn_handle connection handle
 * @param[in] initiator_config pointer to the initiator config
 * @param[in] rtl_config pointer to the RTL library config
 * @param[in] result_cb callback for result
 * @param[in] intermediate_result_cb callback for intermediate result
 * @param[in] error_cb callback for error
 * @param[out] instance_id RTL library instance ID (optional)
 *
 * @return status of the operation.
 ******************************************************************************/
sl_status_t cs_initiator_create(const uint8_t               conn_handle,
                                cs_initiator_config_t       *initiator_config,
                                const rtl_config_t          *rtl_config,
                                cs_result_cb_t              result_cb,
                                cs_intermediate_result_cb_t intermediate_result_cb,
                                cs_error_cb_t               error_cb,
                                uint8_t                     *instance_id);

/***************************************************************************//**
 * Create and configure initiator instances.
 ******************************************************************************/
void cs_initiator_init(void);

/***************************************************************************//**
 * Delete CS Initiator instance associated with the given connection handle.
 * @param[in] conn_handle connection handle
 *
 * @return status of the operation.
 ******************************************************************************/
sl_status_t cs_initiator_delete(const uint8_t conn_handle);

/***************************************************************************//**
 * Deinitialization function of CS Initiator component.
 ******************************************************************************/
void cs_initiator_deinit(void);

// -----------------------------------------------------------------------------
// Event / callback declarations

/***************************************************************************//**
 * Bluetooth stack event handler of the initiator events.
 * @param[in] evt Event coming from the Bluetooth stack.
 * @return true to send the event to the host in NCP case.
 ******************************************************************************/
bool cs_initiator_on_event(sl_bt_msg_t *evt);

#ifdef __cplusplus
};
#endif

/** @} (end addtogroup cs_initiator) */
#endif // CS_INITIATOR_H
