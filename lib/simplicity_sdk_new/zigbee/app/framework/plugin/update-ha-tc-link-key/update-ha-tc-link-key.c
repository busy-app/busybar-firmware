/***************************************************************************//**
 * @file
 * @brief Routines for the Update HA TC Link Key plugin, which provides a way
 *        for trust centers to preemptively update the link key for HA devices.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/include/af.h"

#define R21_STACK_REVISION 21

void sl_zigbee_af_device_database_discovery_complete_cb(const sl_zigbee_af_device_info_t *info)
{
  if (info->stackRevision < R21_STACK_REVISION) {
    sl_802154_short_addr_t nodeId;
    sl_status_t status = sl_zigbee_lookup_node_id_by_eui64(((sl_zigbee_af_device_info_t *)info)->eui64, &nodeId);
    if ( status == SL_STATUS_OK) {
      status = sl_zigbee_send_trust_center_link_key(nodeId, ((sl_zigbee_af_device_info_t *)info)->eui64);
    }
    sl_zigbee_af_core_print("Updating trust center link key of joining device ");
    sl_zigbee_af_print_big_endian_eui64(info->eui64);
    sl_zigbee_af_core_println(": 0x%02X", status);
  }
}
