/*******************************************************************************
 * @file  gpdma_example.c
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
/**============================================================================
 * @section Description
 * This files contains example code to demonstrate the GPDMA memory to memory transfer.
 ============================================================================**/

// Include Files
#include "sl_si91x_gpdma.h"
#include "rsi_debug.h"
#include <stdio.h>

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#define GPDMA_MAX_TRANSFER_LENGTH_CHANNEL0 1024 * 4 // Maximum transfer length allowed for given channel
#define GPDMA_TRANSFER_LENGTH              1024 * 4 // Transfer length in bytes
#define SL_GPDMA_SIMPLE_TRANSFER           1
#define GPDMA_CHANNEL                      0
#define SL_SI91X_GPDMA_SOURCE_BURST        16
#define SL_SI91X_GPDMA_DESTINATION_BURST   16
// Calculate the maximum number of descriptors needed
#define SL_MAX_NUMBER_OF_DESCRIPTORS_CHANNEL0 \
  ((GPDMA_MAX_TRANSFER_LENGTH_CHANNEL0 + MAX_TRANSFER_PER_DESCRIPTOR - 1) / MAX_TRANSFER_PER_DESCRIPTOR)

/*******************************************************************************
 ********************************  BUFFERS  ************************************
 ******************************************************************************/

sl_si91x_gpdma_descriptor_t
  sl_descriptors_memory[SL_MAX_NUMBER_OF_DESCRIPTORS_CHANNEL0]; // Memory for storing the descriptors
uint8_t src[GPDMA_TRANSFER_LENGTH] = { 0 };                     // Source buffer for transfer
uint8_t dst[GPDMA_TRANSFER_LENGTH] = { 0 };                     // Destination buffer for transfer
uint32_t gpdma_channel             = GPDMA_CHANNEL;             // GPDMA Channel used
/*******************************************************************************
 ***************************  GLOBAL VARIABLES  *******************************
 ******************************************************************************/
uint32_t gpdma_transfer_done  = 0; // Transfer done flag
uint32_t gpdma_start_transfer = 1; // Start transfer flag

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
/*******************************************************************************
 * Transfer callback function.
 ******************************************************************************/
static void transfer_complete_callback();

static void gpdma_controller_error_callback();

static void hresp_error_callback();

static void fetch_done_callback();

/*******************************************************************************
  **********************  Local Function definitions   ***************************
  ******************************************************************************/

/*******************************************************************************
  * Transfer callback function.
  ******************************************************************************/
static void transfer_complete_callback()
{
  DEBUGOUT("\r\nTransfer complete callback\r\n");
  gpdma_transfer_done = 1;
}

static void gpdma_controller_error_callback()
{
  DEBUGOUT("\r\nGPDMA controller error callback\r\n");
}

static void hresp_error_callback()
{
  DEBUGOUT("\r\nHRESP error callback\r\n");
}

static void fetch_done_callback()
{
  DEBUGOUT("\r\nFetch done callback\r\n");
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/***************************************************************************/ /**
 * Initialize GPDMA example.
 ******************************************************************************/
void gpdma_example_init(void)
{
  sl_status_t status                     = SL_STATUS_OK;
  uint32_t channel_priority              = 0;
  sl_gpdma_callback_pointer_t callback_t = { 0 };
#if (SL_GPDMA_SIMPLE_TRANSFER != 1)
  sl_si91x_gpdma_descriptor_config_t descriptor_config = { 0 };

  descriptor_config.chnlCtrlConfig.transSize       = (uint32_t)(MAX_TRANSFER_PER_DESCRIPTOR);
  descriptor_config.chnlCtrlConfig.transType       = SL_GPDMA_MEMORY_TO_MEMORY;        // Transfer type
  descriptor_config.chnlCtrlConfig.dmaFlwCtrl      = SL_GPDMAC_FL0W_CTRL;              // Flow control
  descriptor_config.chnlCtrlConfig.mastrIfFetchSel = SL_GPDMA_MASTER_ZERO;             //Master select for source
  descriptor_config.chnlCtrlConfig.mastrIfSendSel  = SL_GPDMA_MASTER_ONE;              // Master select for destination
  descriptor_config.chnlCtrlConfig.linkListMstrSel = SL_GPDMA_MASTER_ZERO;             //Master select for descriptor
  descriptor_config.chnlCtrlConfig.destDataWidth   = SL_GPDMA_DATA_WIDTH_32;           //Destination data width
  descriptor_config.chnlCtrlConfig.srcDataWidth    = SL_GPDMA_DATA_WIDTH_32;           //Source data width
  descriptor_config.chnlCtrlConfig.linkListOn      = SL_LINK_LIST_MODE_ENABLE;         //Linked list mode enable/disable
  descriptor_config.chnlCtrlConfig.linkInterrupt   = SL_LINK_LIST_INTERRUPT_ENABLE;    //Interupt per descriptor
  descriptor_config.chnlCtrlConfig.srcFifoMode     = SL_SOURCE_FIFO_MODE_DISABLE;      //FIFO mode for source
  descriptor_config.chnlCtrlConfig.dstFifoMode     = SL_DESTINATION_FIFO_MODE_DISABLE; //FIFO mode for destination

  descriptor_config.miscChnlCtrlConfig.ahbBurstSize  = SL_GPDMA_AHBBURST_SIZE_8;         //AHB burst size
  descriptor_config.miscChnlCtrlConfig.destDataBurst = SL_SI91X_GPDMA_DESTINATION_BURST; //Source data width
  descriptor_config.miscChnlCtrlConfig.srcDataBurst  = SL_SI91X_GPDMA_SOURCE_BURST;      //Destination data width
  descriptor_config.miscChnlCtrlConfig.dmaProt       = SL_DMA_PROTECTION_DISABLE;
  descriptor_config.miscChnlCtrlConfig.memoryFillEn  = SL_MEMORY_FILL_DISABLE; //Memory fill enable
  descriptor_config.miscChnlCtrlConfig.memoryOneFill = SL_MEMORY_ZERO_FILL;    //Memory fill zero/one
  descriptor_config.src                              = src;
  descriptor_config.dest                             = dst;
  descriptor_config.pNextLink                        = NULL;
#endif

  for (int i = 0; i < GPDMA_TRANSFER_LENGTH; i++) {
    src[i] = i; //Fill the source buffer with data
    dst[i] = 0; //Initialize the destination buffer to zero
  }

  //Call back functions, these are optional user can implement their own call back functions or give it NULL if not need
  callback_t.descripotr_fetch_complete_cb = fetch_done_callback;             // Fetch done callback
  callback_t.gpdmac_error_cb              = gpdma_controller_error_callback; // GPDMA controller error callback
  callback_t.hresp_error_cb               = hresp_error_callback;            // HRESP error callback
  callback_t.transfer_complete_cb         = transfer_complete_callback;      // Transfer complete callback

  //Allocate a channel for GPDMA transfer
  status = sl_si91x_gpdma_allocate_channel(&gpdma_channel, channel_priority, GPDMA_MAX_TRANSFER_LENGTH_CHANNEL0);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Allocate channel fail %lu \r\n", status);
  } else {
    DEBUGOUT("\r\n Allocate channel success\r\n");
  }

  //Register the callback functions for GPDMA transfer
  status = sl_si91x_gpdma_register_callbacks(gpdma_channel, &callback_t);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Register callback fail %lu \r\n", status);
  } else {
    DEBUGOUT("\r\n Register callback success\r\n");
  }

#if SL_GPDMA_SIMPLE_TRANSFER
  //In simple transfer allocate a descriptor for GPDMA transfer
  status = sl_si91x_gpdma_allocate_descriptor(sl_descriptors_memory, GPDMA_TRANSFER_LENGTH, gpdma_channel);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Descriptor allocation failed %lu \r\n", status);
  } else {
    DEBUGOUT("\r\n Descriptor allocation success\r\n");
  }

#else

  //In normal transfer build a descriptor for GPDMA transfer
  status =
    sl_si91x_gpdma_build_descriptor(sl_descriptors_memory, &descriptor_config, GPDMA_TRANSFER_LENGTH, gpdma_channel);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Build descriptor fail %lu \r\n", status);
  } else {
    DEBUGOUT("\r\n Build descriptor success\r\n");
  }

#endif
}

/***************************************************************************/ /**
 * Blinky ticking function.
 ******************************************************************************/
void gpdma_example_process_action(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t count     = 0;
  if (gpdma_start_transfer) {
    status = sl_si91x_gpdma_transfer(gpdma_channel, src, dst);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n Transfer_fail %lu \r\n", status);
    }
    gpdma_start_transfer = 0;
  }
  if (gpdma_transfer_done) {
    for (int i = 0; i < GPDMA_TRANSFER_LENGTH; i++) {
      if (src[i] != dst[i]) {
        break;
      }
      count++;
    }
    if (count == GPDMA_TRANSFER_LENGTH) {
      DEBUGOUT("\r\n GPDMA Transfer success\r\n");
    } else {
      DEBUGOUT("\r\n GPDMA Transfer fail\r\n");
    }
    gpdma_transfer_done = 0;
  }
}
