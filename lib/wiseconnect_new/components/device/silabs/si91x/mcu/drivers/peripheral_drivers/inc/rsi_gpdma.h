/***************************************************************************/ /**
* @file  rsi_gpdma.h
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

// Includes Files

#include "rsi_ccp_common.h"
#include "base_types.h"
#include "rsi_error.h"
#include "rsi_packing.h"

#ifndef RSI_GPDMA_H
#define RSI_GPDMA_H

#ifdef __cplusplus
extern "C" {
#endif

#define RSI_GPDMA_API_VERSION RSI_DRIVER_VERSION_MAJOR_MINOR(02, 00) // API version
#define RSI_GPDMA_DRV_VERSION RSI_DRIVER_VERSION_MAJOR_MINOR(00, 01) // driver version

#define GPDMA_CHANNEL_NUM 7

// GPDMA Status Flags
#define GPDMA_STAT_HRESP_ERROR0      (1UL << 0) // Channel0 dma error
#define GPDMA_STAT_LINK_LIST_DONE0   (1UL << 1) // Channel0 link list fetch done status
#define GPDMA_STAT_PHRL_END_OF_XFER0 (1UL << 2) // Channel0 transfer done status
#define GPDMA_STAT_GPDMAC_ERROR0     (1UL << 3) // Channel0 Transfer size/burst size /h size mismatch/flow ctrl err status
#define GPDMA_STAT_HRESP_ERROR1      (1UL << 4) // Channel1 dma error
#define GPDMA_STAT_LINK_LIST_DONE1   (1UL << 5) // Channel1 link list fetch done status
#define GPDMA_STAT_PHRL_END_OF_XFER1 (1UL << 6) // Channel1 transfer done status
#define GPDMA_STAT_GPDMAC_ERROR1     (1UL << 7) // Channel1 Transfer size/burst size /h size mismatch/flow ctrl err status
#define GPDMA_STAT_HRESP_ERROR2      (1UL << 8)  // Channel2 dma error
#define GPDMA_STAT_LINK_LIST_DONE2   (1UL << 9)  // Channel2 link list fetch done status
#define GPDMA_STAT_PHRL_END_OF_XFER2 (1UL << 10) // Channel2 transfer done status
#define GPDMA_STAT_GPDMAC_ERROR2     (1UL << 11) // Channel2 Transfer size/burst size /h size mismatch/flow ctrl err status
#define GPDMA_STAT_HRESP_ERROR3      (1UL << 12) // Channel3 dma error
#define GPDMA_STAT_LINK_LIST_DONE3   (1UL << 13) // Channel3 link list fetch done status
#define GPDMA_STAT_PHRL_END_OF_XFER3 (1UL << 14) // Channel3 transfer done status
#define GPDMA_STAT_GPDMAC_ERROR3     (1UL << 15) // Channel3 Transfer size/burst size /h size mismatch/flow ctrl err status
#define GPDMA_STAT_HRESP_ERROR4      (1UL << 16) // Channel4 dma error
#define GPDMA_STAT_LINK_LIST_DONE4   (1UL << 17) // Channel4 link list fetch done status
#define GPDMA_STAT_PHRL_END_OF_XFER4 (1UL << 18) // Channel4 transfer done status
#define GPDMA_STAT_GPDMAC_ERROR4     (1UL << 19) // Channel4 Transfer size/burst size /h size mismatch/flow ctrl err status
#define GPDMA_STAT_HRESP_ERROR5      (1UL << 20) // Channel5 dma error
#define GPDMA_STAT_LINK_LIST_DONE5   (1UL << 21) // Channel5 link list fetch done status
#define GPDMA_STAT_PHRL_END_OF_XFER5 (1UL << 22) // Channel5 transfer done status
#define GPDMA_STAT_GPDMAC_ERROR5     (1UL << 23) // Channel5 Transfer size/burst size /h size mismatch/flow ctrl err status
#define GPDMA_STAT_HRESP_ERROR6      (1UL << 24) // Channel6 dma error
#define GPDMA_STAT_LINK_LIST_DONE6   (1UL << 25) // Channel6 link list fetch done status
#define GPDMA_STAT_PHRL_END_OF_XFER6 (1UL << 26) // Channel6 transfer done status
#define GPDMA_STAT_GPDMAC_ERROR6     (1UL << 27) // Channel6 Transfer size/burst size /h size mismatch/flow ctrl err status
#define GPDMA_STAT_HRESP_ERROR7      (1UL << 28) // Channel7 dma error
#define GPDMA_STAT_LINK_LIST_DONE7   (1UL << 29) // Channel7 link list fetch done status
#define GPDMA_STAT_PHRL_END_OF_XFER7 (1UL << 30) // Channel7 transfer done status
#define GPDMA_STAT_GPDMAC_ERROR7     (1UL << 31) // Channel7 Transfer size/burst size /h size mismatch/flow ctrl err status

// GPDMA Event Flags
#define RSI_GPDMA_EVENT_HRESP_ERROR0             1
#define RSI_GPDMA_EVENT_GPDMAC_LINK_LIST_DONE0   2
#define RSI_GPDMA_EVENT_GPDMAC_PHRL_END_OF_XFER0 3
#define RSI_GPDMA_EVENT_GPDMAC_ERROR0            4
#define RSI_GPDMA_EVENT_HRESP_ERROR1             5
#define RSI_GPDMA_EVENT_GPDMAC_LINK_LIST_DONE1   6
#define RSI_GPDMA_EVENT_GPDMAC_PHRL_END_OF_XFER1 7
#define RSI_GPDMA_EVENT_GPDMAC_ERROR1            8
#define RSI_GPDMA_EVENT_HRESP_ERROR2             9
#define RSI_GPDMA_EVENT_GPDMAC_LINK_LIST_DONE2   10
#define RSI_GPDMA_EVENT_GPDMAC_PHRL_END_OF_XFER2 11
#define RSI_GPDMA_EVENT_GPDMAC_ERROR2            12
#define RSI_GPDMA_EVENT_HRESP_ERROR3             13
#define RSI_GPDMA_EVENT_GPDMAC_LINK_LIST_DONE3   14
#define RSI_GPDMA_EVENT_GPDMAC_PHRL_END_OF_XFER3 15
#define RSI_GPDMA_EVENT_GPDMAC_ERROR3            16
#define RSI_GPDMA_EVENT_HRESP_ERROR4             17
#define RSI_GPDMA_EVENT_GPDMAC_LINK_LIST_DONE4   18
#define RSI_GPDMA_EVENT_GPDMAC_PHRL_END_OF_XFER4 19
#define RSI_GPDMA_EVENT_GPDMAC_ERROR4            20
#define RSI_GPDMA_EVENT_HRESP_ERROR5             21
#define RSI_GPDMA_EVENT_GPDMAC_LINK_LIST_DONE5   22
#define RSI_GPDMA_EVENT_GPDMAC_PHRL_END_OF_XFER5 23
#define RSI_GPDMA_EVENT_GPDMAC_ERROR5            24
#define RSI_GPDMA_EVENT_HRESP_ERROR6             25
#define RSI_GPDMA_EVENT_GPDMAC_LINK_LIST_DONE6   26
#define RSI_GPDMA_EVENT_GPDMAC_PHRL_END_OF_XFER6 27
#define RSI_GPDMA_EVENT_GPDMAC_ERROR6            28
#define RSI_GPDMA_EVENT_HRESP_ERROR7             29
#define RSI_GPDMA_EVENT_GPDMAC_LINK_LIST_DONE7   30
#define RSI_GPDMA_EVENT_GPDMAC_PHRL_END_OF_XFER7 31
#define RSI_GPDMA_EVENT_GPDMAC_ERROR7            32

#define MEMORY_MEMORY         0x0
#define MEMORY_PERIPHERAL     0x1
#define PERIPHERAL_MEMORY     0x2
#define PERIPHERAL_PERIPHERAL 0x3

#define DMA_FLW_CTRL      0x0
#define SRC_PERI_CTRL     0x1
#define DST_PERI_CTRL     0x2
#define SRC_DST_PERI_CTRL 0x3

#define MASTER0_FETCH_IFSEL 0x0
#define MASTER1_FETCH_IFSEL 0x1

#define MASTER0_SEND_IFSEL 0x0
#define MASTER1_SEND_IFSEL 0x1

#define SRC_8_DATA_WIDTH  0x0
#define SRC_16_DATA_WIDTH 0x1
#define SRC_32_DATA_WIDTH 0x2

#define DST_8_DATA_WIDTH  0x0
#define DST_16_DATA_WIDTH 0x1
#define DST_32_DATA_WIDTH 0x2

#define LINK_LIST_EN       0x1
#define LINK_LIST_DIS      0x0
#define LINK_MASTER_0_FTCH 0x0
#define LINK_MASTER_1_FTCH 0x1

#define SRC_ADR_CONTIG_EN  0x1
#define SRC_ADR_CONTIG_DIS 0x0
#define DST_ADR_CONTIG_EN  0x1
#define DST_ADR_CONTIG_DIS 0x0

#define RETRY_ON_ERR_EN  0x1
#define RETRY_ON_ERR_DIS 0x0

#define SRC_FIFO_EN  0x1
#define SRC_FIFO_DIS 0x0

#define DST_FIFO_EN  0x1
#define DST_FIFO_DIS 0x0

#define MAX_TRANS_SIZE 4096

#define AHBBURST_SIZE_1  0x0
#define AHBBURST_SIZE_4  0x1
#define AHBBURST_SIZE_8  0x2
#define AHBBURST_SIZE_16 0x3
#define AHBBURST_SIZE_20 0x4
#define AHBBURST_SIZE_24 0x5
#define AHBBURST_SIZE_28 0x6
#define AHBBURST_SIZE_32 0x7

#define AHBBURST_SIZE_MAX 0x7
#define AHBBURST_SIZE_MIN 0x0

#define DST_BURST_SIZE_64  0x0
#define DST_BURST_SIZE_1   0x1
#define DST_BURST_SIZE_2   0x2
#define DST_BURST_SIZE_3   0x3
#define DST_BURST_SIZE_4   0x4
#define DST_BURST_SIZE_5   0x5
#define DST_BURST_SIZE_6   0x6
#define DST_BURST_SIZE_7   0x7
#define DST_BURST_SIZE_8   0x8
#define DST_BURST_SIZE_9   0x9
#define DST_BURST_SIZE_10  0x10
#define DST_BURST_SIZE_11  0x11
#define DST_BURST_SIZE_12  0x12
#define DST_BURST_SIZE_13  0x13
#define DST_BURST_SIZE_14  0x14
#define DST_BURST_SIZE_15  0x15
#define DST_BURST_SIZE_16  0x16
#define DST_BURST_SIZE_17  0x17
#define DST_BURST_SIZE_18  0x18
#define DST_BURST_SIZE_MIN 0x0
#define DST_BURST_SIZE_MAX 63

#define SRC_BURST_SIZE_64  0x0
#define SRC_BURST_SIZE_1   0x1
#define SRC_BURST_SIZE_2   0x2
#define SRC_BURST_SIZE_3   0x3
#define SRC_BURST_SIZE_4   0x4
#define SRC_BURST_SIZE_5   0x5
#define SRC_BURST_SIZE_6   0x6
#define SRC_BURST_SIZE_7   0x7
#define SRC_BURST_SIZE_8   0x8
#define SRC_BURST_SIZE_9   0x9
#define SRC_BURST_SIZE_10  0x10
#define SRC_BURST_SIZE_11  0x11
#define SRC_BURST_SIZE_12  0x12
#define SRC_BURST_SIZE_13  0x13
#define SRC_BURST_SIZE_14  0x14
#define SRC_BURST_SIZE_15  0x15
#define SRC_BURST_SIZE_16  0x16
#define SRC_BURST_SIZE_17  0x17
#define SRC_BURST_SIZE_18  0x18
#define SRC_BURST_SIZE_MIN 0x0
#define SRC_BURST_SIZE_MAX 63

#define DST_CHNL_ID_MIN 0x0
#define DST_CHNL_ID_MAX 47
#define SRC_CHNL_ID_MIN 0x0
#define SRC_CHNL_ID_MAX 47
#define DMA_PROT_EN     0x1
#define DMA_PROT_DIS    0x0
#define MEM_FILL_EN     0x1
#define MEM_FILL_DIS    0x0
#define MEM_FILL_ONE    0x1
#define MEM_FILL_ZERO   0x0
#define SRC_ALIGN_DIS   0x0
#define SRC_ALIGN_EN    0x1

#define LINK_INTR_DIS        0x0
#define LINK_INTR_ON         0x1
#define DESC_FETCH_DONE_INTR 0
#define TRANS_DONE_INTR      1

#define UNMASK_DESC_FETCH_INTR 0x0000FF
#define GPDMA_CHNL0            0
#define GPDMA_CHNL1            1
#define GPDMA_CHNL7            7

#define GPDMA_CHNL_0 0
#define GPDMA_CHNL_1 1
#define GPDMA_CHNL_2 2
#define GPDMA_CHNL_3 3

#define XFER_SIZE_4K 4096

#define AHB_BURST_1  0x0
#define AHB_BURST_4  0x1
#define AHB_BURST_8  0x2
#define AHB_BURST_16 0x3
#define AHB_BURST_20 0x4
#define AHB_BURST_24 0x5
#define AHB_BURST_28 0x6
#define AHB_BURST_32 0x7

#define SRC_BURST_64 0x00
#define SRC_BURST_1  0x01
#define SRC_BURST_16 0x10
#define SRC_BURST_63 0x3F

#define DST_BURST_64 0x00
#define DST_BURST_1  0x01
#define DST_BURST_16 0x10
#define DST_BURST_63 0x3F

#define FILL_ZEROS 0x0
#define FILL_ONES  0x1

#define PRIO_LEVEL_4 0x3
#define PRIO_LEVEL_3 0x2
#define PRIO_LEVEL_2 0x1
#define PRIO_LEVEL_1 0x0

#define MAX_CHANNELS 0x8

#define DESC_COUNT  4
#define DESC_INDEX0 0
#define DESC_INDEX1 1
#define DESC_INDEX2 2
#define DESC_INDEX3 3

#define FIFO_SIZE_MAX 512

#define NO_DST_CHNL_ID 0x0
#define NO_SRC_CHNL_ID 0x0

#define HRESP_ERR  0
#define GPDMAC_ERR 1

#define M4SS_GPDMA_INTR_SEL  (*((uint32_t volatile *)(0x46110000 + 0x04)))
#define CLOCK_BASE           0x46000000
#define CLK_ENABLE_SET_2_REG (*((uint32_t volatile *)(CLOCK_BASE + 0x08)))
#define CLK_ENABLE_SET_1_REG (*((uint32_t volatile *)(CLOCK_BASE + 0x00)))

typedef GPDMA_G_Type RSI_GPDMAG_T;
typedef GPDMA_C_Type RSI_GPDMAC_T;

// GPDMA Error flags
#define HRESP_ERR0  (1UL << 0)
#define HRESP_ERR1  (1UL << 4)
#define HRESP_ERR2  (1UL << 8)
#define HRESP_ERR3  (1UL << 12)
#define HRESP_ERR4  (1UL << 16)
#define HRESP_ERR5  (1UL << 20)
#define HRESP_ERR6  (1UL << 24)
#define HRESP_ERR7  (1UL << 28)
#define GPDMAC_ERR0 (1UL << 3)
#define GPDMAC_ERR1 (1UL << 7)
#define GPDMAC_ERR2 (1UL << 11)
#define GPDMAC_ERR3 (1UL << 15)
#define GPDMAC_ERR4 (1UL << 19)
#define GPDMAC_ERR5 (1UL << 23)
#define GPDMAC_ERR6 (1UL << 27)
#define GPDMAC_ERR7 (1UL << 31)

// GPDMA transfer status flags
#define LINK_LIST_DONE0  (1UL << 1)
#define PHRL_END_OF_TFR0 (1UL << 2)
#define LINK_LIST_DONE1  (1UL << 5)
#define PHRL_END_OF_TFR1 (1UL << 6)
#define LINK_LIST_DONE2  (1UL << 9)
#define PHRL_END_OF_TFR2 (1UL << 10)
#define LINK_LIST_DONE3  (1UL << 13)
#define PHRL_END_OF_TFR3 (1UL << 14)
#define LINK_LIST_DONE04 (1UL << 17)
#define PHRL_END_OF_TFR4 (1UL << 18)
#define LINK_LIST_DONE5  (1UL << 21)
#define PHRL_END_OF_TFR5 (1UL << 22)
#define LINK_LIST_DONE6  (1UL << 25)
#define PHRL_END_OF_TFR6 (1UL << 26)
#define LINK_LIST_DONE7  (1UL << 29)
#define PHRL_END_OF_TFR7 (1UL << 30)

// brief GPDMA Driver Capabilities.
typedef struct {
  unsigned int noOfChannels : 4;         // Total supporting channels
  unsigned int noOfMasterInterfaces : 2; // No of master interfaces supported
  unsigned int noOfPeriSupport : 7;      // total supporting peripherals
  unsigned int noOfPriorityLevels : 3;   // No of priority levels

} RSI_GPDMA_CAPABILITIES_T;

// brief GPDMA Descriptor parameters.

// brief chnl_ctrl_info
typedef PRE_PACK struct POST_PACK {
  unsigned int transSize : 12;       // Transfer lenght in bytes
  unsigned int transType : 2;        // Type of DMA transfer
  unsigned int dmaFlwCtrl : 2;       // Flow control type
  unsigned int mastrIfFetchSel : 1;  // Master controller select to fetch data
  unsigned int mastrIfSendSel : 1;   // Master controller select to send data
  unsigned int destDataWidth : 2;    // Destination data width
  unsigned int srcDataWidth : 2;     // Source data width
  unsigned int srcAlign : 1;         // Source Alignment
  unsigned int linkListOn : 1;       // Linked transfer on
  unsigned int linkListMstrSel : 1;  // Master controller select for link transfers
  unsigned int srcAddContiguous : 1; // Source address contiguous
  unsigned int dstAddContiguous : 1; // Destination address contiguous
  unsigned int retryOnErr : 1;       // Retry on error
  unsigned int linkInterrupt : 1;    // Link interrupt enable
  unsigned int srcFifoMode : 1;      // Source FIFO mode
  unsigned int dstFifoMode : 1;      // Destination FIFO mode
  unsigned int reserved : 1;
} RSI_GPDMA_CHA_CONTROL_T;

//brief Misc_chnl_ctrl_info
typedef PRE_PACK struct POST_PACK {
  unsigned int ahbBurstSize : 3;  // AHB Burst size
  unsigned int destDataBurst : 6; // Destination data Burst size
  unsigned int srcDataBurst : 6;  // source data Burst size
  unsigned int destChannelId : 6; // Dest channel ID
  unsigned int srcChannelId : 6;  // Source channel ID
  unsigned int dmaProt : 3;
  unsigned int memoryFillEn : 1;  // Memory fill enable
  unsigned int memoryOneFill : 1; // Memory fill with 1 or 0
} RSI_GPDMA_MISC_CHA_CONTROL_T;

//brief GPDMA controller handle type
typedef void *RSI_GPDMA_HANDLE_T;

typedef PRE_PACK struct POST_PACK {
  uint32_t *pNextLink;                             // Pointer to next descriptor link in a chain, NULL to end
  void *src;                                       // source address
  void *dest;                                      // destination address
  RSI_GPDMA_CHA_CONTROL_T chnlCtrlConfig;          // Channel control register paramter
  RSI_GPDMA_MISC_CHA_CONTROL_T miscChnlCtrlConfig; // Channel control register paramter
} RSI_GPDMA_DESC_T;

typedef void (*gpdmaTransferCompleteCB)(RSI_GPDMA_HANDLE_T gpdmaHandle, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);
typedef void (*gpdmaTransferDescFetchCompleteCB)(RSI_GPDMA_HANDLE_T gpdmaHandle,
                                                 RSI_GPDMA_DESC_T *pTranDesc,
                                                 uint32_t dmaCh);
typedef void (*gpdmaTransferHrespErrorCB)(RSI_GPDMA_HANDLE_T gpdmaHandle, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);
typedef void (*gpdmaTransferRpdmacErrorCB)(RSI_GPDMA_HANDLE_T gpdmaHandle, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);
typedef void (*gpdmaTransferErrorCB)(RSI_GPDMA_HANDLE_T gpdmaHandle, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);

// @brief GPDMA controller callback IDs
typedef enum {
  RSI_GPDMA_XFERCOMPLETE_CB = 0,      // Callback ID for GPDMA transfer descriptor chain complete
  RSI_GPDMA_XFERDESCFETCHCOMPLETE_CB, // Callback ID for GPDMA transfer descriptor complete
  RSI_GPDMA_XFERHRESPERROR_CB,        // Callback ID for GPDMA transfer error occurance
  RSI_GPDMA_XFERGPDMACERROR_CB
} RSI_GPDMA_CALLBACK_T;

// Private data structure used for the GPDMA controller driver, holds the driver and peripheral context
typedef struct {
  void *pUserData;                     // Pointer to user data used by driver instance, use NULL if not used
  RSI_GPDMAG_T *baseG;                 // GPDMA global registers base
  RSI_GPDMAC_T *baseC;                 // GPDMA channel specific registers base
  RSI_GPDMA_DESC_T *sramBase;          // SRAM descriptor table (all channels)
  gpdmaTransferCompleteCB gpdmaCompCB; // Transfer descriptor chain completion callback
  gpdmaTransferDescFetchCompleteCB gpdmaDescFetchCompCB; // Transfer descriptor fetch completion callback
  gpdmaTransferHrespErrorCB gpdmaHrespErrorCB;           // Transfer error callback
  gpdmaTransferRpdmacErrorCB gpdmaRpdmacErrorCB;         // Transfer error callback
  uint32_t dmaCh;                                        // GPDMA channel
} GPDMA_DATACONTEXT_T;

// GPDMA Init structure
typedef PRE_PACK struct POST_PACK {
  void *pUserData;   // Pointer to user data used by driver instance, use NULL if not used
  uint32_t baseG;    // Pointer to GPDMA global register instance
  uint32_t baseC;    // Pointer to GPDMA channel specific register instance
  uint32_t sramBase; // Pointer to memory used for GPDMA descriptor storage, must be 512 byte aligned
} RSI_GPDMA_INIT_T;

// brief GPDMA transfer channel setup structure (use this structure as const if possible)
typedef PRE_PACK struct POST_PACK {
  uint32_t channelPrio;       // Channel priority level
  uint32_t descFetchDoneIntr; // Desc fetch done interrupt flag
  uint32_t xferDoneIntr;      // Transfer done interrupt flag
  uint32_t hrespErr;          // dma error flag
  uint32_t gpdmacErr;         // Transfer size/burst size /h size mismatch/flow ctrl err
  uint32_t dmaCh;             // Channel number
} RSI_GPDMA_CHA_CFG_T;

/*===================================================*/
/**
 * @fn          void RSI_GPDMA_FIFOConfig( RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh,
                                           uint32_t startAdr, uint32_t size )
 * @brief		Set fifo configuration for data transmission.
 * @param[in]   pHandle  : Pointer to driver context handle
 * @param[in]   dmaCh    : DMA channel number(0-7)
 * @param[in]   startAdr : starting address for data transfer.
 * @param[in]   size     : size of data transfer.
 * @return 		none.
 */
STATIC INLINE void RSI_GPDMA_FIFOConfig(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh, uint32_t startAdr, uint32_t size)
{
  GPDMA_DATACONTEXT_T *pDrv                                            = (GPDMA_DATACONTEXT_T *)pHandle;
  pDrv->baseC->CHANNEL_CONFIG[dmaCh].FIFO_CONFIG_REGS_b.FIFO_STRT_ADDR = (unsigned int)(startAdr & 0x3F);
  pDrv->baseC->CHANNEL_CONFIG[dmaCh].FIFO_CONFIG_REGS_b.FIFO_SIZE      = (unsigned int)(size & 0x3F);
}

// FUNCTION PROTOTYPES
RSI_DRIVER_VERSION_M4 RSI_GPDMA_GetVersion(void);

RSI_GPDMA_CAPABILITIES_T RSI_GPDMA_GetCapabilities(void);

uint32_t gpdma_get_mem_size(void);

RSI_GPDMA_HANDLE_T gpdma_init(void *mem, const RSI_GPDMA_INIT_T *pInit);

void gpdma_register_callback(RSI_GPDMA_HANDLE_T pHandle, uint32_t cbIndex, gpdmaTransferCompleteCB pCB);

rsi_error_t gpdma_abort_channel(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh);

rsi_error_t gpdma_setup_channel(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

rsi_error_t gpdma_build_descriptors(RSI_GPDMA_HANDLE_T pHandle,
                                    RSI_GPDMA_DESC_T *pXferCfg,
                                    RSI_GPDMA_DESC_T *pDesc,
                                    RSI_GPDMA_DESC_T *pDescPrev);

rsi_error_t gpdma_setup_channelTransfer(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh, RSI_GPDMA_DESC_T *pDesc);

void gpdma_interrupt_handler(RSI_GPDMA_HANDLE_T pHandle);

void gpdma_deInit(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

rsi_error_t gpdma_dma_channel_trigger(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh);

uint32_t gpdma_channel_is_enabled(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh);

rsi_error_t gpdma_interrupt_disable(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

rsi_error_t gpdma_interrupt_enable(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

rsi_error_t gpdma_error_status_clear(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

uint32_t gpdma_get_error_status(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

rsi_error_t gpdma_interrupt_clear(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

uint32_t gpdma_interrupt_status(RSI_GPDMA_HANDLE_T pHandle, RSI_GPDMA_CHA_CFG_T *pCfg);

uint8_t RSI_GPDMA_GetChannelActivity(RSI_GPDMA_HANDLE_T pHandle, uint8_t dmaCh);

#ifdef __cplusplus
}
#endif
#endif //RSI_GPDMA_H
