/******************************************************************************
* @file sl_si91x_crc.h
* @brief CRC API implementation
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

#ifndef SL_SI91X_CRC_H_
#define SL_SI91X_CRC_H_
#include <stdint.h>
#include "rsi_crc.h"
#include "sl_status.h"
#include "data_types.h"

/***************************************************************************/
/**
 * @addtogroup CRC CRC Accelerator
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 *
 ******************************************************************************/

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
typedef RSI_CRC_PARAMS_T sl_crc_params_t; ///< Renamed CRC param structure
#define SL_CRC_POLYNOMIAL 0x04C11DB7      ///< Polynomial used for CRC calculation
#define SL_CRC_POLY_WIDTH 31              ///< Width of the polynomial
#define SL_CRC_LFSR_VAL   0x0000          ///< Initial LFSR value
#define SL_CRC_WIDTH_TYPE 0               ///< Type of CRC
#define SL_CRC_DIN_WIDTH  31              ///< Width of the input data
#define SL_CRC_NUM_BYTES  3               ///< Number of bytes to be written to the CRC
#define SL_CRC_AEMPTY     6               ///< Empty threshold level
#define SL_CRC_AFULL      2               ///< Full threshold level
#define SL_CRC_SWAP_DIN   0               ///< Swap the input data
#define SL_CRC_USE_UDMA   1               ///< Use uDMA for CRC calculation
#define SL_CRC_SWAP_LFSR  1               ///< Swap the LFSR value

/*******************************************************************************/
/**
 * @brief Initializes the CRC module with the provided configuration parameters.
 * 
 * @details This API configures the CRC module with the specified parameters by performing
 * the following operations in sequence:
 *   1. Sets the general control for CRC
 *   2. Enables the CRC module
 *   3. Loads the polynomial value
 *   4. Sets the polynomial width
 *   5. Initializes the LFSR value
 *   6. Configures the swapped initialization value
 *   7. Sets the data width type
 *   8. Configures the FIFO thresholds
 * 
 * @param[in] params Pointer to the CRC parameters structure containing the configuration settings.
 * 
 * @return Status code indicating the result:
 *        - SL_STATUS_OK  - Success.
 *        - SL_STATUS_INVALID_STATE  - CRC module is not in a valid state.
 *        - SL_STATUS_NOT_READY  - CRC module is not ready.
 *        - SL_STATUS_INITIALIZATION  - CRC polynomial configuration failed.
 *        - SL_STATUS_NOT_SUPPORTED  - Polynomial width setting is not supported.
 *        - SL_STATUS_NOT_INITIALIZED  - LFSR initialization failed.
 *        - SL_STATUS_NOT_AVAILABLE  - Swapped initialization configuration failed.
 *        - SL_STATUS_BUSY  - Data width type setting is busy.
 *        - SL_STATUS_ABORT  - FIFO threshold configuration failed.
 * * @note This function performs multiple operations to configure the CRC module.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_set_config(sl_crc_params_t *params);

/*******************************************************************************/
/**
 * @brief Clears the FIFO and resets all state machines.
 * 
 * @details This API clears the FIFO and resets all state machines. 
 * It ensures that the CRC module is in a known state before starting a new operation.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameter.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_set_gen_control(void);

/*******************************************************************************/
/**
 * @brief To retrieve the general status of the CRC signals.
 * 
 * @details This API retrieves the general status of the CRC signals and stores it in the provided pointer.
 * 
 * @param[out] reg_status Pointer to store the status of the GEN_STS Register.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_get_general_status(uint32_t *reg_status);

/*******************************************************************************/
/**
 * @brief To load the polynomial value into the polynomial register.
 * 
 * @details This API loads the polynomial value into the polynomial register using the provided CRC parameters.
 * @pre Pre-conditions:
 *      - \ref sl_si91x_crc_enable
 * 
 * @param[in] pCRCParams Pointer to the CRC parameters structure containing the polynomial value.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_NULL_POINTER  - Invalid null pointer received as an argument.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_polynomial(sl_crc_params_t *pCRCParams);

/*******************************************************************************/
/**
 * @brief To set the width of the polynomial for CRC computation.
 * 
 * @details This API sets the width of the polynomial for CRC computation. If a new width needs to be configured,
 * clear the existing length first by writing 0x1F to the polynomial_ctrl_reset register.
 * 
 * @param[in] pCRCParams Pointer to the CRC parameters structure containing the polynomial width.
 * @param[out] reg_status Pointer to store the FIFO status register value.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameters.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_polynomial_width(sl_crc_params_t *pCRCParams, uint32_t *reg_status);

/*******************************************************************************/
/**
 * @brief To initialize the LFSR value for CRC computation.
 * 
 * @details This API initializes the Linear Feedback Shift Register (LFSR) value for CRC computation.
 * The LFSR needs to be initialized with the init value whenever required.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_crc_enable
 * @param[in] pCRCParams Pointer to the CRC parameters structure containing the LFSR value.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Invalid parameters
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_lfsr_init(sl_crc_params_t *pCRCParams);

/*******************************************************************************/
/**
 * @brief To load a swapped version of the LFSR init value into the LFSR state.
 * 
 * @details This API loads a swapped version of the Linear Feedback Shift Register (LFSR) init value into the LFSR state.
 * It is used to initialize the LFSR with a swapped init value for CRC computation.
 * 
 *  @pre Pre-conditions:
 *     - \ref sl_si91x_crc_enable
 * 
 * @param[in] pCRCParams Pointer to the CRC parameters structure.
 * @param[out] reg_status Pointer to store the FIFO status register value.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_use_swapped_init(sl_crc_params_t *pCRCParams, uint32_t *reg_status);

/*******************************************************************************/
/**
 * @brief To set and control the data width types for CRC computation.
 * 
 * @details This API sets and controls the data width types for CRC computation using the provided CRC parameters.
 * 
 * @param[in] pCRCParams Pointer to the CRC parameters structure containing the data width type.
 * @param[out] oper_status Pointer to store the operation status of the API process.
 * 
 * @return Status code:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_set_data_width_type(sl_crc_params_t *pCRCParams, uint32_t *oper_status);

/*******************************************************************************/
/**
 * @brief To set the FIFO threshold levels for CRC computation.
 * 
 * @details This API sets the FIFO threshold levels for CRC computation. The maximum value for both thresholds is 15.
 * 
 * @param[in] pCRCParams Pointer to the CRC parameters structure containing the FIFO thresholds (aempty and afull).
 * @param[out] oper_status Pointer to store the operation status of the API process.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_set_fifo_thresholds(sl_crc_params_t *pCRCParams, uint32_t *oper_status);

/*******************************************************************************/
/**
 * @brief To set the input data for CRC calculation.
 * 
 * @details This API sets the input data for CRC calculation using the provided CRC parameters.
 * 
 *   @pre Pre-conditions:
 *      - \ref  sl_si91x_crc_enable
 *      - \ref  sl_si91x_crc_polynomial
 *      - \ref  sl_si91x_crc_polynomial_width
 *      - \ref  sl_si91x_crc_lfsr_init
 *      - \ref  sl_si91x_crc_set_data_width_type
 * 
 * @param[in] pCRCParams Pointer to the CRC parameters structure.
 * @param[in] data Input data for CRC calculation.
 * @param[out] oper_status Pointer to store the status of the operation.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_write_data(sl_crc_params_t *pCRCParams, uint32_t data, uint32_t *oper_status);

/*******************************************************************************/
/**
 * @brief To monitor the CRC calculation status and return the CRC value.
 * 
 * @details This API monitors the CRC calculation status and returns the calculated CRC value.
 * 
 *  @pre Pre-conditions:
 *      - \ref  sl_si91x_crc_enable
 *      - \ref  sl_si91x_crc_polynomial
 *      - \ref  sl_si91x_crc_polynomial_width
 *      - \ref  sl_si91x_crc_lfsr_init
 *      - \ref  sl_si91x_crc_set_data_width_type
 * 
 * @param[in] pCRCParams Pointer to the CRC parameters structure.
 * @param[out] crc Pointer to store the calculated CRC value.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_monitor_crc_calc(sl_crc_params_t *pCRCParams, uint32_t *crc);

/*******************************************************************************/
/**
 * @brief To write the LFSR state directly for dynamic operation.
 * 
 * @details This API writes the Linear Feedback Shift Register (LFSR) state directly for dynamic operation.
 * 
 *   @pre Pre-conditions:
 *      - \ref sl_si91x_crc_enable
 * 
 * @param[in] pCRCParams Pointer to the CRC parameters structure containing the LFSR state.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_lfsr_dynamic_write(sl_crc_params_t *pCRCParams);

/*******************************************************************************/
/**
 * @brief To reset the FIFO pointer and clear the FIFO.
 * 
 * @details This API resets the FIFO pointer and clears the FIFO, ensuring that the FIFO is empty and ready for new data.
 *   @pre Pre-conditions:
 *      - \ref sl_si91x_crc_enable
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success
 *         - SL_STATUS_FAIL  - Function failed
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_reset_fifo(void);

/*******************************************************************************/
/**
 * @brief To retrieve the FIFO status of the CRC occupancy.
 * 
 * @details This API retrieves the FIFO status of the CRC occupancy and stores the status in the provided register.
 * 
 *  @pre Pre-conditions:
 *      - \ref sl_si91x_crc_enable
 * 
 * @param[out] reg_status Pointer to store the FIFO status register value.
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_get_fifo_status(uint32_t *reg_status);

/*******************************************************************************/
/**
 * @brief To enable the CRC peripheral by enabling its clock.
 * 
 * @details This API enables the CRC peripheral by enabling its clock, allowing CRC operations to be performed.
 * 
 * @return Status code indicating the result::
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_enable(void);

/*******************************************************************************/
/**
 * @brief To disable the CRC peripheral by disabling its clock.
 * 
 * @details This API disables the CRC peripheral by disabling its clock, preventing any further CRC operations.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_crc_enable
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_FAIL  - Function failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 *******************************************************************************/
sl_status_t sl_si91x_crc_disable(void);

/** @} end group CRC */

/******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************/
/***************************************************************************/
/** 
 * @addtogroup CRC CRC Accelerator
 * @{
 *
 * @details
 *
 * @section CRC_Intro Introduction
 *
 * The Cyclic Redundancy Check (CRC) is a method used to verify the integrity of data in embedded systems. 
 * CRCs are commonly employed in communication protocols, storage devices, and error detection algorithms. 
 * By generating a checksum, CRC helps ensure that the data received or stored is accurate and unaltered.
 * 
 * Common use cases for CRC include:
 * - **Error Detection**: Identifying data transmission errors in communication protocols.
 * - **Data Integrity Verification**: Ensuring stored data remains uncorrupted.
 * - **Checksum Calculation**: Generating checksums for data validation.
 *
 * @section CRC_Config Configuration
 *
 * CRC can be configured using several parameters, including:
 * - **Polynomial**: The polynomial used for CRC calculation (@ref sl_crc_params_t).
 * - **Width**: The width of the CRC polynomial.
 * - **Data Input Type**: The width of the data input for the CRC engine.
 *
 * These configurations are encapsulated in the @ref sl_crc_params_t structure and initialized using the respective CRC APIs.
 *
 * For more details on configuration parameters, see the respective peripheral example readme document.
 *
 * @section CRC_Usage Usage
 *
 * After defining the CRC configuration structures and passing an instance of @ref sl_crc_params_t, the following functions can be used to initiate and configure the CRC module. The typical flow for implementation is as follows:
 *
 * 1. Set general control: @ref sl_si91x_crc_set_gen_control
 * 2. Enable CRC peripheral: @ref sl_si91x_crc_enable
 * 3. Load polynomial value: @ref sl_si91x_crc_polynomial
 * 4. Set polynomial width: @ref sl_si91x_crc_polynomial_width
 * 5. Initialize LFSR: @ref sl_si91x_crc_lfsr_init
 * 6. Configure data width type: @ref sl_si91x_crc_set_data_width_type
 * 7. Monitor CRC calculation: @ref sl_si91x_crc_monitor_crc_calc
 * 8. Reset FIFO: @ref sl_si91x_crc_reset_fifo
 * 9. Disable CRC peripheral: @ref sl_si91x_crc_disable
 * /
** @} end group CRC */

#endif /* SL_SI91X_CRC_H_ */
