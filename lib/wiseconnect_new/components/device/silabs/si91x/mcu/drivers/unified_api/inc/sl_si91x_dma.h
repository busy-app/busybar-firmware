/******************************************************************************
* @file sl_si91x_dma.h
* @brief DMA API implementation
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

#ifndef SL_SI91X_DMA_
#define SL_SI91X_DMA_

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
#include "sl_si91x_dma_config.h"
#include "sl_status.h"
#include "UDMA.h"
/***************************************************************************/
/**
* @addtogroup DMA Direct Memory Access
* @ingroup SI91X_PERIPHERAL_APIS
* @{
   ******************************************************************************/
// -----------------------------------------------------------------------------
// Macros
#define SL_STATUS_DMA_CHANNEL_ALLOCATED \
  (sl_status_t)0x45 ///< Status code indicates that the DMA channel already allocated for another transfer
#define SL_STATUS_DMA_NO_CHANNEL_AVAILABLE \
  (sl_status_t)0x46 ///< Status code indicates that no DMA channel is available for allocation
#define SL_STATUS_DMA_CHANNEL_ALREADY_UNALLOCATED \
  (sl_status_t)0X47 ///< Status code indicates that the DMA channel is already unallocated
#define SL_STATUS_DMA_CHANNEL_UNALLOCATED \
  (sl_status_t)0X48         ///< Status code indicates that channel is not allocated for DMA transfer
#define SL_CHANNEL_COUNT 32 ///< DMA channel count

#define ALTERNATE_DESCRIPTOR_DISABLE        0     ///< DMA doesn't use the alternate descriptor
#define ALTERNATE_DESCRIPTOR_ENABLE         1     ///< DMA use alternate descriptor
#define BURST_REQUEST_ENABLE                1     ///< DMA burst request enable
#define BURST_REQUEST_DISABLE               0     ///< DMA burst request disable
#define CHANNEL_PRIO_DISABLE                0     ///< Channel has low priority
#define CHANNEL_PRIO_ENABLE                 1     ///< Channel has high priority
#define PERIPHERAL_ACK_DISABLE              0     ///< Peripheral ACK is disabled
#define PERIPHERAL_REQUEST_DISABLE          0     ///< Peripheral Request is disabled
#define PERIPHERAL_REQUEST_ENABLE           1     ///< Peripheral Request is enabled
#define REQUEST_MASK_DISABLE                0     ///< DMA request mask disabled
#define NEXT_BURST_ENABLE                   1     ///< Next burst enable
#define NEXT_BURST_DISABLE                  0     ///< Next burst disable
#define SOURCE_PROTECT_CONTROL_DISABLE      0x000 ///< Disable source protect control
#define DESTINATION_PROTECT_CONTROL_DISABLE 0x000 ///< Enable source protect control

// -----------------------------------------------------------------------------
// Data Types
/***************************************************************************/
/**
 * @brief Typedef for user-supplied callback function which is called when a DMA transfer completes.
 * 
 * This typedef defines the callback function type that is used by the DMA driver to notify
 * the application when a DMA transfer has completed.
 * 
 * @param[in] channel DMA channel number.
 * @param[in] data    An extra parameter for the user application.
 ******************************************************************************/
typedef void (*sl_dma_transfer_complete)(uint32_t channel, void *data);

/***************************************************************************/
/**
 * @brief Typedef for user-supplied callback function which is called when a DMA error occurs.
 * 
 * This typedef defines the callback function type that is used by the DMA driver to notify
 * the application when a DMA error has occurred.
 * 
 * @param[in] channel DMA channel number.
 * @param[in] data    An extra parameter for the user application.
 ******************************************************************************/
typedef void (*sl_dma_error)(uint32_t channel, void *data);

/**
 * @brief Enumeration to represent DMA transfer types.
 * 
 * This enumeration defines the different types of DMA transfers that can be performed.
 */
typedef enum {
  SL_DMA_MEMORY_TO_MEMORY,     ///< Memory to memory transfer.
  SL_DMA_MEMORY_TO_PERIPHERAL, ///< Memory to peripheral transfer.
  SL_DMA_PERIPHERAL_TO_MEMORY  ///< Peripheral to memory transfer.
} sl_dma_transfer_type_t;

/**
 * @brief Enumeration to represent DMA transfer modes.
 * 
 * This enumeration defines the different transfer modes available for DMA operations.
 */
typedef enum {
  SL_DMA_BASIC_MODE    = UDMA_MODE_BASIC,   ///< Basic DMA mode.
  SL_DMA_PINGPONG_MODE = UDMA_MODE_PINGPONG ///< Ping-pong DMA mode.
} sl_dma_transfer_mode_t;

/**
 * @brief Enumeration that holds peripheral ACK signals for DMA.
 * 
 * This enumeration defines the ACK codes for various peripherals that can be used with DMA.
 */
typedef enum {
  SL_USART0_ACK     = 0x01, ///< ACK code for USART0.
  SL_UART1_ACK      = 0x02, ///< ACK code for UART1.
  SL_UART3_ACK      = 0x03, ///< ACK code for UART3.
  SL_SSI_SLAVE_ACK  = 0x04, ///< ACK code for SSI slave.
  SL_SSI_MASTER_ACK = 0x05, ///< ACK code for SSI master.
  SL_SSI1_SLAVE_ACK = 0x06, ///< ACK code for SSI1 slave.
  SL_I2C_ACK        = 0x07  ///< ACK code for I2C.
} sl_dma_peripheral_ack_t;

/**
 * @brief Enumeration that holds DMA transfer sizes.
 * 
 * This enumeration defines the different transfer sizes available for DMA operations.
 */
typedef enum {
  SL_TRANSFER_SIZE_32 = SRC_SIZE_32, ///< 32-bit (4 bytes) transfer size.
  SL_TRANSFER_SIZE_16 = SRC_SIZE_16, ///< 16-bit (2 bytes) transfer size.
  SL_TRANSFER_SIZE_8  = SRC_SIZE_8   ///< 8-bit (1 byte) transfer size.
} sl_dma_transfer_size_t;

/**
 * @brief Enumeration that holds DMA transfer address increment options for source and destination.
 * 
 * This enumeration defines the different address increment options available for DMA transfers,
 * specifying how the source and destination addresses should be incremented after each transfer.
 */
typedef enum {
  SL_TRANSFER_SRC_INC_32   = SRC_INC_32,   ///< 4 bytes source address increment.
  SL_TRANSFER_SRC_INC_16   = SRC_INC_16,   ///< 2 bytes source address increment.
  SL_TRANSFER_SRC_INC_8    = SRC_INC_8,    ///< 1 byte source address increment.
  SL_TRANSFER_SRC_INC_NONE = SRC_INC_NONE, ///< No source address increment.
  SL_TRANSFER_DST_INC_32   = DST_INC_32,   ///< 4 bytes destination address increment.
  SL_TRANSFER_DST_INC_16   = DST_INC_16,   ///< 2 bytes destination address increment.
  SL_TRANSFER_DST_INC_8    = DST_INC_8,    ///< 1 byte destination address increment.
  SL_TRANSFER_DST_INC_NONE = DST_INC_NONE  ///< No destination address increment.
} sl_dma_transfer_inc_t;

/**
 * @brief Enumeration that holds 8-bit codes used by the callback_type parameter in the
 * sl_si91x_dma_unregister_callbacks function.
 * 
 * This enumeration defines the 8-bit codes for different types of DMA callbacks that can be unregistered.
 */
typedef enum {
  SL_DMA_TRANSFER_DONE_CB = 1, ///< 8-bit code for transfer complete callback.
  SL_DMA_ERROR_CB         = 2  ///< 8-bit code for error callback.
} sl_dma_callback_code_t;

/**
 * @brief Structure to hold DMA channel callbacks.
 * 
 * This structure defines the callback functions for a DMA channel, including the callback for
 * transfer completion and the callback for error handling.
 */
typedef struct {
  sl_dma_transfer_complete transfer_complete_cb; ///< Callback function for data transfer completion.
  sl_dma_error error_cb;                         ///< Callback function for error handling.
} sl_dma_callback_t;

/**
 * @brief Structure to hold DMA channel initialization parameters.
 * 
 * This structure defines the initialization parameters for a DMA channel, including the DMA controller number.
 */
typedef struct {
  uint32_t dma_number; ///< DMA controller number: 0 for UDMA0, 1 for ULP_DMA.
} sl_dma_init_t;

/**
 * @brief Structure to hold DMA channel allocation data.
 * 
 * This structure defines the parameters related to the allocation of a DMA channel, including
 * the channel priority, allocation status, callback functions, transfer type, and transfer mode.
 */
typedef struct {
  uint32_t priority;                ///< Channel priority.
  bool allocated;                   ///< Channel allocation status.
  sl_dma_callback_t dma_callback_t; ///< Structure holding the DMA channel callback functions.
  uint32_t transfer_type;           ///< DMA transfer type.
  uint32_t transfer_mode;           ///< DMA transfer mode.
} sl_channel_data_t;

/**
 * @brief DMA transfer structure.
 * 
 * This structure defines the parameters for a DMA transfer, including source and destination addresses,
 * address increment sizes, transfer size, total transfer length, transfer type, DMA mode, and the peripheral
 * signal that triggers the DMA transfer.
 */
typedef struct {
  uint32_t *src_addr;      ///< Source transfer address.
  uint32_t *dest_addr;     ///< Destination transfer address.
  uint32_t src_inc;        ///< Source address increment size.
  uint32_t dst_inc;        ///< Destination address increment size.
  uint32_t xfer_size;      ///< Transfer data size.
  uint32_t transfer_count; ///< Total transfer length.
  uint8_t transfer_type;   ///< DMA transfer type.
  uint8_t dma_mode;        ///< DMA transfer mode.
  uint8_t signal;          ///< Peripheral signal that triggers the DMA transfer (0 for software trigger).
} sl_dma_xfer_t;

/**
 * @brief DMA0 channel allocation data.
 * 
 * This external variable holds the allocation data for all channels of DMA0, including priority,
 * allocation status, callback functions, transfer type, and transfer mode.
 */
extern sl_channel_data_t sl_dma0_channel_allocation_data_t[SL_DMA0_CHANNEL_COUNT]; ///< DMA0 channel allocator

/**
 * @brief ULP_DMA channel allocation data.
 * 
 * This external variable holds the allocation data for all channels of ULP_DMA, including priority,
 * allocation status, callback functions, transfer type, and transfer mode.
 */
extern sl_channel_data_t sl_ulp_dma_channel_allocation_data_t[SL_ULP_DMA_CHANNEL_COUNT]; ///< ULP_DMA channel allocator

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
   *
   *
   */

/***************************************************************************/
/**
 * @brief To initialize the DMA peripheral.
 * 
 * @details This API enables the DMA clock and clears DMA interrupts. Configure 
 * dma_init->dma_number to 0 for initializing UDMA0 and 1 for initializing ULP_DMA.
 * 
 * @param[in] dma_init Pointer to the DMA initialization structure.
 *                     - dma_init->dma_number: 0 for UDMA0, 1 for ULP_DMA.
 * 
 * @return sl_status_t Initialization status:
*         - SL_STATUS_OK  - Initialization success.
*         - SL_STATUS_NOT_INITIALIZED - Initialization failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_init(const sl_dma_init_t *dma_init);

/***************************************************************************/
/**
 * @brief To de-initialize the DMA peripheral.
 * 
 * @details This API disables the DMA peripheral clock and interrupts. 
 * The DMA will be de-initialized only if there is no ongoing transfer.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_dma_init must be called prior.
 * 
 * @param[in] dma_number 0 for UDMA0, 1 for ULP_DMA.
 * 
 * @return sl_status_t De-initialization status:
*         - SL_STATUS_OK  - De-initialization success.
*         - SL_STATUS_BUSY  - Cannot de-initialize the peripheral due to an ongoing transfer.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_deinit(uint32_t dma_number);

/***************************************************************************/
/**
 * @brief To get the DMA handle.
 * 
 * @details This API returns a pointer to the DMA controller's data context structure,
 * which contains the control base address and other DMA controller information.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_dma_init must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * 
 * @return void* Pointer to the DMA data context structure.
 *         Returns NULL if DMA is not initialized or invalid dma_number is provided.
 ******************************************************************************/
void *sl_si91x_get_dma_handle(uint32_t dma_number);

/***************************************************************************/
/**
 * @brief To allocate a DMA channel for the transfer.
 * 
 * @details This API checks the available DMA channels and allocates one. It sets the priority of the allocated channel
 * and assigns the channel number to the *channel_no variable. If no channel is available, it will return SL_DMA_NO_CHANNEL_AVAILABLE.
 * 
 * Note: Users can also initialize the desired channel number, and this API checks whether the desired channel is available and allocates the
 * channel if available. If users want the driver to allocate an available channel, channel_no should be initialized to 0.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_dma_init must be called prior.
 * 
 * @param[in] dma_number 0 for UDMA0, 1 for ULP_DMA.
 * @param[in] channel_no Pointer to the channel number variable:
 *                       - 0: Automatically allocates an available channel for DMA transfer.
 *                       - 1-32: Allocates the given channel for UDMA0 transfer.
 *                       - 1-12: Allocates the given channel for ULP_DMA transfer.
 * @param[in] priority Priority of the channel:
 *                     - 0: Low priority.
 *                     - 1: High priority.
 * 
 * @return sl_status_t Channel allocation status:
*         - SL_STATUS_OK  - Channel allocated.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.
*         - SL_STATUS_DMA_NO_CHANNEL_AVAILABLE  - All DMA channels are allocated.
*         - SL_STATUS_DMA_CHANNEL_ALLOCATED - The desired channel is already allocated.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_allocate_channel(uint32_t dma_number, uint32_t *channel_no, uint32_t priority);

/***************************************************************************/
/**
 * @brief To deallocate the DMA channel.
 * 
 * @details This API deallocates the DMA channel if there is no ongoing transfer on the channel.
 * Deallocating the channel will also unregister the callbacks for transfer complete and error.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dma_init must be called prior.
 *      - \ref sl_si91x_dma_allocate_channel must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * 
 * @return sl_status_t Channel deallocation status:
*         - SL_STATUS_OK  - Channel deallocated.
*         - SL_STATUS_BUSY  - Cannot deallocate channel due to an ongoing transfer.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_deallocate_channel(uint32_t dma_number, uint32_t channel_no);

/***************************************************************************/
/**
 * @brief To register the DMA callbacks (transfer complete & error).
 * 
 * @details This API registers the DMA callbacks for transfer complete and error events. 
 * The user must update the sl_dma_callback_t structure and pass its address to this function.
 * A separate callback can be present for transfer complete and error for each channel.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dma_init must be called prior.
 *      - \ref sl_si91x_dma_allocate_channel must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * @param[in] callback_t Pointer to the structure containing callback functions.
 * 
 * @return sl_status_t Callback registration status:
*         - SL_STATUS_OK  - Callback registered successfully.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid channel number.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_register_callbacks(uint32_t dma_number, uint32_t channel_no, sl_dma_callback_t *callback_t);

/***************************************************************************/
/**
 * @brief To unregister the DMA callbacks (transfer complete & error).
 * 
 * @details This API unregisters the DMA callbacks for transfer complete and error events. 
 * Users need to update the 8-bit variable callback_type and pass it to the function. 
 * Each bit is mapped to a specific callback.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dma_init must be called prior.
 *      - \ref sl_si91x_dma_allocate_channel must be called prior.
 *      - \ref sl_si91x_dma_register_callbacks must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * @param[in] callback_type Unregister the DMA callbacks based on the callback type (bit-mapped to callbacks):
 *                          - SL_DMA_TRANSFER_DONE_CB: Unregister transfer complete callback.
 *                          - SL_DMA_ERROR_CB: Unregister error callback.
 *                          - SL_DMA_TRANSFER_DONE_CB | SL_DMA_ERROR_CB: Unregister both transfer complete and error callbacks.
 * 
 * @return sl_status_t Callback unregistration status:
*         - SL_STATUS_OK  - Callback unregistered successfully.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid channel number.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_unregister_callbacks(uint32_t dma_number, uint32_t channel_no, uint8_t callback_type);

/***************************************************************************/
/**
 * @brief To start the DMA transfer.
 * 
 * @details This API configures the DMA channel descriptor and initiates the DMA transfer.
 * The DMA primary descriptor is updated in this function, and based on the transfer mode,
 * the alternate descriptor is updated (only for ping pong mode). Other DMA parameters
 * like peripheral ACK signal (for peripheral memory transfers), DMA priority, etc., are
 * also updated in this function.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dma_init must be called prior.
 *      - \ref sl_si91x_dma_allocate_channel must be called prior.
 *      - \ref sl_si91x_dma_register_callbacks must be called prior.
 * 
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
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_transfer(uint32_t dma_number, uint32_t channel_no, sl_dma_xfer_t *dma_transfer_t);

/***************************************************************************/
/**
 * @brief To start a simple memory-to-memory DMA transfer.
 * 
 * @details This API configures the DMA channel descriptor and initiates a simple memory-to-memory
 * DMA transfer. Users need to pass the source address and destination address of the transfer
 * along with the transfer length in bytes.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dma_init must be called prior.
 *      - \ref sl_si91x_dma_allocate_channel must be called prior.
 *      - \ref sl_si91x_dma_register_callbacks must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * @param[in] src_addr Source address.
 * @param[in] dst_addr Destination address.
 * @param[in] data_size Transfer size in bytes.
 * 
 * @return sl_status_t DMA transfer status:
*         - SL_STATUS_OK  - Transfer success.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_SUSPENDED - Transfer initialization failed.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_simple_transfer(uint32_t dma_number,
                                         uint32_t channel_no,
                                         void *src_addr,
                                         void *dst_addr,
                                         uint32_t data_size);

/***************************************************************************/
/**
 * @brief To stop the DMA transfer.
 * 
 * @details This API stops any active transfer on the channel by disabling the DMA channel.
 * If there is no active transfer on the channel, this function returns SL_STATUS_IDLE.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dma_init must be called prior.
 *      - \ref sl_si91x_dma_allocate_channel must be called prior.
 *      - \ref sl_si91x_dma_simple_transfer or \ref sl_si91x_dma_transfer must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * 
 * @return sl_status_t DMA transfer status:
*         - SL_STATUS_OK  - Transfer stopped successfully.
*         - SL_STATUS_IDLE  - There is no active transfer on the channel.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Invalid channel number.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_stop_transfer(uint32_t dma_number, uint32_t channel_no);

/***************************************************************************/
/**
 * @brief To get the DMA channel status.
 * 
 * @details This API returns the status of the specified DMA channel. The possible statuses are:
 *          - SL_STATUS_DMA_CHANNEL_ALREADY_ALLOCATED - DMA channel allocated but no active transfer in progress.
 *          - SL_STATUS_BUSY - Channel is allocated and an active transfer is in progress.
 *          - SL_STATUS_IDLE - Channel is not allocated.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dma_init must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * 
 * @return sl_status_t Channel status:
*         - SL_STATUS_IDLE  - Channel is not allocated.
*         - SL_STATUS_BUSY  - Channel is allocated and busy.
*         - SL_STATUS_DMA_CHANNEL_ALREADY_ALLOCATED - Channel is already allocated and idle.
*         - SL_STATUS_NOT_INITIALIZED - DMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_channel_status_get(uint32_t dma_number, uint32_t channel_no);

/***************************************************************************/
/**
 * @brief To enable the DMA channel.
 * 
 * @details Enabling the channel will trigger the DMA transfer if DMA is already enabled.
 * The channel should be allocated and transfer parameters should be configured before enabling the channel.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_dma_init must be called prior.
 *      - \ref sl_si91x_dma_allocate_channel must be called prior.
 *      - Transfer parameters must be configured.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * 
 * @return sl_status_t Channel enable status:
*         - SL_STATUS_OK  - Channel enabled successfully.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_channel_enable(uint32_t dma_number, uint32_t channel_no);

/***************************************************************************/
/**
 * @brief To disable the DMA channel.
 * 
 * @details Disabling the DMA channel will abort any ongoing active transfers 
 * on the channel. Enabling the channel back will not resume the previous transfer.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_dma_init must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * @param[in] channel_no Channel number:
 *                       - 1-32 for UDMA0.
 *                       - 1-12 for ULP_DMA.
 * 
 * @return sl_status_t Channel disable status:
*         - SL_STATUS_OK  - Channel disabled successfully.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
*         - SL_STATUS_INVALID_PARAMETER  - Channel number is invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_channel_disable(uint32_t dma_number, uint32_t channel_no);

/***************************************************************************/
/**
 * @brief To enable the DMA peripheral.
 * 
 * @details Enabling the DMA will trigger the transfer if the channel is already enabled.
 * The channel should be allocated and transfer parameters should be configured before enabling the DMA.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_dma_init must be called prior.
 * 
 * @param[in] dma_number DMA number:
 *                       - 0 for UDMA0.
 *                       - 1 for ULP_DMA.
 * 
 * @return sl_status_t DMA enable status:
*         - SL_STATUS_OK  - DMA enabled successfully.
*         - SL_STATUS_NOT_INITIALIZED  - DMA peripheral not initialized.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_dma_enable(uint32_t dma_number);

/// @} end group DMA ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/**
* @addtogroup DMA Direct Memory Access
* @{
*
*   @details
*
*   @n @section DMA_Intro Introduction
*
*   Direct Memory Access (DMA) is a process of transferring data from one memory location
*   to another without the direct involvement of the processor (CPU). The main benefit of
*   using DMA is more efficient data movement in the embedded system.
*
*   Here are some common applications of DMA in microcontroller systems:
*
*   @li **Data Transfer**: One of the primary uses of DMA is to facilitate data transfer between
*   peripherals and memory.
*
*   @li **Memory-to-Memory Transfer**: DMA can also be utilized for moving data between different memory locations,
*   providing fast and efficient memory operations without CPU intervention.
*   This is particularly useful for tasks like memory copying, memory clearing, or memory initialization.
*
*   @li **Peripheral Initialization**: DMA can assist in initializing peripheral registers or buffers by
*   transferring predefined data patterns or configurations directly from memory to the peripheral
*   registers, speeding up the initialization process.
*
*   @li **Data Processing**: DMA can be employed for offloading data processing tasks from
*   the CPU to dedicated hardware peripherals.
*
*   @li **Real-Time Systems**: DMA is essential in real-time systems where predictable and
*   deterministic data transfer is crucial. By minimizing CPU involvement in data
*   transfer operations, DMA helps meet strict timing requirements and reduces the
*   likelihood of missed deadlines in time-critical applications.
*
*   @li **Power Management**: DMA can contribute to power efficiency by reducing CPU
*   wake-up times and allowing the CPU to enter low-power modes more frequently.
*   By handling data transfer tasks autonomously, DMA can help optimize power
*   consumption in battery-operated or energy-constrained systems.
*
*   @n @section DMA_Config Configuration
*
*   @li Configuring DMA involves three parameters that can be configured. The DMA instance can be configured
*   through the API @ref sl_si91x_dma_init(). The DMA channel can be configured in the range of 1-32 by passing the instance and
*   channel to the API @ref sl_si91x_dma_allocate_channel(). The transfer size can also be configured in the range of 1-10000.
*
*   @li For more information on configuring available parameters, see the respective peripheral example readme document.
*
*   @n @section DMA_Use Usage
*
*   The common DMA functions can be used after the DMA structures are specified, passing an instance of
*   @ref sl_dma_init_t. These functions will initiate and configure the DMA as described below, which is the flow for implementation.
*
* 1. @ref sl_si91x_dma_init : This API initializes the DMA peripheral.
* 2. @ref sl_si91x_dma_allocate_channel : This API allocates a DMA channel for the transfer.
* 3. @ref sl_si91x_dma_register_callbacks : This API registers the DMA callbacks (transfer complete & error).
* 4. @ref sl_si91x_dma_transfer : This API starts the DMA transfer.
* 5. @ref sl_si91x_dma_deinit : This function de-initializes the DMA peripheral.
*/
/** @} (end addtogroup DMA) */

#ifdef __cplusplus
}
#endif

#endif /*SL_SI917_DMA_*/
