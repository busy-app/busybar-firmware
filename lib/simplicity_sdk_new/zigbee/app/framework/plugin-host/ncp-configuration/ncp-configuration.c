/***************************************************************************//**
 * @file
 * @brief A file for configuring NCP parameters.
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
#include "app/framework/util/af-main.h"

void sl_zigbee_af_ncp_configuration_ncp_init_cb(void)
{
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_BINDING_TABLE_SIZE,
                                     SL_ZIGBEE_BINDING_TABLE_SIZE,
                                     "binding table size");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_MAX_END_DEVICE_CHILDREN,
                                     SL_ZIGBEE_MAX_END_DEVICE_CHILDREN,
                                     "max end device children");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_APS_UNICAST_MESSAGE_COUNT,
                                     SL_ZIGBEE_APS_UNICAST_MESSAGE_COUNT,
                                     "aps unicast message count");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_BROADCAST_TABLE_SIZE,
                                     SL_ZIGBEE_BROADCAST_TABLE_SIZE,
                                     "broadcast table size");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_NEIGHBOR_TABLE_SIZE,
                                     SL_ZIGBEE_NEIGHBOR_TABLE_SIZE,
                                     "neighbor table size");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_END_DEVICE_POLL_TIMEOUT,
                                     SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT,
                                     "end device poll timeout");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_ZLL_GROUP_ADDRESSES,
                                     SL_ZIGBEE_ZLL_GROUP_ADDRESSES,
                                     "zll group addresses");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_ZLL_RSSI_THRESHOLD,
                                     SL_ZIGBEE_ZLL_RSSI_THRESHOLD,
                                     "zll rssi threshold");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_TRANSIENT_KEY_TIMEOUT_S,
                                     SL_ZIGBEE_TRANSIENT_KEY_TIMEOUT_S,
                                     "transient key timeout");
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_RETRY_QUEUE_SIZE,
                                     SL_ZIGBEE_RETRY_QUEUE_SIZE,
                                     "retry size");
}
