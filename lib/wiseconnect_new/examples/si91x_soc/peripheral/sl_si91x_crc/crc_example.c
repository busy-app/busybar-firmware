/*******************************************************************************
* @file crc_example.c
* @brief CRC example
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

/* Includes ------------------------------------------------------------------*/
#include "sw_crc.h"
#include "rsi_debug.h"
#include "sl_si91x_crc.h"
#include "sl_si91x_dma.h"
#include "crc_example.h"

/*******************************************************************************
***************************  Defines / Macros  ********************************
******************************************************************************/
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
/**
 * @brief Input data buffer for CRC calculation.
 */
uint32_t gcrc_tx_Buf[BUFFSIZE] = { 0x6e95c16f, 0x26032526, 0x453ea633, 0x33301c04, 0xa531e3de, 0xb040d130, 0xabac09c0,
                                   0x539d6754, 0x3036bedf, 0xa2c998df, 0xa7d3bced, 0xb93e262e, 0xd096b2a7, 0xf2b32b1c,
                                   0x5192ff8d, 0x6dc046e6, 0x2d3256fe, 0x73ddaa7c, 0x17e73ab6, 0xc3ca122a, 0xb03f5ba4,
                                   0xb34e87a3, 0x9854db27, 0x3c5c176a, 0x67a367ac };

uint32_t gtx_temp_buff[BUFFSIZE];                    // Temporary buffer used for CRC calculation.
sl_crc_params_t crc_params;                          // CRC parameters structure.
uint32_t ghw_crc;                                    // Hardware CRC value.
sl_dma_xfer_t dma_transfer_t       = { 0 };          // DMA transfer structure.
uint32_t gchannel                  = SL_DMA_CHANNEL; // DMA channel number.
volatile uint8_t dma_transfer_done = 0;              // Flag indicating DMA transfer completion.
unsigned int gsw_crc;                                // software CRC value.

/* Private function prototypes -----------------------------------------------*/
static uint32_t crc_convert_tx_buffer(uint32_t num);
/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
******************************************************************************/

/*******************************************************************************
**************************  Transfer callback function. ************************
 ******************************************************************************/
void crc_dma_transfer_complete_callback(uint32_t channel, void *data)
{
  (void)channel;
  (void)data;
  dma_transfer_done = 1;
}

/*******************************************************************************
 ***************************  CRC conversion function.**************************  
 ******************************************************************************/
static uint32_t crc_convert_tx_buffer(uint32_t num)
{
  uint8_t byte0, byte1, byte2, byte3;

  byte0 = (num & 0x000000FF);
  byte1 = (num & 0x0000FF00) >> 8;
  byte2 = (num & 0x00FF0000) >> 16;
  byte3 = (num & 0xFF000000) >> 24;

  return ((uint32_t)byte0 << 24) | ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | ((uint32_t)byte3);
}

/*******************************************************************************
***************************  CRC Example initialization ************************  
 ******************************************************************************/
void crc_example_init(void)
{
  sl_status_t status     = SL_STATUS_OK;
  sl_dma_init_t dma_init = { SL_DMA_INSTANCE };
  sl_dma_callback_t callbacks;

  DEBUGOUT("\n******* CRC EXAMPLE INIT *********\n");
  do {
    status = sl_si91x_dma_init(&dma_init);
    if (status == SL_STATUS_OK) {
      //UDMA initialization success
      DEBUGOUT("UDMA Initialization Success\r\n");
    } else {
      //UDMA initialization fail
      DEBUGOUT("Failed to Initialize UDMA\r\n");
      break;
    }
    //Reset the DMA transfer done flag
    dma_transfer_done = 0;
    //Allocate channel for transfer
    status = sl_si91x_dma_allocate_channel(SL_DMA_INSTANCE, &gchannel, 0);
    if (status == SL_STATUS_OK) {
      //Channel successfully allocated
      DEBUGOUT("Channel Allocated successfully\r\n");
    } else {
      //Channel allocation failed
      DEBUGOUT("Channel not allocated\r\n");
      break;
    }
    //Register transfer complete callback
    callbacks.transfer_complete_cb = crc_dma_transfer_complete_callback;

    //Register transfer complete callback
    status = sl_si91x_dma_register_callbacks(SL_DMA_INSTANCE, gchannel, &callbacks);
    if (status == SL_STATUS_OK) {
      //Callback registration success
      DEBUGOUT("Callbacks registered\r\n");
    } else {
      //Callback not registered
      DEBUGOUT("Callbacks not registered\r\n");
      break;
    }

    //Update source and destination address
    dma_transfer_t.src_addr  = (uint32_t *)gtx_temp_buff;
    dma_transfer_t.dest_addr = (uint32_t *)&CRC->CRC_DIN_FIFO;
    //Update Source and destination data increment
    dma_transfer_t.src_inc = SL_TRANSFER_SRC_INC_32;
    dma_transfer_t.dst_inc = SL_TRANSFER_DST_INC_NONE;
    //Update the single data transfer size
    dma_transfer_t.xfer_size = SL_TRANSFER_SIZE_32;
    //Update total DMA transfer size
    dma_transfer_t.transfer_count = BUFFSIZE;
    //Update DMA transfer type and mode
    dma_transfer_t.dma_mode      = SL_DMA_BASIC_MODE;
    dma_transfer_t.transfer_type = SL_DMA_MEMORY_TO_MEMORY;
    //Perform DMA transfer using generic dma transfer API
  } while (0);
}
/* *******************************************************************************
***************************  CRC DMA transfer function. ************************
 ******************************************************************************/
static sl_status_t crc_dma_transfer(uint32_t *data, uint32_t data_len)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    if (data == NULL || data_len == 0) {
      DEBUGOUT(" Invalid parameters for CRC DMA transfer.\r\n");
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Update source and destination address
    dma_transfer_t.src_addr  = data;
    dma_transfer_t.dest_addr = (uint32_t *)&CRC->CRC_DIN_FIFO;
    // Update total DMA transfer size
    dma_transfer_t.transfer_count = data_len;

    // Perform DMA transfer using generic dma transfer API
    status = sl_si91x_dma_transfer(SL_DMA_INSTANCE, gchannel, &dma_transfer_t);
    if (status != SL_STATUS_OK) {
      DEBUGOUT(" DMA transfer failed. Error code: 0x%08lX\r\n", (unsigned long)status);
      break;
    }
  } while (0);
  return status;
}

/*******************************************************************************
***************************  CRC Example Process action ************************  
 ******************************************************************************/
void crc_example_process_action(void)
{
  uint32_t index = 0, status = SL_STATUS_FAIL;
  static uint32_t counter_for_delay = 0;

  // counter for adding delay to slowdown repetitive operation
  counter_for_delay++;
  if (counter_for_delay % 100000) {
    return;
  }
  DEBUGOUT("\n******* CRC EXAMPLE START *********\n");

  /* Software CRC calculation */
  gsw_crc = sw_crc_compute(BUFF_LEN_8BIT_TYPE);
  DEBUGOUT(" Input Data Buffer:\n {\n\t");
  for (index = 0; index < BUFFSIZE; index++) {
    gtx_temp_buff[index] = crc_convert_tx_buffer(gcrc_tx_Buf[index]);
    DEBUGOUT("0x%08lX ", gtx_temp_buff[index]);

    if (!((index + 1) % 5))
      DEBUGOUT("\n\t");
  }
  DEBUGOUT("\n }\n");

  /* CRC_32 polynomial */
  crc_params.polynomial = SL_CRC_POLYNOMIAL;
  crc_params.polyWidth  = SL_CRC_POLY_WIDTH;
  crc_params.lfsrVal    = SL_CRC_LFSR_VAL;
  crc_params.widthType  = SL_CRC_WIDTH_TYPE;
  crc_params.dinWidth   = SL_CRC_DIN_WIDTH;
  crc_params.numBytes   = SL_CRC_NUM_BYTES;
  crc_params.aempty     = SL_CRC_AEMPTY;
  crc_params.afull      = SL_CRC_AFULL;
  crc_params.swapDin    = SL_CRC_SWAP_DIN;
  crc_params.useUdma    = SL_CRC_USE_UDMA;
  crc_params.swapLfsr   = SL_CRC_SWAP_LFSR;
  do {
    // configure CRC hardware
    status = sl_si91x_crc_set_config(&crc_params);
    if (status != SL_STATUS_OK) {
      DEBUGOUT(" CRC hardware initialization failed. Error code: 0x%08lX\r\n", (unsigned long)status);
      break;
    } else {
      DEBUGOUT(" CRC hardware initialized successfully.\r\n");
    }

    status = crc_dma_transfer(gtx_temp_buff, BUFFSIZE);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("CRC hardware calculation failed. Error code: 0x%08lX\r\n", (unsigned long)status);
      return;
    }

    DEBUGOUT("\n DMA Data transfer Completed\n");
    status = sl_si91x_crc_monitor_crc_calc(&crc_params, &ghw_crc);
    if (status != SL_STATUS_OK) {
      DEBUGOUT(" Failed to monitor CRC calculation. Error code: 0x%08lX\r\n", (unsigned long)status);
      break;
    } else {
      DEBUGOUT(" Hardware CRC Value: 0x%08lX\r\n", (unsigned long)ghw_crc);
      DEBUGOUT(" Software CRC Value: 0x%08lX\r\n", (unsigned long)gsw_crc);
    }
    if (ghw_crc == gsw_crc) {
      DEBUGOUT("\n Both Matched and CRC TEST PASSED \n");
    } else {
      DEBUGOUT("\n CRC TEST FAILED \n");
      break;
    }
    // Disable the CRC
    status = sl_si91x_crc_disable();
    if (status != SL_STATUS_OK) {
      DEBUGOUT(" Failed to disable CRC.\r\n");
    } else {
      DEBUGOUT(" CRC disabled successfully.\r\n");
    }
  } while (false);
  DEBUGOUT("\n******* CRC EXAMPLE END *********\n");
}
