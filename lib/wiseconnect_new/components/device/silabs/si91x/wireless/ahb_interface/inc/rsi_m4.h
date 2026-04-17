/***************************************************************************/ /**
 * @file  rsi_m4.h
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

#ifndef _RSI_M4_HAL_H_
#define _RSI_M4_HAL_H_
#ifdef SLI_SI91X_MCU_INTERFACE

#include "rsi_pkt_mgmt.h"
#include "sl_device.h"
#include "sl_status.h"

/******************************************************
 * *                    Constants
 * ******************************************************/

#define M4_ISR_IRQ 74

#define TA_MEMORY_OFFSET_ADDRESS 0x00400000
#if defined(SLI_SI917)
#define M4_MEMORY_OFFSET_ADDRESS 0x00500000
#else
#define M4_MEMORY_OFFSET_ADDRESS 0x00200000
#endif

#define SCS_BASE (0xE000E000UL) /*!< System Control Space Base Address  */

#define NVIC_BASE (SCS_BASE + 0x0100UL) /*!< NVIC Base Address                  */
#ifndef NVIC
#define NVIC ((NVIC_Type *)NVIC_BASE) /*!< NVIC configuration struct          */
#endif

#define M4SS_P2P_INT_BASE_ADDRESS 0x46008000
#define MCU_PWR_CTRL_BASE_ADDR    0x24048400
#define MISC_CFG_HOST_CTRL        *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x0C)
#ifndef M4SS_P2P_INTR_SET_REG
#define M4SS_P2P_INTR_SET_REG *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x16C)
#endif
#ifndef M4SS_P2P_INTR_CLR_REG
#define M4SS_P2P_INTR_CLR_REG *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x170)
#endif
#define P2P_STATUS_REG         *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x174)
#define TASS_P2P_INTR_MASK_SET *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x178)
#define TASS_P2P_INTR_MASK_CLR *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x17C)
#define TASS_P2P_INTR_CLEAR    *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x180)

#define TASS_P2P_INT_BASE_ADDRESS 0x41050000

#define TASS_P2P_INTR_SET_REG *(volatile uint32_t *)(TASS_P2P_INT_BASE_ADDRESS + 0x8C)
#define TASS_P2P_INTR_CLR_REG *(volatile uint32_t *)(TASS_P2P_INT_BASE_ADDRESS + 0x90)

#define M4_TX_DMA_DESC_REG   *(volatile uint32_t *)(TASS_P2P_INT_BASE_ADDRESS + 0x34)
#define M4_RX_DMA_DESC_REG   *(volatile uint32_t *)(TASS_P2P_INT_BASE_ADDRESS + 0x5C)
#define HOST_INTR_STATUS_REG *(volatile uint32_t *)(TASS_P2P_INT_BASE_ADDRESS + 0x04)

#define DMA_DESC_REG_VALID (0xA0 << 8)

#define TA_wakeup_M4 BIT(2)
#define TA_is_active BIT(3)
#define M4_wakeup_TA BIT(0)
#define M4_is_active BIT(1)

/*Macro used to define the PTE CRC value of the Firmware 17 Boards*/
#define FIRMWARE_17_PTE_CRC_VALUE 0

/*Macro used to notify NWP about M4 XTAL usage*/
#define TURN_ON_XTAL_REQUEST     BIT(9)
#define TURN_OFF_XTAL_REQUEST    BIT(10)
#define M4_IS_USING_XTAL_REQUEST BIT(11)

#define ARM_MASK_1 0xE000E100
#define ARM_MASK_1 0xE000E100
#define ARM_MASK_1 0xE000E100

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

//! This interrupt is raised by M4 to NWP when there is a TX packet from M4 to read
#define RX_BUFFER_VALID          BIT(1)
#define TX_PKT_PENDING_INTERRUPT BIT(2)
#define UPGRADE_M4_IMAGE         BIT(5)
#if defined(SLI_SI917)
#define M4_WAITING_FOR_TA_TO_WR_ON_FLASH BIT(6)
#endif
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
#define SIDE_BAND_CRYPTO_INTR BIT(7)
#endif
#define M4_WAITING_FOR_TA_DEINIT BIT(8)

#define TX_PKT_TRANSFER_DONE_INTERRUPT BIT(2)
//! This interrupt is received from NWP when RX packet is pending from NWP
#define RX_PKT_TRANSFER_DONE_INTERRUPT BIT(1)
//! This interrupt is received from NWP when TX packet transfer from M4 to NWP is done

#define M4_IMAGE_UPGRADATION_PENDING_INTERRUPT BIT(4)
//! This interrupt is raised by NWP to M4 when there is a TX packet from M4 to read

#if defined(SLI_SI917)
//! This interrupt is raised by NWP to M4 when there is a flash write request from M4 to NWP in common flash mode
#define TA_WRITING_ON_COMM_FLASH BIT(5)
#endif

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
#define SIDE_BAND_CRYPTO_DONE BIT(6)
#endif
#define NWP_DEINIT_IN_COMM_FLASH BIT(7)

//! This interrupt is received from TA when SLI_WIFI_BUFFER_FULL is clear in TA
#define TA_RSI_BUFFER_FULL_CLEAR_EVENT BIT(8)

#if defined(SLI_SI917)
//! Option value for m4 app from flash to ram API
#define UPGRADE_M4_IMAGE_OTA    1
#define TA_WRITES_ON_COMM_FLASH 2
#define M4_WAIT_FOR_NWP_DEINIT  3
#endif

#ifdef SLI_SI91X_ENABLE_OS
#define TASS_P2P_INTR_PRI 5
#define SYSTICK_INTR_PRI  7
#else
#define TASS_P2P_INTR_PRI 5
#define SYSTICK_INTR_PRI  (1 << __NVIC_PRIO_BITS) - 1
#endif
/******************************************************
 * *                   Enumerations
 * ******************************************************/

/******************************************************
 * *                 Type Definitions
 * ******************************************************/
typedef struct rsi_m4ta_desc_dword1_s {
  //! Reserved
  uint32_t reserved : 15;

  //! 1 bit : indicates the presence of more descriptors
  //! 1 -  last descriptor
  //! 0 -  more descriptors are present
  uint32_t last_desc : 1;

  //! Buffer queue_no to be transfered for this descriptor
  uint32_t queue_no : 4;

  //! Buffer length to be transfered for this descriptor
  uint32_t length : 12;

} rsi_m4ta_desc_dword1_t;

typedef struct rsi_m4ta_desc_s {
  //! source address
  uint32_t addr;

  uint16_t length;
  //! descriptor control fields

} rsi_m4ta_desc_t;

//! host descriptor structure
typedef struct rsi_frame_desc_s {
  //! Data frame body length. Bits 14:12=queue, 000 for data, Bits 11:0 are the length
  uint8_t frame_len_queue_no[2];
  //! Frame type
  uint8_t frame_type;
  //! Unused , set to 0x00
  uint8_t reserved[9];
  //! Management frame descriptor response status, 0x00=success, else error
  uint8_t status;
  uint8_t reserved1[3];
} rsi_frame_desc_t;

//! P2P registers Backup structure
typedef struct rsi_p2p_intr_status_bkp_s {
  uint32_t tass_p2p_intr_mask_clr_bkp;
  uint32_t m4ss_p2p_intr_set_reg_bkp;
} rsi_p2p_intr_status_bkp_t;

/******************************************************
 * *                    Structures
 * ******************************************************/

/******************************************************
 * *                 Global Variables
 * ******************************************************/
/******************************************************
 * *               Function Declarations
 * ******************************************************/
int16_t rsi_frame_write(rsi_frame_desc_t *uFrameDscFrame, uint8_t *payloadparam, uint16_t size_param);
rsi_pkt_t *rsi_frame_read(void);
int16_t rsi_device_interrupt_status(uint8_t *int_status);

sl_status_t sli_m4_interrupt_isr(void);
void sli_m4_ta_interrupt_init(void);
void sli_si91x_raise_pkt_pending_interrupt_to_ta(void);
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
void sli_si91x_raise_side_band_interrupt_to_ta(void);
#endif
int32_t rsi_send_pkt_to_ta(rsi_m4ta_desc_t *tx_desc);
void rsi_transfer_to_ta_done_isr(void);
void rsi_pkt_pending_from_ta_isr(void);
sl_status_t sli_receive_from_ta_done_isr(void);
int16_t rsi_device_buffer_full_status(void);
int rsi_submit_rx_pkt(void);
void unmask_ta_interrupt(uint32_t interrupt_no);
void mask_ta_interrupt(uint32_t interrupt_no);
void raise_m4_to_ta_interrupt(uint32_t interrupt_no);
void clear_m4_to_ta_interrupt(uint32_t interrupt_no);
void clear_ta_interrupt_mask(void);
void set_ta_interrupt_mask(void);
void clear_ta_to_m4_interrupt(uint32_t interrupt_no);
void sli_mv_m4_app_from_flash_to_ram(int option);
uint32_t NVIC_GetIRQEnable(IRQn_Type IRQn);
void sli_si91x_config_m4_dma_desc_on_reset(void);
void rsi_update_tx_dma_desc(uint8_t skip_dma_valid);
void rsi_update_rx_dma_desc(void);
sl_status_t sli_si91x_req_wakeup(void);
void sli_si91x_ta_events_init(void); /*Function used to create and initialize event mechanism for NWP related events */
bool sli_si91x_is_m4_using_xtal(void);
bool sli_si91x_is_xtal_in_use_by_m4(void);
void sli_si91x_set_m4_is_using_xtal(void);
void sli_si91x_set_xtal_in_use_by_m4(void);
void sli_si91x_xtal_turn_on_request_from_m4_to_TA(void);
void sli_si91x_xtal_turn_off_request_from_m4_to_TA(void);
void sli_si91x_raise_xtal_interrupt_to_ta(uint16_t xtal_enable);
void sli_si91x_send_m4_xtal_usage_notification_to_ta(void);
void sli_si91x_clear_xtal_in_use_by_m4(void);
#endif
#endif
