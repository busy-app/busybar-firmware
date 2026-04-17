/***************************************************************************//**
 * @file sl_wisun_coap_collector_cli.c
 * @brief Wi-SUN CoAP Collector CLI handler
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_cli.h"
#include "sl_status.h"
#include "sl_wisun_cli_core.h"
#include "sl_wisun_coap_collector.h"
#include "sl_wisun_meter_collector_config.h"
#include "sl_wisun_led_driver.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* CLI app led toggle handler */
void app_led_toggle(const sl_cli_command_arg_t * arguments)
{
  const char *meter_ip = NULL;
  uint8_t led_id = 0;
  sockaddr_in6_t meter_addr = {
    .sin6_family = AF_INET6,
    .sin6_addr = { .address = { 0U } },
    .sin6_port = htons(SL_WISUN_METER_COLLECTOR_PORT)
  };

  meter_ip = sl_cli_get_argument_string(arguments, 0);
  led_id = sl_cli_get_argument_uint8(arguments, 1);

  if (meter_ip == NULL) {
    return;
  }

  if ((led_id != 0) && (led_id != 1)) {
    printf("[Failed: invalid led id parameter]\n");
    return;
  }

  if (inet_pton(AF_INET6, meter_ip, &meter_addr.sin6_addr) != 1 ) {
    printf("[Failed: invalid remote address parameter]\n");
  }

  if (sl_wisun_coap_collector_prepare_led_toggle_request(led_id) != SL_STATUS_OK) {
    printf("[Failed: LED toggle request not prepared]\n");
  }

  if (sl_wisun_coap_collector_send_led_toggle_request(&meter_addr) != SL_STATUS_OK) {
    printf("[Failed: Request not sent]\n");
  }
}

/* CLI app register meter handler */
void app_register_meter(const sl_cli_command_arg_t * arguments)
{
  const char *meter_ip      = NULL;
  sockaddr_in6_t meter_addr = {
    .sin6_family  = AF_INET6,
    .sin6_addr    = { .address = { 0U } },
    .sin6_port    = htons(SL_WISUN_METER_COLLECTOR_PORT)
  };

  // Remove broken meters
  sl_wisun_coap_collector_remove_broken_meters(SL_WISUN_MC_REQ_ASYNC);
  sl_wisun_coap_collector_remove_broken_meters(SL_WISUN_MC_REQ_REGISTER);

  meter_ip = sl_cli_get_argument_string(arguments, 0);
  if (meter_ip == NULL) {
    return;
  }

  if (inet_pton(AF_INET6, meter_ip, &meter_addr.sin6_addr) != 1) {
    printf("[Failed: invalid remote address parameter]\n");
    return;
  }

  switch (sl_wisun_coap_collector_register_meter(&meter_addr)) {
    case SL_STATUS_ALREADY_EXISTS:
      printf("[Failed: meter is already registered]\n");
      break;
    case SL_STATUS_FAIL:
      printf("[Failed: meter cannot be registered]\n");
      break;
    default:
      // nothing to do
      break;
  }
}

/* CLI app remove meter handler */
void app_remove_meter(const sl_cli_command_arg_t * arguments)
{
  const char *meter_ip      = NULL;
  sockaddr_in6_t meter_addr = {
    .sin6_family  = AF_INET6,
    .sin6_addr    = { .address = { 0U } },
    .sin6_port    = htons(SL_WISUN_METER_COLLECTOR_PORT)
  };

  meter_ip = sl_cli_get_argument_string(arguments, 0);
  if (meter_ip == NULL) {
    return;
  }

  if (inet_pton(AF_INET6, meter_ip, &meter_addr.sin6_addr) != 1) {
    printf("[Failed: invalid remote address parameter]\n");
    return;
  }

  if (sl_wisun_coap_collector_remove_meter(&meter_addr) == SL_STATUS_OK) {
    printf("[%s meter has been removed]\n", meter_ip);
  } else {
    printf("[Failed: meter is not registered]\n");
  }
}

/* CLI app async request handler */
void app_async_request(const sl_cli_command_arg_t * arguments)
{
  const char *meter_ip      = NULL;
  sockaddr_in6_t meter_addr = {
    .sin6_family  = AF_INET6,
    .sin6_addr    = { .address = { 0U } },
    .sin6_port    = htons(SL_WISUN_METER_COLLECTOR_PORT)
  };

  // Remove broken meters
  sl_wisun_coap_collector_remove_broken_meters(SL_WISUN_MC_REQ_ASYNC);
  sl_wisun_coap_collector_remove_broken_meters(SL_WISUN_MC_REQ_REGISTER);

  meter_ip = sl_cli_get_argument_string(arguments, 0);
  if (meter_ip == NULL) {
    return;
  }

  if (inet_pton(AF_INET6, meter_ip, &meter_addr.sin6_addr) != 1) {
    printf("[Failed: invalid remote address parameter]\n");
    return;
  }

  if (sl_wisun_coap_collector_send_async_request(&meter_addr) == SL_STATUS_FAIL) {
    printf("[Failed: async request failed]\n");
  }
}

/* CLI app list meters handler */
void app_list_meters(const sl_cli_command_arg_t * arguments)
{
  (void) arguments;

  // Remove broken meters
  sl_wisun_coap_collector_remove_broken_meters(SL_WISUN_MC_REQ_ASYNC);
  sl_wisun_coap_collector_remove_broken_meters(SL_WISUN_MC_REQ_REGISTER);

  sl_wisun_coap_collector_print_meters();
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
