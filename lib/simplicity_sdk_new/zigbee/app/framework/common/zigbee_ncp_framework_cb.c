/***************************************************************************//**
 * @brief Zigbee NCP Application Framework Callbacks.
 * This file provides the WEAK definitions of the NCP framework callbacks.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "stack/include/sl_zigbee_types.h"
#include "stack/include/gp-types.h"
#include "app/util/ezsp/ezsp-enum.h"

/** @brief Called when sl_zigbee_internal_trust_center_join_handler is called from the stack.
 *
 * The NCP will perform initial processing based on the current
 * ::SL_ZIGBEE_EZSP_TRUST_CENTER_POLICY value and will pass this callback to the
 * NCP application. Implementations of this callback should return the
 * ::sl_zigbee_join_decision_t value that will dictate the behavior of the trust center
 * in response to the new device joining or rejoining to the network.
 *
 * See documentation on the stack sl_zigbee_internal_trust_center_join_handler callback
 * for more information.
 *
 * @param newNodeId The node ID of the device wishing to join.
 * @param newNodeEui64 The EUI64 of the device wishing to join.
 * @param status The ::EmberUpdateDevice status indicating whether the
 * device is joining/rejoining or leaving, and whether the join/rejoin is
 * secure (using network encryption) or a trust center rejoin.
 * @param parentOfNewNode The node ID of the parent of device wishing to join.
 * @param decisionId The ::sl_zigbee_ezsp_decision_id_t that the NCP is currently using
 * as set by the HOST application.
 * @param joinDecision The current ::sl_zigbee_join_decision_t that the NCP has chosen
 * based on the ::sl_zigbee_ezsp_decision_id_t for the ::SL_ZIGBEE_EZSP_TRUST_CENTER_POLICY as set by the
 * HOST application. Implementations of this callback may want to return a
 * different value based on their desired security.
 *
 * @return An ::sl_zigbee_join_decision_t value that will dictate the behavior of the
 * trust center in response to the new device joining or rejoining to
 * the network.
 */
WEAK(sl_zigbee_join_decision_t sl_zigbee_xncp_security_trust_center_join_cb(sl_802154_short_addr_t newNodeId,
                                                                            const sl_802154_long_addr_t newNodeEui64,
                                                                            sl_zigbee_device_update_t status,
                                                                            sl_802154_short_addr_t parentOfNewNode,
                                                                            sl_zigbee_ezsp_decision_id_t decisionId,
                                                                            sl_zigbee_join_decision_t joinDecision))
{
  (void)newNodeId;
  (void)newNodeEui64;
  (void)status;
  (void)parentOfNewNode;
  (void)decisionId;
  return joinDecision;
}

/** @brief NCP Application hook callback for green power messages.
 *
 * This function is called by the application hook plugin to allow
 * user to take an action gpd message is received. The return code
 * true indicate the user handled the message and false allows the
 * stack to handle it.
 *
 * @param *param                     Ver.: always
 */
WEAK(bool sl_zigbee_af_ncp_gpep_incoming_message_handler(sl_zigbee_gp_params_t *param))
{
  (void)param;
  // return false so that stack will handle it, return true
  // if the message has been handled by user.
  return false;
}

/** @brief NCP Application hook callback for green power messages.
 *
 * This function is called by the application hook plugin to allow
 * user to take an action gpd message is sent. The return code true
 * indicate the user handled the message and false allows the stack
 * to handle it.
 *
 * @param status                     Ver.: always
 * @param gpepHandle                 Ver.: always
 */
WEAK(bool sl_zigbee_af_ncp_d_gp_sent_handler(sl_status_t status, uint8_t gpepHandle))
{
  (void)status;
  (void)gpepHandle;
  // return false so that stack will handle it, return true
  // if the message has been handled by user.
  return false;
}
