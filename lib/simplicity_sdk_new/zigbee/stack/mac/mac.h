/***************************************************************************//**
 * @file
 * @brief
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

#ifndef SILABS_ZIGBEE_MAC_H
#define SILABS_ZIGBEE_MAC_H

// The PHY payload is 127 bytes, including the two-byte CRC (which is
// technically the MAC's responsibility) and not including the initial
// length byte (which is PHY overhead).  The transmit buffer contains
// the length byte but not the CRC, which is appended by the hardware
// during transmission.
//    127 (PHY payload)
//    + 1 (length byte)
//    - 2 (no CRC)
//  = 126
#define TRANSMIT_BUFFER_SIZE 126
#define MAC_CRC_LENGTH 2

// The maximum number of times the mac will attempt retransmission of an
// unacknowledged packet.
#define MAC_MAX_ACKD_RETRIES_DEFAULT  (3)

#endif // SILABS_ZIGBEE_MAC_H
