/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Network Steering plugin.
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
#include "network-steering.h"

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to SL_STATUS_OK to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
WEAK(void sl_zigbee_af_network_steering_complete_cb(sl_status_t status,
                                                    uint8_t totalBeacons,
                                                    uint8_t joinAttempts,
                                                    uint8_t finalState))
{
  sl_zigbee_af_core_println("Network Steering Completed: %s (0x%02X)",
                            (status == SL_STATUS_OK ? "Join Success" : "FAILED"),
                            status);
  sl_zigbee_af_core_println("Finishing state: 0x%02X", finalState);
  sl_zigbee_af_core_println("Beacons heard: %d\nJoin Attempts: %d", totalBeacons, joinAttempts);
}

/** @brief Get Power For Radio Channel
 *
 * This callback is fired when the Network Steering plugin needs to set the
 * power level. The application has the ability to change the max power level
 * used for this particular channel.
 *
 * @param channel The channel that the plugin is inquiring about the power
 * level. Ver.: always
 */
WEAK(int8_t sl_zigbee_af_network_steering_get_power_for_radio_channel_cb(uint8_t channel))
{
  (void)channel;
  return sl_zigbee_af_max_power_level();
}

/** @brief Get Distributed Key
 *
 * This callback is fired when the Network Steering plugin needs to set the distributed
 * key. The application set the distributed key from Zigbee Alliance thru this callback
 * or the network steering will use the default test key.
 *
 * @param pointer to the distributed key struct
 * @return true if the key is loaded successfully, otherwise false.
 * level. Ver.: always
 */
WEAK(bool sl_zigbee_af_network_steering_get_distributed_key_cb(sl_zigbee_key_data_t * key))
{
  (void)key;
  return false;
}

/** @brief Get Node Type
 *
 * This callback allows the application to set the node type that the network
 * steering process will use in joining a network.
 *
 * @param state The current ::sl_zigbee_af_plugin_network_steering_joining_state_t.
 *
 * @return An ::sl_zigbee_node_type_t value that the network steering process will
 * try to join a network as.
 */
WEAK(sl_zigbee_node_type_t sl_zigbee_af_network_steering_get_node_type_cb(sl_zigbee_af_plugin_network_steering_joining_state_t state))
{
  (void)state;
  return ((sli_zigbee_af_current_zigbee_pro_network->nodeType == SL_ZIGBEE_COORDINATOR)
          ? SL_ZIGBEE_ROUTER
          : sli_zigbee_af_current_zigbee_pro_network->nodeType);
}
