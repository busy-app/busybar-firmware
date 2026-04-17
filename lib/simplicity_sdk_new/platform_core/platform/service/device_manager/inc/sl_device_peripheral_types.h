/**************************************************************************//**
 * @file
 * @brief Device Manager API Definition
 ******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
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
 *****************************************************************************/
#ifndef SL_DEVICE_PERIPHERAL_TYPES_H
#define SL_DEVICE_PERIPHERAL_TYPES_H

#include "sl_device_clock.h"
#include "sl_device_dma.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup device_peripheral Device Abstraction Peripheral
 *
 * @{
 ******************************************************************************/

// ----------------------------------------------------------------------------
// TYPEDEFS

/// Define peripheral structure.
typedef struct sl_peripheral {
  uint32_t base;                 ///< Peripheral base address.
  sl_clock_branch_t clk_branch;  ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;      ///< Peripheral bus clock.
} sl_peripheral_val_t;

/// Define peripheral typedef.
typedef const sl_peripheral_val_t* sl_peripheral_t;

/// Define peripheral DMA structure.
typedef struct sl_peripheral_dma {
  uint32_t base;                         ///< Peripheral base address.
  sl_clock_branch_t clk_branch;          ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;              ///< Peripheral bus clock.
  uint32_t dual_destination_map;         ///< A bitmap of the channels supporting dual destination
  uint32_t rule_based_interleaving_map;  ///< A bitmap of the channels supporting rule based interleaving
  uint8_t nbr_channel;                   ///< Number of channels
  uint8_t nbr_sync;                      ///< Number of sync bits
} sl_peripheral_dma_val_t;

/// Define peripheral DMA typedef.
typedef const sl_peripheral_dma_val_t* sl_peripheral_dma_t;

/// Define peripheral I2C structure.
typedef struct sl_peripheral_i2c {
  uint32_t base;                         ///< Peripheral base address.
  sl_clock_branch_t clk_branch;          ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;              ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_rxdatav;    ///< RX data available DMA channel
  sl_dma_signal_t dma_signal_txbl;       ///< TX buffer level DMA channel
} sl_peripheral_i2c_val_t;

/// Define peripheral I2C typedef.
typedef const sl_peripheral_i2c_val_t* sl_peripheral_i2c_t;

/// Define peripheral TIMER structure.
typedef struct sl_peripheral_timer {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_cc0;       ///< CC0 DMA channel
  sl_dma_signal_t dma_signal_cc1;       ///< CC1 DMA channel
  sl_dma_signal_t dma_signal_cc2;       ///< CC2 DMA channel
  sl_dma_signal_t dma_signal_ufof;      ///< Underflow/Overflow DMA channel
  sl_dma_signal_t dma_signal_cc3;       ///< CC3 DMA channel
  sl_dma_signal_t dma_signal_cc4;       ///< CC4 DMA channel
  sl_dma_signal_t dma_signal_cc5;       ///< CC5 DMA channel
  sl_dma_signal_t dma_signal_cc6;       ///< CC6 DMA channel
} sl_peripheral_timer_val_t;

/// Define peripheral TIMER typedef.
typedef const sl_peripheral_timer_val_t* sl_peripheral_timer_t;

/// Define peripheral PROTIMER structure.
typedef struct sl_peripheral_protimer {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_cc0;       ///< CC0 DMA channel
  sl_dma_signal_t dma_signal_cc1;       ///< CC1 DMA channel
  sl_dma_signal_t dma_signal_cc2;       ///< CC2 DMA channel
  sl_dma_signal_t dma_signal_cc3;       ///< CC3 DMA channel
  sl_dma_signal_t dma_signal_bof;       ///< BOF DMA channel
  sl_dma_signal_t dma_signal_pof;       ///< POF DMA channel
  sl_dma_signal_t dma_signal_wof;       ///< WOF DMA channel
} sl_peripheral_protimer_val_t;

/// Define peripheral PROTIMER typedef.
typedef const sl_peripheral_protimer_val_t* sl_peripheral_protimer_t;

/// Define peripheral IADC structure.
typedef struct sl_peripheral_iadc {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_scan;      ///< Scan DMA channel
  sl_dma_signal_t dma_signal_single;    ///< Single DMA channel
} sl_peripheral_iadc_val_t;

/// Define peripheral IADC typedef.
typedef const sl_peripheral_iadc_val_t* sl_peripheral_iadc_t;

/// Define peripheral ADC structure.
typedef struct sl_peripheral_adc {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_scan;      ///< Scan DMA channel
} sl_peripheral_adc_val_t;

/// Define peripheral ADC typedef.
typedef const sl_peripheral_adc_val_t* sl_peripheral_adc_t;

/// Define peripheral EUSART structure.
typedef struct sl_peripheral_eusart {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_rxfl;      ///< RX DMA channel
  sl_dma_signal_t dma_signal_txfl;      ///< TX DMA channel
} sl_peripheral_eusart_val_t;

/// Define peripheral EUSART typedef.
typedef const sl_peripheral_eusart_val_t* sl_peripheral_eusart_t;

/// Define peripheral EUART structure.
typedef struct sl_peripheral_euart {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_rxfl;      ///< RX DMA channel
  sl_dma_signal_t dma_signal_txfl;      ///< TX DMA channel
} sl_peripheral_euart_val_t;

/// Define peripheral EUART typedef.
typedef const sl_peripheral_euart_val_t* sl_peripheral_euart_t;

/// Define peripheral USART structure.
typedef struct sl_peripheral_usart {
  uint32_t base;                           ///< Peripheral base address.
  sl_clock_branch_t clk_branch;            ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;                ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_rxdatav;      ///< RXDATAV DMA channel
  sl_dma_signal_t dma_signal_rxdatavright; ///< RXDATAVRIGHT DMA channel
  sl_dma_signal_t dma_signal_txbl;         ///< TXBL DMA channel
  sl_dma_signal_t dma_signal_txblright;    ///< TXBLRIGHT DMA channel
  sl_dma_signal_t dma_signal_txempty;      ///< TXEMPTY DMA channel
} sl_peripheral_usart_val_t;

/// Define peripheral USART typedef.
typedef const sl_peripheral_usart_val_t* sl_peripheral_usart_t;

/// Define peripheral MSC structure.
typedef struct sl_peripheral_msc {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_wdata;     ///< Write data DMA channel
} sl_peripheral_msc_val_t;

/// Define peripheral MSC typedef.
typedef const sl_peripheral_msc_val_t* sl_peripheral_msc_t;

/// Define peripheral MVP structure.
typedef struct sl_peripheral_mvp {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_req;       ///< Request DMA channel
} sl_peripheral_mvp_val_t;

/// Define peripheral MVP typedef.
typedef const sl_peripheral_mvp_val_t* sl_peripheral_mvp_t;

/// Define peripheral PIXELRZ structure.
typedef struct sl_peripheral_pixelrz {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_txf;       ///< Transmit FIFO DMA channel
} sl_peripheral_pixelrz_val_t;

/// Define peripheral PIXELRZ typedef.
typedef const sl_peripheral_pixelrz_val_t* sl_peripheral_pixelrz_t;

/// Define peripheral PDM structure.
typedef struct sl_peripheral_pdm {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_rxdata;    ///< Receive data DMA channel
} sl_peripheral_pdm_val_t;

/// Define peripheral PDM typedef.
typedef const sl_peripheral_pdm_val_t* sl_peripheral_pdm_t;

/// Define peripheral LCD structure.
typedef struct sl_peripheral_lcd {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_lcd;       ///< LCD DMA channel
} sl_peripheral_lcd_val_t;

/// Define peripheral LCD typedef.
typedef const sl_peripheral_lcd_val_t* sl_peripheral_lcd_t;

/// Define peripheral LESENSE structure.
typedef struct sl_peripheral_lesense {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_fifo;      ///< FIFO DMA channel
} sl_peripheral_lesense_val_t;

/// Define peripheral LESENSE typedef.
typedef const sl_peripheral_lesense_val_t* sl_peripheral_lesense_t;

/// Define peripheral VDAC structure.
typedef struct sl_peripheral_vdac {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_ch0req;    ///< Channel 0 request DMA channel
  sl_dma_signal_t dma_signal_ch1req;    ///< Channel 1 request DMA channel
} sl_peripheral_vdac_val_t;

/// Define peripheral VDAC typedef.
typedef const sl_peripheral_vdac_val_t* sl_peripheral_vdac_t;

/// Define peripheral LDMAXBAR structure.
typedef struct sl_peripheral_ldmaxbar {
  uint32_t base;                        ///< Peripheral base address.
  sl_clock_branch_t clk_branch;         ///< Peripheral clock branch.
  sl_bus_clock_t bus_clock;             ///< Peripheral bus clock.
  sl_dma_signal_t dma_signal_prsreq0;   ///< Channel 0 request DMA channel
  sl_dma_signal_t dma_signal_prsreq1;   ///< Channel 1 request DMA channel
} sl_peripheral_ldmaxbar_val_t;

/// Define peripheral LDMAXBAR typedef.
typedef const sl_peripheral_ldmaxbar_val_t* sl_peripheral_ldmaxbar_t;

/** @} (end addtogroup device_peripheral) */

#ifdef __cplusplus
}
#endif

#endif // SL_DEVICE_PERIPHERAL_TYPES_H
