/******************************************************************************
* @file  sl_si91x_hspi_secondary.c
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_si91x_hspi_secondary.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define HSPI_SECONDARY_RELEASE_VERSION 0 // HSPI Secondary Release version
#define HSPI_SECONDARY_SQA_VERSION     0 // HSPI Secondary SQA version
#define HSPI_SECONDARY_DEV_VERSION     2 // HSPI Secondary Developer version

/*******************************************************************************
 ******************************   VARIABLES   **********************************
 ******************************************************************************/
#if (SL_HSPI_DMA == ENABLE)
#if defined(__CC_ARM)
// Keil alignment to 1024 bytes
__align(1024) RSI_GPDMA_DESC_T GPDMADesc[SL_HSPI_NUM_GPDMA_DESC];
#endif // defined (__CC_ARM)

// IAR support
#if defined(__ICCARM__)
#pragma data_alignment = 1024
static RSI_GPDMA_DESC_T GPDMADesc[SL_HSPI_NUM_GPDMA_DESC];
#endif // defined (__ICCARM__)

#if defined(__GNUC__)
RSI_GPDMA_DESC_T GPDMADesc[SL_HSPI_NUM_GPDMA_DESC];
RSI_GPDMA_DESC_T rx_GPDMADesc[SL_HSPI_NUM_GPDMA_DESC];
#endif // defined (__GNUC__)

static uint32_t hspi_memBuff[30]; //gpdma driver memory context, driver context
                                  // size is less than 30 bytes, hence size is 30
uint32_t memSize = 0, *devMem_hspi = NULL;
RSI_GPDMA_HANDLE_T GPDMAHandle;
#endif
/*******************************************************************************
 ************************       GLOBAL FUNCTIONS      **************************
 ******************************************************************************/
extern __INLINE uint32_t hspi_secondary_get_data_len(void);
extern __INLINE void hspi_secondary_clear_interrupts_status(uint32_t flags);

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

#if (SL_HSPI_DMA == ENABLE)
static sl_status_t hspi_gpdma_init();
static void fill_gpdma_rx_descriptors(void *data_buf);
static void fill_gpmda_tx_descriptors(void *data_buff);
static void setup_gpdma_rx_channel_desc(void *data_buf);
static void setup_gpdma_tx_channel_desc(void *data_buf);
static void GPDMATransferComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh);
#endif
/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
#if (SL_HSPI_DMA == ENABLE)
static sl_hspi_secondary_gpdma_callback_t user_gpdma_callback =
  NULL; // HSPI Secondary user GPDMA callback function variable
RSI_GPDMA_DESC_T rx_XferCfg;
volatile RSI_GPDMA_DESC_T XferCfg;
#endif
static sl_hspi_secondary_callback_t sl_hspi_user_callback = NULL; // hspi Secondary user callback function variable
/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/**
 * HSPI Interrupt Handler. Clears HSPI primary interrupt, sets the event, and
 *         calls the user callback function for user-level handling.
 */
void SL_HSPI_IRQHandler(void)
{
  uint8_t events = 0;
  events         = hspi_secondary_get_pending_interrupts();
  hspi_secondary_clear_interrupts_status(events);
  sl_hspi_user_callback(events);
}

#if (SL_HSPI_DMA == ENABLE)
/**
 * GPDMA Interrupt Handler. Clears the dma interrupt triggered for the
 *         HSPI channel and calls the user callback handler.
 */
void SL_GPDMA_IRQHandler(void)
{
  RSI_GPDMA_InterruptHandler(GPDMAHandle);
}

/**
 * GPDMA controller transfer descriptor chain complete callback.
 */
static void GPDMATransferComplete(RSI_GPDMA_HANDLE_T GPDMAHandle_p, RSI_GPDMA_DESC_T *pTranDesc, uint32_t dmaCh)
{
  (void)pTranDesc;
  (void)GPDMAHandle_p;
  user_gpdma_callback((uint8_t)dmaCh);
}

/**
 * Fills RX GPDMA descriptors.
 */
static void fill_gpdma_rx_descriptors(void *data_buf)
{
  uint32_t j;
  RSI_GPDMA_DESC_T *pPrevDesc;
  uint32_t no_of_desc = 0;

  no_of_desc = SL_HSPI_RX_BUFFER_SIZE
               / SL_HSPI_DMA_BLOCK_SIZE; // TODO: Given Statically, need to implement dynamic data length configurations

  pPrevDesc = NULL;
  for (j = 0; j < no_of_desc; j++) {

    // Channel Control Config
    rx_XferCfg.chnlCtrlConfig.transSize        = SL_HSPI_DMA_BLOCK_SIZE;
    rx_XferCfg.chnlCtrlConfig.transType        = PERIPHERAL_MEMORY;
    rx_XferCfg.chnlCtrlConfig.dmaFlwCtrl       = DMA_FLW_CTRL;
    rx_XferCfg.chnlCtrlConfig.mastrIfFetchSel  = MASTER0_FETCH_IFSEL;
    rx_XferCfg.chnlCtrlConfig.mastrIfSendSel   = MASTER0_SEND_IFSEL;
    rx_XferCfg.chnlCtrlConfig.destDataWidth    = DST_8_DATA_WIDTH;
    rx_XferCfg.chnlCtrlConfig.srcDataWidth     = SRC_32_DATA_WIDTH;
    rx_XferCfg.chnlCtrlConfig.srcAlign         = 0;
    rx_XferCfg.chnlCtrlConfig.linkListOn       = 1;
    rx_XferCfg.chnlCtrlConfig.linkListMstrSel  = LINK_MASTER_0_FTCH;
    rx_XferCfg.chnlCtrlConfig.srcAddContiguous = 0;
    rx_XferCfg.chnlCtrlConfig.dstAddContiguous = 1;
    rx_XferCfg.chnlCtrlConfig.retryOnErr       = 1;
    rx_XferCfg.chnlCtrlConfig.linkInterrupt    = 1;
    rx_XferCfg.chnlCtrlConfig.srcFifoMode      = 1;
    rx_XferCfg.chnlCtrlConfig.dstFifoMode      = 0;

    // Misc Channel Config
    rx_XferCfg.miscChnlCtrlConfig.ahbBurstSize  = AHBBURST_SIZE_16;
    rx_XferCfg.miscChnlCtrlConfig.destDataBurst = DST_BURST_SIZE_16;
    rx_XferCfg.miscChnlCtrlConfig.srcDataBurst  = SRC_BURST_SIZE_16;
    rx_XferCfg.miscChnlCtrlConfig.destChannelId = 0;
    rx_XferCfg.miscChnlCtrlConfig.srcChannelId  = 30;
    rx_XferCfg.miscChnlCtrlConfig.dmaProt       = 0;
    rx_XferCfg.miscChnlCtrlConfig.memoryFillEn  = 0;
    rx_XferCfg.miscChnlCtrlConfig.memoryOneFill = 0;

    rx_XferCfg.dest = (data_buf + (j * SL_HSPI_DMA_BLOCK_SIZE));
    rx_XferCfg.src  = (uint32_t *)(SL_HSPI_RX_SOURCE_ADDR);
    if (RSI_GPDMA_BuildDescriptors(GPDMAHandle, &rx_XferCfg, &rx_GPDMADesc[j], pPrevDesc) != RSI_OK) {
    }
    pPrevDesc = &rx_GPDMADesc[j];
  }
  // Assigning pointer to next descriptor link in a chain with NULL
  rx_GPDMADesc[no_of_desc - 1].pNextLink = NULL;
}

/**
 * Fills TX GPDMA descriptors.
 */
static void fill_gpmda_tx_descriptors(void *data_buff)
{
  uint32_t j;
  RSI_GPDMA_DESC_T *pPrevDesc;
  uint32_t no_of_desc = 0;
  RSI_GPDMA_DESC_T XferCfg;

  no_of_desc = SL_HSPI_TX_BUFFER_SIZE
               / SL_HSPI_DMA_BLOCK_SIZE; // TODO: Given Statically, need to implement dynamic data length configurations

  pPrevDesc = NULL;
  for (j = 0; j < no_of_desc; j++) {
    // Channel Control Config
    XferCfg.chnlCtrlConfig.transSize        = SL_HSPI_DMA_BLOCK_SIZE;
    XferCfg.chnlCtrlConfig.transType        = MEMORY_PERIPHERAL;
    XferCfg.chnlCtrlConfig.dmaFlwCtrl       = DMA_FLW_CTRL;
    XferCfg.chnlCtrlConfig.mastrIfFetchSel  = MASTER0_FETCH_IFSEL;
    XferCfg.chnlCtrlConfig.mastrIfSendSel   = MASTER0_SEND_IFSEL;
    XferCfg.chnlCtrlConfig.destDataWidth    = DST_32_DATA_WIDTH;
    XferCfg.chnlCtrlConfig.srcDataWidth     = SRC_8_DATA_WIDTH;
    XferCfg.chnlCtrlConfig.srcAlign         = 0;
    XferCfg.chnlCtrlConfig.linkListOn       = 1;
    XferCfg.chnlCtrlConfig.linkListMstrSel  = LINK_MASTER_0_FTCH;
    XferCfg.chnlCtrlConfig.srcAddContiguous = 1;
    XferCfg.chnlCtrlConfig.dstAddContiguous = 0;
    XferCfg.chnlCtrlConfig.retryOnErr       = 0;
    XferCfg.chnlCtrlConfig.linkInterrupt    = 1;
    XferCfg.chnlCtrlConfig.srcFifoMode      = 0;
    XferCfg.chnlCtrlConfig.dstFifoMode      = 1;

    // Misc Channel Config
    XferCfg.miscChnlCtrlConfig.ahbBurstSize  = AHBBURST_SIZE_16;
    XferCfg.miscChnlCtrlConfig.destDataBurst = DST_BURST_SIZE_16;
    XferCfg.miscChnlCtrlConfig.srcDataBurst  = SRC_BURST_SIZE_16;
    XferCfg.miscChnlCtrlConfig.destChannelId = 31;
    XferCfg.miscChnlCtrlConfig.srcChannelId  = 0;
    XferCfg.miscChnlCtrlConfig.dmaProt       = 0;
    XferCfg.miscChnlCtrlConfig.memoryFillEn  = 0;
    XferCfg.miscChnlCtrlConfig.memoryOneFill = 0;

    XferCfg.dest = (uint32_t *)(SL_HSPI_TX_SOURCE_ADDR);
    XferCfg.src  = (data_buff + (j * SL_HSPI_DMA_BLOCK_SIZE));
    // Build the descriptor parameters
    if (RSI_GPDMA_BuildDescriptors(GPDMAHandle, &XferCfg, &GPDMADesc[j], pPrevDesc) != RSI_OK) {
    }
    pPrevDesc = &GPDMADesc[j];
  }
  // Assigning pointer to next descriptor link in a chain with NULL
  GPDMADesc[no_of_desc - 1].pNextLink = NULL;
}

/**
 * Sets up GPDMA RX channel descriptor.
 */
static void setup_gpdma_rx_channel_desc(void *data_buf)
{
  RSI_GPDMA_CHA_CFG_T chaCfg;

  chaCfg.channelPrio       = 0;
  chaCfg.descFetchDoneIntr = 0;
  chaCfg.hrespErr          = 1;
  chaCfg.gpdmacErr         = 1;
  chaCfg.xferDoneIntr      = 1;
  chaCfg.dmaCh             = GPDMA_CHNL1;

  RSI_GPDMA_SetupChannel(GPDMAHandle, (RSI_GPDMA_CHA_CFG_T *)&chaCfg);

  fill_gpdma_rx_descriptors(data_buf);

  RSI_GPDMA_SetupChannelTransfer(GPDMAHandle, GPDMA_CHNL1, rx_GPDMADesc);
}

/**
 * Sets up GPDMA TX channel descriptor.
 */
static void setup_gpdma_tx_channel_desc(void *data_buf)
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

  fill_gpmda_tx_descriptors(data_buf);

  RSI_GPDMA_SetupChannelTransfer(GPDMAHandle, GPDMA_CHNL0, GPDMADesc);
}

/**
 * Initializes the GPDMA for HSPI secondary.
 */
static sl_status_t hspi_gpdma_init(void)
{
  uint32_t hspi_memSize, *devMem_p;
  RSI_GPDMA_INIT_T GPDMAInit;
  volatile bool done = false;

  // Clear stack structures before use
  memset(&GPDMAInit, 0, sizeof(RSI_GPDMA_INIT_T));

  // Get needed size for driver context memory
  hspi_memSize = RSI_GPDMA_GetMemSize();
  if (hspi_memSize > sizeof(hspi_memBuff)) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  devMem_p = hspi_memBuff;

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
  return SL_STATUS_OK;
}

/**
 * Receives data from the SPI primary in non-blocking mode.
 *         This API fills the GPDMA receive descriptors and triggers the GPDMA channel.
 */
sl_status_t sl_si91x_hspi_secondary_receive_non_blocking(void *data_buf)
{
  sl_status_t status = SL_STATUS_OK;
  // Validate pointers, if the parameter is NULL, return an error code
  if (data_buf == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  setup_gpdma_rx_channel_desc(data_buf);

  // Trigger channel
  status = RSI_GPDMA_DMAChannelTrigger(GPDMAHandle, GPDMA_CHNL1);

  return status;
}

/**
 * Sends data from the HSPI secondary in non-blocking mode.
 *         This API fills the GPDMA transfer descriptors and triggers the GPDMA channel.
 */
sl_status_t sl_si91x_hspi_secondary_send_non_blocking(void *data_buf)
{
  sl_status_t status = SL_STATUS_OK;
  // Validate pointers, if the parameter is NULL, return an error code
  if (data_buf == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  setup_gpdma_tx_channel_desc(data_buf);

  // Trigger channel
  status = RSI_GPDMA_DMAChannelTrigger(GPDMAHandle, GPDMA_CHNL0);

  return status;
}
#endif

/**
 * Initializes the HSPI secondary and GPDMA.
 *         Returns an error if there is an issue with GPDMA initialization.
 */
sl_status_t sl_si91x_hspi_secondary_init(void)
{
  sl_status_t status = SL_STATUS_OK;
  // Initialize the hspi secondary
  hspi_secondary_peripheral_init();

  //Enable and set the hspi interrupt
  hspi_secondary_enable_interrupts(SL_HSPI_INT_WR | SL_HSPI_INT_RD);

#if (SL_HSPI_DMA == ENABLE)
  // Initialize the GPDMA
  status = hspi_gpdma_init();
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif
  return status;
}

/**
 * De-Initializes the HSPI secondary.
 *         Returns an error if there is an issue with GPDMA De-initialization.
 */
sl_status_t sl_si91x_hspi_secondary_deinit(void)
{
  // Initialize the hspi secondary
  hspi_secondary_peripheral_deinit();

  //Disbale the interrupts
  hspi_secondary_disable_interrupts(SL_HSPI_INT_WR | SL_HSPI_INT_RD);
#if (SL_HSPI_DMA == DISABLE)
  // Validate the function pointer, if the parameter is not NULL, then unregister the call back
  if (sl_hspi_user_callback == NULL)
#endif
#if (SL_HSPI_DMA == ENABLE)
    // Validate the function pointer, if the parameter is not NULL, return an error code
    if (user_gpdma_callback == NULL)
#endif
    {
      return SL_STATUS_NULL_POINTER;
    } else {
      //  Unregisters the callback, i.e., clear the callback function address
      //  and pass NULL value to the variable
      sl_si91x_hspi_secondary_unregister_event_callback();
    }
  return SL_STATUS_OK;
}

/**
 * Registers the event callback.
 *         Stores the callback function address and passes it to the variable that is called in the Interrupt Handler.
 *         If another callback is registered without unregistering the previous callback, it returns an error code.
 */
sl_status_t sl_si91x_hspi_secondary_register_event_callback(sl_hspi_secondary_callback_t callback_event)
{
  sl_status_t status;
  do {
    // Validate the null pointer, if true return an error code
    if (callback_event == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
#if (SL_HSPI_DMA == DISABLE)
    // Enable IRQs for user callbacks for blocking calls
    NVIC_EnableIRQ(HIF0_IRQn);
#endif
#if (SL_HSPI_DMA == DISABLE)
    // Validate the function pointer, if the parameter is not NULL, return an error code
    if (sl_hspi_user_callback != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // User callback address is passed to the static variable which is called at the time of interrupt
    sl_hspi_user_callback = callback_event;
#endif
#if (SL_HSPI_DMA == ENABLE)
    // Validate the function pointer, if the parameter is not NULL, return an error code
    if (user_gpdma_callback != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // Enable user callbacks for non-blocking calls
    user_gpdma_callback = callback_event;
#endif
    // Return SL_STATUS_OK if the callback is successfully registered
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/**
 * Unregisters the callback, i.e., clear the callback function address
 * and pass NULL value to the variable
 */
void sl_si91x_hspi_secondary_unregister_event_callback(void)
{
#if (SL_HSPI_DMA == DISABLE)
  // Pass the NULL value to the static variable which is called at the time of
  // interrupt.
  // It is further validated in register callback API.
  sl_hspi_user_callback = NULL;
#endif
#if (SL_HSPI_DMA == ENABLE)
  /*Enable user call backs for non blocking calls */
  user_gpdma_callback = NULL;
#endif
}

/**
 * Writes the data into HSPI FIFO register
 */
uint32_t sl_si91x_hspi_secondary_send_blocking(const void *data)
{
  uint32_t *data_32 = (uint32_t *)data;
  uint32_t sent = 0, sent_success = 0;
  uint32_t num = 0;

  // Validate pointers, if the parameter is NULL, return an error code
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  num = hspi_secondary_get_data_len();

  /* Align to 4 bytes */
  num = ((num + SL_HSPI_ALIGNMENT_VALUE) & ~SL_HSPI_ALIGNMENT_VALUE);

  /* block until send done, get stuck if send fails */
  while (num > 0) {
    sent = hspi_secondary_write_fifo(data_32, num);
    num -= sent;
    sent_success += sent;
    data_32 += sent;
  }
  return sent_success;
}
/*******************************************************************************
 * This API receives the data from SPI FIFO register and waits for the data.It is a
 * blocking API and waits for the data if not available in FIFO register
 ******************************************************************************/
uint32_t sl_si91x_hspi_secondary_receive_blocking(void *data)
{
  uint32_t *data_32 = (uint32_t *)data;
  uint32_t recv = 0, recv_success = 0;
  uint32_t num = 0;
  // Validate pointers, if the parameter is NULL, return an error code
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  num = hspi_secondary_get_data_len();
  /* Align to 4 bytes */
  num = ((num + SL_HSPI_ALIGNMENT_VALUE) & ~SL_HSPI_ALIGNMENT_VALUE);

  /* block until receive done, get stuck if receive fails */
  while (num > 0) {
    recv = hspi_secondary_read_fifo(data_32, num);
    num -= recv;
    recv_success += recv;
    data_32 += (recv / SL_HSPI_WORD_SIZE);
  }
  return recv_success;
}

/*******************************************************************************
 * To get the release, sqa and dev version of hspi SECONDARY
 * It returns the structure for gspi version.
 * Structure includes three members:
 * - Release version
 * - Major version (SQA version)
 * - Minor version (Dev version)
 ******************************************************************************/
sl_hspi_secondary_version_t sl_si91x_hspi_secondary_get_version(void)
{
  sl_hspi_secondary_version_t version;
  version.release = HSPI_SECONDARY_RELEASE_VERSION;
  version.major   = HSPI_SECONDARY_SQA_VERSION;
  version.minor   = HSPI_SECONDARY_DEV_VERSION;
  return version;
}
