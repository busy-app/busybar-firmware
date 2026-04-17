/******************************************************************************
* @file  sl_si91x_sdio_slave.c
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

#include "sl_si91x_sdio_slave.h"
#include "sl_si91x_sdio_slave_drv_config.h"

/*******************************************************************************
 ******************************   VARIABLES   **********************************
 ******************************************************************************/

volatile uint8_t dmaDone = 0;

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
#endif // defined (__GNUC__)

static uint32_t memBuff[30];
uint32_t memSize = 0, *devMem = NULL;
RSI_GPDMA_HANDLE_T GPDMAHandle;

// function pointer for callback function
sl_sdio_slave_callback_t cb_event;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void GPDMA_TX_Trigger(uint8_t num_of_blocks, uint8_t *data_buf);
static void GPDMA_RX_Trigger(uint8_t *data_buf);
static void Fill_RxDescriptors(uint8_t *data_buf);
static void Fill_TxDescriptors(uint8_t *data_buff, uint8_t num_of_blocks);
static void Setup_RX_ChannelDesc(uint8_t *data_buf);
static void Setup_TX_ChannelDesc(uint8_t *data_buf, uint8_t num_of_blocks);

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/**
 * @fn     void IRQ031_Handler(void)
 * @brief  GPDMA Interrupt Handler
 * @param  None
 * @return None
 */
void IRQ031_Handler(void)
{
  dmaDone = 1;
  RSI_GPDMA_InterruptHandler(GPDMAHandle);
}

void HIF1_IRQHandler(void)
{
  uint8_t events = 0;

  // Mask command53 write interrupt
  SDIO_FN1_INT_MASK_REG = INT_EN_CMD53_WR;

  // Clear command53 write interrupt
  SDIO_FN1_INT_CLEAR = INT_STAT_CMD53_WR;

  events |= HOST_INTR_RECEIVE_EVENT;

  (cb_event)(events);
}

/**
 * @fn          void GPDMATransferComplete(RSI_GPDMA_HANDLE_T GPDMAHandle, RSI_GPDMA_DESC_T *pTranDesc)
 * @brief       GPDMA controller transfer descriptor chain complete callback
 * @param[in]   GPDMAHandle : structure variable to driver context handle
 * @param[in]   pTranDesc   : Pointer to transfer descriptor
 * @return      None
 */
void GPDMATransferComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  (void)dmaCh;
}

/**
 * @fn          void GPDMATransferDescComplete(RSI_GPDMA_HANDLE_T GPDMAHandle, RSI_GPDMA_DESC_T *pTranDesc)
 * @brief       GPDMA controller transfer descriptor complete callback
 * @param[in]   GPDMAHandle: structure variable to driver context handle
 * @param[in]   pTranDesc  : Pointer to transfer descriptor
 * @return      None
 */
void GPDMATransferDescComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  (void)dmaCh;
}

/**
 * @fn          void GPDMATransferError(RSI_GPDMA_HANDLE_T GPDMAHandle, RSI_GPDMA_DESC_T *pTranDesc)
 * @brief       GPDMA controller transfer descriptor error callback
 * @param[in]   GPDMAHandle : structure variable to driver context handle
 * @param[in]   pTranDesc   : Pointer to transfer descriptor
 * @return      None
 */
void GPDMATransferError(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  (void)dmaCh;
}

/**
 * @fn          void GPDMATransferDescFetchComplete(RSI_GPDMA_HANDLE_T GPDMAHandle, RSI_GPDMA_DESC_T *pTranDesc)
 * @brief       DMA controller transfer descriptor error callback
 * @param[in]   GPDMAHandle: structure variable to driver context handle
 * @param[in]   pTranDesc  : Pointer to transfer descriptor
 * @return      None
 */
void GPDMATransferDescFetchComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  (void)dmaCh;
}

/**
 * @fn          void Fill_RxDescriptors(uint8_t *data_buf)
 * @brief       Filled RX GPDMA descriptor
 * @param[in]   data_buf : Pointer to RX buffer 
 * @return      None
 */
void Fill_RxDescriptors(uint8_t *data_buf)
{
  uint32_t j;
  RSI_GPDMA_DESC_T *pPrevDesc;
  uint32_t no_of_desc = 0;
  RSI_GPDMA_DESC_T XferCfg;

  no_of_desc = SDIO_BLK_CNT;

  pPrevDesc = NULL;
  for (j = 0; j < no_of_desc; j++) {

    // Channel Control Config
    XferCfg.chnlCtrlConfig.transSize        = SDIO_BLK_LEN_REG;
    XferCfg.chnlCtrlConfig.transType        = PERIPHERAL_MEMORY;
    XferCfg.chnlCtrlConfig.dmaFlwCtrl       = DMA_FLW_CTRL;
    XferCfg.chnlCtrlConfig.mastrIfFetchSel  = MASTER0_FETCH_IFSEL;
    XferCfg.chnlCtrlConfig.mastrIfSendSel   = MASTER0_SEND_IFSEL;
    XferCfg.chnlCtrlConfig.destDataWidth    = DST_8_DATA_WIDTH;
    XferCfg.chnlCtrlConfig.srcDataWidth     = SRC_32_DATA_WIDTH;
    XferCfg.chnlCtrlConfig.srcAlign         = 0;
    XferCfg.chnlCtrlConfig.linkListOn       = 1;
    XferCfg.chnlCtrlConfig.linkListMstrSel  = LINK_MASTER_0_FTCH;
    XferCfg.chnlCtrlConfig.srcAddContiguous = 0;
    XferCfg.chnlCtrlConfig.dstAddContiguous = 0;
    XferCfg.chnlCtrlConfig.retryOnErr       = 0;
    XferCfg.chnlCtrlConfig.linkInterrupt    = 0;
    XferCfg.chnlCtrlConfig.srcFifoMode      = 1;
    XferCfg.chnlCtrlConfig.dstFifoMode      = 0;

    // Misc Channel Config
    XferCfg.miscChnlCtrlConfig.ahbBurstSize  = AHBBURST_SIZE_4;
    XferCfg.miscChnlCtrlConfig.destDataBurst = DST_BURST_SIZE_4;
    XferCfg.miscChnlCtrlConfig.srcDataBurst  = SRC_BURST_SIZE_4;
    XferCfg.miscChnlCtrlConfig.destChannelId = 0;
    XferCfg.miscChnlCtrlConfig.srcChannelId  = 30;
    XferCfg.miscChnlCtrlConfig.dmaProt       = 0;
    XferCfg.miscChnlCtrlConfig.memoryFillEn  = 0;
    XferCfg.miscChnlCtrlConfig.memoryOneFill = 0;

    XferCfg.dest = (data_buf + (j * SDIO_BLK_LEN_REG));
    XferCfg.src  = (uint32_t *)(RX_SOURCE_ADDR);
    if (RSI_GPDMA_BuildDescriptors(GPDMAHandle, &XferCfg, &GPDMADesc[j], pPrevDesc) != RSI_OK) {
    }
    pPrevDesc = &GPDMADesc[j];
  }
}

/**
 * @fn          void Fill_TxDescriptors(uint8_t *data_buff, uint8_t num_of_blocks)
 * @brief       Filled TX GPDMA descriptor
 * @param[in]   data_buf      : Pointer to TX buffer 
 * @param[in]   num_of_blocks : Numbers of block transmit. 
 * @return      None
 */
void Fill_TxDescriptors(uint8_t *data_buff, uint8_t num_of_blocks)
{
  uint32_t j;
  RSI_GPDMA_DESC_T *pPrevDesc;
  uint32_t no_of_desc = 0;
  RSI_GPDMA_DESC_T XferCfg;

  no_of_desc = num_of_blocks;

  pPrevDesc = NULL;
  for (j = 0; j < no_of_desc; j++) {
    // Channel Control Config
    XferCfg.chnlCtrlConfig.transSize        = TX_TRANSFER_BLOCK_SIZE;
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

    // Misc Channel Config
    XferCfg.miscChnlCtrlConfig.ahbBurstSize  = AHBBURST_SIZE_8;
    XferCfg.miscChnlCtrlConfig.destDataBurst = DST_BURST_SIZE_8;
    XferCfg.miscChnlCtrlConfig.srcDataBurst  = SRC_BURST_SIZE_8;
    XferCfg.miscChnlCtrlConfig.destChannelId = 31;
    XferCfg.miscChnlCtrlConfig.srcChannelId  = 0;
    XferCfg.miscChnlCtrlConfig.dmaProt       = 0;
    XferCfg.miscChnlCtrlConfig.memoryFillEn  = 0;
    XferCfg.miscChnlCtrlConfig.memoryOneFill = 0;

    XferCfg.dest = (uint32_t *)(TX_SOURCE_ADDR);
    XferCfg.src  = (data_buff + (j * TX_TRANSFER_BLOCK_SIZE));

    if (RSI_GPDMA_BuildDescriptors(GPDMAHandle, &XferCfg, &GPDMADesc[j], pPrevDesc) != RSI_OK) {
    }
    pPrevDesc = &GPDMADesc[j];
  }
}

/**
 * @fn          void Setup_RX_ChannelDesc(uint8_t *data_buf)
 * @brief       Setup GPDMA RX channel descriptor 
 * @param[in]   data_buf : Pointer to RX buffer 
 * @return      None
 */
void Setup_RX_ChannelDesc(uint8_t *data_buf)
{
  RSI_GPDMA_CHA_CFG_T chaCfg;

  chaCfg.channelPrio       = 0;
  chaCfg.descFetchDoneIntr = 0;
  chaCfg.hrespErr          = 1;
  chaCfg.gpdmacErr         = 1;
  chaCfg.xferDoneIntr      = 1;
  chaCfg.dmaCh             = 1;

  // Setup channel 0 for trigger operation and M2M transfer
  if (RSI_GPDMA_SetupChannel(GPDMAHandle, (RSI_GPDMA_CHA_CFG_T *)&chaCfg) != RSI_OK) {
    //DEBUGOUT("Error setting up channel\r\n");
  }
  Fill_RxDescriptors(data_buf);

  RSI_GPDMA_SetupChannelTransfer(GPDMAHandle, GPDMA_CHNL1, GPDMADesc);
}

/**
 * @fn          void Setup_TX_ChannelDesc(uint8_t *data_buf, uint8_t num_of_blocks)
 * @brief       Setup GPDMA TX channel descriptor 
 * @param[in]   data_buf      : Pointer to TX buffer 
 * @param[in]   num_of_blocks : Numbers of block transmit. 
 * @return      None
 */
void Setup_TX_ChannelDesc(uint8_t *data_buf, uint8_t num_of_blocks)
{
  RSI_GPDMA_CHA_CFG_T chaCfg;

  chaCfg.channelPrio       = 0;
  chaCfg.descFetchDoneIntr = 0;
  chaCfg.hrespErr          = 1;
  chaCfg.gpdmacErr         = 1;
  chaCfg.xferDoneIntr      = 1;
  chaCfg.dmaCh             = 1;

  // Setup channel 0 for trigger operation and M2M transfer
  if (RSI_GPDMA_SetupChannel(GPDMAHandle, (RSI_GPDMA_CHA_CFG_T *)&chaCfg) != RSI_OK) {
    // DEBUGOUT("Error setting up channel\r\n");
  }

  Fill_TxDescriptors(data_buf, num_of_blocks);

  RSI_GPDMA_SetupChannelTransfer(GPDMAHandle, 1, GPDMADesc);
}

/**
 * @fn          void GPDMA_TX_Trigger(uint8_t num_of_blocks, uint8_t *data_buf)
 * @brief       Trigger GPDMA TX channel 
 * @param[in]   data_buf      : Pointer to TX buffer 
 * @param[in]   num_of_blocks : Numbers of block transmit. 
 * @return      None
 */
void GPDMA_TX_Trigger(uint8_t num_of_blocks, uint8_t *data_buf)
{
  RSI_GPDMA_INIT_T GPDMAInit;
  volatile bool done;
  // Configures the system default clock and power configurations

  dmaDone = false;

  // clear stack structures before use
  memset(&GPDMAInit, 0, sizeof(RSI_GPDMA_INIT_T));

  // Get needed size for driver context memory
  memSize = RSI_GPDMA_GetMemSize();

  if (memSize > sizeof(memBuff)) {
    // DEBUGOUT("Can't allocate memory for driver context\r\n");
  }

  devMem = memBuff;

  // Initialize driver context parameters
  GPDMAInit.pUserData = (void *)&done;
  GPDMAInit.baseC     = (uint32_t)GPDMA_C;
  GPDMAInit.baseG     = (uint32_t)GPDMA_G;
  GPDMAInit.sramBase  = (uint32_t)&GPDMADesc;

  // Initialize GPDMA driver
  GPDMAHandle = RSI_GPDMA_Init(devMem, &GPDMAInit);
  if (GPDMAHandle == NULL) {
    //DEBUGOUT("Error initializing GPDMA\r\n");
  }

  // Enable GPDMA clock enable
  RSI_CLK_PeripheralClkEnable(M4CLK, RPDMA_CLK, ENABLE_STATIC_CLK);

  // Enable the interrupt for the DMA controller
  NVIC_EnableIRQ(GPDMA_IRQn);

  // Register error, descriptor completion, and descriptor chain completion callbacks for channel 0
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERCOMPLETE_CB, (gpdmaTransferCompleteCB)GPDMATransferComplete);
  RSI_GPDMA_RegisterCallback(GPDMAHandle,
                             RSI_GPDMA_XFERDESCFETCHCOMPLETE_CB,
                             (gpdmaTransferDescFetchCompleteCB)GPDMATransferDescFetchComplete);
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERHRESPERROR_CB, (gpdmaTransferCompleteCB)GPDMATransferError);
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERGPDMACERROR_CB, (gpdmaTransferCompleteCB)GPDMATransferError);

  Setup_TX_ChannelDesc(data_buf, num_of_blocks);

  // Program number of blocks
  RX_NUM_CHUNKS = num_of_blocks;

  // Raise interrupt to host
  M4_HOST_INTR_STATUS_REG = BIT(3);

  // Trigger channel
  RSI_GPDMA_DMAChannelTrigger(GPDMAHandle, 1);
}

/**
 * @fn          void GPDMA_RX_Trigger(uint8_t *data_buf)
 * @brief       Trigger GPDMA RX channel 
 * @param[in]   data_buf      : Pointer to TX buffer 
 * @param[in]   num_of_blocks : Numbers of block transmit. 
 * @return      None
 */
void GPDMA_RX_Trigger(uint8_t *data_buf)
{
  uint32_t memSize_t, *devMem_p;
  RSI_GPDMA_INIT_T GPDMAInit;
  volatile bool done;

  dmaDone = false;

  // Clear stack structures before use
  memset(&GPDMAInit, 0, sizeof(RSI_GPDMA_INIT_T));

  // Get needed size for driver context memory
  memSize_t = RSI_GPDMA_GetMemSize();
  if (memSize_t > sizeof(memBuff)) {
    //DEBUGOUT("Can't allocate memory for driver context\r\n");
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
    //DEBUGOUT("Error initializing GPDMA\r\n");
  }

  // Enable GPDMA clock
  RSI_CLK_PeripheralClkEnable(M4CLK, RPDMA_CLK, ENABLE_STATIC_CLK);

  // Enable the interrupt for the DMA controller
  NVIC_EnableIRQ(GPDMA_IRQn);

  // Register error, descriptor completion, and descriptor chain completion callbacks for channel 0
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERCOMPLETE_CB, (gpdmaTransferCompleteCB)GPDMATransferComplete);
  RSI_GPDMA_RegisterCallback(GPDMAHandle,
                             RSI_GPDMA_XFERDESCFETCHCOMPLETE_CB,
                             (gpdmaTransferDescFetchCompleteCB)GPDMATransferDescFetchComplete);
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERHRESPERROR_CB, (gpdmaTransferCompleteCB)GPDMATransferError);
  RSI_GPDMA_RegisterCallback(GPDMAHandle, RSI_GPDMA_XFERGPDMACERROR_CB, (gpdmaTransferCompleteCB)GPDMATransferError);

  Setup_RX_ChannelDesc(data_buf);

  // Trigger channel
  RSI_GPDMA_DMAChannelTrigger(GPDMAHandle, GPDMA_CHNL1);
}

void sl_si91x_sdio_slave_init(sl_sdio_slave_callback_t event_callback)
{
  // Clearing bit to give SDIO pin access to M4
  (*(volatile uint32_t *)(NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR)) = (0x1 << 5);

  // Select SDIO mode
  MISC_CFG_HOST_CTRL |= SDIO_MODE_SELECT;

  //    for (int i = 0; i < 10; i++);

  while ((MISC_CFG_HOST_CTRL & SDIO_MODE_SELECT) != SDIO_MODE_SELECT)
    ;

  // Enable command53 write interrupt
  SDIO_FN1_INT_ENABLE = BIT(0);

  // Unmask command53 write interrupt
  SDIO_FN1_INT_UNMASK_REG = BIT(0);

  // Enable NVIC
  NVIC_EnableIRQ(HIF0_IRQn);

  // Mask the host interrupts
  M4_HOST_INTR_MASK_REG = MASK_HOST_INTERRUPT;

  cb_event = event_callback;
}

void sl_si91x_sdio_slave_receive(uint8_t *data_buf)
{
  GPDMA_RX_Trigger(data_buf);

  // Wait for dma done
  while (dmaDone == 0)
    ;
  dmaDone = 0;

  // Unmask command53 write interrupt
  SDIO_FN1_INT_UNMASK_REG = BIT(0);
}

void sl_si91x_sdio_slave_send(uint8_t num_of_blocks, uint8_t *data_buf)
{
  GPDMA_TX_Trigger(num_of_blocks, data_buf);

  // Wait for dma done
  while (dmaDone == 0)
    ;
  dmaDone = 0;
}