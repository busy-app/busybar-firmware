/***************************************************************************//**
 * @file
 * @brief Zigbee Dynamic Node Type API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_ZIGBEE_DYNAMIC_NODE_TYPE_H
#define SL_ZIGBEE_DYNAMIC_NODE_TYPE_H

#include "sl_status.h"

/**
 * @brief Switch the Zigbee node type to Router.
 *
 * This function attempts to switch the device from a Sleepy End Device to a Router.
 * The device must be currently operating as a Sleepy End Device for this operation
 * to succeed.
 *
 * @note Network rejoin is performed securely using the existing network key.
 *
 * @return SL_STATUS_OK if the role switch was initiated successfully.
 * @return SL_STATUS_FAIL if the current node type is not a Sleepy End Device or
 *         if the network parameters could not be retrieved.
 */
sl_status_t sl_zigbee_af_switch_role_router(void);

/**
 * @brief Switch the Zigbee node type to Sleepy End Device.
 *
 * This function attempts to switch the device from a Router to a Sleepy End Device.
 * The device must be currently operating as a Router for this operation to succeed.
 *
 * @note Network rejoin is performed securely using the existing network key.
 *
 * @param[in] end_device_configuration Configuration flags for end device behavior.
 *            0=None (default), 1=Persist data on parent
 *
 * @return SL_STATUS_OK if the role switch was initiated successfully.
 * @return SL_STATUS_FAIL if the current node type is not a Router or
 *         if the network parameters could not be retrieved.
 */
sl_status_t sl_zigbee_af_switch_role_sleepy_end_device(uint8_t end_device_configuration);

#endif
