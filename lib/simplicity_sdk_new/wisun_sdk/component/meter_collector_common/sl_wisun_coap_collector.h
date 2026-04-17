/***************************************************************************//**
 * @file sl_wisun_coap_collector.h
 * @brief Wi-SUN CoAP Collector
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

#ifndef SL_WISUN_COAP_COLLECTOR_H
#define SL_WISUN_COAP_COLLECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "sl_status.h"
#include "socket/socket.h"
#include "sli_wisun_meter_collector.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Init CoAP Collector.
 * @details Init CoAP Collector component
 *****************************************************************************/
void sl_wisun_coap_collector_init(void);

/**************************************************************************//**
* @brief Prepare LED Toggle request.
* @details It should be used before sending
* @param[in] led_id LED ID
* @return SL_STATUS_OK Success
* @return SL_STATUS_FAIL Failure
******************************************************************************/
sl_status_t sl_wisun_coap_collector_prepare_led_toggle_request(const uint8_t led_id);

/**************************************************************************//**
* @brief Send LED toggle request.
* @details Send the prepared request to the meter
* @param[in] meter_addr Meter address
* @return SL_STATUS_OK Success
* @return SL_STATUS_FAIL Failure
******************************************************************************/
sl_status_t sl_wisun_coap_collector_send_led_toggle_request(const sockaddr_in6_t * meter_addr);

/**************************************************************************//**
 * @brief Register Meter.
 * @details Add meter to the meter storage
 * @param[in] meter_addr meter address structure
 * @return SL_STATUS_OK meter has been successfully added
 * @return SL_STATUS_ALREADY_EXISTS meter had already been added
 * @return SL_STATUS_FAIL on error
 *****************************************************************************/
sl_status_t sl_wisun_coap_collector_register_meter(const sockaddr_in6_t * const meter_addr);

/**************************************************************************//**
 * @brief Remove Meter.
 * @details Remove registered meter from the registered meter storage
 * @param[in] meter_addr meter address structure
 * @return SL_STATUS_OK meter has been successfully removed
 * @return SL_STATUS_FAIL on error
 *****************************************************************************/
sl_status_t sl_wisun_coap_collector_remove_meter(const sockaddr_in6_t * const meter_addr);

/**************************************************************************//**
 * @brief Send async request.
 * @details Send async request to the given meter
 * @param[in] meter_addr meter address structure
 * @return SL_STATUS_OK async request has been successfully sent
 * @return SL_STATUS_FAIL on error
 *****************************************************************************/
sl_status_t sl_wisun_coap_collector_send_async_request(const sockaddr_in6_t * const meter_addr);

/**************************************************************************//**
 * @brief Print Meters.
 * @details Print registered and async meters
 *****************************************************************************/
void sl_wisun_coap_collector_print_meters(void);

/**************************************************************************//**
 * @brief Remove broken meters
 * @details Remove meters from the given mempool which did not reply for the
 *          request in time.
 * @param[in] req Request type
 *****************************************************************************/
void sl_wisun_coap_collector_remove_broken_meters(sl_wisun_request_type_t req);

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_COAP_COLLECTOR_H
