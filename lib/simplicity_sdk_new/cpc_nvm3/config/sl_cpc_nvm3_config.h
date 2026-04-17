/***************************************************************************//**
 * @file
 * @brief CPC NVM3 configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_CPC_NVM3_CONFIG_H
#define SL_CPC_NVM3_CONFIG_H

// <h>CPC Configuration

// <o SL_CPC_NVM3_TX_PAYLOAD_MAX_LENGTH>Max Transmit Payload Length<64-4087>
// <i> Default: 256
// <i> Maximum size of the payload in bytes of each RX buffer
#define SL_CPC_NVM3_TX_PAYLOAD_MAX_LENGTH      (256)

// <o SL_CPC_NVM3_MAXIMUM_WRITE_SIZE>Maximum size for a write operation to any
//  NVM3 instance
// <i> Default: 512
#define SL_CPC_NVM3_MAXIMUM_WRITE_SIZE         (512)

// <o SL_CPC_NVM3_WRITE_LOCK_TIMEOUT_MS>Maximum allowed time to complete a write operation
// <i> Default: 1000
#define SL_CPC_NVM3_WRITE_LOCK_TIMEOUT_MS         (1000)
// </h>

// <<< end of configuration section >>>

#endif /* SL_CPC_NVM3_CONFIG_H */
