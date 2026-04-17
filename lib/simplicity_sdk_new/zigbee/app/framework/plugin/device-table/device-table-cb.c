/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

/** @brief StateChange
 *
 * This callback is called when a device's state changes.
 *
 * @param nodeId   Ver.: always
 * @param state   Ver.: always
 */
WEAK(void sl_zigbee_af_device_table_state_change_cb(sl_802154_short_addr_t nodeId,
                                                    uint8_t state))
{
}

/** @brief Cleared
 *
 * This callback is called when a device table has been cleared.
 *
 */
WEAK(void sl_zigbee_af_device_table_cleared_cb(void))
{
}

/** @brief NewDevice
 *
 * This callback is called when a new device joins the gateway.
 *
 * @param uui64   Ver.: always
 */
WEAK(void sl_zigbee_af_device_table_new_device_cb(sl_802154_long_addr_t eui64))
{
}

/** @brief Rejoin device
 *
 * Called when a device rejoins.
 *
 *@param newNodeEui64:  EUI64 of the rejoined device.
 */
WEAK(void sl_zigbee_af_device_table_rejoin_device_cb(sl_802154_long_addr_t newNodeEui64))
{
}

/*
 *
 * Called when a device leaves.
 *
 *@param newNodeEui64:  EUI64 of the device that left.
 */
WEAK(void sl_zigbee_af_device_table_device_left_cb(sl_802154_long_addr_t newNodeEui64))
{
}

/*
 *
 * Called when the device table has been initialized.
 *
 */
WEAK(void sl_zigbee_af_device_table_initialized(void))
{
}

/*
 *
 * Called when the device has been removed from the table.
 *
 *@param currentIndex:  Index of the removed device.
 *
 */
WEAK(void sl_zigbee_af_device_table_index_removed_cb(uint16_t currentIndex))
{
}

/*
 *
 * Called when the device table has been initialized.
 *
 *@param index:  Index of the removed device.
 *
 */
WEAK(void sl_zigbee_af_device_table_index_added_cb(uint16_t index))
{
}
