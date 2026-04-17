/***************************************************************************//**
 * @brief Connect's radio scheduler priority configuration
 *
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef __SL_CONNECT_RADIO_PRIORITY_CONFIG_H__
#define __SL_CONNECT_RADIO_PRIORITY_CONFIG_H__

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Connect's radio scheduler priority configuration
// <o SL_CONNECT_RADIO_TX_STARTING_PRIORITY> Starting TX priority <1-255>
// <i> Default: 100
// <i> With a multitprotocol rail library, this is Connect's active TX priority for the first try of a TX message
#define SL_CONNECT_RADIO_TX_STARTING_PRIORITY 100

// <o SL_CONNECT_RADIO_TX_STEP_PRIORITY> TX priority step <1-10>
// <i> Default: 2
// <i> With a multitprotocol rail library, this is the value by which TX priority for Connect decrements for each retry
#define SL_CONNECT_RADIO_TX_STEP_PRIORITY 2

// <o SL_CONNECT_RADIO_RX_PRIORITY> Active Rx priority  <1-255>
// <i> Default: 255
// <i> With a multitprotocol rail library, this is Connect's rx priority
#define SL_CONNECT_RADIO_RX_PRIORITY 255

// </h>

// <<< end of configuration section >>>
#endif //__SL_CONNECT_RADIO_PRIORITY_CONFIG_H__
