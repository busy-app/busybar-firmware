/***************************************************************************//**
 * @file
 * @brief CS result API.
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
#ifndef CS_RESULT_H
#define CS_RESULT_H

// -----------------------------------------------------------------------------
// Includes

#include "sl_enum.h"
#include "sl_bt_api.h"
#include "cs_result_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Macros
// Only kept for backward compatibility, will be deprecated
#define PRINT_HEAD_AND_DATA(counter, is_data) \
  for (uint8_t is_data = (((counter - 1) % CS_INITIATOR_HEADER_LOG) > 0); is_data <= 1; is_data++)
  
// -----------------------------------------------------------------------------
// Enums, structs, typedefs

/// Field type values
/// @note mainmode is always PBR, submode is always RTT!
SL_ENUM(cs_result_field_type_t) {
  CS_RESULT_FIELD_DISTANCE_MAINMODE = 0x00, ///< filtered distance value (mainmode)
  CS_RESULT_FIELD_DISTANCE_SUBMODE,         ///< filtered distance value (submode)
  CS_RESULT_FIELD_DISTANCE_RAW_MAINMODE,    ///< raw distance value (mainmode)
  CS_RESULT_FIELD_DISTANCE_RAW_SUBMODE,     ///< raw distance value (submode)
  CS_RESULT_FIELD_LIKELINESS_MAINMODE,      ///< likeliness (mainmode)
  CS_RESULT_FIELD_LIKELINESS_SUBMODE,       ///< likeliness (submode)
  CS_RESULT_FIELD_DISTANCE_RSSI,            ///< distance value based on RSSI
  CS_RESULT_FIELD_VELOCITY_MAINMODE,        ///< velocity (mainmode)
  CS_RESULT_FIELD_VELOCITY_SUBMODE,         ///< velocity (submode)
  CS_RESULT_FIELD_BIT_ERROR_RATE            ///< bit error rate for RTT only
};

/// Result sesion data
typedef struct {
  uint8_t type_count;          ///< number of field types in the result buffer
  uint8_t last_type;           ///< last field type added to the result buffer
  uint8_t write_pos;           ///< actual end of the processed content
  uint8_t size;                ///< full size of the result buffer
} cs_result_session_data_t;

// -----------------------------------------------------------------------------
// Function declarations

/***************************************************************************//**
* Get the size of the field type.
* @param[in] type selected field type.
*
* @return size of the field type in bytes. 0 if the type is invalid.
*******************************************************************************/
uint8_t cs_result_get_type_length(cs_result_field_type_t type);

/***************************************************************************//**
 * Initialize the result data structure.
 * @param[in] result_data pointer to the result session data.
 ******************************************************************************/
void cs_result_initialize_results_data(cs_result_session_data_t *result_data);

/***************************************************************************//**
 * Append field type and value to the result buffer if possible.
 * @param[in] result_data pointer to the result data.
 * @param[in] target field type to be added.
 * @param[in] target_value pointer to the value to be added.
 * @param[in] buffer pointer to the result buffer.
 *
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t cs_result_append_field(cs_result_session_data_t *result_data,
                                   cs_result_field_type_t target,
                                   uint8_t *target_value,
                                   uint8_t *buffer);

/***************************************************************************//**
 * Extract field type and value from the end of the result buffer.
 * @param[in] result_data pointer to the result data.
 * @param[in] target field type to be extracted.
 * @param[in] buffer pointer to the result buffer.
 * @param[out] target_value pointer to the extracted value.
 *
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t cs_result_extract_field(cs_result_session_data_t *result_data,
                                    cs_result_field_type_t target,
                                    uint8_t *buffer,
                                    uint8_t *target_value);

/***************************************************************************//**
 * Create data for the result buffer.
 * @param[in] buffer pointer to the result buffer.
 * @param[in] buffer_length length of the result buffer.
 * @param[out] result_data pointer to the new session data.
 *
 * @return Status of the operation.
 ******************************************************************************/
sl_status_t cs_result_create_session_data(uint8_t *buffer,
                                          uint8_t buffer_length,
                                          cs_result_session_data_t *result_data);

/***************************************************************************//**
 * Print result data to the console.
 * @param[in] field_type type of the field to be printed.
 * @param[in] header true if header should be printed, false otherwise.
 * @param[in] result_data pointer to the result data.
 ******************************************************************************/
void cs_initiator_print_result(cs_result_field_type_t field_type,
                               bool header,
                               void *result_data);

/***************************************************************************//**
 * Print Bluetooth address to the console.
 * @param[in] header true if header should be printed, false otherwise.
 * @param[in] bt_address pointer to the Bluetooth address to be printed.
 ******************************************************************************/
void cs_initiator_print_bt_address(bool header,
                                   const bd_addr *bt_address);

#ifdef __cplusplus
}
#endif

#endif // CS_RESULT_H
