/*******************************************************************************
 * @file  sl_si91x_hspi_secondary_drv_config.h
 * @brief
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
#ifndef __SL_SI91X_HSPI_SECONDARY_DRV_CONFIG_H_
#define __SL_SI91X_HSPI_SECONDARY_DRV_CONFIG_H_

// <<< Use Configuration Wizard in Context Menu >>>
// <h>HSPI Configuration

// <e> DMA
// <i> Default: 1
#define SL_HSPI_DMA 1
// <o SL_HSPI_DMA_BLOCK_SIZE> DMA Block Size <256=>256 <512=>512 <1024=>1024
// <i> Default: 1024
// <i> This option is available only if DMA is enabled
#define SL_HSPI_DMA_BLOCK_SIZE 1024
// <o SL_HSPI_TX_BUFFER_SIZE> TX Size <0-4096>
// <i> Default: 1024
// <i> This option is available only if DMA is enabled
#define SL_HSPI_TX_BUFFER_SIZE 1024
// <o SL_HSPI_RX_BUFFER_SIZE> RX Size <0-4096>
// <i> Default: 1024
// <i> This option is available only if DMA is enabled
#define SL_HSPI_RX_BUFFER_SIZE 1024
// </e>

// </h>
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <hspi_secondary signal=MOSI,MISO,SCK,CS> SL_HSPI_SECONDARY
// $[HSPI_SECONDARY_SL_HSPI_SECONDARY]
#ifndef SL_HSPI_SECONDARY_PERIPHERAL
#define SL_HSPI_SECONDARY_PERIPHERAL HSPI_SECONDARY
#endif

// HSPI_SECONDARY MOSI on GPIO_27
#ifndef SL_HSPI_SECONDARY_MOSI_PORT
#define SL_HSPI_SECONDARY_MOSI_PORT HP
#endif
#ifndef SL_HSPI_SECONDARY_MOSI_PIN
#define SL_HSPI_SECONDARY_MOSI_PIN 27
#endif
#ifndef SL_HSPI_SECONDARY_MOSI_LOC
#define SL_HSPI_SECONDARY_MOSI_LOC 2
#endif

// HSPI_SECONDARY MISO on GPIO_28
#ifndef SL_HSPI_SECONDARY_MISO_PORT
#define SL_HSPI_SECONDARY_MISO_PORT HP
#endif
#ifndef SL_HSPI_SECONDARY_MISO_PIN
#define SL_HSPI_SECONDARY_MISO_PIN 28
#endif
#ifndef SL_HSPI_SECONDARY_MISO_LOC
#define SL_HSPI_SECONDARY_MISO_LOC 3
#endif

// HSPI_SECONDARY SCK on GPIO_25
#ifndef SL_HSPI_SECONDARY_SCK_PORT
#define SL_HSPI_SECONDARY_SCK_PORT HP
#endif
#ifndef SL_HSPI_SECONDARY_SCK_PIN
#define SL_HSPI_SECONDARY_SCK_PIN 25
#endif
#ifndef SL_HSPI_SECONDARY_SCK_LOC
#define SL_HSPI_SECONDARY_SCK_LOC 0
#endif

// HSPI_SECONDARY CS on GPIO_26
#ifndef SL_HSPI_SECONDARY_CS_PORT
#define SL_HSPI_SECONDARY_CS_PORT HP
#endif
#ifndef SL_HSPI_SECONDARY_CS_PIN
#define SL_HSPI_SECONDARY_CS_PIN 26
#endif
#ifndef SL_HSPI_SECONDARY_CS_LOC
#define SL_HSPI_SECONDARY_CS_LOC 1
#endif
// [HSPI_SECONDARY_SL_HSPI_SECONDARY]$
// <<< sl:end pin_tool >>>

#endif //__SL_SI91X_HSPI_SECONDARY_DRV_CONFIG_H_