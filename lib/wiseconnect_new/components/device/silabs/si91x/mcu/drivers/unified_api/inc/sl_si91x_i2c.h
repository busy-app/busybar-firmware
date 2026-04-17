/******************************************************************************
* @file sl_si91x_i2c.h
* @brief I2C API implementation
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
//// Includes
#ifndef SL_SI91X_I2C_H
#define SL_SI91X_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_peripheral_i2c.h"
#include "stdio.h"
#include "sl_i2c_board.h"

/***************************************************************************/ /**
* @addtogroup I2C I2C
* @ingroup SI91X_PERIPHERAL_APIS
* @{
******************************************************************************/
/***************************************************************************/ /**
* @brief I2C Peripheral
*
* ##Overview
* I2C standard compliant bus interface with open-drain pins.
* Configurable as Primary or Secondary.
* Four speed modes: Standard Mode (100 kbps), Fast Mode (400 kbps),
* Fast Mode Plus (1 Mbps), and High-Speed Mode (3.4 Mbps).
* 7 or 10-bit addressing.
* 7 or 10-bit combined format transfers.
* Support for clock synchronization and bus clear.
* Programmable SDA hold time.
* Integrated transmit and receive buffers with support for DMA.
* Bulk transmit mode in I2C slave mode.
* Interrupt-based operation (polled mode also available).
*
* ## Initialization
* Call init API with the init parameters.
* Call set follower address API in follower mode.
* Call set FIFO threshold API.
*
******************************************************************************/

// -----------------------------------------------------------------------------
// Macros for i2c parameters
#define SL_I2C0_DMA_TX_CHANNEL    31                        ///< I2C0 DMA TX channel number
#define SL_I2C0_DMA_RX_CHANNEL    30                        ///< I2C0 DMA RX channel number
#define SL_I2C1_DMA_TX_CHANNEL    3                         ///< I2C1 DMA TX channel number
#define SL_I2C1_DMA_RX_CHANNEL    2                         ///< I2C1 DMA RX channel number
#define SL_ULP_I2C_DMA_TX_CHANNEL 5                         ///< ULP_I2C DMA TX channel number
#define SL_ULP_I2C_DMA_RX_CHANNEL 4                         ///< ULP_I2C DMA RX channel number
#define SL_I2C2_DMA_TX_CHANNEL    SL_ULP_I2C_DMA_TX_CHANNEL ///< ULP_I2C DMA TX channel number for backward compatibility
#define SL_I2C2_DMA_RX_CHANNEL    SL_ULP_I2C_DMA_RX_CHANNEL ///< ULP_I2C DMA RX channel number for backward compatibility

// -----------------------------------------------------------------------------
// Data Types

/**
 * @brief Enumeration to represent I2C instances.
 * 
 * This enumeration defines the different I2C instances available in the system.
 */
typedef enum {
  SL_I2C0,              ///< I2C Instance 0.
  SL_I2C1,              ///< I2C Instance 1.
  SL_I2C2,              ///< I2C Instance 2 (ULP_I2C).
  SL_ULP_I2C = SL_I2C2, ///< ULP_I2C Instance
  SL_I2C_LAST,          ///< Last member of enum for validation.
} sl_i2c_instance_t;

/**
 * @brief Enumeration to represent I2C driver status values.
 * 
 * This enumeration defines the various status values that can be returned by the I2C driver.
 */
typedef enum {
  SL_I2C_SUCCESS,                  ///< Operation completed successfully.
  SL_I2C_IDLE,                     ///< I2C is idle.
  SL_I2C_7BIT_ADD,                 ///< I2C 7-bit address transfer.
  SL_I2C_10BIT_ADD,                ///< I2C 10-bit address transfer.
  SL_I2C_10BIT_ADD_WITH_REP_START, ///< I2C 10-bit address transfer with repeated start.
  SL_I2C_ACKNOWLEDGE,              ///< I2C acknowledge received.
  SL_I2C_NACK,                     ///< I2C not acknowledge received.
  SL_I2C_DATA_TRANSFER_COMPLETE,   ///< I2C data transfer complete.
  SL_I2C_ARIBITRATION_LOST,        ///< I2C arbitration lost.
  SL_I2C_BUS_ERROR,                ///< I2C bus error.
  SL_I2C_BUS_HOLD,                 ///< I2C bus held by another master.
  SL_I2C_SDA_ERROR,                ///< I2C SDA at loopback path is not equal to SDA output.
  SL_I2C_SCL_ERROR,                ///< I2C SCL at loopback path is not equal to SCL output.
  SL_I2C_CALLBACK_BUSY,            ///< I2C instance callback already registered.
  SL_I2C_DMA_TRANSFER_ERROR,       ///< I2C DMA transfer error.
  SL_I2C_INVALID_PARAMETER,        ///< Invalid parameter.
  SL_I2C_TIMEOUT,                  ///< I2C timeout for blocking calls.
} sl_i2c_status_t;

/**
 * @brief Enumeration to represent I2C transfer type.
 * 
 * This enumeration defines the different transfer types that can be used by the I2C driver.
 */
typedef enum {
  SL_I2C_USING_NON_DMA,                          ///< The driver will use polling to perform I2C transfer
  SL_I2C_USING_INTERRUPT = SL_I2C_USING_NON_DMA, ///< Same as SL_I2C_USING_NON_DMA, retained for backward compatibility.
  SL_I2C_USING_DMA,                              ///< The driver will use DMA to perform I2C transfer
  SL_I2C_TRANFER_TYPE_LAST,                      ///< For Validation
} sl_i2c_transfer_type_t;

/**
 * @brief Enumeration to represent I2C operating modes.
 * 
 * This enumeration defines the different operating modes available for the I2C driver.
 */
typedef enum {
  SL_I2C_STANDARD_MODE = 1,   ///< Standard-mode, bidirectional data transfers up to 100 kbit/s.
  SL_I2C_FAST_MODE,           ///< Fast-mode, bidirectional data transfers up to 400 kbit/s.
  SL_I2C_HIGH_SPEED_MODE,     ///< High-speed mode, bidirectional data transfers up to 3.4 Mbit/s.
  SL_I2C_FAST_PLUS_MODE,      ///< Fast-mode Plus, bidirectional data transfers up to 1 Mbit/s.
  SL_I2C_OPERATING_MODE_LAST, ///< Last member for validation purposes.
} sl_i2c_operating_mode_t;

/**
 * @brief Callback function type for I2C driver events.
 * 
 * This typedef defines the callback function type that is used by the I2C driver to notify
 * the application of various I2C events.
 * 
 * @param[in] i2c_instance The I2C instance that generated the event.
 * @param[in] status The status code indicating the result of the I2C operation.
 */
typedef void (*sl_i2c_callback_t)(sl_i2c_instance_t i2c_instance, uint32_t status);

/**
 * @brief Structure to hold the configuration parameters of an I2C instance.
 * 
 * This structure defines the configuration parameters for an I2C instance, including the mode,
 * operating speed, transfer type, and callback function.
 */
typedef struct {
  sl_i2c_mode_t mode;                     ///< Leader/Follower mode. 0 for leader mode and 1 for follower mode.
  sl_i2c_operating_mode_t operating_mode; ///< Speed mode. See \ref sl_i2c_operating_mode_t for possible values.
  sl_i2c_transfer_type_t transfer_type;   ///< Transfer type. See \ref sl_i2c_transfer_type_t for possible values.
  sl_i2c_callback_t i2c_callback;         ///< I2C callback function. See \ref sl_i2c_callback_t.
} sl_i2c_config_t;

/**
 * @brief Structure to hold the parameters of DMA configuration.
 * 
 * This structure defines the configuration parameters for DMA channels used in I2C operations,
 * including the transmit and receive channel numbers.
 */
typedef struct {
  uint32_t dma_tx_channel; ///< DMA transmit channel number.
  uint32_t dma_rx_channel; ///< DMA receive channel number.
} sl_i2c_dma_config_t;

/**
 * @brief Structure to hold the parameters of I2C transfer configuration.
 * 
 * This structure defines the configuration parameters for an I2C transfer, including pointers
 * to the transmit and receive data buffers and the lengths of the data to be transmitted and received.
 */
typedef struct {
  uint8_t *tx_buffer; ///< Pointer to the transmit data buffer.
  uint32_t tx_len;    ///< Number of bytes to transmit.
  uint8_t *rx_buffer; ///< Pointer to the receive data buffer.
  uint32_t rx_len;    ///< Number of bytes to receive.
} sl_i2c_transfer_config_t;

/**
 * @brief Structure to hold the port and pin configuration for I2C.
 * 
 * This structure defines the configuration parameters for the I2C pins, including the port, pin,
 * mux, and pad settings for both SDA and SCL lines.
 */
typedef struct {
  uint8_t sda_port; ///< I2C SDA GPIO port.
  uint8_t sda_pin;  ///< I2C SDA GPIO pin.
  uint8_t sda_mux;  ///< I2C SDA GPIO mux.
  uint8_t sda_pad;  ///< I2C SDA GPIO pad.
  uint8_t scl_port; ///< I2C SCL GPIO port.
  uint8_t scl_pin;  ///< I2C SCL GPIO pin.
  uint8_t scl_mux;  ///< I2C SCL GPIO mux.
  uint8_t scl_pad;  ///< I2C SCL GPIO pad.
  uint8_t instance; ///< I2C instance.
} sl_i2c_pin_init_t;

/**
 * @brief Enumeration to represent I2C power modes.
 * 
 * This enumeration defines the different power modes available for the I2C driver.
 */
typedef enum {
  SL_I2C_ULP_MODE,        ///< Ultra-Low Power (ULP) mode for the I2C driver.
  SL_I2C_HP_MODE,         ///< High Power (HP) mode for the I2C driver.
  SL_I2C_POWER_MODE_LAST, ///< Last member for validation purposes.
} sl_i2c_power_modes_t;
// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/
/**
 * @brief To initialize the I2C module and configure its parameters.
 * 
 * @details This function configures the I2C instance with the specified settings, including I2C leader/follower mode,
 * operating mode (bus speed), and transfer type (interrupt or DMA).
 * If DMA is selected as the transfer type, the function will also initialize the DMA module.
 * Additionally, it registers the callback for the I2C instance, clears any pending interrupts,
 * and configures the SDL and SCL pins as per the instance requirements.
 *
 * @param[in] i2c_instance The I2C instance to be initialized. See \ref sl_i2c_instance_t.
 * 
 * @param[in] p_user_config A pointer to the I2C configuration structure. See \ref sl_i2c_config_t.
 *
 * @return sl_i2c_status_t Status of the initialization:
 *         - \ref SL_I2C_SUCCESS  - Initialization was successful.
 *         - \ref SL_I2C_CALLBACK_BUSY - The driver is currently busy and cannot process the request.
 *         - \ref SL_I2C_INVALID_PARAMETER  - One or more parameters are invalid.
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_init(sl_i2c_instance_t i2c_instance, const sl_i2c_config_t *p_user_config);

/***************************************************************************/
/**
 * @brief To configure the follower address of the I2C module.
 * 
 * @details This API is intended to be used only in follower mode after initializing the I2C instance. 
 * It updates the follower address register so that the I2C leader device can initiate transfers with the 
 * address that matches the follower device's address register.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_i2c_driver_init must be called prior.
 *      - I2C mode must be set to follower mode.
 * 
 * @param[in] i2c_instance The I2C instance to be configured. See \ref sl_i2c_instance_t.
 * 
 * @param[in] address The follower's own address (supports 7-Bit and 10-Bit addressing) between the range of 1-1023.
 * 
 * @return sl_i2c_status_t Status code indicating the result of the operation:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_set_follower_address(sl_i2c_instance_t i2c_instance, uint16_t address);

/***************************************************************************/
/**
 * @brief To configure the TX and RX FIFO threshold levels for the I2C instance.
 * 
 * @details This API allows you to set the threshold levels for the transmit (TX) and receive (RX) FIFO buffers.
 * The valid range is 0-255, where 0 corresponds to a threshold of 1 byte entry, and 255 corresponds to a threshold of 256 byte entries.
 * Note that the value must not exceed the depth of the hardware buffer. Whenever the buffer reaches the threshold value, the respective interrupt occurs.
 * 
 * @pre Pre-condition:
 *      - \ref sl_i2c_driver_init must be called prior.
 * 
 * @param[in] i2c_instance The I2C instance to be configured. See \ref sl_i2c_instance_t.
 * @param[in] tx_threshold_value The TX FIFO threshold value, which must be between 0 and 255.
 * @param[in] rx_threshold_value The RX FIFO threshold value, which must be between 0 and 255.
 * 
 * @return sl_i2c_status_t Status code indicating the result of the operation:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_configure_fifo_threshold(sl_i2c_instance_t i2c_instance,
                                                       uint8_t tx_threshold_value,
                                                       uint8_t rx_threshold_value);

/***************************************************************************/
/**
 * @brief To read the current I2C clock frequency.
 * 
 * @details This API retrieves the current system core clock frequency in Hz by reading the system core clock frequency.
 * 
 * @pre Pre-condition:
 *      - \ref sl_i2c_driver_init must be called prior.
 * 
 * @param[in] i2c_instance The I2C instance used. See \ref sl_i2c_instance_t.
 * @param[out] frequency Pointer to a variable where the current system core clock frequency will be stored.
 * 
 * @return sl_i2c_status_t Status code indicating the result of the operation:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_get_frequency(sl_i2c_instance_t i2c_instance, uint32_t *frequency);

/***************************************************************************/
/**
 * @brief To send data in blocking mode.
 * 
 * @details This API sends data in blocking mode, where it sets the follower address 
 * if used for leader mode. The call remains blocked until the last byte is transferred.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_i2c_driver_init must be called prior.
 *      - Transfer type must be set to 'using interrupt'.
 *      - \ref sl_i2c_driver_set_follower_address (if sending from follower).
 *      - \ref sl_i2c_driver_configure_fifo_threshold.
 * 
 * @param[in] i2c_instance The I2C instance to be used. See \ref sl_i2c_instance_t.
 * @param[in] address Follower address (7-bit: 0-0x7F, 10-bit: 0-0x3FF). Ignored in follower mode.
 * @param[in] tx_buffer Pointer to the transmit data buffer.
 * @param[in] tx_len Data length in bytes (range: 1-80,000).
 * 
 * @note It is recommended to use up to 80,000 bytes maximum transfer length, 
 * but this limit may vary depending on the available RAM size.
 * 
 * @return sl_i2c_status_t Status code indicating the result:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_send_data_blocking(sl_i2c_instance_t i2c_instance,
                                                 uint16_t address,
                                                 uint8_t *tx_buffer,
                                                 uint32_t tx_len);

/***************************************************************************/
/**
 * @brief To send data in non-blocking mode using DMA.
 * 
 * @details This API sends data using DMA in a non-blocking mode. It sets the follower address 
 * when used in leader mode, configures the DMA TX and RX channels, registers the DMA callback, 
 * and enables the DMA channels. The transfer complete flag \ref SL_I2C_DATA_TRANSFER_COMPLETE 
 * is updated after data transfer completion.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_i2c_driver_init 
 *      - Here transfer-type should be set as DMA-type
 *      - \ref sl_i2c_driver_set_follower_address, if used in salve application
 * @param[in] i2c_instance I2C Instance.
 * @param[in] address Follower address can be provided in 7-bit length (0-0x7F)
 * or in 10-bit length(0-0x3FF).
 * @param[in] tx_buffer A pointer to transmit data buffer
 * @param[in] tx_len Data length in number of bytes in the range of 1-30000 bytes.
 * @param[in] p_dma_config A pointer to DMA configuration structure \ref sl_i2c_dma_config_t.
 * @note Maximum tx_len values can be 30000 (receives back in around 4 seconds)
 * @note The `tx_buffer` parameter is of type `uint32_t`. It is recommended to use 8-bit (0-7 bits) data in the buffer,
 * with the remaining bits filled in by the driver internally.
 * @return status 0 if successful, else error code as follow
 *         - \ref SL_I2C_SUCCESS (0x0000) - Success 
 *         - \ref SL_I2C_INVALID_PARAMETER (0x000F) - Parameters are invalid 
 *         - \ref SL_I2C_DMA_TRANSFER_ERROR (0x000E) - DMA parameters are invalid 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_send_data_non_blocking(sl_i2c_instance_t i2c_instance,
                                                     uint16_t address,
                                                     uint32_t *tx_buffer,
                                                     uint32_t tx_len,
                                                     sl_i2c_dma_config_t *p_dma_config);

/***************************************************************************/
/**
 * @brief To receive data in blocking mode.
 * 
 * @details This API receives data in blocking mode, where it sets the follower address 
 * if used in leader mode. The call remains blocked until the last byte is received.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_i2c_driver_init must be called prior.
 *      - Transfer type must be set to 'using interrupt'.
 *      - \ref sl_i2c_driver_set_follower_address (if receiving from follower).
 *      - \ref sl_i2c_driver_configure_fifo_threshold.
 * 
 * @param[in] i2c_instance The I2C instance to be used. See \ref sl_i2c_instance_t.
 * @param[in] address Follower address (7-bit: 0-0x7F, 10-bit: 0-0x3FF). Ignored in follower mode.
 * @param[in] rx_buffer Pointer to the receive data buffer.
 * @param[in] rx_len Data length in bytes (range: 1-80,000).
 * 
 * @note It is recommended to use up to 80,000 bytes maximum transfer length, 
 * but this limit may vary depending on the available RAM size.
 * 
 * @return sl_i2c_status_t Status code indicating the result:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_receive_data_blocking(sl_i2c_instance_t i2c_instance,
                                                    uint16_t address,
                                                    uint8_t *rx_buffer,
                                                    uint32_t rx_len);

/***************************************************************************/
/**
 * @brief To receive data in non-blocking mode using DMA.
 * 
 * @details This API receives data using DMA in non-blocking mode. It sets the follower address 
 * when used in leader mode, configures DMA RX and TX channels, registers the DMA callback, 
 * and enables the DMA channels. The transfer complete flag \ref SL_I2C_DATA_TRANSFER_COMPLETE 
 * is updated after data transfer completion.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_i2c_driver_init must be called prior.
 *      - Transfer type must be set to 'using DMA'.
 *      - \ref sl_i2c_driver_set_follower_address if used in follower application.
 * 
 * @param[in] i2c_instance The I2C instance to be used. See \ref sl_i2c_instance_t.
 * @param[in] address Follower address, which can be in 7-bit (0-0x7F) or 10-bit (0-0x3FF) format.
 * @param[in] rx_buffer Pointer to the receive data buffer.
 * @param[in] rx_len Data length in bytes, ranging from 1 to 30,000.
 * @param[in] p_dma_config Pointer to the DMA configuration structure. See \ref sl_i2c_dma_config_t.
 * 
 *  @return status 0 if successful, else error code as follow
 *         - \ref SL_I2C_SUCCESS (0x0000) - Success 
 *         - \ref SL_I2C_INVALID_PARAMETER (0x000F) - Parameters are invalid 
 *         - \ref SL_I2C_DMA_TRANSFER_ERROR (0x000E) - DMA parameters are invalid
 * 
 * @note 
 *       -  Maximum rx_len values can be 30000 (receives back in around 4 seconds).
 *       - The default values cannot be any of the reserved address locations: that is, 0x00 to 0x07, 
 *        or 0x78 to 0x7f.
 *       - Use of this API is not recommended, as DMA support is limited to transmit mode only.
 *       - Moving forward, this API will be deprecated. This is retained for backward compatibility.
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_receive_data_non_blocking(sl_i2c_instance_t i2c_instance,
                                                        uint16_t address,
                                                        uint8_t *rx_buffer,
                                                        uint32_t rx_len,
                                                        sl_i2c_dma_config_t *p_dma_config)
  SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To transmit and then receive data from an I2C device in blocking mode.
 * 
 * @details This API first transmits data and then receives data from an I2C device in blocking mode.
 * It sets the follower address when used in leader mode.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_i2c_driver_init must be called prior.
 *      - Transfer type must be set to 'using interrupt'.
 *      - \ref sl_i2c_driver_set_follower_address if used in follower application.
 * 
 * @param[in] i2c_instance The I2C instance to be used. See \ref sl_i2c_instance_t.
 * @param[in] address Follower address, which can be in 7-bit (0-0x7F) or 10-bit (0-0x3FF) format.
 * @param[in] p_transfer_config Pointer to the transfer configuration structure. See \ref sl_i2c_transfer_config_t.
 * 
 * @note 
 *       - It is recommended to use up to 80,000 bytes maximum transfer length, 
 *         but this limit may vary depending on the available RAM size.
 *       - The default values cannot be any of the reserved address locations: 0x00 to 0x07, or 0x78 to 0x7f.
 * 
 * @return sl_i2c_status_t Status code indicating the result:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_transfer_data(sl_i2c_instance_t i2c_instance,
                                            sl_i2c_transfer_config_t const *p_transfer_config,
                                            uint16_t address);

/***************************************************************************/
/**
 * @brief To de-initialize the I2C peripheral and clock.
 * 
 * @details This API de-initializes the I2C peripheral, disables the peripheral clock, and unregisters the callback.
 * 
 * @pre Pre-condition:
 *      - \ref sl_i2c_driver_init must be called prior.
 * 
 * @param[in] i2c_instance The I2C instance to be de-initialized. See \ref sl_i2c_instance_t.
 * 
 * @return sl_i2c_status_t Status code indicating the result:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 * @note When the I2C module is used in combination with other peripherals, refer to the following notes:
 *   1. When `sl_i2c_driver_deinit()` is called, it will disable the clock for the peripheral. To power off the peripheral, 
 *      the power domain (PERI_EFUSE) containing different peripherals (e.g., USART, UART, I2C, SSI Master, SSI Slave, Generic-SPI Master, 
 *      I2S Master, I2S Slave, Micro-DMA Controller, Config Timer, Random-Number Generator, CRC Accelerator, SIO, QEI, MCPWM, and EFUSE) 
 *      must be powered down. Use the following API to power down the specific power domain if other peripherals are not being used:
 *      `sl_si91x_peri_efuse_power_down(power_down)`.
 *   2. Some peripherals (ULP Peripherals, UULP Peripherals, GPDMA, and SDIO-SPI) have separate domains that can be powered down independently. 
 *      For additional details, see the Power Architecture section in the Hardware Reference Manual. The ULP_UART has a separate power domain 
 *      (ULPSS_PWRGATE_ULP_UART) that can be powered down independently. See the `rsi_power_save.h` file for all power gate definitions.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_deinit(sl_i2c_instance_t i2c_instance);

/***************************************************************************/
/**
 * @brief To configure the I2C pins.
 * 
 * @details Configures the SDA and SCL pins and enables internal pull-ups for the SDA and SCL lines of the follower device.
 * 
 * @param[in] pin_init Pointer to the pin initialization structure. See \ref sl_i2c_pin_init_t.
 * 
 * @return sl_i2c_status_t Status code indicating the result:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_si91x_i2c_pin_init(sl_i2c_pin_init_t *pin_init);

/***************************************************************************/
/**
 * @brief To re-configure the I2C leader device on power mode changes.
 * 
 * @details Re-configures the I2C leader instance (ULP_I2C) for ULP/HP mode, adjusting the I2C clock frequency 
 * required for the I2C operating (speed) mode to run in ULP/HP mode.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_i2c_driver_init must be called prior.
 *      - The instance should be in ULP mode when switching to HP mode.
 *      - The instance should be in HP mode when switching to ULP mode.
 *      - The instance used should be the ULP-I2C instance.
 * 
 * @param[in] new_power_mode New power state to switch to. See \ref sl_i2c_power_modes_t.
 * 
 * @return sl_i2c_status_t Status code indicating the result:
 *         - \ref SL_I2C_SUCCESS  - Success.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_leader_reconfig_on_power_mode_change(sl_i2c_power_modes_t new_power_mode);

/***************************************************************************/
/**
 * @brief To enable or disable repeated start and combined format transfer.
 * 
 * @details This API should be called by a leader application to enable repeated start before starting data transfer.
 * By enabling this, combined format transfers can be used, allowing the I2C to perform combined write/read operations 
 * to the same or different followers without releasing the line, ensuring that the operation is not interrupted.
 * 
 * @pre Pre-condition:
 *      - \ref sl_i2c_driver_init must be called prior.
 * 
 * @param[in] i2c_instance The I2C instance to be configured. See \ref sl_i2c_instance_t.
 * @param[in] enable_rep_start Boolean value: true to enable, false to disable repeated start.
 * 
 * @return sl_i2c_status_t Status code indicating the result:
 *         - \ref SL_I2C_SUCCESS  - Operation was successful.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Invalid parameters were provided.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_i2c_driver_enable_repeated_start(sl_i2c_instance_t i2c_instance, boolean_t enable_rep_start);

/***************************************************************************/
/**
 * @brief To wait until the I2C bus is idle.
 * 
 * @details This API polls the activity status bit of the leader and follower to determine 
 * if the I2C bus is idle. It can be used for synchronization purposes.
 * 
 * @param[in] i2c_instance The I2C instance to be checked. See \ref sl_i2c_instance_t.
 * 
 * @return sl_i2c_status_t Status code indicating the result:
 *         - \ref SL_I2C_SUCCESS  - Success.
 *         - \ref SL_I2C_INVALID_PARAMETER  - Parameters are invalid.
 * 
 ******************************************************************************/
sl_i2c_status_t sl_si91x_i2c_wait_till_i2c_is_idle(sl_i2c_instance_t i2c_instance);

/// @} end group I2C ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/**
 * @addtogroup I2C I2C
 * @{
 *
 * @details
 *
 * @section I2C_Intro Introduction
 *
* Inter-Integrated Circuit (I2C) is a serial communication protocol widely used in microcontrollers and integrated circuits
* for communication between various peripheral devices and sensors. I2C uses only two wires: a serial data line (SDA) and a serial clock line (SCL),
* making it efficient in terms of pin usage, especially in systems with limited pin counts.
 * 
 * @image html i2c_frame.png "I2C Frame Format"
 * 
 * I2C is favored in embedded systems for its simplicity, flexibility, and low pin count requirements. Common use cases include:
 * - **Sensor Integration**: Connecting various sensors for data collection.
 * - **Sensor Hubs and Fusion Algorithms**: Combining sensor data for enhanced applications.
 * - **Memory Expansion**: Adding EEPROMs or other memory devices.
 * - **Real-Time Clock (RTC)**: Maintaining accurate timekeeping.
 * - **Display Interfaces**: Driving LCDs and other display modules.
 * - **Peripheral Control**: Managing additional peripheral devices.
 * - **Communication with Other Microcontrollers and Modules**: Enabling inter-device communication.
 * - **System Configuration and Control**: Configuring system settings and parameters.
 *
 * @section I2C_Configuration Configuration
 *
 * I2C can be configured using several parameters, including:
 * - **Mode**: Leader or Follower mode (sl_i2c_mode_t)
 * - **Transfer Type**: Using interrupts or DMA (@ref sl_i2c_transfer_type_t)
 * - **Operating Speed**: Various speed modes (@ref sl_i2c_operating_mode_t)
 *
 * These configurations are encapsulated in the @ref sl_i2c_config_t structure and initialized using the @ref sl_i2c_driver_init() API.
 *
 * For more details on configuration parameters, see the respective peripheral example readme document.
 *
 * @section I2C_Usage Usage
 * 
 * After defining the I2C configuration structures and passing an instance of @ref sl_i2c_config_t, the common I2C functions can be used to initiate and configure the I2C module. The typical flow for implementation is as follows:
 * 
 * - Initialize the I2C driver: @ref sl_i2c_driver_init
 * - Set the follower address (if in follower mode): @ref sl_i2c_driver_set_follower_address
 * - Configure the FIFO threshold: @ref sl_i2c_driver_configure_fifo_threshold
 * - Perform data operations:
 *   - Blocking mode use below APIs:
 *     - **Receive data:** @ref sl_i2c_driver_receive_data_blocking
 *     - **Send data:** @ref sl_i2c_driver_send_data_blocking
 *   - Non-blocking mode (Recommended for larger buffers):
 *     - **Send data:** @ref sl_i2c_driver_send_data_non_blocking
 * - Deinitialize the I2C driver: @ref sl_i2c_driver_deinit
 * 
 * - **Recommendations:**
 * 
 *   - For larger buffer transfers (greater than 16 bytes), prefer non-blocking APIs to avoid stalling the CPU, especially in high-throughput applications.
 *   - When using non-blocking APIs, ensure that the transfer type is configured as SL_I2C_USING_DMA to leverage DMA for efficient data handling.
 *   - When using blocking APIs, ensure that the transfer type is configured as SL_I2C_USING_INTERRUPT.
 *
 * @} (end addtogroup I2C)
 */
#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_I2C_H */
