/******************************************************************************
* @file sl_si91x_dma.c
* @brief DMA API  implemAPIentation
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
#include "sl_si91x_dma.h"
#include "sl_status.h"
#include "rsi_rom_udma_wrapper.h"
#include "rsi_udma.h"
#include "si91x_device.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/

#define DMA0_IRQ_HANDLER           IRQ033_Handler // IRQ handler for UDMA0
#define DMA1_IRQ_HANDLER           IRQ010_Handler // IRQ handler for ULP_DMA
#define DMA_MAX_TRANSFER_COUNT     1024           // Maximum transfer count of DMA in one cycle
#define DMA_CHANNEL_32             32             // Channel no 32
#define ERROR_CALLBACK             2              // Error callback bit position
#define TRANSFER_COMPLETE_CALLBACK 1              // Transfer complete callback bit position
#define DMA_INSTANCE0              0              // DMA0 Instance
#define ULP_DMA_INSTANCE           1              // ULP_DMA Instance
#define TOTAL_DMA_INSTANCES        2              // Total number of DMA instances
#define DMA_DRIVER_ROM_BUFF_SIZE   30             // Size of dma_driver_rom_buff0
#define VALID_CALLBACK_RANGE_LOW   1              // Validation of low range callback type
#define VALID_CALLBACK_RANGE_HIGH  3              // Validation of high range callback type
#define EXTRACT_LSB                1
#define DMA_CHANNEL_12             12
#define DMA_CHANNEL_PRIORITY_HIGH  1
#define TRANSFER_MODE_MAX          0x07
#define DMA_COUNT_MAX              0x03FF

/*******************************************************************************
 ***************************  EXTERN VARIABLES  ********************************
 ******************************************************************************/
extern RSI_UDMA_DESC_T __attribute__((section(".udma_addr0")))
UDMA0_Table[CONTROL_STRUCT0]; // UDMA0 DMA descriptors SRAM base address
extern RSI_UDMA_DESC_T __attribute__((section(".udma_addr1")))
UDMA1_Table[CONTROL_STRUCT1];                             // ULP_DMA descriptors SRAM base address
extern UDMA_RESOURCES UDMA0_Resources;                    // UDMA0 resources(address, irq no and DMA descriptor)
extern UDMA_RESOURCES UDMA1_Resources;                    // ULP_DMA resources(address, irq no and DMA descriptor)
extern UDMA_Channel_Info udma0_chnl_info[DMA_CHANNEL_32]; // UDMA0 channels information
extern UDMA_Channel_Info udma1_chnl_info[DMA_CHANNEL_12]; // ULP_DMA channels information
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
RSI_UDMA_HANDLE_T
udmaHandle[TOTAL_DMA_INSTANCES]; // Array stores driver and peripheral context fo UDMA peripheral
uint32_t dma_driver_rom_buff0[DMA_DRIVER_ROM_BUFF_SIZE]; // Pointer to memory area used to UDMA0 driver context
uint32_t dma_driver_rom_buff1[DMA_DRIVER_ROM_BUFF_SIZE]; // Pointer to memory area used to ULP_DMA driver context
uint32_t *dma_rom_buff[TOTAL_DMA_INSTANCES] = {
  dma_driver_rom_buff0,
  dma_driver_rom_buff1
}; // Array stores addresses of UDMA0 & ULP_DMA dma_driver_rom_buff
sl_channel_data_t sl_dma0_channel_allocation_data_t[SL_DMA0_CHANNEL_COUNT]       = { 0 }; // DMA0 channel allocator
sl_channel_data_t sl_ulp_dma_channel_allocation_data_t[SL_ULP_DMA_CHANNEL_COUNT] = { 0 }; // ULP_DMA channel allocator
UDMA_RESOURCES *UDMA_driver_resources[TOTAL_DMA_INSTANCES]                       = {
                        &UDMA0_Resources,
                        &UDMA1_Resources
}; // Array stores addresses of UDMA0 & ULP_DMA driver resources
UDMA_Channel_Info *udma_driver_channel_info[TOTAL_DMA_INSTANCES] = {
  udma0_chnl_info,
  udma1_chnl_info
}; // Array stores addresses of UDMA0 & ULP_DMA driver channel resources
RSI_UDMA_DESC_T *udma_driver_table[TOTAL_DMA_INSTANCES] = {
  UDMA0_Table,
  UDMA1_Table
}; // Array stores addresses of UDMA0 & ULP_DMA descriptors
static void dma_internal_callback(uint32_t event, uint32_t channel);
static void ulp_dma_internal_callback(uint32_t event, uint32_t channel);

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
/***************************************************************************/ /**
 * Return DMA channel status
 *
 * @param[in] dma_number - dma instance
 * @param[in] channel -    dma channel number
 *
 * @return true - DMA channel busy
 *         false - DMA channel idle
 ******************************************************************************/
static boolean_t channel_status(uint32_t dma_number, uint32_t channel);

#ifdef SL_DMA_IRQ_HANDLER
/***************************************************************************/ /**
 * Process dma interrupt handler.
 *
 * @param[in] dma_number - dma instance
 * @param[in] channel -    dma channel number
 * @param[in] irq_type -    irq type,
 *                          SL_DMA_TRANSFER_DONE_CB - transfer complete interrupt
 *                          SL_DMA_ERROR_CB - channel error callback
 *
 * @return none
 ******************************************************************************/
__STATIC_INLINE void process_dma_irq(uint32_t dma_number, uint32_t channel, uint32_t irq_type);
#endif // SL_DMA_IRQ_HANDLER

/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
 * This function return DMA channel status
 * true - DMA channel busy
 * false - DMA channel idle
 ******************************************************************************/
static boolean_t channel_status(uint32_t dma_number, uint32_t channel)
{

  return ((UDMA_driver_resources[dma_number]->reg->CHANNEL_STATUS_REG >> channel) & EXTRACT_LSB);
}

#ifdef SL_DMA_IRQ_HANDLER
/*******************************************************************************
 * This function process DMA IRQ.
 * If DMA transfer size is greater than DMA transfer count (1024) then multiple DMA requests are
 * triggered by updating DMA descriptors.
 * This function calls transfer and error callback functions if registered.
 ******************************************************************************/
__STATIC_INLINE void process_dma_irq(uint32_t dma_number, uint32_t channel, uint32_t irq_type)
{

  uint32_t remaining_data_size;
  uint32_t dma_count;
  uint32_t src_inc = 0;
  uint32_t dst_inc = 0;
  // Obtain the DMA channel info
  UDMA_Channel_Info *channel_info =
    (UDMA_Channel_Info *)((UDMA_Channel_Info *)(udma_driver_channel_info[dma_number]) + channel);
  // Obtain DMA descriptor info
  RSI_UDMA_DESC_T *udma_table = (RSI_UDMA_DESC_T *)((RSI_UDMA_DESC_T *)(udma_driver_table[dma_number]) + channel);

  if (irq_type == SL_DMA_TRANSFER_DONE_CB) {
    // Interrupt is triggered by transfer complete
    if (channel_info->Cnt < channel_info->Size) {
      // Transfer size is greater than max DMA transfer count
      // Obtain remaining transfer data size
      remaining_data_size = channel_info->Size - channel_info->Cnt;
      // Update next transfer count
      if (remaining_data_size >= DMA_MAX_TRANSFER_COUNT) {
        remaining_data_size = DMA_MAX_TRANSFER_COUNT;
        dma_count           = DMA_MAX_TRANSFER_COUNT - 1;
      }
      if ((remaining_data_size > 0) && (remaining_data_size < DMA_MAX_TRANSFER_COUNT)) {
        dma_count = remaining_data_size - 1;
      }
      channel_info->Cnt += remaining_data_size;
      // Update source and dest increment for next transfer
      src_inc = udma_table->vsUDMAChaConfigData1.srcInc;
      dst_inc = udma_table->vsUDMAChaConfigData1.dstInc;

      // No need to update source end address if src_inc is configured to UDMA_SRC_INC_NONE. Only update for
      //SRC_INC_8/16/32
      if (src_inc != UDMA_SRC_INC_NONE) {
        udma_table->pSrcEndAddr = (void *)((uint32_t)udma_table->pSrcEndAddr + (remaining_data_size << src_inc));
      } else {
        udma_table->pSrcEndAddr = (void *)((uint32_t)udma_table->pSrcEndAddr);
      }
      // No need to update destination end address if dst_inc is configured to UDMA_DST_INC_NONE. Only update for
      //SRC_INC_8/16/32
      if (dst_inc != UDMA_DST_INC_NONE) {
        udma_table->pDstEndAddr = (void *)((uint32_t)udma_table->pDstEndAddr + (remaining_data_size << dst_inc));
      } else {
        udma_table->pDstEndAddr = (void *)((uint32_t)udma_table->pDstEndAddr);
      }

      // Update transfer type and transfer count
      if (dma_number == DMA_INSTANCE0) {
        udma_table->vsUDMAChaConfigData1.transferType =
          (unsigned int)(sl_dma0_channel_allocation_data_t[channel].transfer_mode & TRANSFER_MODE_MAX);
        udma_table->vsUDMAChaConfigData1.totalNumOfDMATrans = (unsigned int)(dma_count & DMA_COUNT_MAX);
      } else {
        udma_table->vsUDMAChaConfigData1.transferType =
          (unsigned int)(sl_ulp_dma_channel_allocation_data_t[channel].transfer_mode & TRANSFER_MODE_MAX);
        udma_table->vsUDMAChaConfigData1.totalNumOfDMATrans = (unsigned int)(dma_count & DMA_COUNT_MAX);
      }
      // Enable channel for next transfer
      UDMA_driver_resources[dma_number]->reg->CHNL_ENABLE_SET = (1U << channel);
      if (((dma_number == DMA_INSTANCE0)
           && (sl_dma0_channel_allocation_data_t[channel].transfer_type == SL_DMA_MEMORY_TO_MEMORY))
          || ((dma_number == ULP_DMA_INSTANCE)
              && (sl_ulp_dma_channel_allocation_data_t[channel].transfer_type == SL_DMA_MEMORY_TO_MEMORY))) {
        // for memory to memory request, trigger SW request
        UDMA_driver_resources[dma_number]->reg->CHNL_SW_REQUEST = (1U << channel);
      }
    } else {
      if ((dma_number == DMA_INSTANCE0)
          && (sl_dma0_channel_allocation_data_t[channel].dma_callback_t.transfer_complete_cb != NULL)) {
        // Transfer complete callback
        sl_dma0_channel_allocation_data_t[channel].dma_callback_t.transfer_complete_cb(channel, NULL);
      }
      if ((dma_number == ULP_DMA_INSTANCE)
          && (sl_ulp_dma_channel_allocation_data_t[channel].dma_callback_t.transfer_complete_cb != NULL)) {
        // Transfer complete callback
        sl_ulp_dma_channel_allocation_data_t[channel].dma_callback_t.transfer_complete_cb(channel, NULL);
      }
    }
  }
  if (irq_type == SL_DMA_ERROR_CB) {
    if ((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[channel].dma_callback_t.error_cb != NULL)) {
      // Transfer complete callback
      sl_dma0_channel_allocation_data_t[channel].dma_callback_t.error_cb(channel, NULL);
    }
    if ((dma_number == ULP_DMA_INSTANCE)
        && (sl_ulp_dma_channel_allocation_data_t[channel].dma_callback_t.error_cb != NULL)) {
      // Transfer complete callback
      sl_ulp_dma_channel_allocation_data_t[channel].dma_callback_t.error_cb(channel, NULL);
    }
  }
}
#endif //SL_DMA_IRQ_HANDLER
/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/

/*******************************************************************************
* This function should be called only once for initializing UDMA0/ULP_DMA peripheral
* before starting any channel transfer
 *******************************************************************************/
sl_status_t sl_si91x_dma_init(const sl_dma_init_t *dma_init)
{

  sl_status_t status                               = SL_STATUS_OK;
  const RSI_UDMA_DATACONTEXT_T *udma_driver_handle = (const RSI_UDMA_DATACONTEXT_T *)udmaHandle[dma_init->dma_number];
  if (dma_init->dma_number <= ULP_DMA_INSTANCE) {
    // If the handle is NULL, it is the first time initialization, if the handle is already present, it means it is sleep-wakeup scenario which needs dma initialization.
    if ((udmaHandle[dma_init->dma_number] == NULL)
        || (udma_driver_handle->base == UDMA0 || udma_driver_handle->base == UDMA1)) {
      // Initialize dma peripheral
      udmaHandle[dma_init->dma_number] = UDMAx_Initialize(UDMA_driver_resources[dma_init->dma_number],
                                                          udma_driver_table[dma_init->dma_number],
                                                          udmaHandle[dma_init->dma_number],
                                                          dma_rom_buff[dma_init->dma_number]);

      if (udmaHandle[dma_init->dma_number] == NULL) {
        // DMA init fail
        status = SL_STATUS_NOT_INITIALIZED;
      } else {
        //DMA init pass
        status = SL_STATUS_OK;
      }
    } else {
      status = SL_STATUS_NOT_INITIALIZED;
    }
  } else {
    status = SL_STATUS_INVALID_PARAMETER;
  }
  return status;
}

/*******************************************************************************
 * This function de-initializes DMA peripheral
  *******************************************************************************/
sl_status_t sl_si91x_dma_deinit(uint32_t dma_number)
{
  sl_status_t status     = SL_STATUS_OK;
  uint32_t channel_count = 0;
  uint32_t channel;

  if (dma_number > ULP_DMA_INSTANCE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (dma_number == DMA_INSTANCE0) {
    channel_count = SL_DMA0_CHANNEL_COUNT;
  } else {
    channel_count = SL_ULP_DMA_CHANNEL_COUNT;
  }
  for (channel = 0; channel < channel_count; channel++) {
    if (channel_status(dma_number, channel)) {
      // DMA channel is busy
      status = SL_STATUS_BUSY;
      break;
    }
  }
  if (status == SL_STATUS_OK) {
    //Unregister callbacks for all DMA channels
    for (channel = 0; channel < channel_count; channel++) {
      if (((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[channel].allocated == true))
          || ((dma_number == ULP_DMA_INSTANCE) && (sl_ulp_dma_channel_allocation_data_t[channel].allocated == true))) {
        sl_si91x_dma_deallocate_channel(dma_number, channel + 1);
      }
    }
    // Uninitialize DMA
    status = (sl_status_t)UDMAx_Uninitialize(UDMA_driver_resources[dma_number]);
    if (status == SL_STATUS_OK) {
      // Clearing the udmaHandle for udma0 or udma1 as per the dma number.
      udmaHandle[dma_number] = NULL;
    }
  }

  return status;
}

/*******************************************************************************
 * This function returns the DMA handle for the specified DMA instance
 * 
 * @param[in] dma_number DMA instance number:
 *                       - 0 for UDMA0
 *                       - 1 for ULP_DMA
 * 
 * @return void* Pointer to the DMA handle context
 *         - Valid pointer if dma_number is valid and DMA is initialized
 *         - NULL if dma_number is invalid
 * *****************************************************************************/
void *sl_si91x_get_dma_handle(uint32_t dma_number)
{
  if (dma_number > ULP_DMA_INSTANCE) {
    return NULL;
  }
  return udmaHandle[dma_number];
}

/*===================================================*/
/**
 * @fn          sl_status_t scan_available_dma_channel(uint32_t dma_number, uint32_t **channel_no, uint32_t priority)
 * @brief       scans and allocates dma channel
 * @return      status and if status is ok, channel number is stored in **channel_no
 */
STATIC INLINE sl_status_t scan_available_dma_channel(uint32_t dma_number, uint32_t **channel_no, uint32_t priority)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t channel_count;

  if (dma_number == DMA_INSTANCE0) {
    channel_count = SL_DMA0_CHANNEL_COUNT;
  } else {
    channel_count = SL_ULP_DMA_CHANNEL_COUNT;
  }

  // Scan for available channels
  for (uint32_t ch = 0; ch < channel_count; ch++) {
    if ((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[ch].allocated == false)) {
      // Channel available
      **channel_no = ch + 1;
      // Allocate the channel using channel allocator
      sl_dma0_channel_allocation_data_t[ch].priority                            = priority;
      sl_dma0_channel_allocation_data_t[ch].allocated                           = true;
      sl_dma0_channel_allocation_data_t[ch].dma_callback_t.transfer_complete_cb = NULL;
      sl_dma0_channel_allocation_data_t[ch].dma_callback_t.error_cb             = NULL;
    } else if ((dma_number == ULP_DMA_INSTANCE) && (sl_ulp_dma_channel_allocation_data_t[ch].allocated == false)) {
      // Channel availablesl_si91x_adc
      **channel_no = ch + 1;
      // Allocate the channel using channel allocator
      sl_ulp_dma_channel_allocation_data_t[ch].priority                            = priority;
      sl_ulp_dma_channel_allocation_data_t[ch].allocated                           = true;
      sl_ulp_dma_channel_allocation_data_t[ch].dma_callback_t.transfer_complete_cb = NULL;
      sl_ulp_dma_channel_allocation_data_t[ch].dma_callback_t.error_cb             = NULL;
    }
    if (**channel_no != 0) {
      break;
    }
  }
  if (**channel_no == 0) {
    // No DMA channel is available
    status = SL_STATUS_DMA_NO_CHANNEL_AVAILABLE;
  }

  return status;
}

/*******************************************************************************
* This Allocates channel for DMA transfer
* If channel no is 0, available channel is allocated
* If channel no is between 1 - 32, channel availability is chacked and allocated
* accordingly
 *******************************************************************************/
sl_status_t sl_si91x_dma_allocate_channel(uint32_t dma_number, uint32_t *channel_no, uint32_t priority)
{
  sl_status_t status = SL_STATUS_OK;

  if ((channel_no == NULL) || (dma_number > ULP_DMA_INSTANCE)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (((dma_number == DMA_INSTANCE0) && (*channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (*channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (priority > DMA_CHANNEL_PRIORITY_HIGH) {
    // Invalid channel priority
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (*channel_no == 0) {
    // Scan for available channels
    status = scan_available_dma_channel(dma_number, &channel_no, priority);
  } else {
    if ((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[*channel_no - 1].allocated == false)) {
      // Allocate desired DMA channel, if it is available
      sl_dma0_channel_allocation_data_t[*channel_no - 1].priority                            = priority;
      sl_dma0_channel_allocation_data_t[*channel_no - 1].allocated                           = true;
      sl_dma0_channel_allocation_data_t[*channel_no - 1].dma_callback_t.transfer_complete_cb = NULL;
      sl_dma0_channel_allocation_data_t[*channel_no - 1].dma_callback_t.error_cb             = NULL;
    } else if ((dma_number == ULP_DMA_INSTANCE)
               && (sl_ulp_dma_channel_allocation_data_t[*channel_no - 1].allocated == false)) {
      // Allocate desired DMA channel, if it is available
      sl_ulp_dma_channel_allocation_data_t[*channel_no - 1].priority                            = priority;
      sl_ulp_dma_channel_allocation_data_t[*channel_no - 1].allocated                           = true;
      sl_ulp_dma_channel_allocation_data_t[*channel_no - 1].dma_callback_t.transfer_complete_cb = NULL;
      sl_ulp_dma_channel_allocation_data_t[*channel_no - 1].dma_callback_t.error_cb             = NULL;
    } else {
      // Desired DMA channel is already allocated
      status = SL_STATUS_DMA_CHANNEL_ALLOCATED;
    }
  }

  return status;
}

/*******************************************************************************
 * This function De-allocates channel for DMA transfer
 * Clears the channel allocator data and makes available for
 * other transfers
 * *****************************************************************************/
sl_status_t sl_si91x_dma_deallocate_channel(uint32_t dma_number, uint32_t channel_no)
{
  sl_status_t status = SL_STATUS_OK;

  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (channel_status(dma_number, channel_no - 1)) {
    // Transfer is in progress
    return SL_STATUS_BUSY;
  }
  if ((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[channel_no - 1].allocated == true)) {
    // Clear channel allocator data
    sl_dma0_channel_allocation_data_t[channel_no - 1].allocated                           = false;
    sl_dma0_channel_allocation_data_t[channel_no - 1].dma_callback_t.transfer_complete_cb = NULL;
    sl_dma0_channel_allocation_data_t[channel_no - 1].dma_callback_t.error_cb             = NULL;
  } else if ((dma_number == ULP_DMA_INSTANCE)
             && (sl_ulp_dma_channel_allocation_data_t[channel_no - 1].allocated == true)) {
    // Clear channel allocator data
    sl_ulp_dma_channel_allocation_data_t[channel_no - 1].allocated                           = false;
    sl_ulp_dma_channel_allocation_data_t[channel_no - 1].dma_callback_t.transfer_complete_cb = NULL;
    sl_ulp_dma_channel_allocation_data_t[channel_no - 1].dma_callback_t.error_cb             = NULL;
  } else {
    // DMA channel is already unallocated
    status = SL_STATUS_DMA_CHANNEL_ALREADY_UNALLOCATED;
  }

  return status;
}

/*******************************************************************************
 * This function registers DMA callbacks
 * sl_dma_transfer_complete called when transfer is completed
 * sl_dma_error is called when there is bus error while channel transfer
 * *****************************************************************************/
sl_status_t sl_si91x_dma_register_callbacks(uint32_t dma_number, uint32_t channel_no, sl_dma_callback_t *callback_t)
{
  sl_status_t status = SL_STATUS_OK;

  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (callback_t == NULL) {
    // Invalid callback structure
    return SL_STATUS_NULL_POINTER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  // Register callbacks
  if (dma_number == DMA_INSTANCE0) {
    sl_dma0_channel_allocation_data_t[channel_no - 1].dma_callback_t.transfer_complete_cb =
      callback_t->transfer_complete_cb;
    sl_dma0_channel_allocation_data_t[channel_no - 1].dma_callback_t.error_cb = callback_t->error_cb;
  } else {
    sl_ulp_dma_channel_allocation_data_t[channel_no - 1].dma_callback_t.transfer_complete_cb =
      callback_t->transfer_complete_cb;
    sl_ulp_dma_channel_allocation_data_t[channel_no - 1].dma_callback_t.error_cb = callback_t->error_cb;
  }

  return status;
}

/*******************************************************************************
 * This function unregisters DMA callbacks
 * *****************************************************************************/
sl_status_t sl_si91x_dma_unregister_callbacks(uint32_t dma_number, uint32_t channel_no, uint8_t callback_type)
{
  sl_status_t status = SL_STATUS_OK;

  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((callback_type > VALID_CALLBACK_RANGE_HIGH) || (callback_type < VALID_CALLBACK_RANGE_LOW)) {
    // Invalid callback type
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (callback_type & TRANSFER_COMPLETE_CALLBACK) {
    // unregister transfer complete callback
    if (dma_number == DMA_INSTANCE0) {
      sl_dma0_channel_allocation_data_t[channel_no - 1].dma_callback_t.transfer_complete_cb = NULL;
    } else {
      sl_ulp_dma_channel_allocation_data_t[channel_no - 1].dma_callback_t.transfer_complete_cb = NULL;
    }
  }
  if (callback_type & ERROR_CALLBACK) {
    // unregister error callback
    if (dma_number == DMA_INSTANCE0) {
      sl_dma0_channel_allocation_data_t[channel_no - 1].dma_callback_t.error_cb = NULL;
    } else {
      sl_ulp_dma_channel_allocation_data_t[channel_no - 1].dma_callback_t.error_cb = NULL;
    }
  }

  return status;
}

/*******************************************************************************
 * This function perform simple memory to memory DMA transfer.
 * User needs to allocate channel before initializing transfer using this function
 * User no need to update any DMA descriptors if using this function
 * *****************************************************************************/
sl_status_t sl_si91x_dma_simple_transfer(uint32_t dma_number,
                                         uint32_t channel_no,
                                         void *src_addr,
                                         void *dst_addr,
                                         uint32_t data_size)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t channel   = channel_no - 1;
  UDMA_SignalEvent_t dma_callback;
  RSI_UDMA_CHA_CONFIG_DATA_T control;
  RSI_UDMA_CHA_CFG_T config;

  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((src_addr == NULL) || (dst_addr == NULL)) {
    // Invalid src/dst addr
    return SL_STATUS_NULL_POINTER;
  }
  if (data_size == 0) {
    //Invalid data size
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[channel].allocated != true))
      || ((dma_number == ULP_DMA_INSTANCE) && (sl_ulp_dma_channel_allocation_data_t[channel].allocated != true))) {
    // Channel is unallocated
    return SL_STATUS_DMA_CHANNEL_UNALLOCATED;
  }

  // Update channel configuration structure with basic parameters
  config.altStruct       = ALTERNATE_DESCRIPTOR_DISABLE;
  config.burstReq        = BURST_REQUEST_ENABLE;
  config.channelPrioHigh = CHANNEL_PRIO_DISABLE;
  config.dmaCh           = channel;
  config.periAck         = PERIPHERAL_ACK_DISABLE;
  config.periphReq       = PERIPHERAL_REQUEST_DISABLE;
  config.reqMask         = REQUEST_MASK_DISABLE;
  control.transferType   = SL_DMA_BASIC_MODE;
  control.nextBurst      = NEXT_BURST_DISABLE;
  if (data_size > (DMA_MAX_TRANSFER_COUNT - 1)) {
    // Transfer size is more than maximum transfer size in one DMA cycle
    control.totalNumOfDMATrans = (DMA_MAX_TRANSFER_COUNT - 1);
  } else {
    control.totalNumOfDMATrans = (unsigned int)((data_size - 1) & 0x03FF);
  }
  // Update DMA descriptor
  control.rPower      = ARBSIZE_1024;
  control.srcProtCtrl = SOURCE_PROTECT_CONTROL_DISABLE;
  control.dstProtCtrl = DESTINATION_PROTECT_CONTROL_DISABLE;
  control.srcSize     = SRC_SIZE_32;
  control.srcInc      = SRC_INC_32;
  control.dstSize     = DST_SIZE_32;
  control.dstInc      = DST_INC_32;

  // Update DMA transfer type and transfer mode in channel allocator
  if (dma_number == DMA_INSTANCE0) {
    sl_dma0_channel_allocation_data_t[channel].transfer_type = SL_DMA_MEMORY_TO_MEMORY;
    sl_dma0_channel_allocation_data_t[channel].transfer_mode = SL_DMA_BASIC_MODE;
  } else {
    sl_ulp_dma_channel_allocation_data_t[channel].transfer_type = SL_DMA_MEMORY_TO_MEMORY;
    sl_ulp_dma_channel_allocation_data_t[channel].transfer_mode = SL_DMA_BASIC_MODE;
  }

  if (dma_number) {
    dma_callback = ulp_dma_internal_callback;
  } else {
    dma_callback = dma_internal_callback;
  }
  // Configure dma channel for transfer
  status = (sl_status_t)UDMAx_ChannelConfigure(UDMA_driver_resources[dma_number],
                                               ((uint8_t)channel),
                                               (uint32_t)src_addr,
                                               (uint32_t)dst_addr,
                                               data_size,
                                               control,
                                               &config,
                                               dma_callback,
                                               udma_driver_channel_info[dma_number],
                                               udmaHandle[dma_number]);

  if (status != SL_STATUS_OK) {
    return status;
  }
  // Enable DMA channel
  status =
    (sl_status_t)UDMAx_ChannelEnable((uint8_t)channel, UDMA_driver_resources[dma_number], udmaHandle[dma_number]);

  if (status != SL_STATUS_OK) {
    return status;
  }
  // Enable DMA peripheral
  status = (sl_status_t)UDMAx_DMAEnable(UDMA_driver_resources[dma_number], udmaHandle[dma_number]);

  if (status != SL_STATUS_OK) {
    return status;
  }
  // Start transfer using software trigger
  status = (sl_status_t)RSI_UDMA_ChannelSoftwareTrigger(udmaHandle[dma_number], (uint8_t)channel);

  return status;
}

/*===================================================*/
/**
 * @fn          sl_status_t validate_dma_transfer_params(uint32_t dma_number,
                                                       uint32_t channel_no,
                                                       sl_dma_xfer_t *dma_transfer_t)
 * @brief       validate the input parameters for DMA transfer
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * @param[in] dma_transfer_t Pointer to the channel transfer data structure containing the channel descriptor and other basic DMA parameters.
 * 
 * @return sl_status_t DMA transfer status:
*         - SL_STATUS_OK  - Transfer started successfully.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_SUSPENDED - Transfer initialization failed.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.  
 */
STATIC INLINE sl_status_t validate_dma_transfer_params(uint32_t dma_number,
                                                       uint32_t channel_no,
                                                       const sl_dma_xfer_t *dma_transfer_t)
{
  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (dma_transfer_t->transfer_count == 0) {
    // Invalid data size
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((dma_transfer_t->src_addr == NULL) || (dma_transfer_t->dest_addr == NULL)) {
    // Invalid src/dst addr
    return SL_STATUS_NULL_POINTER;
  }
  if ((dma_transfer_t->dst_inc > SL_TRANSFER_DST_INC_NONE) || (dma_transfer_t->src_inc > SL_TRANSFER_SRC_INC_NONE)) {
    // Invalid addr increment
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((dma_transfer_t->dma_mode != SL_DMA_BASIC_MODE) && (dma_transfer_t->dma_mode != SL_DMA_PINGPONG_MODE)) {
    // DMA mode not supported
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (dma_transfer_t->transfer_type > SL_DMA_PERIPHERAL_TO_MEMORY) {
    // Transfer type not supported
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (dma_transfer_t->signal > SL_I2C_ACK) {
    // Invalid DMA signal
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (dma_transfer_t->xfer_size > SL_TRANSFER_SIZE_32) {
    // Invalid transfer size
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA peripheral not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[channel_no - 1].allocated != true))
      || ((dma_number == ULP_DMA_INSTANCE)
          && (sl_ulp_dma_channel_allocation_data_t[channel_no - 1].allocated != true))) {
    // Channel is unallocated
    return SL_STATUS_DMA_CHANNEL_UNALLOCATED;
  }
  return SL_STATUS_OK;
}
/*******************************************************************************
 * This function DMA transfer.
 * User needs to allocate channel before initializing transfer using this function
 * User need to update sl_dma_xfer_t and pass it to function.
 * This function perform all types of available DMA transfers
 * *****************************************************************************/
sl_status_t sl_si91x_dma_transfer(uint32_t dma_number, uint32_t channel_no, sl_dma_xfer_t *dma_transfer_t)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t channel   = channel_no - 1;
  UDMA_SignalEvent_t dma_callback;
  RSI_UDMA_CHA_CONFIG_DATA_T control;
  RSI_UDMA_CHA_CFG_T config;

  // Validate DMA transfer parameters
  status = validate_dma_transfer_params(dma_number, channel_no, (const sl_dma_xfer_t *)dma_transfer_t);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Update DMA mode
  if (dma_transfer_t->dma_mode == SL_DMA_BASIC_MODE) {
    config.altStruct = ALTERNATE_DESCRIPTOR_DISABLE;
  } else {
    config.altStruct = ALTERNATE_DESCRIPTOR_ENABLE;
  }
  // Upadte DMA priority
  if (((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[channel].priority == true))
      || ((dma_number == ULP_DMA_INSTANCE) && (sl_ulp_dma_channel_allocation_data_t[channel].priority == true))) {
    config.channelPrioHigh = CHANNEL_PRIO_ENABLE;
  } else {
    config.channelPrioHigh = CHANNEL_PRIO_DISABLE;
  }
  if ((dma_transfer_t->transfer_type == SL_DMA_MEMORY_TO_PERIPHERAL)
      || (dma_transfer_t->transfer_type == SL_DMA_PERIPHERAL_TO_MEMORY)) {
    // Memory-Peripheral/Peripheral-Memory transfer
    config.periphReq = PERIPHERAL_REQUEST_ENABLE;
    control.rPower   = ARBSIZE_1;
  } else {
    // Memory-memory transfer
    config.periphReq = PERIPHERAL_REQUEST_DISABLE;
    control.rPower   = ARBSIZE_1024;
  }
  if ((dma_transfer_t->signal > 0) && (dma_transfer_t->signal < 8)) {
    // Update the peripheral ACK
    config.periAck = dma_transfer_t->signal;
  } else {
    config.periAck = PERIPHERAL_ACK_DISABLE;
  }
  config.dmaCh = channel;
#if ((defined(SL_SI91X_GSPI_DMA) && (SL_SI91X_GSPI_DMA == ENABLE)) \
     || (defined(SL_SI91X_SSI_DMA) && (SL_SI91X_SSI_DMA == ENABLE)))
  config.burstReq = BURST_REQUEST_DISABLE;
#else
  config.burstReq = BURST_REQUEST_ENABLE;
#endif
  config.reqMask = REQUEST_MASK_DISABLE;
  if (dma_transfer_t->dma_mode == SL_DMA_BASIC_MODE) {
    control.transferType = SL_DMA_BASIC_MODE;
  } else {
    control.transferType = SL_DMA_PINGPONG_MODE;
  }
  control.nextBurst = NEXT_BURST_DISABLE;

  if (dma_transfer_t->transfer_count >= DMA_MAX_TRANSFER_COUNT) {
    // Transfer size is more than maximum transfer size in one DMA cycle
    control.totalNumOfDMATrans = (DMA_MAX_TRANSFER_COUNT - 1);
  } else {
    control.totalNumOfDMATrans = (unsigned int)((dma_transfer_t->transfer_count - 1) & 0x03FF);
  }
  // Update DMA descriptor
  control.srcProtCtrl = SOURCE_PROTECT_CONTROL_DISABLE;
  control.dstProtCtrl = DESTINATION_PROTECT_CONTROL_DISABLE;
  control.srcSize     = (unsigned int)(dma_transfer_t->xfer_size & 0x03);
  control.srcInc      = (unsigned int)(dma_transfer_t->src_inc & 0x03);
  control.dstSize     = (unsigned int)(dma_transfer_t->xfer_size & 0x03);
  control.dstInc      = (unsigned int)(dma_transfer_t->dst_inc & 0x03);

  // Update DMA transfer type and transfer mode in channel allocator
  if (dma_number == DMA_INSTANCE0) {
    sl_dma0_channel_allocation_data_t[channel].transfer_type = dma_transfer_t->transfer_type;
    sl_dma0_channel_allocation_data_t[channel].transfer_mode = control.transferType;
  } else {
    sl_ulp_dma_channel_allocation_data_t[channel].transfer_type = dma_transfer_t->transfer_type;
    sl_ulp_dma_channel_allocation_data_t[channel].transfer_mode = control.transferType;
  }

  if (dma_number) {
    dma_callback = ulp_dma_internal_callback;
  } else {
    dma_callback = dma_internal_callback;
  }

  // Configure dma channel for transfer
  status = (sl_status_t)UDMAx_ChannelConfigure(UDMA_driver_resources[dma_number],
                                               ((uint8_t)channel),
                                               (uint32_t)dma_transfer_t->src_addr,
                                               (uint32_t)dma_transfer_t->dest_addr,
                                               dma_transfer_t->transfer_count,
                                               control,
                                               &config,
                                               dma_callback,
                                               udma_driver_channel_info[dma_number],
                                               udmaHandle[dma_number]);
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (dma_transfer_t->transfer_type == SL_DMA_MEMORY_TO_MEMORY) {
    //For memory-to-peripheral and peripheral-to-memory transfer types, below functions should be called explicitly by application
    // Enable DMA channel
    UDMAx_ChannelEnable((uint8_t)channel, UDMA_driver_resources[dma_number], udmaHandle[dma_number]);

    // Enable DMA peripheral
    UDMAx_DMAEnable(UDMA_driver_resources[dma_number], udmaHandle[dma_number]);

    // Start transfer using software trigger
    status = (sl_status_t)RSI_UDMA_ChannelSoftwareTrigger(udmaHandle[dma_number], (uint8_t)channel);
  }

  return status;
}

/*******************************************************************************
 * Stop on-going DMA transfer
 * *****************************************************************************/
sl_status_t sl_si91x_dma_stop_transfer(uint32_t dma_number, uint32_t channel_no)
{
  sl_status_t status = SL_STATUS_OK;

  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid DMA instance
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  // Disable channel for stopping transfer
  if (RSI_UDMA_ChannelDisable(udmaHandle[dma_number], (uint8_t)(channel_no - 1))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }

  return status;
}

/*******************************************************************************
 * Get the status of channel
 * SL_STATUS_BUSY - Transfer is in progress
 * SL_STATUS_DMA_CHANNEL_ALLOCATED - Channel is allocated but idle
 * *****************************************************************************/
sl_status_t sl_si91x_dma_channel_status_get(uint32_t dma_number, uint32_t channel_no)
{
  sl_status_t status = SL_STATUS_IDLE;

  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid DMA instance
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (channel_status(dma_number, channel_no - 1)) {
    // DMA channel is busy (i.e) transfer in progress
    return SL_STATUS_BUSY;
  }
  if (((dma_number == DMA_INSTANCE0) && (sl_dma0_channel_allocation_data_t[channel_no - 1].allocated == true))
      || ((dma_number == ULP_DMA_INSTANCE)
          && (sl_ulp_dma_channel_allocation_data_t[channel_no - 1].allocated == true))) {
    // Channel is allocated by idle
    return SL_STATUS_DMA_CHANNEL_ALLOCATED;
  }

  return status;
}

/*******************************************************************************
 * Enable DMA Channel
 * SL_STATUS_NOT_INITIALIZED - DMA is not initialized
 * SL_STATUS_INVALID_PARAMETER - Invalid channel number
 * *****************************************************************************/
sl_status_t sl_si91x_dma_channel_enable(uint32_t dma_number, uint32_t channel_no)
{
  sl_status_t status = SL_STATUS_OK;

  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid DMA instance
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (RSI_UDMA_ChannelEnable(udmaHandle[dma_number], (uint8_t)channel_no - 1)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }

  return status;
}

/*******************************************************************************
 * Disable DMA Channel
 * SL_STATUS_NOT_INITIALIZED - DMA is not initialized
 * SL_STATUS_INVALID_PARAMETER - Invalid channel number
 * *****************************************************************************/
sl_status_t sl_si91x_dma_channel_disable(uint32_t dma_number, uint32_t channel_no)
{

  sl_status_t status = SL_STATUS_OK;

  if ((dma_number > ULP_DMA_INSTANCE) || (channel_no == 0)) {
    // Invalid DMA instance
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (((dma_number == DMA_INSTANCE0) && (channel_no > SL_DMA0_CHANNEL_COUNT))
      || ((dma_number == ULP_DMA_INSTANCE) && (channel_no > SL_ULP_DMA_CHANNEL_COUNT))) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (udmaHandle[dma_number] == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (RSI_UDMA_ChannelDisable(udmaHandle[dma_number], (uint8_t)channel_no - 1)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }

  return status;
}

/*******************************************************************************
 * Enable DMA peripheral
 * *****************************************************************************/
sl_status_t sl_si91x_dma_enable(uint32_t dma_number)
{

  sl_status_t status = SL_STATUS_OK;
  if (dma_number <= ULP_DMA_INSTANCE) {
    if (udmaHandle[dma_number] == NULL) {
      // DMA not initialized
      status = SL_STATUS_NOT_INITIALIZED;
    } else {
      if (UDMAx_DMAEnable(UDMA_driver_resources[dma_number], udmaHandle[dma_number])) {
        status = SL_STATUS_FAIL;
      }
    }
  } else {
    status = SL_STATUS_INVALID_PARAMETER;
  }
  return status;
}

#ifdef SL_DMA_IRQ_HANDLER
/*******************************************************************************
 * Interrupt handler for UDMA0 peripheral.
 * This function clears the interrupts
 * If transfer size is greater than DMA max transfer size, process_dma_irq
 * will initiate the transfer again until all the bytes are transferred
 * *****************************************************************************/
void __attribute__((optimize("O0"))) DMA0_IRQ_HANDLER(void)
{
  // Removed optimization to support large chunk of data transfer as a temporary solution, maybe removed in future
  uint32_t channel;
  volatile uint32_t interrupt_status;
  volatile uint32_t error_status;
  uint32_t irq_type = 0;
  // Scan the channel which triggered interrupt
  for (channel = 0; channel < SL_DMA0_CHANNEL_COUNT; channel++) {
    irq_type = 0;
    // Check for transfer complete interrupt
    interrupt_status = UDMA_driver_resources[0]->reg->UDMA_DONE_STATUS_REG;
    if (interrupt_status & (1 << channel)) {
      // Interrupt is due to transfer complete
      UDMA_driver_resources[0]->reg->UDMA_DONE_STATUS_REG = (1 << channel);
      irq_type                                            = TRANSFER_COMPLETE_CALLBACK;
    } else {
      // Check for bus error interrupt
      error_status = UDMA_driver_resources[0]->reg->ERR_CLR;
      if (error_status & (1 << channel)) {
        // Interrupt is due to bus error
        UDMA_driver_resources[0]->reg->ERR_CLR &= CLR_BIT(channel);
        UDMA_driver_resources[0]->reg->UDMA_DONE_STATUS_REG = (1 << channel);
        irq_type                                            = ERROR_CALLBACK;
      }
    }
    if (irq_type) {
      // Process the interrupt.
      process_dma_irq(DMA_INSTANCE0, channel, irq_type);
    }
  }
}
/*******************************************************************************
 * Interrupt handler for ULP_DMA peripheral.
 * This function clears the interrupts
 * If transfer size is greater than DMA max transfer size, process_dma_irq
 * will initiate the transfer again until all the bytes are transferred
 * *****************************************************************************/
void DMA1_IRQ_HANDLER(void)
{
  uint32_t channel;
  volatile uint32_t interrupt_status;
  volatile uint32_t error_status;
  uint32_t irq_type = 0;

  // Scan the channel which triggered interrupt
  for (channel = 0; channel < SL_ULP_DMA_CHANNEL_COUNT; channel++) {
    irq_type = 0;
    // Check for transfer complete interrupt
    interrupt_status = UDMA_driver_resources[1]->reg->UDMA_DONE_STATUS_REG;
    if (interrupt_status & (1 << channel)) {
      // Interrupt is due to transfer complete
      UDMA_driver_resources[1]->reg->UDMA_DONE_STATUS_REG = (1 << channel);
      irq_type                                            = TRANSFER_COMPLETE_CALLBACK;
    } else {
      // Check for bus error interrupt
      error_status = UDMA_driver_resources[1]->reg->ERR_CLR;
      if (error_status & (1 << channel)) {
        // Interrupt is due to bus error
        UDMA_driver_resources[1]->reg->ERR_CLR &= CLR_BIT(channel);
        UDMA_driver_resources[1]->reg->UDMA_DONE_STATUS_REG = (1 << channel);
        irq_type                                            = ERROR_CALLBACK;
      }
    }
    if (irq_type) {
      // Process the interrupt.
      process_dma_irq(ULP_DMA_INSTANCE, channel, irq_type);
    }
  }
}
#endif /*SL_DMA_IRQ_HANDLER */

/*******************************************************************************
 * Callback function for DMA instance 0
 * It is called from the RSI Layer.
 * It provides the transfer complete / error callback to the SL layer
 * *****************************************************************************/
static void dma_internal_callback(uint32_t event, uint32_t channel)
{
  switch (event) {
    case UDMA_EVENT_XFER_DONE:
      if (sl_dma0_channel_allocation_data_t[channel].dma_callback_t.transfer_complete_cb != NULL) {
        // Transfer complete callback
        sl_dma0_channel_allocation_data_t[channel].dma_callback_t.transfer_complete_cb(channel, NULL);
      }
      break;
    case UDMA_EVENT_ERROR:
      if (sl_dma0_channel_allocation_data_t[channel].dma_callback_t.error_cb != NULL) {
        // error complete callback
        sl_dma0_channel_allocation_data_t[channel].dma_callback_t.error_cb(channel, NULL);
      }
      break;
    default:
      break;
  }
}

/*******************************************************************************
 * Callback function for DMA instance 1
 * It is called from the RSI Layer.
 * It provides the transfer complete / error callback to the SL layer
 * *****************************************************************************/
static void ulp_dma_internal_callback(uint32_t event, uint32_t channel)
{
  switch (event) {
    case UDMA_EVENT_XFER_DONE:
      if (sl_ulp_dma_channel_allocation_data_t[channel].dma_callback_t.transfer_complete_cb != NULL) {
        // Transfer complete callback
        sl_ulp_dma_channel_allocation_data_t[channel].dma_callback_t.transfer_complete_cb(channel, NULL);
      }
      break;
    case UDMA_EVENT_ERROR:
      if (sl_ulp_dma_channel_allocation_data_t[channel].dma_callback_t.error_cb != NULL) {
        // error complete callback
        sl_ulp_dma_channel_allocation_data_t[channel].dma_callback_t.error_cb(channel, NULL);
      }
      break;
    default:
      break;
  }
}
