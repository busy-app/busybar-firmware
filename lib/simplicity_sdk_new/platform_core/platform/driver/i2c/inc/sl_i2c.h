/***************************************************************************//**
 * @file
 * @brief Inter-Integrated Circuit (I2C) driver API
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

#ifndef SL_I2C_H
#define SL_I2C_H

#include <stddef.h>
#include "sl_status.h"
#include "sl_enum.h"
#include "sl_device_peripheral.h"
#include "sl_device_i2c.h"
#include "sl_device_gpio.h"
#include "dmadrv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */
// *****************************************************************************
/// @addtogroup i2c I2C - Inter-Integrated Circuit
/// @brief Inter-Integrated Circuit driver
///
/// @li @ref i2c_intro
///
///@n @section i2c_intro Introduction
///  This is an I2C Driver.
///
/// @{
// *****************************************************************************
/* *INDENT-ON* */

/*******************************************************************************
 ********************************  DEFINES  ************************************
 ******************************************************************************/
#define SL_I2C_DMA_MAX_TX_DESCRIPTOR_COUNT 2
#define SL_I2C_DMA_MAX_RX_DESCRIPTOR_COUNT 4

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
/// Freq mode enum
SL_ENUM(sl_i2c_freq_mode_t) {
  SL_I2C_FREQ_STANDARD_MODE,     /// Standard mode (up to 100 kHz) assuming using 4:4 ratio for Nlow:Nhigh
  SL_I2C_FREQ_FAST_MODE,         /// Fast mode (up to 400 kHz) assuming using 6:3 ratio for Nlow:Nhigh
  SL_I2C_FREQ_FASTPLUS_MODE,     /// Fast mode+ (up to 1 MHz) assuming using 11:6 ratio for Nlow:Nhigh
};

/// Transfer direction enum
SL_ENUM(sl_i2c_transfer_direction_t) {
  SL_I2C_WRITE = 0,            /// Write: Send data to the I2C bus
  SL_I2C_READ = 1,             /// Read: Receive data from the I2C bus
  SL_I2C_WRITE_READ = 2,       /// Write followed by Read: Send then receive data in a single transaction
};

/// I2C Transfer event enum
SL_ENUM(sl_i2c_event_t) {
  SL_I2C_EVENT_IN_PROGRESS = 0,         /// Transfer in progress
  SL_I2C_EVENT_COMPLETED = 1,           /// Transfer completed successfully
  SL_I2C_EVENT_ADDR_NACK = 2,           /// Address byte not acknowledged
  SL_I2C_EVENT_DATA_NACK = 3,           /// Data byte not acknowledged
  SL_I2C_EVENT_ARBITRATION_LOST = 4,    /// Arbitration lost
  SL_I2C_EVENT_BUS_ERROR = 5,           /// Bus error occurred
  SL_I2C_EVENT_CLOCK_TIMEOUT = 6,       /// Clock timeout
  SL_I2C_EVENT_STOP = 7,                /// Transfer stopped
  SL_I2C_EVENT_SW_FAULT = 8,            /// Transfer fault due to software
  SL_I2C_EVENT_IDLE = 9,                /// Instance is idle
  SL_I2C_EVENT_INVALID_ADDR = 10,       /// Address invalid error
  SL_I2C_EVENT_TIMEOUT = 11,            /// Timeout
  SL_I2C_EVENT_BUFFER_FULL = 12         /// Buffer full, cannot receive more data
};

/// I2C state enum
SL_ENUM(sl_i2c_transaction_state_t) {
  SL_I2C_STATE_ERROR = 0,                                    /// Indicates an error occurred.
  SL_I2C_STATE_IDLE = 1,                                     /// I2C is idle.
  SL_I2C_STATE_SEND_START_AND_ADDR = 2,                      /// Send start and address byte.
  SL_I2C_STATE_ADDR_WAIT_FOR_ACK_OR_NACK = 3,                /// Wait for ACK/NACK on address byte.
  SL_I2C_STATE_ADDR_2ND_BYTE_10BIT_WAIT_FOR_ACK_OR_NACK = 4, /// Wait for ACK/NACK on 2nd byte (10-bit).
  SL_I2C_STATE_ADDRESS_MATCH = 5,                            /// Wait for address matching.
  SL_I2C_STATE_10BIT_ADDRESS_MATCH = 6,                      /// Wait for 10-bit address match.
  SL_I2C_STATE_SEND_REPEATED_START_AND_ADDR = 7,             /// Send repeated start and address byte.
  SL_I2C_STATE_REPEATED_ADDR_WAIT_FOR_ACK_OR_NACK = 8,       /// Wait for ACK/NACK for repeated start.
  SL_I2C_STATE_REP_ADDR_MATCH = 9,                           /// Wait for repeated start and address match.
  SL_I2C_STATE_SEND_DATA = 10,                               /// Send data byte.
  SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK = 11,               /// Wait for ACK or NACK after sending data.
  SL_I2C_STATE_RECEIVE_DATA = 12,                            /// Data to be received.
  SL_I2C_STATE_SEND_STOP = 13,                               /// Send stop signal.
  SL_I2C_STATE_WAIT_FOR_STOP = 14,                           /// Wait for stop condition.
  SL_I2C_STATE_DONE = 15                                     /// Transfer completed.
};

/*******************************************************************************
 *******************************   TYPEDEFS   **********************************
 ******************************************************************************/

typedef struct sl_i2c_handle_t sl_i2c_handle_t;  ///< Forward declaration of I2C handle type

/***************************************************************************//**
 * @brief Transfer Complete Callback
 *
 * @note  Invoked exactly once when an asynchronous I²C operation completes
 *        successfully (no errors). This applies to:
 *          - Transmit (TX)
 *          - Receive  (RX)
 *          - Combined Transmit-Receive (TX→RX / repeated-start)
 *
 * @param[in]  i2c_handle   Pointer to the I2C driver handle.
 * @param[in]  user_data    User-defined data provided during the asynchronous
 *                          transaction
 *
 * @return     Status code.
 ******************************************************************************/
typedef sl_status_t (*sl_i2c_transfer_complete_callback_t)(sl_i2c_handle_t *i2c_handle, void *user_data);

/***************************************************************************//**
 * @brief I2C Event Callback
 *
 * @note Invoked on I2C events other than transfer completion.
 *
 * @param[in] i2c_handle   Pointer to the I2C driver handle.
 * @param[in] event        Event type (see @ref sl_i2c_event_t).
 * @param[in] user_data    User-defined data provided during the asynchronous transaction.
 *
 * @return     Status code.
 ******************************************************************************/
typedef sl_status_t (*sl_i2c_event_callback_t)(sl_i2c_handle_t *i2c_handle, sl_i2c_event_t event, void *user_data);

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// I2C Tx and Rx DMA Channel and trigger source.
typedef struct {
  unsigned int dma_tx_channel;  /// DMA Channel assigned for Tx operations
  unsigned int dma_rx_channel;  /// DMA Channel assigned for Rx operations
} sl_i2c_dma_channel_info_t;

/// DMA descriptors for I2C transfers
typedef struct {
#if defined(EMDRV_DMADRV_LDMA)
  LDMA_Descriptor_t tx_desc[SL_I2C_DMA_MAX_TX_DESCRIPTOR_COUNT];
  LDMA_Descriptor_t rx_desc[SL_I2C_DMA_MAX_RX_DESCRIPTOR_COUNT];
#elif defined(EMDRV_DMADRV_LDMA_S3)
  sl_hal_ldma_descriptor_t tx_desc[SL_I2C_DMA_MAX_TX_DESCRIPTOR_COUNT];
  sl_hal_ldma_descriptor_t rx_desc[SL_I2C_DMA_MAX_RX_DESCRIPTOR_COUNT];
#endif
} sl_i2c_dma_descriptors_t;

/**
 * @struct sl_i2c_init_params_t
 * @brief Initialization parameters for an I2C Driver instance.
 *        This structure is passed to @ref sl_i2c_init() during initialization
 *        of an I2C instance.
 *
 * @note Usage of `follower_address` depends on the operating mode:
 *       Leader Mode   : Used to specify the address of the follower device.
 *       Follower Mode : Used to set the I2C device's own (self) address.
 */
typedef struct {
  sl_peripheral_t i2c_peripheral;                 ///< I2C Peripheral Instance
  sl_i2c_operating_mode_t operating_mode;         ///< Operating mode: Leader or Follower
  sl_i2c_freq_mode_t frequency_mode;              ///< I2C Speed mode (Standard, Fast, etc.)
  sl_gpio_t scl_gpio;                             ///< SCL GPIO Port and Pin (Serial Clock Line)
  sl_gpio_t sda_gpio;                             ///< SDA GPIO Port and Pin (Serial Data Line)
} sl_i2c_init_params_t;

/**
 * @struct sl_i2c_handle_t
 * @brief Represents an I2C instance handle.
 *        Contains configuration, buffers, state, and callbacks.
 *
 * @warning
 *       This structure is defined in the public header for driver implementation
 *       purposes only. Applications must NOT access, modify, or rely upon any
 *       members of this structure directly.
 *       Use only the provided API functions to interact with I2C handles.
 *
 * @note Usage of `follower_address` depends on the operating mode:
 *       Leader Mode   : Used to specify the address of the follower device.
 *       Follower Mode : Used to set the I2C device's own (self) address.
 */
typedef struct sl_i2c_handle_t {
  // Peripheral and configuration
  sl_peripheral_t                 i2c_peripheral;         ///< I2C Peripheral Instance
  sl_i2c_operating_mode_t         operating_mode;         ///< Operating mode: Leader or Follower
  sl_i2c_freq_mode_t              frequency_mode;         ///< I2C Speed mode (Standard, Fast, etc.)
  uint16_t                        follower_address;       ///< Follower address (see usage note above)
  sl_gpio_t                       scl_gpio;               ///< SCL GPIO Port and Pin (Serial Clock Line)
  sl_gpio_t                       sda_gpio;               ///< SDA GPIO Port and Pin (Serial Data Line)

  // DMA configuration
  sl_i2c_dma_channel_info_t       dma_channel;            ///< DMA channel info for Tx/Rx
  sl_i2c_dma_descriptors_t        dma_desc;               ///< DMA descriptors for I2C transfers

  // Buffer information
  uint32_t                        tx_offset;               ///< Number of bytes transferred (TX)
  uint32_t                        rx_offset;               ///< Number of bytes received (RX)

  // Transaction state and event
  sl_i2c_transaction_state_t      state;                  ///< Current transaction state
  sl_i2c_event_t                  event;                  ///< Current event
  sl_i2c_transfer_direction_t     transfer_direction;     ///< Transfer direction (Read/Write)
  uint8_t                         addr_buffer[3];         ///< Address buffer.

  // Callbacks and user data
  sl_i2c_transfer_complete_callback_t transfer_complete_callback;  ///< Transfer complete callback
  sl_i2c_event_callback_t             event_callback;              ///< Event callback
  void*                               user_data;                   ///< User defined data
} sl_i2c_handle_t;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * Initializes the I2C Module.
 *
 * @details Configures the I2C peripheral with the specified
 *          initialization parameters and prepares it for operation.
 *
 * @param[out] i2c_handle    A pointer to the I2C instance handle.
 * @param[in]  init_params   A pointer to initialization parameters.
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_ALREADY_INITIALIZED if already initialized.
 *   - SL_STATUS_INVALID_PARAMETER for invalid config.
 *   - SL_STATUS_NOT_SUPPORTED if peripheral is not valid.
 *   - SL_STATUS_ALLOCATION_FAILED if DMA allocation fails.
 ******************************************************************************/
sl_status_t sl_i2c_init(sl_i2c_handle_t *i2c_handle,
                        const sl_i2c_init_params_t *init_params);

/***************************************************************************//**
 * Deinitializes and resets the I2C instance.
 *
 * @details
 *   Disables interrupts, DMA, clocks, and resets hardware and GPIOs.
 *
 * @param[in] i2c_handle   Pointer to the I2C instance handle.
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if handle is NULL.
 ******************************************************************************/
sl_status_t sl_i2c_deinit(sl_i2c_handle_t *i2c_handle);

/***************************************************************************//**
 * Sets the frequency of the I2C bus.
 *
 * @param[in] i2c_handle        A pointer to the I2C instance handle.
 * @param[in] frequency_mode    Operating speed mode (e.g., Standard, Fast, Fast-Plus).
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if handle is NULL.
 *   - SL_STATUS_INVALID_PARAMETER if mode is invalid.
 ******************************************************************************/
sl_status_t sl_i2c_set_frequency(sl_i2c_handle_t *i2c_handle,
                                 sl_i2c_freq_mode_t frequency_mode);

/***************************************************************************//**
 * Get the current configured frequency of the I2C module.
 *
 * @param[in]  i2c_handle      Pointer to the I2C instance handle.
 * @param[out] frequency_mode  Pointer to store the current mode.
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_CONFIGURATION if frequency is out of range.
 ******************************************************************************/
sl_status_t sl_i2c_get_frequency(sl_i2c_handle_t *i2c_handle,
                                 sl_i2c_freq_mode_t *frequency_mode);

/***************************************************************************//**
 * Set the follower device address for I2C communication.
 *
 * @details Sets the address of the follower device that the I2C leader
 *          will communicate with during transactions.
 *          Validates address ranges and checks for reserved addresses.
 *
 * @param[in] i2c_handle      Pointer to the I2C instance handle.
 * @param[in] address         Follower device address (7-bit or 10-bit).
 * @param[in] is_10bit_addr   true for 10-bit address, false for 7-bit address.
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if handle is NULL.
 *   - SL_STATUS_INVALID_MODE if not in follower mode.
 *   - SL_STATUS_INVALID_PARAMETER if address is invalid.
 ******************************************************************************/
sl_status_t sl_i2c_set_follower_address(sl_i2c_handle_t *i2c_handle,
                                        uint16_t address,
                                        bool is_10bit_addr);

/***************************************************************************//**
 * Configures the DMA channels for the I2C instance.
 *
 * @note  This is an optional API that allows the user to assign custom DMA channels,
 *        overriding the default channels allocated by the driver.
 *
 * @param[in] i2c_handle   Pointer to the I2C instance handle.
 * @param[in] dma_channel  Pointer to the DMA channel info structure.
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_PARAMETER if channel info is invalid.
 ******************************************************************************/
sl_status_t sl_i2c_configure_dma(sl_i2c_handle_t *i2c_handle,
                                 sl_i2c_dma_channel_info_t *dma_channel);

/***************************************************************************//**
 * Leader Mode: Send data to the specified follower device (blocking).
 *
 * @details Transmits data to the follower device. Blocks until the transfer
 *          completes successfully or an error occurs.
 *
 * @param[in] i2c_handle         Pointer to the I2C instance handle.
 * @param[in] address            Address of the follower device.
 * @param[in] tx_buffer          Pointer to the transmit data buffer.
 * @param[in] tx_len             Length of the data to transmit.
 * @param[in] timeout            Timeout duration in milliseconds (0 = no timeout).
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in leader mode.
 *   - SL_STATUS_INVALID_PARAMETER if length/address invalid.
 *   - SL_STATUS_TIMEOUT if operation timed out.
 *   - SL_STATUS_NOT_FOUND if address NACK received.
 *   - SL_STATUS_ABORT if data NACK received.
 *   - SL_STATUS_TRANSMIT or SL_STATUS_IO for bus errors.
 ******************************************************************************/
sl_status_t sl_i2c_leader_send_blocking(sl_i2c_handle_t *i2c_handle,
                                        uint16_t address,
                                        const uint8_t *tx_buffer,
                                        uint32_t tx_len,
                                        uint32_t timeout);

/***************************************************************************//**
 * Leader Mode: Receive data from the specified follower device (blocking).
 *
 * @details Receives data from the follower device. Blocks until the transfer
 *          completes successfully or an error occurs.
 *
 * @param[in] i2c_handle        Pointer to the I2C instance handle.
 * @param[in] address           Address of the follower device.
 * @param[out] rx_buffer        Pointer to the receive data buffer.
 * @param[in] rx_len            Length of the data to receive.
 * @param[in] timeout           Timeout duration in milliseconds (0 = no timeout).
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in leader mode.
 *   - SL_STATUS_INVALID_PARAMETER if length/address invalid.
 *   - SL_STATUS_TIMEOUT if operation timed out.
 *   - SL_STATUS_NOT_FOUND if address NACK received.
 *   - SL_STATUS_ABORT if data NACK received.
 *   - SL_STATUS_TRANSMIT or SL_STATUS_IO for bus errors.
 ******************************************************************************/
sl_status_t sl_i2c_leader_receive_blocking(sl_i2c_handle_t *i2c_handle,
                                           uint16_t address,
                                           uint8_t *rx_buffer,
                                           uint32_t rx_len,
                                           uint32_t timeout);

/***************************************************************************//**
 * Leader Mode: Send then receive data from the specified follower device (blocking).
 *
 * @details Performs a combined write-read operation using a repeated start
 *          condition. Blocks until both operations complete or an error occurs.
 *
 * @param[in] i2c_handle        Pointer to the I2C instance handle.
 * @param[in] address           Address of the follower device.
 * @param[in] tx_buffer         Pointer to the transmit data buffer.
 * @param[in] tx_len            Length of the data to transmit.
 * @param[out] rx_buffer        Pointer to the receive data buffer.
 * @param[in] rx_len            Length of the data to receive.
 * @param[in] timeout           Timeout duration in milliseconds (0 = no timeout).
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in leader mode.
 *   - SL_STATUS_INVALID_PARAMETER if lengths/address invalid.
 *   - SL_STATUS_TIMEOUT if operation timed out.
 *   - SL_STATUS_NOT_FOUND if address NACK received.
 *   - SL_STATUS_ABORT if data NACK received.
 *   - SL_STATUS_TRANSMIT or SL_STATUS_IO for bus errors.
 ******************************************************************************/
sl_status_t sl_i2c_leader_transfer_blocking(sl_i2c_handle_t *i2c_handle,
                                            uint16_t address,
                                            const uint8_t *tx_buffer,
                                            uint32_t tx_len,
                                            uint8_t *rx_buffer,
                                            uint32_t rx_len,
                                            uint32_t timeout);

/***************************************************************************//**
 * Leader Mode: Send data to the specified follower device (non-blocking).
 *
 * @details Initiates a data transmission using DMA and interrupts. Returns
 *          immediately; completion is signaled via registered callbacks.
 *
 * @param[in] i2c_handle        Pointer to the I2C instance handle.
 * @param[in] address           Address of the follower device.
 * @param[in] tx_buffer         Pointer to the transmit data buffer.
 * @param[in] tx_len            Length of the data to transmit.
 * @param[in] user_data         User-defined data passed to callbacks.
 *
 * @return
 *   - SL_STATUS_OK if transfer initiated successfully.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in leader mode.
 *   - SL_STATUS_INVALID_PARAMETER if length/address invalid.
 *   - Other error codes for DMA/IRQ setup failures.
 ******************************************************************************/
sl_status_t sl_i2c_leader_send_non_blocking(sl_i2c_handle_t *i2c_handle,
                                            uint16_t address,
                                            const uint8_t *tx_buffer,
                                            uint32_t tx_len,
                                            void *user_data);

/***************************************************************************//**
 * Leader Mode: Receive data from the specified follower device (non-blocking).
 *
 * @details Initiates a data reception using DMA and interrupts. Returns
 *          immediately; completion is signaled via registered callbacks.
 *
 * @param[in] i2c_handle        Pointer to the I2C instance handle.
 * @param[in] address           Address of the follower device.
 * @param[out] rx_buffer        Pointer to the receive data buffer.
 * @param[in] rx_len            Length of the data to receive.
 * @param[in] user_data         User-defined data passed to callbacks.
 *
 * @return
 *   - SL_STATUS_OK if transfer initiated successfully.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in leader mode.
 *   - SL_STATUS_INVALID_PARAMETER if length/address invalid.
 *   - Other error codes for DMA/IRQ setup failures.
 ******************************************************************************/
sl_status_t sl_i2c_leader_receive_non_blocking(sl_i2c_handle_t *i2c_handle,
                                               uint16_t address,
                                               uint8_t *rx_buffer,
                                               uint32_t rx_len,
                                               void *user_data);

/***************************************************************************//**
 * Leader Mode: Send then receive data from the specified follower device (non-blocking).
 *
 * @details Initiates a combined write-read operation using a repeated start
 *          condition. Uses DMA and interrupts. Returns immediately; completion
 *          is signaled via registered callbacks.
 *
 * @param[in] i2c_handle        Pointer to the I2C instance handle.
 * @param[in] address           Address of the follower device.
 * @param[in] tx_buffer         Pointer to the transmit data buffer.
 * @param[in] tx_len            Length of the data to transmit.
 * @param[out] rx_buffer        Pointer to the receive data buffer.
 * @param[in] rx_len            Length of the data to receive.
 * @param[in] user_data         User-defined data passed to callbacks.
 *
 * @return
 *   - SL_STATUS_OK if transfer initiated successfully.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in leader mode.
 *   - SL_STATUS_INVALID_PARAMETER if lengths/address invalid.
 *   - Other error codes for DMA/IRQ setup failures.
 ******************************************************************************/
sl_status_t sl_i2c_leader_transfer_non_blocking(sl_i2c_handle_t *i2c_handle,
                                                uint16_t address,
                                                const uint8_t *tx_buffer,
                                                uint32_t tx_len,
                                                uint8_t *rx_buffer,
                                                uint32_t rx_len,
                                                void *user_data);

/***************************************************************************//**
 * Follower Mode: Send data to the I2C leader (blocking).
 *
 * @details Transmits data when addressed by the leader. Blocks until the
 *          transfer completes or an error occurs.
 *
 * @param[in] i2c_handle   Pointer to the I2C instance handle.
 * @param[in] tx_buffer    Pointer to the transmit data buffer.
 * @param[in] tx_len       Number of bytes to transmit.
 * @param[in] timeout      Timeout duration in milliseconds (0 = no timeout).
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in follower mode.
 *   - SL_STATUS_INVALID_PARAMETER if length invalid.
 *   - SL_STATUS_TIMEOUT if operation timed out.
 *   - SL_STATUS_FULL if buffer full.
 *   - SL_STATUS_TRANSMIT or SL_STATUS_IO for bus errors.
 ******************************************************************************/
sl_status_t sl_i2c_follower_send_blocking(sl_i2c_handle_t *i2c_handle,
                                          const uint8_t *tx_buffer,
                                          uint32_t tx_len,
                                          uint32_t timeout);

/***************************************************************************//**
 * Follower Mode: Receive data from the I2C leader (blocking).
 *
 * @details Receives data when addressed by the leader. Blocks until the
 *          transfer completes or an error occurs.
 *
 * @param[in] i2c_handle   Pointer to the I2C instance handle.
 * @param[out] rx_buffer   Pointer to the receive data buffer.
 * @param[in] rx_len       Number of bytes to receive.
 * @param[in] timeout      Timeout duration in milliseconds (0 = no timeout).
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in follower mode.
 *   - SL_STATUS_INVALID_PARAMETER if length invalid.
 *   - SL_STATUS_TIMEOUT if operation timed out.
 *   - SL_STATUS_FULL if buffer full.
 *   - SL_STATUS_TRANSMIT or SL_STATUS_IO for bus errors.
 ******************************************************************************/
sl_status_t sl_i2c_follower_receive_blocking(sl_i2c_handle_t *i2c_handle,
                                             uint8_t *rx_buffer,
                                             uint32_t rx_len,
                                             uint32_t timeout);

/***************************************************************************//**
 * Follower Mode: Send data to the I2C leader (non-blocking).
 *
 * @details Initiates data transmission when addressed by the leader using
 *          DMA and interrupts. Returns immediately; completion is signaled
 *          via registered callbacks.
 *
 * @param[in] i2c_handle   Pointer to the I2C instance handle.
 * @param[in] tx_buffer    Pointer to the transmit data buffer.
 * @param[in] tx_len       Number of bytes to transmit.
 * @param[in] user_data    User-defined data passed to callbacks.
 *
 * @return
 *   - SL_STATUS_OK if transfer initiated successfully.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in follower mode.
 *   - SL_STATUS_INVALID_PARAMETER if length invalid.
 *   - Other error codes for DMA/IRQ setup failures.
 ******************************************************************************/
sl_status_t sl_i2c_follower_send_non_blocking(sl_i2c_handle_t *i2c_handle,
                                              const uint8_t *tx_buffer,
                                              uint32_t tx_len,
                                              void *user_data);

/***************************************************************************//**
 * Follower Mode: Receive data from the I2C leader (non-blocking).
 *
 * @details Initiates data reception when addressed by the leader using
 *          DMA and interrupts. Returns immediately; completion is signaled
 *          via registered callbacks.
 *
 * @param[in] i2c_handle   Pointer to the I2C instance handle.
 * @param[out] rx_buffer   Pointer to the receive data buffer.
 * @param[in] rx_len       Number of bytes to receive.
 * @param[in] user_data    User-defined data passed to callbacks.
 *
 * @return
 *   - SL_STATUS_OK if transfer initiated successfully.
 *   - SL_STATUS_NULL_POINTER if arguments are NULL.
 *   - SL_STATUS_INVALID_MODE if not in follower mode.
 *   - SL_STATUS_INVALID_PARAMETER if length invalid.
 *   - Other error codes for DMA/IRQ setup failures.
 ******************************************************************************/
sl_status_t sl_i2c_follower_receive_non_blocking(sl_i2c_handle_t *i2c_handle,
                                                 uint8_t *rx_buffer,
                                                 uint32_t rx_len,
                                                 void *user_data);

/***************************************************************************//**
 * Register a transfer complete callback for non-blocking APIs.
 *
 * @details
 *   The callback is called once on successful completion of a non-blocking transfer.
 *
 * @note
 *   Passing NULL as the callback parameter will unset/disable the callback.
 *
 * @param[in] i2c_handle   Pointer to the I2C driver handle.
 * @param[in] callback     Transfer complete callback function.
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if handle is NULL.
 ******************************************************************************/
sl_status_t sl_i2c_set_transfer_complete_callback(sl_i2c_handle_t *i2c_handle,
                                                  sl_i2c_transfer_complete_callback_t callback);

/***************************************************************************//**
 * Register an event callback for non-blocking APIs.
 *
 * @details
 *   The callback is called on I2C events other than transfer completion.
 *
 * @note
 *   Passing NULL as the callback parameter will unset/disable the callback.
 *
 * @param[in] i2c_handle   Pointer to the I2C driver handle.
 * @param[in] callback     Event callback function.
 *
 * @return
 *   - SL_STATUS_OK on success.
 *   - SL_STATUS_NULL_POINTER if handle is NULL.
 ******************************************************************************/
sl_status_t sl_i2c_set_event_callback(sl_i2c_handle_t *i2c_handle,
                                      sl_i2c_event_callback_t callback);

/** @} (end addtogroup i2c driver) */

#ifdef __cplusplus
}
#endif

#endif /* SL_I2C_H */
