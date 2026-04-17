/***************************************************************************//**
 * @brief Zigbee BDB 3.1 Rejoin Algorithm component configuration header.
 *\n*******************************************************************************
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

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Zigbee BDB 3.1 Rejoin Algorithm configuration

// <o SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_ALGORITHM_BACKOFF_SEC> Rejoin backoff in seconds
// <i> Default: 30
// <i> The amount of time in seconds between sets of rejoins. When connectivity to the Trust Center is lost, the rejoin algorithm will attempt a TC rejoin on the current channel, followed by a rejoin on primary channels, and ending with a rejoin on secondary channels. If all of these rejoins fail, the process is repeated. This configuration backoff determines how long to wait before the next batch of rejoins is issued.
#define SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_ALGORITHM_BACKOFF_SEC     30

// </h>

// <<< end of configuration section >>>
