/***************************************************************************//**
 * @file
 * @brief Host specific routines for performing service discovery.
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
#include "app/util/zigbee-framework/zigbee-device-host.h"

//------------------------------------------------------------------------------

sl_status_t sli_zigbee_af_send_match_descriptor(sl_802154_short_addr_t target,
                                                sl_zigbee_af_profile_id_t profileId,
                                                sl_zigbee_af_cluster_id_t clusterId,
                                                bool serverCluster)
{
  uint8_t inCount = 0;
  uint8_t outCount = 0;
  sl_zigbee_af_cluster_id_t inCluster, outCluster;

  if (serverCluster) {
    inCount = 1;
    inCluster = clusterId;
    outCluster = 0x0000;
  } else {
    outCount = 1;
    outCluster = clusterId;
    inCluster = 0x0000;
  }

  return sl_zigbee_ezsp_match_descriptors_request(target,
                                                  profileId,
                                                  inCount,
                                                  outCount,
                                                  &inCluster,
                                                  &outCluster,
                                                  SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
}
