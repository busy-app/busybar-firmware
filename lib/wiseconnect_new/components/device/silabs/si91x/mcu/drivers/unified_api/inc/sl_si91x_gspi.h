/******************************************************************************
* @file sl_si91x_gspi.h
* @brief GSPI API implementation
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

#ifndef SL_SI91X_GSPI_H
#define SL_SI91X_GSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "GSPI.h"
#include "base_types.h"

/***************************************************************************/
/**
 * @addtogroup GSPI Generic SPI
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Data Types

typedef ARM_SPI_SignalEvent_t sl_gspi_signal_event_t; ///< Renamed signal event structure
typedef ARM_SPI_STATUS sl_gspi_status_t;              ///< Renamed status structure
typedef ARM_DRIVER_SPI sl_gspi_driver_t;              ///< Renamed GSPI driver structure
typedef const void *sl_gspi_handle_t;                 ///< Created GSPI handle type

/***************************************************************************/
/**
 * @brief Enumeration for different GSPI callback events.
 * 
 * @details This enumeration defines the possible events that can trigger a GSPI callback. 
 * Each event corresponds to a specific condition or state in the GSPI operation.
 */
typedef enum {
  SL_GSPI_TRANSFER_COMPLETE = ARM_SPI_EVENT_TRANSFER_COMPLETE, ///< Transfer complete event
  SL_GSPI_DATA_LOST         = ARM_SPI_EVENT_DATA_LOST,         ///< Data lost event
  SL_GSPI_MODE_FAULT        = ARM_SPI_EVENT_MODE_FAULT,        ///< Mode fault event
} gspi_event_typedef_t;

/***************************************************************************/
/**
 * @brief Enumeration for GSPI power states.
 * 
 * @details This enumeration defines the possible power states for the GSPI peripheral. 
 * Each state corresponds to a specific power mode that the GSPI can operate in.
 */
/// @brief Enumeration for GSPI power states.
typedef enum {
  SL_GSPI_POWER_OFF  = ARM_POWER_OFF,  ///< Power mode OFF
  SL_GSPI_LOW_POWER  = ARM_POWER_LOW,  ///< Low power mode
  SL_GSPI_FULL_POWER = ARM_POWER_FULL, ///< Full power mode
  SL_GSPI_POWER_MODE_LAST,             ///< Last member of enum for validation
} sl_gspi_power_state_t;

/***************************************************************************/
/**
 * @brief Enumeration for GSPI clock modes.
 * 
 * @details This enumeration defines the possible clock modes for the GSPI peripheral. 
 * Each mode corresponds to a specific combination of clock polarity (CPOL) and clock phase (CPHA).
 */
typedef enum {
  SL_GSPI_MODE_0 = ARM_SPI_CPOL0_CPHA0, ///< Mode 0: CPOL0, CPHA0
  SL_GSPI_MODE_3 = ARM_SPI_CPOL1_CPHA1, ///< Mode 3: CPOL1, CPHA1
} clock_mode_typedef_t;

/***************************************************************************/
/**
 * @brief Enumeration for GSPI primary modes.
 * 
 * @details This enumeration defines the possible primary modes for the GSPI peripheral. 
 * Each mode indicates whether the GSPI is active or inactive in primary mode.
 */
typedef enum {
  SL_GSPI_MASTER_INACTIVE = ARM_SPI_MODE_INACTIVE, ///< Primary mode inactive
  SL_GSPI_MASTER_ACTIVE   = ARM_SPI_MODE_MASTER,   ///< Primary mode active
  SL_GSPI_MASTER_MODE_LAST,                        ///< Last member of enum for validation
} master_mode_typedef_t;

/***************************************************************************/
/**
 * @brief Enumeration for GSPI secondary select modes.
 * 
 * @details This enumeration defines the possible secondary select modes for the GSPI peripheral. 
 * Each mode indicates how the secondary select (SS) signal is managed in primary mode.
 */
typedef enum {
  SL_GSPI_MASTER_UNUSED    = ARM_SPI_SS_MASTER_UNUSED,    ///< Primary unused mode
  SL_GSPI_MASTER_SW        = ARM_SPI_SS_MASTER_SW,        ///< Primary software mode
  SL_GSPI_MASTER_HW_OUTPUT = ARM_SPI_SS_MASTER_HW_OUTPUT, ///< Primary hardware output mode
  SL_GSPI_SLAVE_SELECT_MODE_LAST,                         ///< Last member of enum for validation
} slave_select_mode_typedef_t;

/***************************************************************************/
/**
 * @brief Enumeration for GSPI Primary instance.
 * 
 * @details This enumeration defines the GSPI Primary instance. It currently has one member, 
 * with a provision for future expansion.
 */
typedef enum {
  SL_GSPI_MASTER,             ///< GSPI Primary instance
  SL_GSPI_INSTANCE_LAST_ENUM, ///< Last member of the enum for validation
} sl_gspi_instance_t;

/***************************************************************************/
/**
 * @brief Enumeration for GSPI slave numbers.
 * 
 * @details This enumeration defines the possible slave numbers for the GSPI peripheral. 
 * Each value represents a specific slave device connected to the GSPI.
 */
typedef enum {
  GSPI_SLAVE_0,         ///< Secondary No. 0
  GSPI_SLAVE_1,         ///< Secondary No. 1
  GSPI_SLAVE_2,         ///< Secondary No. 2
  GSPI_SLAVE_LAST_ENUM, ///< Last member of enum for validation
} sl_gspi_slave_number_t;

/***************************************************************************/
/**
 * @brief Structure to hold the configuration members for the GSPI peripheral.
 * 
 * @details This structure contains the configuration members for the GSPI peripheral, 
 * including options for swapping read/write operations, bit width, clock mode, 
 * slave select mode, and bitrate.
 */
typedef struct {
  boolean_t swap_read;        ///< true to enable and false to disable swap read
  boolean_t swap_write;       ///< true to enable and false to disable swap write
  uint8_t bit_width;          ///< Bit width, from 1-16 bits.
  uint32_t clock_mode;        ///< Clock mode, either Mode 0 or Mode 3 of GSPI
  uint32_t slave_select_mode; ///< Secondary select mode, either software or hardware output
  uint32_t bitrate;           ///< Bitrate for setting the clock division factor
} sl_gspi_control_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the clock configuration members for the GSPI peripheral.
 * 
 * @details This structure contains the clock configuration members for the GSPI peripheral, 
 * including SoC PLL count value, interface PLL control value, clock frequencies, and division factor.
 */
typedef struct {
  uint8_t soc_pll_mm_count_value;      ///< SoC PLL count value
  uint16_t intf_pll_500_control_value; ///< Interface PLL control value
  uint32_t intf_pll_clock;             ///< Interface PLL clock frequency
  uint32_t intf_pll_reference_clock;   ///< Interface PLL reference clock frequency
  uint32_t soc_pll_clock;              ///< SoC PLL clock frequency
  uint32_t soc_pll_reference_clock;    ///< SoC PLL reference clock frequency
  uint16_t division_factor;            ///< Clock division factor
} sl_gspi_clock_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the version numbers of the peripheral API.
 * 
 * @details This structure contains the version numbers for the peripheral API, 
 * including the release version, major version, and minor version.
 */
typedef struct {
  uint8_t release; ///< Release version number
  uint8_t major;   ///< Major version number
  uint8_t minor;   ///< Minor version number
} sl_gspi_version_t;

/***************************************************************************/
/**
 * @brief This API is no longer supported due to the restriction on peripheral drivers to configuring clocks.
 * @brief To configure the clock for the GSPI module.
 * 
 * @details This API sets the clock for the GSPI peripheral. It configures the PLL clock and 
 * SOC clock according to the values set by the user in the clock configuration structure.
 * 
 * @param[in] clock_configuration Pointer to the clock configuration structure \ref sl_gspi_clock_config_t.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_configure_clock(sl_gspi_clock_config_t *clock_configuration)
  SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To initialize the GSPI module.
 * 
 * @details This API initializes the GSPI module. If DMA is enabled, it also initializes the DMA module. 
 * The API takes the address of a pointer to store the GSPI primary handle, which can be used for subsequent function calls.
 * 
 * @param[in] instance GSPI instance \ref sl_gspi_instance_t.
 * @param[in] gspi_handle Double pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_init(sl_gspi_instance_t instance, sl_gspi_handle_t *gspi_handle);

/***************************************************************************/
/**
 * @brief To uninitialize the GSPI module.
 * 
 * @details This API uninitializes the GSPI module. If DMA is enabled, it also uninitializes the DMA module.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_gspi_init must be called before this function.
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * 
 * @note
 *   When the GSPI module is used in combination with other peripherals, see the notes below while deinitializing in the application:
 *   1. Whenever sl_si91x_gspi_deinit() is called, it will disable the clock for the peripheral. To power off the peripheral, you must power down the
 *      power domain (PERI_EFUSE), which contains different peripherals mentioned below:
 *      USART, UART, I2C, SSI Primary, SSI Secondary, Generic-SPI Primary, I2S Primary, I2S Secondary, Micro-DMA Controller, Config Timer,
 *      Random-Number Generator, CRC Accelerator, SIO, QEI, MCPWM, and EFUSE.
 *      Use the following API to power down the particular power domain if other peripherals are not being used:
 *      sl_si91x_peri_efuse_power_down(power_down); 
 * 
 *   2. A few peripherals (ULP Peripherals, UULP Peripherals, GPDMA, and SDIO-SPI) have separate domains; those can be powered down independently. For additional details, see the Power architecture section in the Hardware Reference Manual.
 *      For example, ULP_UART has a separate power domain ULPSS_PWRGATE_ULP_UART, which can be powered down independently. Refer to the rsi_power_save.h file for all power gate definitions.
 ******************************************************************************/
sl_status_t sl_si91x_gspi_deinit(sl_gspi_handle_t gspi_handle);

/***************************************************************************/
/**
 * @brief To configure the GSPI module.
 * 
 * @details The GSPI control and configuration options are listed below:
 *   - swap_read (enable/disable)
 *   - swap_write (enable/disable)
 *   - bit_width (1-16 bits)
 *   - clock_mode (mode0/mode3)
 *   - slave_select_mode (hw_output/sw)
 *   - bitrate
 * 
 * @note 
 *   Swap Read and Swap Write can be used only if the bit_width is configured as 16.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init 
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * @param[in] control_configuration Pointer to the configuration structure \ref sl_gspi_control_config_t.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_NOT_SUPPORTED  - Parameter is not supported.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_MODE  - Secondary select mode is invalid.
 *         - SL_STATUS_INVALID_TYPE  - SPI frame format is not valid.
 *         - SL_STATUS_INVALID_RANGE  - Data bits (frame length) are not in range.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_set_configuration(sl_gspi_handle_t gspi_handle,
                                            sl_gspi_control_config_t *control_configuration);

/***************************************************************************/
/**
 * @brief To receive data from the secondary device.
 * 
 * @details If DMA is enabled, it configures the DMA channel and required parameters.
 * When the received data is equal to the data_length passed in this function,
 * a callback event is generated which can be registered using \ref sl_si91x_gspi_register_event_callback.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init 
 *      - \ref sl_si91x_gspi_set_configuration 
 *      - \ref sl_si91x_gspi_set_slave_number
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * @param[in] data Pointer to the variable that will store the received data.
 * @param[in] data_length (uint32_t) Number of data items to receive.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_receive_data(sl_gspi_handle_t gspi_handle, void *data, uint32_t data_length);

/***************************************************************************/
/**
 * @brief To receive data from the secondary device in blocking mode.
 *
 * @details This API receives data from the secondary device and blocks until all data is received
 * or the timeout expires.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init
 *      - \ref sl_si91x_gspi_set_configuration
 *      - \ref sl_si91x_gspi_set_slave_number
 *
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * @param[in] data Pointer to the variable that will store the received data.
 * @param[in] data_length (uint32_t) Number of data items to receive.
 * @param[in] timeout (uint32_t) Timeout in milliseconds.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_TIMEOUT  - Timeout occurred before all data was received.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_receive_data_blocking(sl_gspi_handle_t gspi_handle,
                                                void *data,
                                                uint32_t data_length,
                                                uint32_t timeout);

/***************************************************************************/
/**
 * @brief To send data to the secondary device.
 * 
 * @details If DMA is enabled, it configures the DMA channel and required parameters.
 * When the sent data is equal to the data_length passed in this function,
 * a callback event is generated which can be registered using \ref sl_si91x_gspi_register_event_callback.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init 
 *      - \ref sl_si91x_gspi_set_configuration 
 *      - \ref sl_si91x_gspi_set_slave_number 
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * @param[in] data Const pointer to the variable that contains the data to be sent.
 * @param[in] data_length (uint32_t) Number of data items to send.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_send_data(sl_gspi_handle_t gspi_handle, const void *data, uint32_t data_length);

/***************************************************************************/
/**
 * @brief To send data to the secondary device in blocking mode.
 *
 * @details This API sends data to the secondary device and blocks until all data is sent
 * or the timeout expires.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init
 *      - \ref sl_si91x_gspi_set_configuration
 *      - \ref sl_si91x_gspi_set_slave_number
 *
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * @param[in] data Const pointer to the variable that contains the data to be sent.
 * @param[in] data_length (uint32_t) Number of data items to send.
 * @param[in] timeout (uint32_t) Timeout in milliseconds.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_TIMEOUT  - Timeout occurred before all data was sent.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ***************************************************************************/
sl_status_t sl_si91x_gspi_send_data_blocking(sl_gspi_handle_t gspi_handle,
                                             const void *data,
                                             uint32_t data_length,
                                             uint32_t timeout);

/***************************************************************************/
/**
 * @brief To send and receive data to and from the secondary device simultaneously.
 * 
 * @details If DMA is enabled, it configures the DMA channel and required parameters.
 * When the received data and sent data are equal to the data_length passed in this function,
 * a callback event is generated which can be registered using \ref sl_si91x_gspi_register_event_callback.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init 
 *      - \ref sl_si91x_gspi_set_configuration 
 *      - \ref sl_si91x_gspi_set_slave_number 
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * @param[in] data_out Const pointer to the variable that has data which needs to be sent.
 * @param[in] data_in Pointer to the variable that will store the received data.
 * @param[in] data_length (uint32_t) Number of data items to receive.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_transfer_data(sl_gspi_handle_t gspi_handle,
                                        const void *data_out,
                                        void *data_in,
                                        uint32_t data_length);

/***************************************************************************/
/**
 * @brief To send and receive data to and from the secondary device simultaneously in blocking mode.
 *
 * @details This API sends and receives data to and from the secondary device and blocks until all data is transferred
 * or the timeout expires.
 *
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init
 *      - \ref sl_si91x_gspi_set_configuration
 *      - \ref sl_si91x_gspi_set_slave_number
 *
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * @param[in] data_out Const pointer to the variable that has data which needs to be sent.
 * @param[in] data_in Pointer to the variable that will store the received data.
 * @param[in] data_length (uint32_t) Number of data items to transfer.
 * @param[in] timeout (uint32_t) Timeout in milliseconds.
 *
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_TIMEOUT  - Timeout occurred before all data was transferred.
 *
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_transfer_data_blocking(sl_gspi_handle_t gspi_handle,
                                                 const void *data_out,
                                                 void *data_in,
                                                 uint32_t data_length,
                                                 uint32_t timeout);

/***************************************************************************/
/**
 * @brief To set the primary state as active or inactive.
 * 
 * @details Sets the primary state (i.e., activate/deactivate).
 * If the primary is not required in the application for a specific time, 
 * it can be turned on/off at runtime by using this API.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init 
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle \ref sl_gspi_handle_t.
 * @param[in] value (boolean_t) Enable or Disable.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 *         - SL_STATUS_INVALID_MODE  - Mode is invalid (fails to activate primary).
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_set_master_state(sl_gspi_handle_t gspi_handle, boolean_t value);

/***************************************************************************/
/**
 * @brief To register the user callback function for GSPI events.
 * 
 * @details At the time of the event, the function passed in the parameter is called with the respective
 * event as the parameter.
 * Before calling this function again, it is mandatory to call the \ref sl_si91x_gspi_unregister_event_callback
 * function to unregister the callback; otherwise, it returns SL_STATUS_BUSY error code.
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle ( \ref sl_gspi_handle_t).
 * @param[in] callback_event Pointer to the function that needs to be called at the time of interrupt
 *            ( \ref sl_gspi_signal_event_t).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_gspi_register_event_callback(sl_gspi_handle_t gspi_handle, sl_gspi_signal_event_t callback_event);

/***************************************************************************/
/**
 * @brief To unregister the user callback function for GSPI events.
 * 
 * @details This API unregisters the user callback function that was previously registered
 * using \ref sl_si91x_gspi_register_event_callback. It is mandatory to call this function
 * before registering the callback again to avoid conflicts.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_gspi_register_event_callback
 ******************************************************************************/
void sl_si91x_gspi_unregister_event_callback(void);

/***************************************************************************/
/**
 * @brief To fetch the GSPI version.
 * 
 * @details This API reads and returns the current API version of GSPI.
 * 
 * @return \ref sl_gspi_version_t type structure containing the version information.
 ******************************************************************************/
sl_gspi_version_t sl_si91x_gspi_get_version(void);

/***************************************************************************/
/**
 * @brief To get the transfer status of GSPI.
 * 
 * @details This API returns the \ref sl_gspi_status_t type structure. The members are:
 *   - Busy
 *   - Data Lost
 *   - Mode Fault
 * It is used to poll the busy status of the GSPI Primary.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init 
 *      - \ref sl_si91x_gspi_set_configuration 
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle ( \ref sl_gspi_handle_t).
 * 
 * @return \ref sl_gspi_status_t type structure.
 ******************************************************************************/
sl_gspi_status_t sl_si91x_gspi_get_status(sl_gspi_handle_t gspi_handle);

/***************************************************************************/
/**
 * @brief To fetch the data receive count of the GSPI.
 * 
 * @details If a receive operation is started, this function can be used to get the number 
 * of data bytes received.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_gspi_init 
 *      - \ref sl_si91x_gspi_set_configuration 
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle ( \ref sl_gspi_handle_t).
 * 
 * @return uint32_t value of the RX data count.
 ******************************************************************************/
uint32_t sl_si91x_gspi_get_rx_data_count(sl_gspi_handle_t gspi_handle);

/***************************************************************************/
/**
 * @brief To get the transmit data count of GSPI.
 * 
 * @details If a send operation is started, this function can be used to get the number 
 * of data bytes sent.
 * 
 * @pre Pre-conditions:
 *  - \ref sl_si91x_gspi_init 
 *  - \ref sl_si91x_gspi_set_configuration
 * 
 * @return uint32_t Value of the TX data count.
 ******************************************************************************/
uint32_t sl_si91x_gspi_get_tx_data_count(void);

/***************************************************************************/
/**
 * @brief To fetch the clock division factor.
 * 
 * @details The clock division factor is calculated based on the configured peripheral clock. 
 * It determines the baud rate of GSPI.
 * 
 * @param[in] gspi_handle Pointer to the GSPI driver handle ( \ref sl_gspi_handle_t).
 * 
 * @return factor (int32_t) The value of the clock division factor.
 ******************************************************************************/
int32_t sl_si91x_gspi_get_clock_division_factor(sl_gspi_handle_t gspi_handle);

/***************************************************************************/
/**
 * @brief To fetch the frame length, i.e., bit width.
 * 
 * @details This API fetches the frame length. The frame length ranges between 1 and 15.
 * 
 * @return frame_length (uint32_t) The value of the frame length.
 ******************************************************************************/
uint32_t sl_si91x_gspi_get_frame_length(void);

/***************************************************************************/
/**
 * @brief To set the secondary number in multi-secondary operation.
 * 
 * @details For a single secondary, this API needs to be called before transferring the data.
 * 
 * @param[in] number Secondary number ( \ref sl_gspi_slave_number_t).
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
__STATIC_INLINE sl_status_t sl_si91x_gspi_set_slave_number(uint8_t number)
{
  sl_status_t status;
  if (number > GSPI_SLAVE_LAST_ENUM) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    RSI_GSPI_SetSlaveSelectNumber(number);
    status = SL_STATUS_OK;
  }
  return status;
}

/// @} end group GSPI ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup GSPI Generic SPI
 * @{
 * @details
 * @n @section GSPI_Intro Introduction
 * The Generic SPI Primary (GSPI) is available for MCU HP peripherals and provides
 * an I/O interface for a wide range of SPI-compatible peripheral devices.
 * SPI is a synchronous four-wire interface composed of two data pins (MOSI and MISO),
 * a device select pin (CSN), and a gated clock pin (SCLK).
 * With its two data pins, GSPI supports full-duplex operation with other SPI-compatible devices.
 * GSPI can be utilized in various applications within embedded systems where high-speed, synchronous serial communication is essential. Common use cases include:
 * 1. Test and Measurement Equipment
 * 2. Sensor Interfacing
 * 3. Display Interfaces
 * 4. Memory Devices
 * 5. Communication Interfaces
 * 6. Motor Control
 * 7. Audio Processing
 * 8. Wireless Communication
 * 
 * The GSPI controller's internal architecture is depicted in the block diagram below. The following diagram provides a visual representation of the key components and their interactions within the GSPI controller.
 * 
 * @image html gspi_block_diagram.png "GSPI Block Diagram"
 *
 * @n @section GSPI_Configuration Configuration
 * Configuring the GSPI in a microcontroller involves several steps, including setting up the peripheral registers, configuring GPIO pins, and selecting the appropriate operating mode and settings.
 * To configure the GSPI operating mode:
 * - Modify @ref clock_mode_typedef_t (Modes 0 and 3 can be configured)
 * - Set the Bit Rate (Range: 1 - 116,000,000)
 * - Set the Data Width from 1-16 bits
 * - Configure the GSPI secondary select mode by setting @ref slave_select_mode_typedef_t
 * All of these parameters must be configured in the @ref sl_gspi_control_config_t structure and then passed to the API @ref sl_si91x_gspi_set_configuration().
 * For more details on configuring available parameters, see the respective peripheral example README document.
 * 
 * The following images illustrate the GSPI bus operations. The first image shows the waveform for a GSPI write operation, detailing how data is transmitted 
 * from the primary to the secondary. The second image depicts the waveform for a GSPI read operation, highlighting how data is received from the secondary to the primary.
 * 
 * GSPI bus write operation waveform:
 * This diagram illustrates the sequence of signals during a write operation, including the timing of the clock, data, and chip select signals.
 * 
 * @image html gspi_write_operation.png "GSPI Write Operation"
 *
 * GSPI bus read operation waveform:
 * This diagram shows the timing and sequence of signals during a read operation, highlighting how data is transferred from the secondary to the primary.
 * 
 * @image html gspi_read_operation.png "GSPI Read Operation"
 * 
 * @n @section GSPI_Usage Usage
 * After specifying the GSPI configuration through the @ref sl_gspi_control_config_t structure, you can use the following common GSPI functions to initiate and configure GSPI:
 * 1. @ref sl_si91x_gspi_init - Initializes the GSPI module.
 * 2. @ref sl_si91x_gspi_set_configuration - Sets the GSPI configuration.
 * 3. @ref sl_si91x_gspi_register_event_callback - Registers a callback for GSPI events.
 * 4. @ref sl_si91x_gspi_send_data - Sends data using the GSPI.
 * 5. @ref sl_si91x_gspi_receive_data - Receives data using the GSPI.
 * 6. @ref sl_si91x_gspi_transfer_data - Transfers data using the GSPI.
 * 7. @ref sl_si91x_gspi_deinit - De-initializes the GSPI module.
 */
/** @} (end addtogroup GSPI) */

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_GSPI_H */
