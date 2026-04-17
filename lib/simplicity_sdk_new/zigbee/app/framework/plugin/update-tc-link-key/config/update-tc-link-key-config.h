/***************************************************************************//**
 * @brief Zigbee Update TC Link Key component configuration header.
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

// <h>Zigbee Update TC Link Key configuration

// <o SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_MAX_ATTEMPTS> Max Attempts <1-255>
// <i> Default: 3
// <i> The maximum number of attempts the node should make each TCLK Update Iteration when sending or retrying the Node Descriptor, Request Key, or Verify Key messages. This field corresponds to the bdbTcLinkKeyExchangeAttemptsMax attribute in the Base Device Behavior specification.
#define SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_MAX_ATTEMPTS   3

// <o SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_MAX_ITERATIONS> Max Attempts <1-255>
// <i> Default: 1
// <i> The maximum number of iterations of the TCLK update permitted before a failure is declared using the On-Network TCLK Update procedure. This field corresponds to the bdbcfTCLKUpdateMaxIterations attribute in the Base Device Behavior specification.
#define SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_MAX_ITERATIONS   1

// <o SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_TIMER_SEC> TCLK update timer <0-4294967>
// <i> Default: 86400
// <i> The time to wait (in seconds) after one TCLK update operation completes before issuing a new TCLK update operation. 0 means no new TCLK update operation will be issued.
#define SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_TIMER_SEC   86400

// </h>

// <<< end of configuration section >>>
