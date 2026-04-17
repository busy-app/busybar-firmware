/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_protocol_types.h"
#include "sl_net_constants.h"
#include "sl_net_ip_types.h"
#ifdef SLI_SI91X_INTERNAL_MDNS
#include "sl_mdns.h"
#endif
#ifdef SLI_SI91X_INTERNAL_HTTP_CLIENT
#include "sl_http_client.h"
#endif

sl_status_t sli_convert_rsi_ipv4_address_to_sl_ip_address(sl_ip_address_t *ip_address_buffer,
                                                          const sli_si91x_rsp_ipv4_params_t *ip_params);
sl_status_t sli_convert_si91x_dns_response(sl_ip_address_t *ip_address,
                                           const sli_si91x_dns_response_t *si91x_dns_response);
sl_status_t sli_convert_si91x_event_to_sl_net_event(const uint16_t *event,
                                                    sl_net_event_t *sl_net_event,
                                                    const sl_wifi_system_packet_t *packet);
#ifdef SLI_SI91X_INTERNAL_MDNS
sl_status_t sli_convert_si91x_mdns_response(sl_mdns_response_t *mdns_result, const sli_net_mdns_response_t *raw_result);
#endif
bool sli_wifi_is_ip_address_zero(const sl_ip_address_t *ip_addr);
#ifdef SLI_SI91X_INTERNAL_HTTP_CLIENT
void convert_itoa(uint32_t val, uint8_t *str);
sl_status_t sli_convert_si91x_event_to_sl_http_client_event(const uint16_t *event,
                                                            sl_http_client_event_t *sl_http_client_event);
#endif
sl_status_t sli_si91x_configure_ip_address(sl_net_ip_configuration_t *address,
                                           uint8_t virtual_ap_id,
                                           const uint32_t timeout);
