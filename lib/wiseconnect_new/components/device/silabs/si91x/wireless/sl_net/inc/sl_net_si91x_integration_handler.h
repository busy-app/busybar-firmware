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
#include "sli_wifi_types.h"
#include "sl_si91x_types.h"

#define SL_NET_EVENT_DISPATCH_HANDLER(data, packet) sl_net_si91x_event_dispatch_handler(data, packet)

void sl_net_si91x_event_dispatch_handler(sli_si91x_queue_packet_t *data, sl_wifi_system_packet_t *packet);

#define SLI_NETWORK_CLEANUP_HANDLER() sli_si91x_network_cleanup_handler()

void sli_si91x_network_cleanup_handler();

#if defined(SL_WIFI_COMPONENT_INCLUDED)
/*
This function flushes all transmit (TX) command and data queues for sockets that match a given destination IP address.
It iterates through all available sockets, checks if a socket's remote address matches the specified IP address,
and then flushes the command and data queues for that socket based on the provided frame_status*/
sl_status_t sli_si91x_flush_all_socket_tx_queues_based_on_dest_ip_address(uint16_t frame_status,
                                                                          const sl_ip_address_t *dest_ip_add,
                                                                          uint8_t vap_id);

void sli_si91x_flush_third_party_station_dependent_sockets(const sli_si91x_ap_disconnect_resp_t *ap_disconnect_resp);
#endif
