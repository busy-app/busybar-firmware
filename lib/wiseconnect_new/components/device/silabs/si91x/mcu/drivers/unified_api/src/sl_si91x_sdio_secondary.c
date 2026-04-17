/******************************************************************************
* @file  sl_si91x_sdio_secondary.c
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

#include "sl_si91x_sdio_secondary.h"
#include "sl_si91x_peripheral_sdio_secondary.h"
#include "sl_si91x_sdio_secondary_drv_config.h"
#include "rsi_rom_gpdma.h"
#include "rsi_rom_clks.h"
#include "si91x_device.h"
#include "sl_assert.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define SDIO_SECONDARY_RELEASE_VERSION 0    // SDIO Secondary Release version
#define SDIO_SECONDARY_SQA_VERSION     0    // SDIO Secondary SQA version
#define SDIO_SECONDARY_DEV_VERSION     2    // SDIO Secondary Developer version
#define MAX_DESCRIPTOR_SIZE            4095 // Maximum DMA transfer size per descriptor (12-bit field)
/*******************************************************************************
 ******************************   VARIABLES   **********************************
 ******************************************************************************/

#if defined(__CC_ARM)
// Keil alignment to 1024 bytes
__align(1024) RSI_GPDMA_DESC_T GPDMADesc[NUMGPDMADESC];
#endif // defined (__CC_ARM)

// IAR support
#if defined(__ICCARM__)
#pragma data_alignment = 1024
static RSI_GPDMA_DESC_T GPDMADesc[NUMGPDMADESC];
#endif // defined (__ICCARM__)

#if defined(__GNUC__)
RSI_GPDMA_DESC_T GPDMADesc[NUMGPDMADESC];
RSI_GPDMA_DESC_T rx_GPDMADesc[NUMGPDMADESC];
#endif // defined (__GNUC__)

static uint32_t memBuff[30]; //gpdma driver memory context, driver context
                             //size is less than 30 bypes that's why size is 30
uint32_t memSize = 0, *devMem_sdio = NULL;
RSI_GPDMA_HANDLE_T GPDMAHandle;

/*******************************************************************************
 ************************       GLOBAL FUNCTIONS      **************************
 ******************************************************************************/
extern __INLINE uint32_t sl_si91x_sdio_secondary_get_block_cnt(void);
extern __INLINE uint32_t sl_si91x_sdio_secondary_get_block_len(void);
extern __INLINE void sl_si91x_sdio_secondary_set_tx_blocks(uint8_t no_of_blocks);
extern __INLINE void sl_si91x_sdio_secondary_set_interrupts(uint32_t flags);
extern __INLINE void sl_si91x_sdio_secondary_clear_interrupts(uint32_t flags);

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t sl_si91x_gpdma_init();
static void sl_si91x_fill_rx_descriptors(uint8_t *data_buf);
static void sl_si91x_fill_tx_descriptors(uint8_t *data_buff, uint8_t num_of_blocks);
static void sl_si91x_setup_rx_channel_desc(uint8_t *data_buf);
static void sl_si91x_setup_tx_channel_desc(uint8_t *data_buf, uint8_t num_of_blocks);
void GPDMATransferComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);
void GPDMATransferDescComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);
void GPDMATransferError(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);
void GPDMATransferDescFetchComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
static sl_sdio_secondary_callback_t user_callback = NULL; // SDIO Secondary user callback function variable
static sl_sdio_secondary_gpdma_callback_t user_gpdma_callback =
  NULL; // SDIO Secondary user gpdma callback function varialbe

RSI_GPDMA_DESC_T rx_XferCfg;
volatile RSI_GPDMA_DESC_T XferCfg;
/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 * @fn     void GPDMA_Handler(void)
 * @brief  GPDMA Interrupt Handler. Clears the dma interrupt triggered for the 
 *         SDIO channel. And calls the user callback handler.
 * @param  None
 * @return None
 ******************************************************************************/
void GPDMA_Handler(void)
{
  RSI_GPDMA_InterruptHandler(GPDMAHandle);
}

/*******************************************************************************
 * @fn     void SDIO_Handler(void)
 * @brief  SDIO Interrupt Handler ,clear sdio primary got , sets the event and 
 *         calls the user callback function to user level handling
 * @param  None
 * @return None
 ******************************************************************************/
void SDIO_Handler(void)
{
  uint8_t events       = 0;
  uint32_t intr_status = SDIO->SDIO_INTR_FN1_STATUS_CLEAR_REG;

  // Handle receive event
  if (intr_status & HOST_INTR_RECEIVE_EVENT) {
    sl_si91x_sdio_secondary_clear_interrupts(SL_SDIO_WR_INT_MSK);
    events |= HOST_INTR_RECEIVE_EVENT;
  }

  // Handle send event (can happen simultaneously)
  if (intr_status & HOST_INTR_SEND_EVENT) {
    sl_si91x_sdio_secondary_clear_interrupts(SL_SDIO_RD_INT_MSK);
    events |= HOST_INTR_SEND_EVENT;
  }

  // Handle CMD52 event (can happen simultaneously)
  if (intr_status & HOST_INTR_CMD52_EVENT) {
    sl_si91x_sdio_secondary_clear_interrupts(SL_SDIO_CMD52_INT_MSK);
    events |= HOST_INTR_CMD52_EVENT;
  }

  // Handle error events
  if (intr_status & HOST_INTR_ERROR_EVENT) // Check if this event exists in your defines
  {
    sl_si91x_sdio_secondary_clear_interrupts(SL_SDIO_ERROR_INT_MSK); // Define this mask
    events |= HOST_INTR_ERROR_EVENT;
  }
  // Call user callback only if events are present
  if (events != 0) {
    user_callback(events);
  } else {
    // Handle unknown events - no recognized events, clear all interrupts as safety measure
    sl_si91x_sdio_secondary_clear_interrupts(0xFF); // or appropriate mask
  }
}

/*******************************************************************************
 * @fn          void GPDMATransferComplete(RSI_GPDMA_HANDLE_T GPDMAHandle, RSI_GPDMA_DESC_T *pTranDesc)
 * @brief       GPDMA controller transfer descriptor chain complete callback
 * @param[in]   GPDMAHandle : structure variable to driver context handle
 * @param[in]   pTranDesc   : Pointer to transfer descriptor
 * @return      None
 ******************************************************************************/
void GPDMATransferComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  user_gpdma_callback((uint8_t)dmaCh);
  // Re-enable SDIO interrupts after transfer completion
  sl_si91x_sdio_secondary_set_interrupts(SL_SDIO_WR_INT_UNMSK | SL_SDIO_RD_INT_UNMSK | SL_SDIO_CMD52_INT_UNMSK);
}

/*******************************************************************************
 * @fn          void GPDMATransferDescComplete(RSI_GPDMA_HANDLE_T GPDMAHandle, RSI_GPDMA_DESC_T *pTranDesc)
 * @brief       GPDMA controller transfer descriptor complete callback
 * @param[in]   GPDMAHandle: structure variable to driver context handle
 * @param[in]   pTranDesc  : Pointer to transfer descriptor
 * @return      None
 ******************************************************************************/
void GPDMATransferDescComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  (void)dmaCh;
}

/*******************************************************************************
 * @fn          void GPDMATransferError(RSI_GPDMA_HANDLE_T GPDMAHandle, RSI_GPDMA_DESC_T *pTranDesc)
 * @brief       GPDMA controller transfer descriptor error callback
 * @param[in]   GPDMAHandle : structure variable to driver context handle
 * @param[in]   pTranDesc   : Pointer to transfer descriptor
 * @return      None
 ******************************************************************************/
void GPDMATransferError(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  (void)dmaCh;
}

/*******************************************************************************
 * @fn          void GPDMATransferDescFetchComplete(RSI_GPDMA_HANDLE_T GPDMAHandle, RSI_GPDMA_DESC_T *pTranDesc)
 * @brief       DMA controller transfer descriptor error callback
 * @param[in]   GPDMAHandle: structure variable to driver context handle
 * @param[in]   pTranDesc  : Pointer to transfer descriptor
 * @return      None
 ******************************************************************************/
void GPDMATransferDescFetchComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  (void)dmaCh;
}

/*******************************************************************************
 * @fn          void sl_si91x_fill_rx_descriptors(uint8_t *data_buf)
 * @brief       Filled RX GPDMA descriptor
 * @param[in]   data_buf : Pointer to RX buffer
 * @return      None
 ******************************************************************************/
void sl_si91x_fill_rx_descriptors(uint8_t *data_buf)
{
  RSI_GPDMA_DESC_T *pPrevDesc;
  uint32_t no_of_desc      = 0;
  uint32_t block_len       = 0;
  uint32_t block_cnt       = 0;
  uint32_t total_bytes     = 0;
  uint32_t remaining_bytes = 0;
  uint32_t current_offset  = 0;

  // Get the SDIO block count and block length from the last received CMD53
  block_cnt   = sl_si91x_sdio_secondary_get_block_cnt();
  block_len   = sl_si91x_sdio_secondary_get_block_len();
  total_bytes = block_cnt * block_len;

  // Calculate the minimum number of descriptors needed to transfer all data
  // Each descriptor can handle up to 4095 bytes, so we divide total bytes
  // and round up if there's a remainder
  no_of_desc = total_bytes / MAX_DESCRIPTOR_SIZE;
  if (total_bytes % MAX_DESCRIPTOR_SIZE != 0) {
    no_of_desc++;
  }
  remaining_bytes = total_bytes;

  // Configure channel control settings (common for all descriptors)
  // These settings are set once outside the loop for better performance
  pPrevDesc                                  = NULL;
  rx_XferCfg.chnlCtrlConfig.transType        = PERIPHERAL_MEMORY;
  rx_XferCfg.chnlCtrlConfig.dmaFlwCtrl       = DMA_FLW_CTRL;
  rx_XferCfg.chnlCtrlConfig.mastrIfFetchSel  = MASTER0_FETCH_IFSEL;
  rx_XferCfg.chnlCtrlConfig.mastrIfSendSel   = MASTER0_SEND_IFSEL;
  rx_XferCfg.chnlCtrlConfig.destDataWidth    = DST_8_DATA_WIDTH; //DST_8_DATA_WIDTH;
  rx_XferCfg.chnlCtrlConfig.srcDataWidth     = SRC_32_DATA_WIDTH;
  rx_XferCfg.chnlCtrlConfig.srcAlign         = 0;
  rx_XferCfg.chnlCtrlConfig.linkListOn       = 1;
  rx_XferCfg.chnlCtrlConfig.linkListMstrSel  = LINK_MASTER_0_FTCH;
  rx_XferCfg.chnlCtrlConfig.srcAddContiguous = 0;
  rx_XferCfg.chnlCtrlConfig.dstAddContiguous = 0;
  rx_XferCfg.chnlCtrlConfig.retryOnErr       = 0;
  rx_XferCfg.chnlCtrlConfig.linkInterrupt    = 0;
  rx_XferCfg.chnlCtrlConfig.srcFifoMode      = 1;
  rx_XferCfg.chnlCtrlConfig.dstFifoMode      = 0;

  // Misc Channel Config
  rx_XferCfg.miscChnlCtrlConfig.ahbBurstSize  = AHBBURST_SIZE_4;
  rx_XferCfg.miscChnlCtrlConfig.destDataBurst = DST_BURST_SIZE_4;
  rx_XferCfg.miscChnlCtrlConfig.srcDataBurst  = SRC_BURST_SIZE_4;
  rx_XferCfg.miscChnlCtrlConfig.destChannelId = 0;
  rx_XferCfg.miscChnlCtrlConfig.srcChannelId  = 30;
  rx_XferCfg.miscChnlCtrlConfig.dmaProt       = 0;
  rx_XferCfg.miscChnlCtrlConfig.memoryFillEn  = 0;
  rx_XferCfg.miscChnlCtrlConfig.memoryOneFill = 0;

  // Create descriptor chain - each descriptor handles up to 4095 bytes
  for (uint32_t j = 0; j < no_of_desc; j++) {
    // Calculate transfer size for this descriptor (either max size or remaining bytes)
    uint32_t transfer_size = (remaining_bytes > MAX_DESCRIPTOR_SIZE) ? MAX_DESCRIPTOR_SIZE : remaining_bytes;

    // Set the transfer size for this specific descriptor
    rx_XferCfg.chnlCtrlConfig.transSize = (transfer_size & 0xfff);

    // Set destination address (offset into the user buffer)
    rx_XferCfg.dest = (data_buf + current_offset);

    // Set source address (SDIO RX FIFO - same for all descriptors)
    rx_XferCfg.src = (uint32_t *)(RX_SOURCE_ADDR);

    // Build the descriptor and link it to the previous one
    if (RSI_GPDMA_BuildDescriptors(GPDMAHandle, &rx_XferCfg, &rx_GPDMADesc[j], pPrevDesc) != RSI_OK) {
    }

    // Update the previous descriptor pointer for the next iteration
    pPrevDesc = &rx_GPDMADesc[j];

    // Update offset and remaining bytes for next descriptor
    current_offset += transfer_size;
    remaining_bytes -= transfer_size;
  }

  // Mark the end of the descriptor chain
  rx_GPDMADesc[no_of_desc - 1].pNextLink = NULL;
}

/*******************************************************************************
 * @fn          void sl_si91x_fill_tx_descriptors(uint8_t *data_buff, uint8_t num_of_blocks)
 * @brief       Filled TX GPDMA descriptor
 * @param[in]   data_buf      : Pointer to TX buffer
 * @param[in]   num_of_blocks : Numbers of block transmit.
 * @return      None
 ******************************************************************************/
void sl_si91x_fill_tx_descriptors(uint8_t *data_buff, uint8_t num_of_blocks)
{
  RSI_GPDMA_DESC_T *pPrevDesc;
  RSI_GPDMA_DESC_T XferCfg;
  uint32_t no_of_desc      = 0;
  uint32_t total_bytes     = 0;
  uint32_t remaining_bytes = 0;
  uint32_t current_offset  = 0;

  // Calculate total bytes to transfer based on number of blocks
  total_bytes = num_of_blocks * TX_TRANSFER_BLOCK_SIZE;

  // Calculate the minimum number of descriptors needed to transfer all data
  // Each descriptor can handle up to 4095 bytes, so we divide total bytes
  // and round up if there's a remainder
  no_of_desc = total_bytes / MAX_DESCRIPTOR_SIZE;
  if (total_bytes % MAX_DESCRIPTOR_SIZE != 0) {
    no_of_desc++;
  }
  remaining_bytes = total_bytes;

  // Configure channel control settings (common for all descriptors)
  // These settings are set once outside the loop for better performance
  XferCfg.chnlCtrlConfig.transType        = MEMORY_PERIPHERAL;
  XferCfg.chnlCtrlConfig.dmaFlwCtrl       = DMA_FLW_CTRL;
  XferCfg.chnlCtrlConfig.mastrIfFetchSel  = MASTER0_FETCH_IFSEL;
  XferCfg.chnlCtrlConfig.mastrIfSendSel   = MASTER0_SEND_IFSEL;
  XferCfg.chnlCtrlConfig.destDataWidth    = DST_32_DATA_WIDTH;
  XferCfg.chnlCtrlConfig.srcDataWidth     = SRC_8_DATA_WIDTH;
  XferCfg.chnlCtrlConfig.srcAlign         = 0;
  XferCfg.chnlCtrlConfig.linkListOn       = 1;
  XferCfg.chnlCtrlConfig.linkListMstrSel  = LINK_MASTER_0_FTCH;
  XferCfg.chnlCtrlConfig.srcAddContiguous = 0;
  XferCfg.chnlCtrlConfig.dstAddContiguous = 0;
  XferCfg.chnlCtrlConfig.retryOnErr       = 0;
  XferCfg.chnlCtrlConfig.linkInterrupt    = 1;
  XferCfg.chnlCtrlConfig.srcFifoMode      = 0;
  XferCfg.chnlCtrlConfig.dstFifoMode      = 1;

  // Misc Channel Config (set once for all descriptors)
  XferCfg.miscChnlCtrlConfig.ahbBurstSize  = AHBBURST_SIZE_8;
  XferCfg.miscChnlCtrlConfig.destDataBurst = DST_BURST_SIZE_8;
  XferCfg.miscChnlCtrlConfig.srcDataBurst  = SRC_BURST_SIZE_8;
  XferCfg.miscChnlCtrlConfig.destChannelId = 31;
  XferCfg.miscChnlCtrlConfig.srcChannelId  = 0;
  XferCfg.miscChnlCtrlConfig.dmaProt       = 0;
  XferCfg.miscChnlCtrlConfig.memoryFillEn  = 0;
  XferCfg.miscChnlCtrlConfig.memoryOneFill = 0;

  // Create descriptor chain - each descriptor handles up to 4095 bytes
  pPrevDesc = NULL;
  for (uint32_t j = 0; j < no_of_desc; j++) {
    // Calculate transfer size for this descriptor (either max size or remaining bytes)
    uint32_t transfer_size = (remaining_bytes > MAX_DESCRIPTOR_SIZE) ? MAX_DESCRIPTOR_SIZE : remaining_bytes;

    // Set the transfer size for this specific descriptor
    XferCfg.chnlCtrlConfig.transSize = transfer_size;

    // Set destination address (SDIO TX FIFO - same for all descriptors)
    XferCfg.dest = (uint32_t *)(TX_SOURCE_ADDR);

    // Set source address (offset into the user buffer)
    XferCfg.src = (data_buff + current_offset);

    // Build the descriptor and link it to the previous one
    if (RSI_GPDMA_BuildDescriptors(GPDMAHandle, &XferCfg, &GPDMADesc[j], pPrevDesc) != RSI_OK) {
    }

    // Update the previous descriptor pointer for the next iteration
    pPrevDesc = &GPDMADesc[j];

    // Update offset and remaining bytes for next descriptor
    current_offset += transfer_size;
    remaining_bytes -= transfer_size;
  }

  // Mark the end of the descriptor chain
  GPDMADesc[no_of_desc - 1].pNextLink = NULL;
}

/*******************************************************************************
 * @fn          void sl_si91x_setup_rx_channel_desc(uint8_t *data_buf)
 * @brief       Setup GPDMA RX channel descriptor
 * @param[in]   data_buf : Pointer to RX buffer
 * @return      None
 ******************************************************************************/
void sl_si91x_setup_rx_channel_desc(uint8_t *data_buf)
{
  RSI_GPDMA_CHA_CFG_T chaCfg;

  chaCfg.channelPrio       = 0;
  chaCfg.descFetchDoneIntr = 0;
  chaCfg.hrespErr          = 1;
  chaCfg.gpdmacErr         = 1;
  chaCfg.xferDoneIntr      = 1;
  chaCfg.dmaCh             = GPDMA_CHNL1;

  RSI_GPDMA_SetupChannel(GPDMAHandle, (RSI_GPDMA_CHA_CFG_T *)&chaCfg);

  sl_si91x_fill_rx_descriptors(data_buf);

  RSI_GPDMA_SetupChannelTransfer(GPDMAHandle, GPDMA_CHNL1, rx_GPDMADesc);
}

/*******************************************************************************
 * @fn          void sl_si91x_setup_tx_channel_desc(uint8_t *data_buf, uint8_t num_of_blocks)
 * @brief       Setup GPDMA TX channel descriptor
 * @param[in]   data_buf      : Pointer to TX buffer
 * @param[in]   num_of_blocks : Numbers of block transmit.
 * @return      None
 ******************************************************************************/
void sl_si91x_setup_tx_channel_desc(uint8_t *data_buf, uint8_t num_of_blocks)
{
  RSI_GPDMA_CHA_CFG_T chaCfg;

  chaCfg.channelPrio       = 0;
  chaCfg.descFetchDoneIntr = 0;
  chaCfg.hrespErr          = 1;
  chaCfg.gpdmacErr         = 1;
  chaCfg.xferDoneIntr      = 1;
  chaCfg.dmaCh             = GPDMA_CHNL0;

  // Setup channel 1 for trigger operation and M2M transfer
  RSI_GPDMA_SetupChannel(GPDMAHandle, (RSI_GPDMA_CHA_CFG_T *)&chaCfg);

  sl_si91x_fill_tx_descriptors(data_buf, num_of_blocks);

  RSI_GPDMA_SetupChannelTransfer(GPDMAHandle, GPDMA_CHNL0, GPDMADesc);
}

/*******************************************************************************
 * Initialize the gpdma
 ******************************************************************************/
sl_status_t sl_si91x_gpdma_init()
{
  uint32_t memSize_t, *devMem_p;
  RSI_GPDMA_INIT_T GPDMAInit;
  volatile bool done;

  // Clear stack structures before use
  memset(&GPDMAInit, 0, sizeof(RSI_GPDMA_INIT_T));

  // Get needed size for driver context memory
  memSize_t = RSI_GPDMA_GetMemSize();
  if (memSize_t > sizeof(memBuff)) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  devMem_p = memBuff;

  // Initialize driver context parameters
  GPDMAInit.pUserData = (void *)&done;
  GPDMAInit.baseC     = (uint32_t)GPDMA_C;
  GPDMAInit.baseG     = (uint32_t)GPDMA_G;
  GPDMAInit.sramBase  = (uint32_t)&GPDMADesc;

  // Initialize driver
  GPDMAHandle = RSI_GPDMA_Init(devMem_p, &GPDMAInit);
  if (GPDMAHandle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Enable GPDMA clock
  RSI_CLK_PeripheralClkEnable(M4CLK, RPDMA_CLK, ENABLE_STATIC_CLK);

  // Enable the interrupt for the DMA controller
  NVIC_EnableIRQ(GPDMA_IRQn);

  // Register error, descriptor completion, and descriptor chain completion callbacks for channel 1
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERCOMPLETE_CB, (gpdmaTransferCompleteCB)GPDMATransferComplete);
  RSI_GPDMA_RegisterCallback(GPDMAHandle,
                             RSI_GPDMA_XFERDESCFETCHCOMPLETE_CB,
                             (gpdmaTransferDescFetchCompleteCB)GPDMATransferDescFetchComplete);
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERHRESPERROR_CB, (gpdmaTransferCompleteCB)GPDMATransferError);
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERGPDMACERROR_CB, (gpdmaTransferCompleteCB)GPDMATransferError);

  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API initializes the sdio secondary and gpdma initialization and returns
 * error if any in gpdma initialization
 ******************************************************************************/
sl_status_t sl_si91x_sdio_secondary_init(void)
{
  sl_status_t status;
  // Initialize the SDIO secondary
  sl_si91x_sdio_secondary_peripheral_init();
  // Initialize the GPDMA
  status = sl_si91x_gpdma_init();

  // Return the status
  EFM_ASSERT(status == SL_STATUS_OK);
  return status;
}

/*******************************************************************************
 * To register the event callback
 * It registers the callback, i.e., stores the callback function address
 * and pass to the variable that is called in Interrupt Handler.
 * If another callback is registered without unregistering previous callback then, it
 * returns an error code, so it is mandatory to unregister the callback before registering
 * another callback.
 * It will returns error if any callback is already registered.
 ******************************************************************************/
sl_status_t sl_si91x_sdio_secondary_register_event_callback(sl_sdio_secondary_callback_t callback_event, uint32_t flag)
{
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if (callback_event == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    //Enable and set the SDIO interrupt
    sl_si91x_sdio_secondary_enable_interrupts(flag);

    sl_si91x_sdio_secondary_set_interrupts(flag);
    // To validate the function pointer if the parameters is not NULL then, it
    // returns an error code
    if (user_callback != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // User callback address is passed to the static variable which is called at the time of
    // interrupt
    user_callback = callback_event;
    // Returns SL_STATUS_OK if callback is successfully registered
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * It unregisters the callback, i.e., clear the callback function address
 * and pass NULL value to the variable
 ******************************************************************************/
void sl_si91x_sdio_secondary_unregister_event_callback(uint32_t flag)
{
  //Disbale the interrupts
  sl_si91x_sdio_secondary_disable_interrupts(flag);

  // Pass the NULL value to the static variable which is called at the time of
  // interrupt.
  // It is further validated in register callback API.
  user_callback = NULL;
}

/*******************************************************************************
 * To register the event callback
 * It registers the callback, i.e., stores the callback function address
 * and pass to the variable that is called in Interrupt Handler.
 * If another callback is registered without unregistering previous callback then, it
 * returns an error code, so it is mandatory to unregister the callback before registering
 * another callback.
 * It will returns error if any callback is already registered.
 ******************************************************************************/
sl_status_t sl_si91x_sdio_secondary_gpdma_register_event_callback(sl_sdio_secondary_gpdma_callback_t callback_event)
{
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if (callback_event == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // To validate the function pointer if the parameters is not NULL then, it
    // returns an error code
    if (user_gpdma_callback != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // User callback address is passed to the static variable which is called at the time of
    // interrupt
    user_gpdma_callback = callback_event;
    // Returns SL_STATUS_OK if callback is successfully registered
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * It unregisters the callback, i.e., clear the callback function address
 * and pass NULL value to the variable
 ******************************************************************************/
void sl_si91x_sdio_secondary_gpdma_unregister_event_callback(void)
{
  // Pass the NULL value to the static variable which is called at the time of
  // interrupt.
  // It is further validated in register callback API.
  user_gpdma_callback = NULL;
}

/*******************************************************************************
 * This API is used to receive the data from sdio primary ,this api will fill
 * the gpdma receive decscriptors and triggers the gpdma channel.
 * This API also enables the sdio fn interrupt to recive next packets
 ******************************************************************************/
void sl_si91x_sdio_secondary_receive(uint8_t *data_buf)
{
  sl_si91x_setup_rx_channel_desc(data_buf);

  // Trigger channel
  RSI_GPDMA_DMAChannelTrigger(GPDMAHandle, GPDMA_CHNL1);
  // Enable the SDIO write interrupt
  // sl_si91x_sdio_secondary_set_interrupts(SL_SDIO_WR_INT_UNMSK);
}

/*******************************************************************************
 * This API is used to request permission to send data to the host/primary device.
 * It signals the host that data is ready for transmission.
 ******************************************************************************/
void sl_si91x_sdio_secondary_request_to_send(void)
{
  // Raise interrupt to host
  M4_HOST_INTR_STATUS_REG = BIT(3);
}

/*******************************************************************************
 * This API is used to send the data from sdio seccondary ,this api will fill
 * the gpdma transfer decscriptors and triggers the gpdma channel.
 * This also enables the interrupt to sdio primary
 ******************************************************************************/
void sl_si91x_sdio_secondary_send(uint8_t num_of_blocks, uint8_t *data_buf)
{
  sl_si91x_setup_tx_channel_desc(data_buf, num_of_blocks);

  // Program number of blocks
  sl_si91x_sdio_secondary_set_tx_blocks(num_of_blocks);

  // Trigger channel
  RSI_GPDMA_DMAChannelTrigger(GPDMAHandle, GPDMA_CHNL0);
}

/*******************************************************************************
 * To get the release, sqa and dev version of SDIO SECONDARY
 * It returns the structure for gspi version.
 * Structure includes three members:
 * - Release version
 * - Major version (SQA version)
 * - Minor version (Dev version)
 ******************************************************************************/
sl_sdio_secondary_version_t sl_si91x_sdio_secondary_get_version(void)
{
  sl_sdio_secondary_version_t version;
  version.release = SDIO_SECONDARY_RELEASE_VERSION;
  version.major   = SDIO_SECONDARY_SQA_VERSION;
  version.minor   = SDIO_SECONDARY_DEV_VERSION;
  return version;
}
