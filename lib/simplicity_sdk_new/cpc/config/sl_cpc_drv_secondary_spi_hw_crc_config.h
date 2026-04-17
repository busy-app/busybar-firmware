/***************************************************************************//**
 * @file
 * @brief CPC DRV Secondary SPI CRC Hardware Acceleration configuration file.
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

#ifndef SL_CPC_CDRV_SECONDARY_SPI_HW_CRC_CONFIG_H
#define SL_CPC_CDRV_SECONDARY_SPI_HW_CRC_CONFIG_H

// <h>SYNC Trig bit Configuration

// <o SL_CPC_DRV_SPI_GPCRC_SYNC_BIT_CH> Launch SYNC Trig bit <0-7>
// <i> Default: 4
// <i> When using the GPCRC, an additional DMA channel is used to move the data through the GPCRC.
// <i> In order to synchronize this DMA channel with the main RX channel, an additional SYNC trig bit is used.
// <i> Specify which SYNCTRIG bit is used. Modify this value to avoid collisions if specific LDMA SYNCTRIG bits need to be used elsewhere in the project.
#define SL_CPC_DRV_SPI_GPCRC_SYNC_BIT_CH   (4)

// </h>

// <<< end of configuration section >>>

#endif /* SL_CPC_CDRV_SECONDARY_SPI_HW_CRC_CONFIG_H */
