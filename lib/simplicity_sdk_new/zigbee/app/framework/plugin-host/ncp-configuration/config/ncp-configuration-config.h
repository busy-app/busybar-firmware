/***************************************************************************//**
 * @brief Zigbee NCP Configuration component configuration header.
 *\n*******************************************************************************
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

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Zigbee NCP Configuration configuration

// <o SL_ZIGBEE_BINDING_TABLE_SIZE> Binding Table Size <0-127>
// <i> Default: 2
// <i> The number of entries that the binding table can hold.
#define SL_ZIGBEE_BINDING_TABLE_SIZE   2

// <o SL_ZIGBEE_MAX_END_DEVICE_CHILDREN> Child Table Size <6-64>
// <i> Default: 6
// <i> The maximum number of ZigBee PRO End Devices that can be supported by a single device.
#define SL_ZIGBEE_MAX_END_DEVICE_CHILDREN   6

// <o SL_ZIGBEE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE> End Device keep alive support mode
// <i> End Device keep alive support mode
// <SL_802154_DATA_POLL_KEEP_ALIVE=> SL_802154_DATA_POLL_KEEP_ALIVE
// <SL_ZIGBEE_END_DEVICE_TIMEOUT_KEEP_ALIVE=> SL_ZIGBEE_END_DEVICE_TIMEOUT_KEEP_ALIVE
// <SL_ZIGBEE_KEEP_ALIVE_SUPPORT_ALL=> SL_ZIGBEE_KEEP_ALIVE_SUPPORT_ALL
// <i> Default: SL_ZIGBEE_KEEP_ALIVE_SUPPORT_ALL
// <i> End Device keep alive support mode on the router could be set here.
#define SL_ZIGBEE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE   SL_ZIGBEE_KEEP_ALIVE_SUPPORT_ALL

// <o SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT> End Device Poll Timeout Value
// <i> End Device Poll Timeout Value
// <SECONDS_10=> SECONDS_10
// <MINUTES_2=> MINUTES_2
// <MINUTES_4=> MINUTES_4
// <MINUTES_8=> MINUTES_8
// <MINUTES_16=> MINUTES_16
// <MINUTES_32=> MINUTES_32
// <MINUTES_64=> MINUTES_64
// <MINUTES_128=> MINUTES_128
// <MINUTES_256=> MINUTES_256
// <MINUTES_512=> MINUTES_512
// <MINUTES_1024=> MINUTES_1024
// <MINUTES_2048=> MINUTES_2048
// <MINUTES_4096=> MINUTES_4096
// <MINUTES_8192=> MINUTES_8192
// <MINUTES_16384=> MINUTES_16384
// <i> Default: MINUTES_256
// <i> The amount of time that must pass without hearing a MAC data poll from the device before the end device is removed from the child table.  For a router device this applies to its children.  For an end device, this is the amount of time before it automatically times itself out.
#define SL_ZIGBEE_END_DEVICE_POLL_TIMEOUT   MINUTES_256

// <o SL_ZIGBEE_ZLL_GROUP_ADDRESSES> ZLL group addresses <0-255>
// <i> Default: 0
// <i> The number of unique group identifiers that this device requires.
#define SL_ZIGBEE_ZLL_GROUP_ADDRESSES   0

// <o SL_ZIGBEE_ZLL_RSSI_THRESHOLD> ZLL RSSI threshold <-128..127>
// <i> Default: -40
// <i> The RSSI threshold applied to incoming scan requests.  The stack will ignore scan requests with corrected RSSI values less than this threshold.
#define SL_ZIGBEE_ZLL_RSSI_THRESHOLD   -40

// <o SL_ZIGBEE_TRANSIENT_KEY_TIMEOUT_S> Transient key timeout (in seconds) <0-65535>
// <i> Default: 300
// <i> The amount of time a device will store a transient link key that can be used to join a network.
#define SL_ZIGBEE_TRANSIENT_KEY_TIMEOUT_S   300

// <o SL_ZIGBEE_APS_UNICAST_MESSAGE_COUNT> APS Unicast Message Queue Size <1-255>
// <i> Default: 10
// <i> The maximum number of APS unicast messages that can be queued up by the stack.  A message is considered queued when sl_zigbee_send_unicast() is called and is de-queued when the sl_zigbee_message_sent_handler() is called.
#define SL_ZIGBEE_APS_UNICAST_MESSAGE_COUNT   10

// <o SL_ZIGBEE_APS_DUPLICATE_REJECTION_MAX_ENTRIES> APS unicast Message Duplicate Rejection table Size <1-255>
// <i> Default: 5
// <i> The maximum number of APS unicast messages that can be stored in the stack, to reject duplicate processing/forwarding of APS messages.
// <i> Size of 1 is basically the same thing as no duplicate rejection
#define SL_ZIGBEE_APS_DUPLICATE_REJECTION_MAX_ENTRIES 5

// <o SL_ZIGBEE_BROADCAST_TABLE_SIZE> Broadcast Table Size <15-254>
// <i> Default: 15
// <i> The size of the parametrized broadcast table.
#define SL_ZIGBEE_BROADCAST_TABLE_SIZE   15

// <o SL_ZIGBEE_NEIGHBOR_TABLE_SIZE> Neighbor Table Size
// <i> Neighbor Table Size
// <1=> 1
// <16=> 16
// <26=> 26
// <i> Default: 16
// <i> The size of the neighbor table.
#define SL_ZIGBEE_NEIGHBOR_TABLE_SIZE   16

// </h>

// <<< end of configuration section >>>
