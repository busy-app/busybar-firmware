/***************************************************************************/ /**
 * @file  sl_si91x_bus.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_status.h"
#include "sl_si91x_types.h"
#include "system_si91x.h"
#include "rsi_m4.h"
#include "sl_constants.h"
#include "cmsis_os2.h"
#include "sl_cmsis_utility.h"
#include "rsi_power_save.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_driver.h"
#include <stddef.h>
#include <stdlib.h>
#include "sl_rsi_utility.h"
#include "sli_wifi_utility.h"
rsi_m4ta_desc_t tx_desc[2];
rsi_m4ta_desc_t rx_desc[2];
sli_wifi_buffer_queue_t sli_ahb_bus_rx_queue;

/******************************************************
 * *               Function Declarations
 * ******************************************************/
sl_status_t sli_si91x_submit_rx_pkt(void);
sl_status_t sli_submit_rx_buffer(void);
void sli_si91x_raise_pkt_pending_interrupt_to_ta(void);

sl_status_t sl_si91x_bus_init(void)
{
  sli_ahb_bus_rx_queue.head = NULL;
  sli_ahb_bus_rx_queue.tail = NULL;
  mask_ta_interrupt(TA_RSI_BUFFER_FULL_CLEAR_EVENT);
  return RSI_SUCCESS;
}

/**
 * @fn          sl_status_t sli_si91x_submit_rx_pkt(void)
 * @brief       Submit receiver packets
 * @param[in]   None
 * @return      0 - Success \n
 *          Non-Zero - Failure
 */
sl_wifi_buffer_t *rx_pkt_buffer;
sl_status_t sli_si91x_submit_rx_pkt(void)
{
  sl_status_t status;
  uint16_t data_length = 0;
  sl_wifi_system_packet_t *packet;
  int8_t *pkt_buffer = NULL;

  if (M4SS_P2P_INTR_SET_REG & RX_BUFFER_VALID) {
    return -2;
  }

  // Allocate packet to receive packet from module
  status = sli_si91x_host_allocate_buffer(&rx_pkt_buffer, SL_WIFI_RX_FRAME_BUFFER, 1616, 1000);
  if (status != SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
    return SL_STATUS_ALLOCATION_FAILED;
  }

  packet     = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(rx_pkt_buffer, 0, &data_length);
  pkt_buffer = (int8_t *)&packet->desc[0];

  // Fill source address in the TX descriptors
  rx_desc[0].addr = (M4_MEMORY_OFFSET_ADDRESS + (uint32_t)pkt_buffer);

  // Fill source address in the TX descriptors
  rx_desc[0].length = 16;

  // Fill source address in the TX descriptors
  rx_desc[1].addr = (M4_MEMORY_OFFSET_ADDRESS + (uint32_t)(pkt_buffer + 16));

  // Fill source address in the TX descriptors
  rx_desc[1].length = 1600;

  raise_m4_to_ta_interrupt(RX_BUFFER_VALID);

  return SL_STATUS_OK;
}

sl_status_t sli_si91x_bus_read_frame(sl_wifi_buffer_t **buffer)
{
  sl_status_t status = sli_si91x_remove_from_queue(&sli_ahb_bus_rx_queue, buffer);
  VERIFY_STATUS_AND_RETURN(status);

  return SL_STATUS_OK;
}

/**
 * @fn          sl_status_t sli_si91x_bus_write_frame(sl_wifi_system_packet_t *packet,
 *                  uint8_t *payloadparam, uint16_t size_param)
 * @brief       writing a command to the module.
 * @param[in]   payloadparam - pointer to the command payload parameter structure
 * @param[in]   size_param - size of the payload for the command
 * @return      0              - Success \n
 *              Negative Value - Failure
 */

sl_status_t sli_si91x_bus_write_frame(sl_wifi_system_packet_t *packet, const uint8_t *payloadparam, uint16_t size_param)
{

  // Fill source address in the TX descriptors
  tx_desc[0].addr = (M4_MEMORY_OFFSET_ADDRESS + (uint32_t)&packet->desc[0]);

  // Fill source address in the TX descriptors
  tx_desc[0].length = 16;

  // Fill source address in the TX descriptors
  tx_desc[1].addr = (M4_MEMORY_OFFSET_ADDRESS + (uint32_t)payloadparam);

  // Fill source address in the TX descriptors
  tx_desc[1].length = size_param;

  sli_si91x_raise_pkt_pending_interrupt_to_ta();

  return SL_STATUS_OK;
}

sl_status_t sli_submit_rx_buffer(void)
{
  sl_status_t status = SL_STATUS_OK;
  mask_ta_interrupt(RX_PKT_TRANSFER_DONE_INTERRUPT);

  //! submit to NWP submit packet
  status = sli_si91x_submit_rx_pkt();
  if (status != SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\n RX Buffer submission failed with status: %d \r\n", status);
  }

  unmask_ta_interrupt(RX_PKT_TRANSFER_DONE_INTERRUPT);
  return status;
}

/**
 * @fn           void rsi_update_tx_dma_desc(uint8 skip_dma_valid)
 * @brief        This function updates the TX DMA descriptor address
 * @param[in]    skip_dma_valid
 * @param[out]   none
 * @return       none
 * @section description
 * This function updates the TX DMA descriptor address
 *
 *
 */

void rsi_update_tx_dma_desc(uint8_t skip_dma_valid)
{
  if (!skip_dma_valid
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
      && !(M4_ULP_SLP_STATUS_REG & MCU_ULP_WAKEUP)
#endif
  ) {
    while (M4_TX_DMA_DESC_REG & DMA_DESC_REG_VALID)
      ;
  }
  M4_TX_DMA_DESC_REG = (uint32_t)&tx_desc;
}

/*==============================================*/
/**
 * @fn           void rsi_update_rx_dma_desc()
 * @brief        This function updates the RX DMA descriptor address
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @section description
 * This function updates the RX DMA descriptor address
 *
 *
 */
void rsi_update_rx_dma_desc(void)
{
  M4_RX_DMA_DESC_REG = (uint32_t)&rx_desc;
}

void sli_si91x_config_m4_dma_desc_on_reset(void)
{

  //! Wait for NWP to wakeup and should be in bootloader
  while (!(P2P_STATUS_REG & TA_is_active))
    ;
  SL_DEBUG_LOG("\r\nTA is in active state\r\n");
  //! TBD Need to address why soft reset expecting delay
  osDelay(SLI_SYSTEM_MS_TO_TICKS(100));
  //! Update M4 Tx and Rx DMA descriptors
  M4_TX_DMA_DESC_REG = (uint32_t)&tx_desc;
  M4_RX_DMA_DESC_REG = (uint32_t)&rx_desc;
}
