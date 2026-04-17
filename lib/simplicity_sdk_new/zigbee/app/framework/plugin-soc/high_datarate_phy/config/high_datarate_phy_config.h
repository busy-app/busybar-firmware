/***************************************************************************//**
 * @brief Zigbee Direct ZDD component configuration header.
 *
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

// <h>HDR PHY configuration

#ifndef SILABS_HIGH_DATARATE_PHY_CONFIG_H
#define SILABS_HIGH_DATARATE_PHY_CONFIG_H

// <o SL_HDR_PHY_MAX_PACKET_SIZE> SL_HDR_PHY_MAX_PACKET_SIZE <1-2045>
// <i> Default: 253
// <i> Max supported HDR PHY payload is 253 for series 1-2, or 2045 for series 3.
// <i> (255 or 2047 bytes) + 2-byte PHR - 4-byte CRC
#define SL_HDR_PHY_MAX_PACKET_SIZE 253
// </h>

#endif //SILABS_HIGH_DATARATE_PHY_CONFIG_H

// <<< end of configuration section >>>
