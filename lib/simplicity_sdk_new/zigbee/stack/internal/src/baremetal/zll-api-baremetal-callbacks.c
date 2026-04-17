/***************************************************************************//**
 * @file zll-api-baremetal-callbacks.c
 * @brief internal dispatch for 'zll-api' callbacks as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
// automatically generated from zll-api.h.  Do not manually edit
#include "stack/include/zll-api.h"
#include "stack/internal/inc/zll-api-internal-def.h"

void sli_zigbee_stack_zll_address_assignment_handler(sl_zigbee_zll_address_assignment_t *addressInfo,
                                                     sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_zll_address_assignment_handler(addressInfo,
                                           packetInfo);
}

void sli_zigbee_stack_zll_network_found_handler(sl_zigbee_zll_network_t *networkInfo,
                                                bool isDeviceInfoNull,
                                                sl_zigbee_zll_device_info_record_t *deviceInfo,
                                                sl_zigbee_rx_packet_info_t *packetInfo)
{
  sl_zigbee_zll_network_found_handler(networkInfo,
                                      isDeviceInfoNull,
                                      deviceInfo,
                                      packetInfo);
}

void sli_zigbee_stack_zll_scan_complete_handler(sl_status_t status)
{
  sl_zigbee_zll_scan_complete_handler(status);
}

void sli_zigbee_stack_zll_touch_link_target_handler(const sl_zigbee_zll_network_t *networkInfo)
{
  sl_zigbee_zll_touch_link_target_handler(networkInfo);
}
