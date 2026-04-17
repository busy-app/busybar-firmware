/***************************************************************************/ /**
 * @file sli_hal_si91x.h
 * @brief
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
#include "sli_routing_utility_types.h"
/**
 * @brief Initialize the HAL instance.
 *        This API is responsible for initializing all the components, tasks and variables/resources
 *        required for the operation of HAL Component
 *
 * @return Status of the initialization.
 */
sl_status_t sli_hal_si91x_init(void);

/**
 * @brief Send command packet to HAL instance.
 *        This API is responsible for sending command to the HAL layer which
 *        will send it ultimately to the NWP.
 *
 * @param[in] packet Pointer to command packet.
 * @param[in] packet_size Command packet buffer size.
 * @param[in] packet_status_handler Pointer to function which will be invoked by HAL layer to Acknowledge Packet TX in case of Asynchronous behavior of send. 
 * @param[in] context Pointer to component specific context pointer.   
 *
 * @return Status of the TX API.
 *         This API should return SL_STATUS_OK if the API succeeds in sending data synchronously.
 *         This API should return SL_STATUS_IN_PROGRESS if the API is sending data asynchronously in which case the sender has to wait for TX Acknowledgement.
 *         This API should return appropriate error values for all other error cases
 */
sl_status_t sli_hal_si91x_command_send_packet(void *packet,
                                              uint32_t packet_size,
                                              sli_routing_utility_packet_status_handler_t packet_status_handler,
                                              void *context);

/**
 * @brief Send data packet to HAL instance.
 *        This API is responsible for sending data to the HAL layer which
 *        will send it ultimately to the NWP.
 *
 * @param[in] packet Pointer to data packet.
 * @param[in] packet_size Data packet buffer size.
 * @param[in] packet_status_handler Pointer to function which will be invoked by HAL layer to Acknowledge Packet TX in case of Asynchronous behavior of send. 
 * @param[in] context Pointer to component specific context pointer.   
 *
 * @return Status of the TX API.
 *         This API should return SL_STATUS_OK if the API succeeds in sending data synchronously.
 *         This API should return SL_STATUS_IN_PROGRESS if the API is sending data asynchronously in which case the sender has to wait for TX Acknowledgement.
 *         This API should return appropriate error values for all other error cases
 */
sl_status_t sli_hal_si91x_data_send_packet(void *packet,
                                           uint32_t packet_size,
                                           sli_routing_utility_packet_status_handler_t packet_status_handler,
                                           void *context);

/**
 * @brief Send common packet to HAL instance.
 *        This API is responsible for sending common packet to the HAL layer which
 *        will send it ultimately to the NWP.
 *
 * @param[in] packet Pointer to common packet.
 * @param[in] packet_size Common packet buffer size.
 * @param[in] packet_status_handler Pointer to function which will be invoked by HAL layer to Acknowledge Packet TX in case of Asynchronous behavior of send. 
 * @param[in] context Pointer to component specific context pointer.   
 *
 * @return Status of the TX API.
 *         This API should return SL_STATUS_OK if the API succeeds in sending data synchronously.
 *         This API should return SL_STATUS_IN_PROGRESS if the API is sending data asynchronously in which case the sender has to wait for TX Acknowledgement.
 *         This API should return appropriate error values for all other error cases
 */
sl_status_t sli_hal_si91x_common_send_packet(void *packet,
                                             uint32_t packet_size,
                                             sli_routing_utility_packet_status_handler_t packet_status_handler,
                                             void *context);

/**
 * @brief Send BLE packet to HAL instance.
 *        This API is responsible for sending BLE packet to the HAL layer which
 *        will send it ultimately to the NWP.
 *
 * @param[in] packet Pointer to BLE packet.
 * @param[in] packet_size BLE packet buffer size.
 * @param[in] packet_status_handler Pointer to function which will be invoked by HAL layer to Acknowledge Packet TX in case of Asynchronous behavior of send. 
 * @param[in] context Pointer to component specific context pointer.   
 *
 * @return Status of the TX API.
 *         This API should return SL_STATUS_OK if the API succeeds in sending data synchronously.
 *         This API should return SL_STATUS_IN_PROGRESS if the API is sending data asynchronously in which case the sender has to wait for TX Acknowledgement.
 *         This API should return appropriate error values for all other error cases
 */
sl_status_t sli_hal_si91x_ble_send_packet(void *packet,
                                          uint32_t packet_size,
                                          sli_routing_utility_packet_status_handler_t packet_status_handler,
                                          void *context);

/**
 * @brief Deinitialize the HAL instance.
 *        This API is responsible for de-initializing and freeing all the resources initialized
 *        by sli_si91x_hal_init().
 *
 *
 * @return Status of the deinitialization.
 */
sl_status_t sli_hal_si91x_deinit(void);