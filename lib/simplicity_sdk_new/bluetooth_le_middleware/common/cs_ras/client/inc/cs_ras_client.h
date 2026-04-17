/***************************************************************************//**
 * @file
 * @brief CS RAS Client API
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

#ifndef CS_RAS_CLIENT_H
#define CS_RAS_CLIENT_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "cs_ras_common.h"
#include "cs_ras_client_timeout.h"

/// Ranging Service (RAS) UUID
#define CS_RAS_SERVICE_UUID 0x185B

/// RAS Client configuration structure type
typedef struct {
  uint8_t real_time_ranging_data_indication : 1;
  uint8_t on_demand_ranging_data_indication : 1;
  uint8_t ranging_data_ready_notification : 1;
  uint8_t ranging_data_overwritten_notification : 1;
} cs_ras_client_config_t;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Create RAS Client.
 *
 * @param[in] connection Connection handle.
 * @param[in] handles    GATT database handles.
 * @param[in] att_mtu    Initial MTU size.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_create(uint8_t                 connection,
                                 cs_ras_gattdb_handles_t *handles,
                                 uint16_t                att_mtu);

/**************************************************************************//**
 * Get features supported by the RAS Server.
 *
 * @note Client must be created and initialized before calling this function.
 *
 * @param[in] connection Connection handle.
 * @param[in] handles    Characteristic handles
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_get_features(uint8_t           connection,
                                       cs_ras_features_t *features);

/**************************************************************************//**
 * Start receiving Real-Time Ranging Data from the RAS server.
 *
 * @note Should be called to enable reception after enabling Real-Time mode and
 *       also after a reception is finished.
 *
 * @param[in] connection      Connection handle.
 * @param[in] data_size       Size of data buffer.
 * @param[in] data            Data buffer.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_real_time_receive(uint8_t  connection,
                                            uint32_t data_size,
                                            uint8_t  *data);

/**************************************************************************//**
 * Read data ready from the RAS server.
 *
 * @param[in] connection      Connection handle.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_read_data_ready(uint8_t connection);

/**************************************************************************//**
 * Read data overwritten from the RAS server.
 *
 * @param[in] connection      Connection handle.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_read_data_overwritten(uint8_t connection);

/**************************************************************************//**
 * Abort On-Demand Ranging Data transfer.
 *
 * @param[in] connection Connection handle.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_abort(uint8_t connection);

/**************************************************************************//**
 * Acknowledge reception of On-Demand Ranging Data.
 *
 * @param[in] connection      Connection handle.
 * @param[in] ranging_counter Ranging counter value.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_ack(uint8_t                  connection,
                              cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Configure RAS client modes.
 *
 * @param[in] connection Connection handle.
 * @param[in] config     Client configuration structure.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_configure(uint8_t                connection,
                                    cs_ras_client_config_t config);

/**************************************************************************//**
 * Select a mode for the RAS client to operate with.
 *
 * @param[in] connection Connection handle.
 * @param[in] mode       Mode to select.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_select_mode(uint8_t connection,
                                      cs_ras_mode_t mode);

/**************************************************************************//**
 * Get current mode of the RAS client.
 *
 * @param[in]  connection Connection handle.
 * @param[out] mode       Current mode.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_get_mode(uint8_t connection, cs_ras_mode_t *mode);

/**************************************************************************//**
 * Get (request) Ranging Data from the RAS server.
 *
 * @param[in] connection      Connection handle.
 * @param[in] ranging_counter Ranging counter value.
 * @param[in] data_size       Size of data buffer.
 * @param[in] data            Data buffer.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_get_ranging_data(uint8_t  connection,
                                           uint16_t ranging_counter,
                                           uint32_t data_size,
                                           uint8_t  *data);

/**************************************************************************//**
 * Retrieve lost segments.
 *
 * @param[in] connection      Connection handle.
 * @param[in] ranging_counter Ranging counter value.
 * @param[in] start_segment   Start segment to retrieve from (included).
 * @param[in] end_segment     End segment of the retrieval (included).
 * @param[in] data_size       Size of data buffer.
 * @param[in] data            Data buffer.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t cs_ras_client_retreive_lost_segments(uint8_t  connection,
                                                 uint16_t ranging_counter,
                                                 uint8_t  start_segment,
                                                 uint8_t  end_segment,
                                                 uint32_t data_size,
                                                 uint8_t  *data);

/**************************************************************************//**
 * Callback indicates that the client has been initialized and features of the
 * RAS server has been read.
 *
 * @param[in] connection Connection handle.
 * @param[in] features   Characteristic handles.
 * @param[in] sc         Result of the initialization (creation).
 *****************************************************************************/
void cs_ras_client_on_initialized(uint8_t           connection,
                                  cs_ras_features_t features,
                                  sl_status_t       sc);

/**************************************************************************//**
 * Callback indicates that the RAS mode has been changed.
 *
 * @param[in] connection Connection handle.
 * @param[in] mode       Applied mode
 * @param[in] sc         Status code
 *****************************************************************************/
void cs_ras_client_on_mode_changed(uint8_t       connection,
                                   cs_ras_mode_t mode,
                                   sl_status_t   sc);

/**************************************************************************//**
 * Callback indicates that new ranging data is ready.
 *
 * @param[in] connection      Connection handle.
 * @param[in] ranging_counter Ranging counter of the ranging data available.
 *****************************************************************************/
void cs_ras_client_on_ranging_data_ready(uint8_t                  connection,
                                         cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Callback indicates that ranging data was overwritten on the RAS Server.
 *
 * @param[in] connection      Connection handle.
 * @param[in] ranging_counter Ranging counter of the overwritten ranging data.
 *****************************************************************************/
void cs_ras_client_on_ranging_data_overwritten(uint8_t                  connection,
                                               cs_ras_ranging_counter_t ranging_counter);

/**************************************************************************//**
 * Callback indicates the end of a data reception in Real-Time or On-Demand
 * mode.
 *
 * @param[in] connection         Connection handle.
 * @param[in] real_time          True for Real-Time, false for On-Demand data.
 * @param[in] retrieve_lost      True for retrieve lost segments procedure.
 * @param[in] sc                 Status of the operation.
 * @param[in] response           Response code.
 * @param[in] ranging_counter    Ranging counter of the request.
 * @param[in] start_segment      Start segment of the request.
 * @param[in] end_segment        End segment of the request.
 * @param[in] recoverable        Data reception is recoverable.
 * @param[in] size               Size of the data in the buffer.
 * @param[in] last_arrived       Last segment arrived.
 * @param[in] last_known_segment Last known segment.
 * @param[in] lost_segments      Lost segments bitmask.
 *****************************************************************************/
void cs_ras_client_on_ranging_data_reception_finished(uint8_t                         connection,
                                                      bool                            real_time,
                                                      bool                            retrieve_lost,
                                                      sl_status_t                     sc,
                                                      cs_ras_cp_response_code_value_t response,
                                                      cs_ras_ranging_counter_t        ranging_counter,
                                                      uint8_t                         start_segment,
                                                      uint8_t                         end_segment,
                                                      bool                            recoverable,
                                                      uint32_t                        size,
                                                      bool                            last_arrived,
                                                      uint8_t                         last_known_segment,
                                                      uint64_t                        lost_segments);

/**************************************************************************//**
 * Callback indicates that a Control Point operation has been finished.
 *
 * @param[in] connection         Connection handle.
 * @param[in] opcode             OpCode.
 * @param[in] sc                 Status of the operation.
 * @param[in] response           Response code.
 *****************************************************************************/
void cs_ras_client_on_operation_finished(uint8_t                         connection,
                                         cs_ras_cp_opcode_t              opcode,
                                         sl_status_t                     sc,
                                         cs_ras_cp_response_code_value_t response);

/**************************************************************************//**
 * Callback indicates that the Ack Control Point operation has been finished.
 *
 * @param[in] connection         Connection handle.
 * @param[in] sc                 Status of the operation.
 * @param[in] response           Response code.
 *****************************************************************************/
void cs_ras_client_on_ack_finished(uint8_t                         connection,
                                   sl_status_t                     sc,
                                   cs_ras_cp_response_code_value_t response);

/**************************************************************************//**
 * Callback indicates that the Abort Control Point operation has been finished.
 *
 * @param[in] connection         Connection handle.
 * @param[in] sc                 Status of the operation.
 * @param[in] response           Response code.
 *****************************************************************************/
void cs_ras_client_on_abort_finished(uint8_t                         connection,
                                     sl_status_t                     sc,
                                     cs_ras_cp_response_code_value_t response);

#ifdef __cplusplus
};
#endif

#endif // CS_RAS_CLIENT_H
