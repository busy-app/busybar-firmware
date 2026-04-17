/***************************************************************************//**
 * @file
 * @brief SOC specific routines for performing service discovery.
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
#include "app/util/zigbee-framework/zigbee-device-library.h"

//------------------------------------------------------------------------------

sl_status_t sli_zigbee_af_send_match_descriptor(sl_802154_short_addr_t target,
                                                sl_zigbee_af_profile_id_t profileId,
                                                sl_zigbee_af_cluster_id_t clusterId,
                                                bool serverCluster)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (serverCluster) {
    status = sl_zigbee_match_descriptors_request(target,
                                                 profileId,
                                                 1,
                                                 (uint16_t*)&clusterId,
                                                 0,
                                                 NULL,
                                                 SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
  } else {
    status = sl_zigbee_match_descriptors_request(target,
                                                 profileId,
                                                 0,
                                                 NULL,
                                                 1,
                                                 (uint16_t*)&clusterId,
                                                 SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS);
  }
  return status;
}
