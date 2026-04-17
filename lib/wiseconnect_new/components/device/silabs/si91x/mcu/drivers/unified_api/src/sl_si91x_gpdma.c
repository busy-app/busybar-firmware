/******************************************************************************
* @file  sl_si91x_gpdma.c
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

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "sl_si91x_gpdma.h"
#include <core_cm4.h>
#include "rsi_rom_clks.h"

/*******************************************************************************
 ***********************  Global function Prototypes *************************
 ******************************************************************************/
void sl_si91x_gpdma_irq_handler(void);

/****************************************************************************************************
*********************************** Variable declarations *******************************************
*****************************************************************************************************/

static sl_si91x_gpdma_data_context_t gpdma_driver; // GPDMA driver context

static sl_gpdma_channel_data_t sl_gpdma_channel_allocation_data_t[SL_GPDMA_MAX_CHANNEL + 1] = {
  0
}; // GPDMA channel data

static sl_si91x_gpdma_resources_Data_t gpdma_resource_data = {
  // GPDMA resource data
  .max_fifo                  = 64,       //maximum FIFO size is 64 bytes
  .top_fifo                  = 0,        //top FIFO is 0 by default
  .channel_allocation_bitmap = 0X00000FF //all the channels are unallocated by default
};

/*******************************************************************************
 * GPDMA IRQ Handler
 * 
 * @param none
 * @return none
 ******************************************************************************/
void sl_si91x_gpdma_irq_handler(void)
{          //GPDMA interrupt handler
  __DMB(); //Ensure all memory accesses are complete before proceeding for global variable update.
  RSI_GPDMA_InterruptHandler((void *)&gpdma_driver);
}

/*******************************************************************************************************************
 **********************************   LOCAL STATIC FUNCTION PROTOTYPES   *******************************************
 *******************************************************************************************************************/

/**
 * @brief Callback function for GPDMA host response handling
 *
 * This function serves as a callback that is triggered when a host response error
 * is received for a GPDMA operation. It handles the response for the specified
 * GPDMA descriptor on the given channel.
 *
 * @param[in] pDrv         Pointer to the GPDMA driver handle
 * @param[in] pDescriptor  Pointer to the GPDMA descriptor associated with the operation
 * @param[in] channel_no   Channel number (0-7) on which the GPDMA operation was performed
 *
 * @return None
 *
 * @note This is an internal function and not intended to be called by the application
 */
static void sli_si91x_gpdma_hresp_callback(sl_si91x_gpdma_handle_t pDrv,
                                           sl_si91x_gpdma_descriptor_t *pDescriptor,
                                           uint32_t channel_no);

/**
 * @brief Callback function invoked when a GPDMA descriptor fetch is completed
 *
 * This function is called internally when the GPDMA hardware has completed 
 * fetching a descriptor from memory. It handles the descriptor fetch completion
 * event for the specified channel.
 *
 * @param[in] pDrv         Pointer to the GPDMA driver handle
 * @param[in] pDescriptor  Pointer to the GPDMA descriptor that was fetched
 * @param[in] channel_no   Channel number on which the descriptor fetch occurred
 *
 * @note This is an internal callback function not intended to be called directly
 */
static void sli_si91x_gpdma_desc_fetch_done_callback(sl_si91x_gpdma_handle_t pDrv,
                                                     sl_si91x_gpdma_descriptor_t *pDescriptor,
                                                     uint32_t channel_no);

/**
 * @brief Callback function invoked when a GPDMA transfer is completed
 *
 * This internal function is called when a GPDMA transfer operation has been completed.
 * It processes the transfer completion event for the specified channel.
 *
 * @param[in] pDrv          Pointer to the GPDMA driver handle
 * @param[in] pDescriptor   Pointer to the GPDMA descriptor that completed the transfer
 * @param[in] channel_no    Channel number of the completed GPDMA transfer
 *
 * @note This is an internal callback function and not intended to be called by the user application.
 */
static void sli_si91x_gpdma_transfer_done_callback(sl_si91x_gpdma_handle_t pDrv,
                                                   sl_si91x_gpdma_descriptor_t *pDescriptor,
                                                   uint32_t channel_no);

/**
 * @brief Error callback function for GPDMA controller
 *
 * This function is called when an error occurs during GPDMA transfer operation.
 * It handles error conditions reported by the GPDMA controller.
 *
 * @param[in] pDrv         Pointer to the GPDMA driver handle
 * @param[in] pDescriptor  Pointer to the GPDMA descriptor structure associated with the error
 * @param[in] channel_no   Channel number on which the error occurred
 *
 * @note This is an internal callback function not intended to be called directly by applications.
 */
static void sli_si91x_gpdma_gpdmac_error_callback(sl_si91x_gpdma_handle_t pDrv,
                                                  sl_si91x_gpdma_descriptor_t *pDescriptor,
                                                  uint32_t channel_no);

/**
 * @brief Builds a linked list of GPDMA descriptors for memory transfer operations
 *
 * This function creates a chain of DMA descriptors based on the provided parameters.
 * It configures the descriptor list to handle the specified transfer size, potentially
 * breaking it into multiple descriptors if needed.
 *
 * @param[in,out] descriptor    Pointer to the first descriptor in the chain
 * @param[in]     desc_memory   Memory area to store additional descriptors
 * @param[in]     transfer_size Total size of the DMA transfer in bytes
 *
 * @note This is a helper function used internally by the GPDMA module
 */
static void sli_si91x_gpdma_build_descriptor_list(sl_si91x_gpdma_descriptor_t *descriptor,
                                                  sl_si91x_gpdma_descriptor_t *desc_memory,
                                                  uint32_t transfer_size);

/**
 * @brief Determines the position of a resource in a resource tracking variable
 *
 * @param[in] resource_variable Pointer to the resource tracking variable
 *
 * @return uint32_t The position/index of the set bit in the resource variable
 *
 * @note This is an internal helper function used for GPDMA resource management
 */
static uint32_t sli_si91x_resource_position(uint32_t *resource_variable);

static sl_status_t convert_arm_to_sl_error_code(rsi_error_t error_status);

/*******************************************************************************************************************
 ****************************************   STATIC INLINE FUNCTIONS   **********************************************
 *******************************************************************************************************************/

/**
    * @brief Get the GPDMA driver handle
    * @return Current GPDMA driver handle address
*/
__STATIC_INLINE sl_si91x_gpdma_data_context_t *sl_si91x_get_gpdma_handle(void)
{
  return &gpdma_driver;
}

/**
 * @brief Get the GPDMA resource data
    * @return Current GPDMA resource data variable address
*/
__STATIC_INLINE sl_si91x_gpdma_resources_Data_t *sl_si91x_get_gpdma_resources(void)
{
  return &gpdma_resource_data;
}

/*******************************************************************************************************************
 **********************************   LOCAL STATIC FUNCTION DEFINITIONS   *******************************************
 *******************************************************************************************************************/

/*
 1.hresp callback function calls user defined hresp callback function for HRESP error for the channel.
 2. It clears the channel allocation for the channel. 
*/
static void sli_si91x_gpdma_hresp_callback(sl_si91x_gpdma_handle_t pDrv,
                                           sl_si91x_gpdma_descriptor_t *pDescriptor,
                                           uint32_t channel_no)
{

  (void)pDrv;
  (void)pDescriptor;
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_HRESP_ERROR_CB]();
}

/*
 1.gpdma descriptor fetch done callback function calls user defined callback function for fetch done of given channel.
 2. It clears the channel allocation for the channel. 
*/
void sli_si91x_gpdma_desc_fetch_done_callback(sl_si91x_gpdma_handle_t pDrv,
                                              sl_si91x_gpdma_descriptor_t *pDescriptor,
                                              uint32_t channel_no)
{
  (void)pDrv;
  (void)pDescriptor;
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_DESCRIPTOR_FETCH_DONE_CB]();
}

/*
 1.Transfer done callback function calls user defied callback function for the transfer done of the  given channel.
 2. It clears the channel allocation for the channel. 
*/

static void sli_si91x_gpdma_transfer_done_callback(sl_si91x_gpdma_handle_t pDrv,
                                                   sl_si91x_gpdma_descriptor_t *pDescriptor,
                                                   uint32_t channel_no)
{
  (void)pDrv;
  (void)pDescriptor;
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_TRANSFER_DONE_CB]();
  pRes->channel_allocation_bitmap &= ~(1 << (channel_no));
}

/*
 1.Gpdma controller error callback function calls user defied callback function for gpdmac error of the  given channel.
 2. It clears the channel allocation for the channel. 
*/
void sli_si91x_gpdma_gpdmac_error_callback(sl_si91x_gpdma_handle_t pDrv,
                                           sl_si91x_gpdma_descriptor_t *pDescriptor,
                                           uint32_t channel_no)
{
  (void)pDrv;
  (void)pDescriptor;
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_GPDMAC_ERROR_CB]();
}

/*
 1.This function builds a linked list of GPDMA descriptors for memory transfer operations with the configurations of given descriptor.
*/
static void sli_si91x_gpdma_build_descriptor_list(sl_si91x_gpdma_descriptor_t *descriptor,
                                                  sl_si91x_gpdma_descriptor_t *desc_memory,
                                                  uint32_t transfer_size)
{
  sl_si91x_gpdma_descriptor_t *descriptor_memory = desc_memory;
  uint32_t no_of_descripotrs          = (transfer_size + MAX_TRANSFER_PER_DESCRIPTOR - 1) / MAX_TRANSFER_PER_DESCRIPTOR;
  descriptor_memory[0].chnlCtrlConfig = descriptor->chnlCtrlConfig;
  descriptor_memory[0].miscChnlCtrlConfig = descriptor->miscChnlCtrlConfig;
  for (uint8_t i = 1; i < no_of_descripotrs; i++) {
    descriptor_memory[i].chnlCtrlConfig                  = descriptor->chnlCtrlConfig;
    descriptor_memory[i].miscChnlCtrlConfig              = descriptor->miscChnlCtrlConfig;
    descriptor_memory[i - 1].pNextLink                   = (uint32_t *)(&descriptor[i]);
    descriptor_memory[i].chnlCtrlConfig.srcAddContiguous = 1;
    descriptor_memory[i].chnlCtrlConfig.dstAddContiguous = 1;
    transfer_size -= MAX_TRANSFER_PER_DESCRIPTOR;
  }
  descriptor_memory[no_of_descripotrs - 1].chnlCtrlConfig.transSize = transfer_size;
}

/*
 1.This function finds the first set bit in the resource variable and marks it as allocated.
 2. It returns the position of the first set bit in the resource variable.
*/
static uint32_t sli_si91x_resource_position(uint32_t *resource_variable)
{
  if (*resource_variable == 0)
    return 0xFFFF;                                 // No free descriptors
  int pos = __builtin_ffs(*resource_variable) - 1; // Find first set bit
  *resource_variable &= ~(1 << (pos));             // Mark as allocated
  return (pos);
}

/*******************************************************************************************************************
 *********************************************  FUNCTION DEFINITIONS  **********************************************
 *******************************************************************************************************************/

/*
 1. This function returns the status of the given channel.
*/
sl_status_t sl_si91x_gpdma_get_channel_status(uint32_t channel_number)
{
  sl_si91x_gpdma_data_context_t *pDrv   = sl_si91x_get_gpdma_handle();
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  uint8_t channel_no                    = channel_number;
  if (channel_number > SL_GPDMA_MAX_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (RSI_GPDMA_ChannelIsEnabled((void *)pDrv, channel_no)) {
    return SL_STATUS_BUSY;
  }
  if (pRes->channel_allocation_bitmap & (1 << channel_number)) {
    // Channel is not allocated
    return SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED;
  }
  return SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED;
}

/*
1.This function initializes the GPDMA driver and sets up the necessary configurations.
2. It enables the GPDMA clock ,NVIC interupt and clears any pending interrupts.
3. It also sets the callback functions for transfer completion, descriptor fetch completion, HRESP error, and GPDMA controller error.
*/
sl_status_t sl_si91x_gpdma_init()
{
  sl_status_t status                  = SL_STATUS_OK;
  sl_si91x_gpdma_data_context_t *pDrv = sl_si91x_get_gpdma_handle();
  pDrv->baseC                         = GPDMA_C;
  pDrv->baseG                         = GPDMA_G;
  pDrv->gpdmaCompCB                   = sli_si91x_gpdma_transfer_done_callback;
  pDrv->gpdmaDescFetchCompCB          = sli_si91x_gpdma_desc_fetch_done_callback;
  pDrv->gpdmaHrespErrorCB             = sli_si91x_gpdma_hresp_callback;
  pDrv->gpdmaRpdmacErrorCB            = sli_si91x_gpdma_gpdmac_error_callback;

  status = RSI_CLK_PeripheralClkEnable(M4CLK, RPDMA_CLK, ENABLE_STATIC_CLK);
  if (status != SL_STATUS_OK) {
    return status;
  }
  NVIC_ClearPendingIRQ(GPDMA_IRQn);
  NVIC_EnableIRQ(GPDMA_IRQn);
  return SL_STATUS_OK;
}

/*
1. This function deinitializes the GPDMA driver by disabling the GPDMA clock and NVIC interrupt.
2. It also clears any pending interrupts and resets the GPDMA driver context to NULL.
3. It also clears the callback functions for transfer completion, descriptor fetch completion, HRESP error, and GPDMA controller error.
4. It also resets the GPDMA resource data, channel allocation data to its default state.
*/
sl_status_t sl_si91x_gpdma_deinit()
{
  sl_status_t status                    = SL_STATUS_OK;
  sl_si91x_gpdma_data_context_t *pDrv   = sl_si91x_get_gpdma_handle();
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  pDrv->baseC                           = NULL;
  pDrv->baseG                           = NULL;
  pDrv->gpdmaCompCB                     = NULL;
  pDrv->gpdmaDescFetchCompCB            = NULL;
  pDrv->gpdmaHrespErrorCB               = NULL;
  pDrv->gpdmaRpdmacErrorCB              = NULL;
  pRes->channel_allocation_bitmap       = 0x000000FF;
  pRes->top_fifo                        = 0;
  memset(sl_gpdma_channel_allocation_data_t, 0, sizeof(sl_gpdma_channel_allocation_data_t));
  status = RSI_CLK_PeripheralClkDisable(M4CLK, RPDMA_CLK);
  if (status != SL_STATUS_OK) {
    return status;
  }
  NVIC_ClearPendingIRQ(GPDMA_IRQn);
  NVIC_DisableIRQ(GPDMA_IRQn);

  return SL_STATUS_OK;
}
/*
1. This function allocates FIFO for the given channel and sets the FIFO size.
*/
sl_status_t sl_si91x_gpdma_allocate_fifo(uint32_t channel_number, uint32_t fifo_size)
{
  sl_status_t status                    = SL_STATUS_OK;
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  sl_si91x_gpdma_data_context_t *pDrv   = sl_si91x_get_gpdma_handle();
  uint32_t fifo_start_address           = 0;
  if (channel_number > SL_GPDMA_MAX_CHANNEL || fifo_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  status = sl_si91x_gpdma_get_channel_status(channel_number);
  if (status != SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED) {
    return status;
  }
  if (((fifo_size - 1) + pRes->top_fifo) > pRes->max_fifo) {
    return SL_STATUS_GPDMA_FIFO_MEMORY_NOT_AVAILABLE;
  } else {
    fifo_start_address = (unsigned int)((pRes->top_fifo) & 0x3F);
    RSI_GPDMA_SET_CHANNEL_FIFO_SIZE((sl_si91x_gpdma_handle_t)pDrv, channel_number, fifo_size, fifo_start_address);
    pRes->top_fifo += fifo_size;
    sl_gpdma_channel_allocation_data_t[channel_number].fifo_size = fifo_size;
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
* This Allocates channel for DMA transfer
* If channel no is 0xFF, available channel is allocated
* If channel no is between 0 - 7, channel availability is chacked and allocated
 accordingly
*******************************************************************************/
sl_status_t sl_si91x_gpdma_allocate_channel(uint32_t *channel_no, uint32_t priority, uint32_t max_transfer_size)
{
  sl_status_t status                    = SL_STATUS_OK;
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  sl_si91x_gpdma_data_context_t *pDrv   = sl_si91x_get_gpdma_handle();
  uint32_t channel_alloc                = 0;
  if ((channel_no == NULL) || (priority > SL_GPDMA_MAX_CHANNEL_PRIORITY)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (*channel_no > SL_GPDMA_MAX_CHANNEL && *channel_no != 0xFF) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (pDrv->baseC == NULL || pDrv->baseG == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (*channel_no == 0xFF) {
    // Scan for available channels
    channel_alloc = sli_si91x_resource_position(&pRes->channel_allocation_bitmap);
    if (channel_alloc == 0xFFFF) {
      // No DMA channel is available
      status = SL_STATUS_GPDMA_NO_CHANNEL_AVAILABLE;
      return status;
    }
    *channel_no = channel_alloc;
    // Allocate the channel using channel allocator
    sl_gpdma_channel_allocation_data_t[channel_alloc].gpdma_callback[SL_GPDMA_HRESP_ERROR_CB]           = NULL;
    sl_gpdma_channel_allocation_data_t[channel_alloc].gpdma_callback[SL_GPDMA_DESCRIPTOR_FETCH_DONE_CB] = NULL;
    sl_gpdma_channel_allocation_data_t[channel_alloc].gpdma_callback[SL_GPDMA_TRANSFER_DONE_CB]         = NULL;
    sl_gpdma_channel_allocation_data_t[channel_alloc].gpdma_callback[SL_GPDMA_GPDMAC_ERROR_CB]          = NULL;
  } else {
    status = sl_si91x_gpdma_get_channel_status(*channel_no);
    if (status != SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED) {
      return status;
    }
    if (pRes->channel_allocation_bitmap & (1 << *channel_no)) {
      // Allocate the channel using channel allocator
      sl_gpdma_channel_allocation_data_t[*channel_no].gpdma_callback[SL_GPDMA_HRESP_ERROR_CB]           = NULL;
      sl_gpdma_channel_allocation_data_t[*channel_no].gpdma_callback[SL_GPDMA_DESCRIPTOR_FETCH_DONE_CB] = NULL;
      sl_gpdma_channel_allocation_data_t[*channel_no].gpdma_callback[SL_GPDMA_TRANSFER_DONE_CB]         = NULL;
      sl_gpdma_channel_allocation_data_t[*channel_no].gpdma_callback[SL_GPDMA_GPDMAC_ERROR_CB]          = NULL;
    } else {
      // Desired DMA channel is already allocated
      status = SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED;
    }
  }
  sl_gpdma_channel_allocation_data_t[*channel_no].max_transfer_Size = max_transfer_size;
  pRes->channel_allocation_bitmap &= ~(1 << *channel_no);
  RSI_GPDMA_SET_CHANNEL_PRIORITY((sl_si91x_gpdma_handle_t)pDrv, *channel_no, priority);
  status = sl_si91x_gpdma_allocate_fifo(*channel_no, 8);
  if (status != SL_STATUS_OK) {
    return status;
  }
  return status;
}

/*******************************************************************************
 * This function De-allocates channel for DMA transfer
 * Clears the channel allocator data and makes available for
 * other transfers
 * *****************************************************************************/
sl_status_t sl_si91x_gpdma_deallocate_channel(uint32_t channel_no)
{
  sl_status_t status                    = SL_STATUS_OK;
  sl_si91x_gpdma_data_context_t *pDrv   = sl_si91x_get_gpdma_handle();
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  if (channel_no > SL_GPDMA_MAX_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (pDrv->baseC == NULL || pDrv->baseG == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  status = sl_si91x_gpdma_get_channel_status(channel_no);
  if (status != SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED) {
    return status;
  }

  pRes->channel_allocation_bitmap |= (1 << channel_no);
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_HRESP_ERROR_CB]           = NULL;
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_DESCRIPTOR_FETCH_DONE_CB] = NULL;
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_TRANSFER_DONE_CB]         = NULL;
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_GPDMAC_ERROR_CB]          = NULL;

  RSI_GPDMA_SET_CHANNEL_PRIORITY((sl_si91x_gpdma_handle_t)pDrv, channel_no, 0);
  sl_gpdma_channel_allocation_data_t[channel_no].max_transfer_Size = 0;

  return SL_STATUS_OK;
}

/*******************************************************************************
 * This function registers DMA callbacks
 * sl_dma_transfer_complete called when transfer is completed
 * sl_dma_error is called when there is bus error while channel transfer
 * *****************************************************************************/
sl_status_t sl_si91x_gpdma_register_callbacks(uint32_t channel_no, sl_gpdma_callback_pointer_t *callback_t)
{
  sl_status_t status                             = SL_STATUS_OK;
  sl_si91x_gpdma_data_context_t *pDrv            = sl_si91x_get_gpdma_handle();
  sl_si91x_gpdma_channel_config_t channel_config = { 0 };

  if (pDrv->baseC == NULL || pDrv->baseG == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (channel_no > SL_GPDMA_MAX_CHANNEL) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (callback_t == NULL) {
    // Invalid callback structure
    return SL_STATUS_NULL_POINTER;
  }
  status = sl_si91x_gpdma_get_channel_status(channel_no);
  if (status != SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED) {
    return status;
  }
  // Register callbacks
  if (callback_t->transfer_complete_cb != NULL) {
    sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_TRANSFER_DONE_CB] =
      callback_t->transfer_complete_cb;
    channel_config.xferDoneIntr = 1;
  }
  if (callback_t->descripotr_fetch_complete_cb != NULL) {
    sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_DESCRIPTOR_FETCH_DONE_CB] =
      callback_t->descripotr_fetch_complete_cb;
    channel_config.descFetchDoneIntr = 1;
  }
  channel_config.dmaCh                                                                    = channel_no;
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_HRESP_ERROR_CB]  = callback_t->hresp_error_cb;
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[SL_GPDMA_GPDMAC_ERROR_CB] = callback_t->gpdmac_error_cb;

  RSI_GPDMA_InterruptEnable((sl_si91x_gpdma_handle_t)pDrv, &channel_config);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_gpdma_unregister_callbacks(uint32_t channel_no, uint8_t callback_type)
{
  sl_status_t status                             = SL_STATUS_OK;
  sl_si91x_gpdma_data_context_t *pDrv            = sl_si91x_get_gpdma_handle();
  sl_si91x_gpdma_channel_config_t channel_config = { 0 };
  if (pDrv->baseC == NULL || pDrv->baseG == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if ((channel_no > SL_GPDMA_MAX_CHANNEL) || (callback_type > SL_GPDMA_MAX_CALLBACK_TYPE)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  status = sl_si91x_gpdma_get_channel_status(channel_no);
  if (status != SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED) {
    return status;
  }
  sl_gpdma_channel_allocation_data_t[channel_no].gpdma_callback[callback_type] = NULL;
  if (callback_type == SL_GPDMA_TRANSFER_DONE_CB) {
    channel_config.xferDoneIntr = 1;
  } else if (callback_type == SL_GPDMA_DESCRIPTOR_FETCH_DONE_CB) {
    channel_config.descFetchDoneIntr = 1;
  }
  channel_config.dmaCh = channel_no;
  RSI_GPDMA_InterruptDisable((sl_si91x_gpdma_handle_t)pDrv, &channel_config);
  return SL_STATUS_OK;
}

/*
 1. This function allocates descriptors for the GPDMA channel in the given descriptor memory.
*/
sl_status_t sl_si91x_gpdma_allocate_descriptor(sl_si91x_gpdma_descriptor_t *pDescriptor_memory,
                                               uint32_t transfer_size,
                                               uint32_t channel_number)
{
  sl_status_t status                 = SL_STATUS_OK;
  sl_si91x_gpdma_descriptor_t *pDesc = pDescriptor_memory;
  uint32_t no_of_descripotrs         = (transfer_size + MAX_TRANSFER_PER_DESCRIPTOR - 1) / MAX_TRANSFER_PER_DESCRIPTOR;
  sl_si91x_gpdma_descriptor_t sl_gpdma_default_config = {
    .chnlCtrlConfig.transSize         = (uint32_t)(MAX_TRANSFER_PER_DESCRIPTOR),
    .chnlCtrlConfig.transType         = SL_GPDMA_MEMORY_TO_MEMORY,
    .chnlCtrlConfig.dmaFlwCtrl        = SL_GPDMAC_FL0W_CTRL,
    .chnlCtrlConfig.mastrIfFetchSel   = SL_GPDMA_MASTER_ZERO,
    .chnlCtrlConfig.mastrIfSendSel    = SL_GPDMA_MASTER_ONE,
    .chnlCtrlConfig.destDataWidth     = (unsigned int)SL_GPDMA_DATA_WIDTH_32,
    .chnlCtrlConfig.srcDataWidth      = (unsigned int)SL_GPDMA_DATA_WIDTH_32,
    .chnlCtrlConfig.srcAlign          = 1,
    .chnlCtrlConfig.linkListOn        = SL_LINK_LIST_MODE_ENABLE,
    .chnlCtrlConfig.linkListMstrSel   = SL_GPDMA_MASTER_ZERO,
    .chnlCtrlConfig.linkInterrupt     = SL_LINK_LIST_INTERRUPT_ENABLE,
    .chnlCtrlConfig.srcFifoMode       = SL_SOURCE_FIFO_MODE_DISABLE,
    .chnlCtrlConfig.dstFifoMode       = SL_DESTINATION_FIFO_MODE_DISABLE,
    .miscChnlCtrlConfig.ahbBurstSize  = (unsigned int)SL_GPDMA_AHBBURST_SIZE_8,
    .miscChnlCtrlConfig.destDataBurst = SL_GPDMA_DESTINATION_DATA_BURST,
    .miscChnlCtrlConfig.srcDataBurst  = SL_GPDMA_SOURCE_DATA_BURST,
    .miscChnlCtrlConfig.destChannelId = 0,
    .miscChnlCtrlConfig.srcChannelId  = 0,
    .miscChnlCtrlConfig.dmaProt       = SL_DMA_PROTECTION_DISABLE,
    .miscChnlCtrlConfig.memoryFillEn  = SL_MEMORY_FILL_DISABLE,
    .miscChnlCtrlConfig.memoryOneFill = SL_MEMORY_ZERO_FILL,
  };
  if (channel_number > SL_GPDMA_MAX_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (pDescriptor_memory == NULL || transfer_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  status = sl_si91x_gpdma_get_channel_status(channel_number);
  if (status != SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED) {
    return status;
  }
  if (no_of_descripotrs > sl_gpdma_channel_allocation_data_t[channel_number].max_transfer_Size) {
    return SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_NOT_SUFFICIENT;
  }

  if (sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory == NULL) {
    sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory = pDescriptor_memory;
  } else if (sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory != pDescriptor_memory) {
    return SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_ALREADY_ALLOCATED;
  }
  pDesc[0].chnlCtrlConfig     = sl_gpdma_default_config.chnlCtrlConfig;
  pDesc[0].miscChnlCtrlConfig = sl_gpdma_default_config.miscChnlCtrlConfig;
  pDesc[0].dest               = NULL;
  pDesc[0].src                = NULL;
  pDesc[0].pNextLink          = NULL;
  sli_si91x_gpdma_build_descriptor_list(pDesc, pDescriptor_memory, transfer_size);
  sl_gpdma_channel_allocation_data_t[channel_number].number_of_descriptors = no_of_descripotrs;
  return SL_STATUS_OK;
}

/*
 1. This function deallocates the descriptor memory for the given channel.
*/
sl_status_t sl_Si91x_gpdma_deallocate_descriptor(uint32_t channel_number)
{
  sl_status_t status = SL_STATUS_OK;
  if (channel_number > SL_GPDMA_MAX_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  status = sl_si91x_gpdma_get_channel_status(channel_number);
  if (status != SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED) {
    return status;
  }
  if (sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory == NULL) {
    return SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_NOT_ALLOCATED;
  }
  sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory = NULL;
  return SL_STATUS_OK;
}

/*
 1. This function initiates a GPDMA transfer using the specified channel, source, and destination addresses.
*/

sl_status_t sl_si91x_gpdma_transfer(uint32_t channel_number, void *pSource, void *pDestination)
{
  sl_status_t status                       = SL_STATUS_OK;
  sl_si91x_gpdma_data_context_t *pDrv      = sl_si91x_get_gpdma_handle();
  sl_si91x_gpdma_descriptor_t *descriptors = sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory;
  uint32_t number_of_descriptors           = sl_gpdma_channel_allocation_data_t[channel_number].number_of_descriptors;
  if (pSource == NULL || pDestination == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (channel_number > SL_GPDMA_MAX_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  status = sl_si91x_gpdma_get_channel_status(channel_number);
  if (status != SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED) {
    return status;
  }
  for (uint32_t i = 0; i < number_of_descriptors; i++) {
    descriptors[i].src  = (void *)((uintptr_t)pSource + i * MAX_TRANSFER_PER_DESCRIPTOR);
    descriptors[i].dest = (void *)((uintptr_t)pDestination + i * MAX_TRANSFER_PER_DESCRIPTOR);
  }
  if (sl_gpdma_channel_allocation_data_t[channel_number].link_list_mode_disable == false) {
    RSI_GPDMA_Enable_Link_List_Mode((void *)pDrv, channel_number);

    RSI_GPDMA_Link_List_Pointer_Register((void *)pDrv, descriptors, channel_number);
  }
  RSI_GPDMA_DMAChannelTrigger((void *)pDrv, channel_number);

  return SL_STATUS_OK;
}

/*
 1. This function stops the GPDMA transfer for the specified channel using GPDMA squash register.
*/
sl_status_t sl_si91x_gpdma_stop_transfer(uint32_t channel_no)
{
  sl_status_t status                    = SL_STATUS_OK;
  sl_si91x_gpdma_data_context_t *pDrv   = sl_si91x_get_gpdma_handle();
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  if (pDrv->baseC == NULL || pDrv->baseG == NULL) {
    // DMA not initialized
    return SL_STATUS_NOT_INITIALIZED;
  }
  if ((channel_no > SL_GPDMA_MAX_CHANNEL)) {
    // Invalid channel number
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (pRes->channel_allocation_bitmap & (1 << channel_no)) {
    return SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED;
  }
  status = sl_si91x_gpdma_get_channel_status(channel_no);
  if (status != SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED) {
    return SL_STATUS_IDLE;
  }
  RSI_GPDMA_AbortChannel((sl_si91x_gpdma_handle_t)pDrv, channel_no);
  return SL_STATUS_OK;
}
/**
 1. Builds a GPDMA descriptor with the specified configuration.
 2. Create a linked list of descriptors based on that descriptor and the transfer size.
 */
sl_status_t sl_si91x_gpdma_build_descriptor(sl_si91x_gpdma_descriptor_t *pDescriptor_memory,
                                            sl_si91x_gpdma_descriptor_config_t *pDesc_config,
                                            uint32_t transfer_size,
                                            uint32_t channel_number)
{
  sl_status_t status                    = SL_STATUS_OK;
  sl_si91x_gpdma_data_context_t *pDrv   = sl_si91x_get_gpdma_handle();
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  uint32_t no_of_descripotrs         = (transfer_size + MAX_TRANSFER_PER_DESCRIPTOR - 1) / MAX_TRANSFER_PER_DESCRIPTOR;
  rsi_error_t error_status           = RSI_OK;
  sl_si91x_gpdma_descriptor_t *pDesc = pDescriptor_memory;
  if (pDesc_config == NULL || pDescriptor_memory == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (transfer_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (pRes->channel_allocation_bitmap & (1 << channel_number)) {
    return SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED;
  }
  if (pDesc_config->chnlCtrlConfig.linkListOn == false) {
    if (transfer_size > 4095) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    error_status = RSI_GPDMA_SetupChannelTransfer((void *)pDrv, channel_number, pDesc_config);
    status       = convert_arm_to_sl_error_code(error_status);
    if (status != SL_STATUS_OK) {
      return status;
    }
    sl_gpdma_channel_allocation_data_t[channel_number].link_list_mode_disable = true;
    return SL_STATUS_OK;
  }
  sl_gpdma_channel_allocation_data_t[channel_number].link_list_mode_disable = false;

  if (sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory == NULL) {
    sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory = pDescriptor_memory;
  } else if (sl_gpdma_channel_allocation_data_t[channel_number].descriptor_memory != pDescriptor_memory) {
    return SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_ALREADY_ALLOCATED;
  }
  error_status = RSI_GPDMA_BuildDescriptors((void *)pDrv, pDesc_config, pDesc, NULL);
  status       = convert_arm_to_sl_error_code(error_status);
  if (status != SL_STATUS_OK) {
    return status;
  }
  sli_si91x_gpdma_build_descriptor_list(pDesc, pDescriptor_memory, transfer_size);
  sl_gpdma_channel_allocation_data_t[channel_number].number_of_descriptors = no_of_descripotrs;
  return status;
}

/*
1.Return FIFO allocated to a channel
*/
sl_status_t sl_si91x_get_channel_fifo_size(uint32_t channel_number, uint32_t *fifo_size)
{
  sl_si91x_gpdma_resources_Data_t *pRes = sl_si91x_get_gpdma_resources();
  if (channel_number > SL_GPDMA_MAX_CHANNEL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (pRes->channel_allocation_bitmap & (1 << channel_number)) {
    // Channel not allocated
    return SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED;
  }
  *fifo_size = sl_gpdma_channel_allocation_data_t[channel_number].fifo_size;
  return SL_STATUS_OK;
}

sl_status_t convert_arm_to_sl_error_code(rsi_error_t error_status)
{
  sl_status_t status = SL_STATUS_OK;
  switch (error_status) {
    case RSI_OK:
      status = SL_STATUS_OK;
      break;
    case ERROR_GPDMA_INVALID_ARG:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_GPDMA_INVALID_TRANS_LEN:
      //  status = SL_STATUS_GPDMA_INVALID_TRANSFER_LENGTH;
      break;
    case ERROR_GPDMA_INVALID_XFERMODE:
      //  status = SL_STATUS_GPDMA_TRANSFER_INVALID_MODE;
      break;
    case ERROR_GPDMA_FLW_CTRL:
      //  status = SL_STATUS_GPDMA_INVALID_FLOW_CONTROL;
      break;
    default:
      status = SL_STATUS_FAIL;
      break;
  }
  return status;
}
#ifdef __cplusplus
}
#endif
