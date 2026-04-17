/******************************************************************************
* @file  sl_si91x_gpdma.h
* @brief GPDMA API header file
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

#ifndef SL_SI91X_GPDMA_H
#define SL_SI91X_GPDMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "sl_si91x_gpdma_config.h"
#include "rsi_rom_gpdma.h"

#define sl_si91x_gpdma_irq_handler IRQ031_Handler

/***************************************************************************/
/**
* @addtogroup GPDMA GPDMA
* @ingroup SI91X_PERIPHERAL_APIS
* @{
   ******************************************************************************/
// -----------------------------------------------------------------------------

/****************************************************************************************************************
**************************************** Callback Functions typedefs ********************************************
*****************************************************************************************************************/
typedef void (*sl_gpdma_callback_t)(); ///< Callback function type

/******************************************************************************************************************
**************************************************    DEFINES   ***************************************************
*******************************************************************************************************************/

#define SL_STATUS_GPDMA_CHANNEL_ALLOCATED \
  (sl_status_t)0x61 ///< Status code indicates that the DMA channel already allocated for another transfer

#define SL_STATUS_GPDMA_NO_CHANNEL_AVAILABLE \
  (sl_status_t)0x62 ///< Status code indicates that no DMA channel is available for allocation

#define SL_STATUS_GPDMA_CHANNEL_ALREADY_UNALLOCATED \
  (sl_status_t)0X63 ///< Status code indicates that the DMA channel is already unallocated

#define SL_STATUS_GPDMA_CHANNEL_UNALLOCATED \
  (sl_status_t)0X64 ///< Status code indicates that channel is not allocated for DMA transfer

#define SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED \
  (sl_status_t)0x65 ///< Status code indicates that the channel is not available for allocation

#define SL_STATUS_GPDMA_FIFO_MEMORY_NOT_AVAILABLE \
  (sl_status_t)0x66 ///< Status code indicates that no DMA channel is available for allocation

#define SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_ALREADY_ALLOCATED \
  (sl_status_t)0x67 ///< Status code indicates that the descriptor memory buffer is already allocated

#define SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_NOT_SUFFICIENT \
  (sl_status_t)0x68 ///< Status code indicates that the descriptor memory buffer is not sufficient

#define SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED \
  (sl_status_t)0x69 ///< Status code indicates that the channel is not allocated for DMA transfer

#define SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_NOT_ALLOCATED \
  (sl_status_t)0x6A ///< Status code indicates that Memory buffer is not allocated for the given channel

#define SL_GPDMA_MAX_CHANNEL_PRIORITY 3 ///< Maximum GPDMA channel priority

#define SL_GPDMA_DATA_WIDTH_8  0x0 ///< Data width 8 bits
#define SL_GPDMA_DATA_WIDTH_16 0x1 ///< Data width 16 bits
#define SL_GPDMA_DATA_WIDTH_32 0x2 ///< Data width 32 bits

#define SL_GPDMACONTROLLER          0x0 ///< GPDMA controller
#define SL_SOURCE_PERIPHERAL        0x1 ///< Source peripheral
#define SL_DESTINATION_PERIPHERAL   0x2 ///< Destination peripheral
#define SL_SRC_AND_DEST_PERIPHERALS 0x3 ///< Source and destination peripherals

#define SL_GPDMA_MEMORY_TO_MEMORY         0x0 ///< Memory to memory transfer
#define SL_GPDMA_MEMORY_TO_PERIPHERAL     0x1 ///< Memory to peripheral transfer
#define SL_GPDMA_PERIPHERAL_TO_MEMORY     0x2 ///< Peripheral to memory transfer
#define SL_GPDMA_PERIPHERAL_TO_PERIPHERAL 0x3 ///< Peripheral to peripheral transfer

#define SL_GPDMA_MASTER_ZERO 0x0 ///< Master 0
#define SL_GPDMA_MASTER_ONE  0x1 ///< Master 1

#define SL_GPDMA_AHBBURST_SIZE_1  0b000 ///< AHB burst size 1
#define SL_GPDMA_AHBBURST_SIZE_4  0b001 ///< AHB burst size 4
#define SL_GPDMA_AHBBURST_SIZE_8  0b010 ///< AHB burst size 8
#define SL_GPDMA_AHBBURST_SIZE_16 0b011 ///< AHB burst size 16
#define SL_GPDMA_AHBBURST_SIZE_20 0b100 ///< AHB burst size 20
#define SL_GPDMA_AHBBURST_SIZE_24 0b101 ///< AHB burst size 24
#define SL_GPDMA_AHBBURST_SIZE_28 0b110 ///< AHB burst size 28
#define SL_GPDMA_AHBBURST_SIZE_32 0b111 ///< AHB burst size 32

#define SL_GPDMA_DESTINATION_DATA_BURST 16 ///< Destination data burst size
#define SL_GPDMA_SOURCE_DATA_BURST      16 ///< Source data burst size

#define SL_LINK_LIST_INTERRUPT_ENABLE  1 ///< Enable link list fetch done interrupt
#define SL_LINK_LIST_INTERRUPT_DISABLE 0 ///< Disable link list fetch done interrupt

#define SL_GPDMAC_FL0W_CTRL                  0x0 ///< DMA controller controls flow control
#define SL_SOURCE_PERIPHERAL_CTRL            0x1 ///< Source peripheral controls flow control
#define SL_DESTINATION_PERIIPHERAL_CTRL      0x2 ///< Destination peripheral controls flow control
#define SL_SOURCE_DESTIATION_PERIPHERAL_CTRL 0x3 ///< Source and destination peripheral controls flow control

#define SL_LINK_LIST_MODE_ENABLE  1 ///< Enable link list mode
#define SL_LINK_LIST_MODE_DISABLE 0 ///< Disable link list mode

#define SL_SOURCE_FIFO_MODE_ENABLE  1 ///< Enable source FIFO mode
#define SL_SOURCE_FIFO_MODE_DISABLE 0 ///< Disable source FIFO mode

#define SL_DESTINATION_FIFO_MODE_ENABLE  1 ///< Enable destination FIFO mode
#define SL_DESTINATION_FIFO_MODE_DISABLE 0 ///< Disable destination FIFO mode

#define SL_MEMORY_FILL_ENABLE  1 ///< Enable memory fill
#define SL_MEMORY_FILL_DISABLE 0 ///< Disable memory fill

#define SL_MEMORY_ONE_FILL  1 ///< Memory one fill
#define SL_MEMORY_ZERO_FILL 0 ///< Memory zero fill

#define SL_DMA_PROTECTION_ENABLE  1 ///< Enable DMA protection
#define SL_DMA_PROTECTION_DISABLE 0 ///< Disable DMA protection

#define MAX_TRANSFER_PER_DESCRIPTOR 4095 ///< Maximum transfer size per descriptor

#define SL_GPDMA_MAX_FIFO_SIZE 64 ///< Maximum FIFO size

#define SL_GPDMA_MAX_CALLBACK_TYPE 3 ///< Maximum number of callback types

/********************************************************************************************************************
 * **********************************************  typedefs *********************************************************
 ********************************************************************************************************************/

typedef RSI_GPDMA_DESC_T sl_si91x_gpdma_descriptor_t;                   ///< GPDMA descriptor structure
typedef GPDMA_DATACONTEXT_T sl_si91x_gpdma_data_context_t;              ///< GPDMA data context structure
typedef sl_si91x_gpdma_descriptor_t sl_si91x_gpdma_descriptor_config_t; ///< GPDMA descriptor configuration structure
typedef RSI_GPDMA_HANDLE_T sl_si91x_gpdma_handle_t;                     ///< GPDMA handle type
typedef RSI_GPDMA_CHA_CFG_T sl_si91x_gpdma_channel_config_t;            ///< GPDMA channel configuration structure
typedef RSI_GPDMA_INIT_T sl_si91x_gpdma_init_t;                         ///< GPDMA initialization structure
typedef gpdmaTransferCompleteCB sl_si91x_gpdma_transfer_complete_cb;    ///< GPDMA transfer complete callback type

/**
 * @brief Enumeration that holds 8-bit codes used by the callback_type parameter in the
 * sl_si91x_gpdma_unregister_callbacks function.
 * 
 * This enumeration defines the 8-bit codes for different types of DMA callbacks that can be unregistered.
 */
typedef enum {
  SL_GPDMA_HRESP_ERROR_CB           = 0, ///< HRESP error callback code
  SL_GPDMA_DESCRIPTOR_FETCH_DONE_CB = 1, ///< Descriptor fetch done callback code
  SL_GPDMA_TRANSFER_DONE_CB         = 2, ///< Transfer done callback code
  SL_GPDMA_GPDMAC_ERROR_CB          = 3, ///< GPDMA controller error callback code
} sl_gpdma_callback_code_t;

/**
  * @brief Structure to hold GPDMA channel callbacks.
  * 
  * This structure defines the callback functions for a DMA channel, including the callback for
  * transfer completion and the callback for error handling.
  */
typedef struct {
  sl_gpdma_callback_t transfer_complete_cb;         ///< Callback function for data transfer completion.
  sl_gpdma_callback_t descripotr_fetch_complete_cb; ///< Callback function for descriptor fetch completion.
  sl_gpdma_callback_t hresp_error_cb;               ///< Callback function for HRESP error handling.
  sl_gpdma_callback_t gpdmac_error_cb;              ///< Callback function for GPDMA controller error handling.
} sl_gpdma_callback_pointer_t;

/**
 * @brief Structure to hold GPDMA channel allocation data.
 * 
 * This structure defines the parameters related to the allocation of a GPDMA channel, including
 * callback functions, transfer type, descriptor memory location, number of descriptors, and FIFO size.
 */
typedef struct {
  sl_gpdma_callback_t gpdma_callback[4];          ///< Structure holding the DMA channel callback functions.
  sl_si91x_gpdma_descriptor_t *descriptor_memory; ///< Pointer to the descriptor memory for the channel.
  uint32_t link_list_mode_disable;                ///< Link list mode for the channel.
  uint32_t fifo_size;                             ///< FIFO size for the channel.
  uint32_t number_of_descriptors;                 ///< Number of descriptors allocated for the channel.
  uint32_t max_transfer_Size;                     ///< Maximum transfer size for the channel.
} sl_gpdma_channel_data_t;

/**
 * @brief Structure to hold GPDMA resource data.
 * 
 * This structure holds the resource data of GPDMA, including the number of allocated channels, channel priority bitmap, transfer type bitmap,
 * top FIFO, and transfer type.
 *
 * @details 
 * channel allocation bitmap: bit 0-7 for channel 0-7, 1 for unallocated and 0 for allocated
 * top_fifo: points to the FIFO from where FIFO is available               
 **/

typedef struct {
  uint32_t channel_allocation_bitmap; ///< Channel allocation bitmap
  uint32_t top_fifo;                  ///< Top FIFO number
  uint32_t no_of_allocated_channels;  ///< Number of allocated channels
  const uint32_t max_fifo;            ///< Maximum FIFO size
} sl_si91x_gpdma_resources_Data_t;

/********************************************************************************************************************
 * *********************************** function prototypes **************************************************************/
/***************************************************************************/
/**
 * @brief To initialize the GPDMA peripheral.
 * 
 * @details This API 
 * 1. Enables the GPDMA clock
 * 2. Clears GPDMA NVIC interrupt
 * 3. Enables GPDMA NVIC interrupt
 * 4. Masks GPDMA interruptsAssigns 
 * 5. Clears GPDMA interrupts
 * 
 * @return sl_status_t Initialization status:
*         - SL_STATUS_OK  - Initialization success.
*         - SL_STATUS_NOT_INITIALIZED - Initialization failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gpdma_init();

/***************************************************************************/
/**
 * @brief To allocate a GPDMA channel for the transfer.
 * 
 * @details This API checks the available GPDMA channels and allocates one. It sets the priority of the allocated channel
 * and assigns the channel number to the channel_no variable. If no channel is available, it will return SL_GPDMA_NO_CHANNEL_AVAILABLE.
 * 
 * Note: Users can also initialize the desired channel number, and this API checks whether the desired channel is available and allocates the
 * channel if available. If users want the driver to allocate an available channel, channel_no should be initialized to 0.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_dma_init must be called prior.
 * 
 * @param[in] channel_no Pointer to the channel number variable:
 *                       - 0xFF: Automatically allocates an available channel for DMA transfer.
 *                       - 0-7: Allocates the given channel for GPDMA transfer.
 * @param[in] priority Priority of the channel:
 *                     - 0: priority 1.
 *                     - 1: priority 2.
 *                     - 2: priority 2
 *                     - 3: priority 3
 * @param[in] max_transfer_size Maximum transfer size in the given channel.
 * 
 * @return sl_status_t Channel allocation status:
*         - SL_STATUS_OK  - Channel allocated.
*         - SL_STATUS_NOT_INITIALIZED  - GPDMA not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.
*         - SL_STATUS_GPDMA_NO_CHANNEL_AVAILABLE  - All GPDMA channels are allocated.
*         - SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED - The desired channel is already allocated.
*         - SL_STATUS_BUSY - Channel is busy. 
* 
* For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gpdma_allocate_channel(uint32_t *channel_no, uint32_t priority, uint32_t max_transfer_size);

/***************************************************************************/
/**
 * @brief To deallocate the GPDMA channel.
 * 
 * @details This API deallocates the GPDMA channel if there is no ongoing transfer on the channel.
 * Deallocating the channel will also unregister the callbacks for transfer complete and error.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gpdma_init must be called prior.
 *      - \ref sl_si91x_gpdma_allocate_channel must be called prior.
 * 
 * @param[in] channel_no Channel number:
 *                       - 0-7 for GPDMA.
 * 
 * @return sl_status_t Channel deallocation status:
*         - SL_STATUS_OK  - Channel deallocated.
*         - SL_STATUS_BUSY  - Cannot deallocate channel due to an ongoing transfer.
*         - SL_STATUS_NOT_INITIALIZED  - GPDMA not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.
 *        - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED  - Given channel is not allocated
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gpdma_deallocate_channel(uint32_t channel_no);

/***************************************************************************/
/**
 * @brief To register the GPDMA callbacks (transfer complete & error).
 * 
 * @details This API registers the GPDMA callbacks for transfer complete and error events. 
 * The user must update the sl_dma_callback_t structure and pass its address to this function.
 * A separate callback can be present for transfer complete and error for each channel.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gpdma_init must be called prior.
 *      - \ref sl_si91x_gpdma_allocate_channel must be called prior.
 * 
 * @param[in] channel_no Channel number:
 *                       - 0-7 for GPDMA.
 * @param[in] callback_t Pointer to the structure containing callback functions.
 * 
 * @return sl_status_t Callback registration status:
 *         - SL_STATUS_OK  - Callback registered successfully.
 *         - SL_STATUS_NOT_INITIALIZED  - GPDMA peripheral not initialized.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid channel number.
 *         - SL_STATUS_NULL_POINTER - Callback pointer is NULL. 
 *         - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED - Given channel is not allocated.
 *         - SL_STATUS_BUSY - channel is busy. 
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gpdma_register_callbacks(uint32_t channel_no, sl_gpdma_callback_pointer_t *callback_t);

/***************************************************************************/
/**
 * @brief To unregister the GPDMA callbacks (transfer complete and error).
 * 
 * @details This API unregisters the DMA callbacks for transfer complete and error events. 
 * Users need to update the 8-bit variable callback_type and pass it to the function. 
 * Each bit is mapped to a specific callback.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gpdma_init must be called prior.
 *      - \ref sl_si91x_gpdma_allocate_channel must be called prior.
 *      - \ref sl_si91x_gpdma_register_callbacks must be called prior.
 * 
 * @param[in] channel_no Channel number:
 *                       - 0-7 for UDMA0.
 * @param[in] callback_type Unregister the GPDMA callbacks based on the callback type (8-bit callback number):
 *                          - SL_GPDMA_HRESP_ERROR_CB: Unregister HRESP error callback.
 *                          - SL_GPDMA_DESCRIPTOR_FETCH_DONE_CB: Unregister descriptor fetch done callback.
 *                          - SL_GPDMA_TRANSFER_DONE_CB: Unregister transfer complete callback.
 *                          - SL_GPDMA_GPDMAC_ERROR_CB: Unregister GPDMA controller error callback.
 * 
 * @return sl_status_t Callback unregistration status:
*         - SL_STATUS_OK  - Callback unregistered successfully.
*         - SL_STATUS_NOT_INITIALIZED  - GPDMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid channel number or callback type.
*         - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED - Given channel is not allocated.
*         - SL_STATUS_BUSY - Channel is busy. 
* 
* For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gpdma_unregister_callbacks(uint32_t channel_no, uint8_t callback_type);

/***************************************************************************/
/**
 * @brief To start a simple memory-to-memory DMA transfer.
 * 
 * @details This API configures the DMA channel descriptor and initiates a simple memory-to-memory
 * DMA transfer. Users need to pass the source address and destination address of the transfer
 * along with the channel number.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gpdma_init must be called prior.
 *      - \ref sl_si91x_gpdma_allocate_channel must be called prior.
 *      - \ref sl_si91x_gpdma_register_callbacks must be called prior.
 * 
 * @param[in] channel Channel number: 0-7
 * @param[in] src Source address.
 * @param[in] dest Destination address.
 * 
 * @return sl_status_t DMA transfer status:
*         - SL_STATUS_OK  - Transfer success.
*         - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED - Given channel is not allocated.
*         - SL_STATUS_NOT_INITIALIZED  - GPDMA peripheral not initialized.
*         - SL_STATUS_SUSPENDED - Transfer initialization failed.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid or source or destination is NULL.
*         - SL_STATUS_BUSY - Channel is busy. 
* 
* For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gpdma_transfer(uint32_t channel, void *src, void *dest);

/***************************************************************************/
/**
 * @brief To stop the GPDMA transfer.
 * 
 * @details This API stops any active transfer on the channel by disabling the GPDMA channel.
 * If there is no active transfer on the channel, this function returns SL_STATUS_IDLE.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gpdma_init must be called prior.
 *      - \ref sl_si91x_gpdma_allocate_channel must be called prior.
 *      - \ref sl_si91x_gpdma_transfer must be called prior.
 * 
 * @param[in] channel_no Channel number:
 *                       - 0-7 for GPDMA.
 * 
 * @return sl_status_t DMA transfer status:
*         - SL_STATUS_OK  - Transfer stopped successfully.
*         - SL_STATUS_IDLE  - There is no active transfer on the channel.
*         - SL_STATUS_NOT_INITIALIZED  - GPDMA peripheral not initialized.
*         - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED - Given channel is not allocated.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid channel number.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gpdma_stop_transfer(uint32_t channel_no);

/***************************************************************************/

/**
 * @brief Allocates a GPDMA descriptor with default config for given channel based on tranfer size.
 *
 * @details This function allocates GPDMA descriptor 
 * in the provided memory location, fills the descriptors with default values, and attaches it to the given channel.
 * The descriptor is configured for the specified transfer size and channel number.
 *
 * @param[in,out] pDesc_memory   Pointer to the memory location where the descriptor
 *                               will be allocated and initialized,this memory must be pre-allocated by the caller.
 * @param[in]     transfer_size  Size of the data transfer in bytes.
 * @param[in]     channel_number GPDMA channel number 0-7 to be used for the transfer.
 *
 * @return sl_status_t
 *         - SL_STATUS_OK: Descriptor successfully allocated and initialized.
 *         - SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_NOT_SUFFICIENT: descriptor memory buffer size is not sufficient.
 *         - SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_ALREADY_ALLOCATED : descriptor memory buffer already allocated to given channel.
 *         - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED: Given channel is not allocated.
 *         - SL_STATUS_INVALID_PARAMETER: Invalid input parameters.
 *         - SL_STATUS_BUSY - Channel is busy. 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *************************************************************************************/

/***************************************************************************/
sl_status_t sl_si91x_gpdma_allocate_descriptor(sl_si91x_gpdma_descriptor_t *pDesc_memory,
                                               uint32_t transfer_size,
                                               uint32_t channel_number);

/**
 * @brief Deallocates a DMA descriptor for the specified channel number
 *
 * @details This function deallocates a previously allocated DMA descriptor associated
 *          with the given channel number.
 * @pre Pre-condition:
 *      - \ref sl_si91x_gpdma_init must be called prior.
 *      - \ref sl_si91x_gpdma_allocate_descriptor must be called prior.
 * @param[in] channel_number The channel number 0-7 from which to deallocate the descriptor.
 *
 * @return sl_status_t SL_STATUS_OK if the descriptor was successfully deallocated,
 *                     SL_STATUS_INVALID_PARAMETER if the channel number is invalid,
 *                     SL_STATUS_BUSY if the transfer is still in progress in given channel,
 *                     SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED given channel is not allocated,
 *
 * @note The channel must be disabled before calling this function to avoid unpredictable behavior.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 **************************************************************************************/

/****************************************************************************/
sl_status_t sl_Si91x_gpdma_deallocate_descriptor(uint32_t channel_number);

/**
 * @brief Builds a GPDMA descriptor with the specified configuration.
 *
 * This function initializes a GPDMA descriptor with the provided configuration. The descriptor can then be used for GPDMA
 * operations.
 *
 * @param[in,out] pDescriptor_memory Pointer to the memory location where the descriptor will be built.
 *                                    This memory must be pre-allocated by the caller.
 * @param[in] pDesc_config Pointer to the configuration structure containing the descriptor settings.
 * @param[in] transfer_size The size of the data transfer in bytes.
 * @param[in] channel_number The GPDMA 0-7 channel number to be used for the transfer.
 *
 * @return Status of the operation.
 *         - SL_STATUS_OK: If the descriptor was successfully built.
 *         - SL_STATUS_INVALID_PARAMETER: If any of the input parameters are invalid.
 *         - SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_NOT_SUFFICIENT: descriptor memory buffer size is not sufficient.
 *         - SL_STATUS_GPDMA_DESCRIPTOR_MEMORY_BUFFER_ALREADY_ALLOCATED : descriptor memory buffer already allocated to given channel.
 *         - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED: Given channel is not allocated.
 *         - SL_STATUS_BUSY - Channel is busy. 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *****************************************************************************/
/****************************************************************************/
sl_status_t sl_si91x_gpdma_build_descriptor(sl_si91x_gpdma_descriptor_t *pDescriptor_memory,
                                            sl_si91x_gpdma_descriptor_config_t *pDesc_config,
                                            uint32_t transfer_size,
                                            uint32_t channel_number);

/**
 * @brief Allocates a FIFO buffer of the specified size to a GPDMA channel
 *
 * This function allocates a FIFO (First In, First Out) buffer of the specified size
 * to the selected GPDMA channel.
 *
 * @param[in] channel_no The GPDMA channel number to allocate the FIFO for.
 * @param[in] fifo_size  The size of the FIFO buffer to allocate (in bytes).
 * @return    Status of the operation.
 *           - SL_STATUS_OK: If the FIFO was successfully allocated.
 *           - SL_STATUS_INVALID_PARAMETER: If the channel number is invalid or the FIFO size is not supported.
 *           - SL_STATUS_GPDMA_FIFO_MEMORY_NOT_AVAILABLE: If the FIFO memory is not available.
 *           - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED: Given channel is not allocated.
 *           - SL_STATUS_BUSY - Channel is busy. 
 *
 * @note The availability of channels and maximum FIFO size may vary based on hardware configuration.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *****************************************************************************/
/****************************************************************************/
sl_status_t sl_si91x_gpdma_allocate_fifo(uint32_t channel_no, uint32_t fifo_size);

/**
 * @brief Gets the FIFO size of a specified GPDMA channel
 *
 * This function retrieves the FIFO buffer size configured for the specified GPDMA channel.
 *
 * @param[in] channel_no The GPDMA channel number to query
 * @param[out] fifo_size Pointer to store the retrieved FIFO size value
 *
 * @return sl_status_t SL_STATUS_OK if successful,
 *                     SL_STATUS_INVALID_PARAMETER if channel number is invalid,
 *                     SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED if given channel is not allocated.
 *
 * @note The channel number should be within the valid range of available GPDMA channels.
 */
sl_status_t sl_si91x_get_channel_fifo_size(uint32_t channel_no, uint32_t *fifo_size);

/**
 * @brief Gets the current status of a specified GPDMA channel
 *
 * This function retrieves the status of the specified GPDMA channel.
 *
 * @param[in] channel_number The GPDMA channel number for which to get the status
 *
 * @return status of the operation.
 *        - SL_STATUS_OK: If the channel is free.
 *        - SL_STATUS_BUSY: If the transfer is in progress for given channel.
 *        - SL_STATUS_GPDMA_CHANNEL_NOT_ALLOCATED: If the channel is not allocated.
 *        - SL_STATUS_GPDMA_CHANNEL_ALREADY_ALLOCATED: If the channel is already allocated.
 *
 */
sl_status_t sl_si91x_gpdma_get_channel_status(uint32_t channel_number);

/**
 * @brief This function deinitializes the GPDMA driver.
 *
 * @details This API performs the following actions:
 * 1. Deinitializes the GPDMA driver by disabling the GPDMA clock and NVIC interrupt.
 * 2. Clears any pending interrupts and resets the GPDMA driver context to NULL.
 * 3. Clears the callback functions for transfer completion, descriptor fetch completion, HRESP error, and GPDMA controller error.
 * 4. Resets the GPDMA resource data and channel allocation data to the default state.
 *
 * @return Status of the operation.
 *        - SL_STATUS_OK: de-Initialization success.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 */
sl_status_t sl_si91x_gpdma_deinit(void);

/// @} end group GPDMA ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/*******************************************************************************/
/**
 * @addtogroup GPDMA GPDMA
 * @{
 *
 * @details
 *
 * @section GPDMA_Intro Introduction
 *
 * The GPDMA (General Purpose Direct Memory Access) peripheral enables efficient data transfers between memory and peripherals or between memory regions without CPU intervention. This offloads the CPU, reduces latency, and improves overall system performance.
 *
 * **Key Features**:
 * 1. Supports multiple independent DMA channels.
 * 2. Configurable channel priorities.
 * 3. Supports memory-to-memory, memory-to-peripheral, peripheral-to-memory, and peripheral-to-peripheral transfers.
 * 4. Flexible data width and burst size configuration.
 * 5. Linked list (descriptor-based) transfer support for complex or large data moves.
 * 6. FIFO buffer allocation for optimized throughput.
 * 7. Callback support for transfer completion and error handling.
 * 8. Hardware flow control options.
 *
 * @section GPDMA_Config Configuration
 *
 * The GPDMA can be configured for various transfer types and operational modes, including:
 * - **Memory-to-Memory**: Direct transfer between two memory locations.
 * - **Memory-to-Peripheral**: Transfer from memory to a peripheral register.
 * - **Peripheral-to-Memory**: Transfer from a peripheral register to memory.
 * - **Peripheral-to-Peripheral**: Transfer between two peripheral registers.
 * - **Linked List Mode**: Use descriptors for chained or scatter-gather transfers.
 * - **FIFO Allocation**: Assign FIFO buffers to channels for burst transfers.
 * - **Callback Registration**: Register callbacks for transfer complete, descriptor fetch, and error events.
 * These configurations are encapsulated in the @ref sl_si91x_gpdma_channel_config_t and related structures, and initialized using the provided GPDMA APIs.
 *
 * For more details on configuration parameters, see the respective peripheral example readme document.
 *
 * @section GPDMA_Usage Usage
 *
 * After defining the GPDMA configuration structures and passing the appropriate parameters, the following functions can be used to initialize and operate the GPDMA peripheral. The typical flow for implementation is as follows:
 * 1. Initialize the GPDMA driver: @ref sl_si91x_gpdma_init
 * 2. Allocate a DMA channel: @ref sl_si91x_gpdma_allocate_channel
 * 3. Register callbacks for transfer and error events: @ref sl_si91x_gpdma_register_callbacks
 * 4. Allocate and configure descriptors as needed: @ref sl_si91x_gpdma_allocate_descriptor, @ref sl_si91x_gpdma_build_descriptor
 * 5. Start a transfer: @ref sl_si91x_gpdma_transfer
 * 6. Optionally allocate FIFO buffers: @ref sl_si91x_gpdma_allocate_fifo
 * 7. Stop or deallocate the channel when done: @ref sl_si91x_gpdma_stop_transfer, @ref sl_si91x_gpdma_deallocate_channel
 *
 */
/** @} end group GPDMA */

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_GPDMA_H
