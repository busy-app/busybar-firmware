/******************************************************************************
* @file sl_si91x_efuse.h
* @brief EFUSE API implementation
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

#ifndef SL_SI91X_EFUSE_H
#define SL_SI91X_EFUSE_H

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
#include "rsi_rom_clks.h"
#include "rsi_rom_efuse.h"
#include "sl_status.h"
#include "sl_si91x_clock_manager.h"
/***************************************************************************/
/**
 * @addtogroup EFUSE eFuse Controller
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 *
 ******************************************************************************/

/*******************************************************************************
 ***************************  DEFINES / MACROS  ********************************
 ******************************************************************************/

/*******************************************************************************
 ********************************   Local Variables   **************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  Global  VARIABLES ********************************
 ******************************************************************************/

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/**
 * @brief Structure to hold the version number of the peripheral API.
 */
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_efuse_version_t;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************/
/**
 * @brief To retrieve the release, major and minor version of eFuse.
 * 
 * @details The API is used to retrieve the release, major, and minor version of eFuse.
 * 
 * @return \ref sl_efuse_version_t type structure containing the version information.
 ******************************************************************************/
sl_efuse_version_t sl_si91x_efuse_get_version(void);

/***************************************************************************/
/**
 * @brief To enable the peripheral clock for eFuse.
 * 
 * @details The API is used to enable the peripheral clock for eFuse.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully enabled the peripheral clock.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_enable_clock(void);

/***************************************************************************/
/**
 * @brief To disable the peripheral clock for eFuse.
 * 
 * @details The API is used to disable the peripheral clock for eFuse.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 *      - \ref sl_si91x_efuse_write_bit_v2()
 *      - \ref sl_si91x_efuse_memory_mapped_read_byte()
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully disabled the peripheral clock.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_disable_clock(void);

/***************************************************************************/
/**
 * @brief To initialize the eFuse by enabling its clock and the eFuse itself.
 * 
 * @details The API first enables the eFuse clock and then enables the eFuse.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Successfully initialized the eFuse.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_init(void);

/***************************************************************************/
/**
 * @brief To un-initialize the eFuse by disabling the eFuse and its clock.
 * 
 * @details The API first disables the eFuse and then disables the eFuse clock.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 *      - \ref sl_si91x_efuse_write_bit_v2()
 *      - \ref sl_si91x_efuse_memory_mapped_read_byte()
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_deinit(void);
/***************************************************************************/
/**
 * @brief To set the eFuse address for read and write operations.
 * 
 * @details The API is used to set the eFuse address for read and write operations.
 * 
 * @pre \ref sl_si91x_efuse_init()
 * 
 * @param[in] address Holds the address at which the data has to be written
 *                    in the eFuse range 0x0 to 0x31.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_OK  - Success.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_set_address(uint16_t address);

/***************************************************************************/
/**
 * @brief To get the eFuse address for read and write operations.
 * 
 * @details The API is used to retrieve the eFuse address for read and write operations.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 * 
 * @param[out] read_address Pointer holds the address at which the data has
 *                          to be written in the eFuse.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_get_address(uint16_t *read_address);

/***************************************************************************/
/**
 * @brief To write a bit in the position of an 8-bit location in the specified eFuse address.
 * 
 * @details The API writes a bit to a specific 8-bit location position at the designated
 *          eFuse address. There is only one programming scheme, which is single bit programming.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 * 
 * @param[in] address Holds the address at which the data has to be written in
 *                    the eFuse range 0 to 31.
 * @param[in] bit_pos Variable that holds the position of the bit on which the
 *                    data will be placed (0 - 7).
 * @param[in] hold_time eFuse strobe hold time, depends on the clock frequency of the eFuse controller.
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Bit write operation completed successfully.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_FAIL - Bit write operation failed.
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_write_bit(uint16_t address,
                                     uint8_t bit_pos,
                                     uint32_t hold_time) SL_DEPRECATED_API_WISECONNECT_4_0;

/***************************************************************************/
/**
 * @brief To write a bit in the position of an 8-bit location in the specified eFuse address.
 * 
 * @details The API writes a bit to a specific 8-bit location position at the designated
 *          eFuse address. There is only one programming scheme, which is single-bit programming.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 * 
 * @param[in] address Holds the address at which the data has to be written in
 *                    the eFuse range 0 to 31.
 * @param[in] bit_pos Variable that holds the position of the bit on which the
 *                    data will be placed (0 - 7).
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK - Bit write operation completed successfully.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_FAIL - Bit write operation failed.
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_write_bit_v2(uint16_t address, uint8_t bit_pos);

/***************************************************************************/
/**
 * @brief To read a 1-word (16 bits) of data from the 256-byte eFuse memory (OTP) in memory-mapped mode.
 * 
 * @details The API is used to read a 1-word (16 bits) of data from the 256-byte eFuse memory (OTP) in memory-mapped mode.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 *      - \ref sl_si91x_efuse_write_bit_v2()
 * 
 * @param[in] address Holds the address from where we are reading the 1 word (max value is 0x31).
 * @param[out] read_word Pointer that points to the data which is stored in the eFuse.
 * @param[in] soc_clk Input clock frequency of eFuse controller in range between (1000000 - 180000000).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_memory_mapped_read_word(uint16_t address, uint16_t *read_word, uint32_t soc_clk);

/***************************************************************************/
/**
 * @brief To read 1 byte of data from 32x8 byte eFuse memory (OTP) in memory-mapped mode.
 * 
 * @details The API is used to read 1 byte of data from 32x8 byte eFuse memory (OTP) in memory-mapped mode.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 *      - \ref sl_si91x_efuse_write_bit_v2()
 * 
 * @param[in] address Holds the address from where we are reading the 1 byte of data (max value is 0x31).
 * @param[out] read_byte Pointer that points to the 8-bit data which is stored in the eFuse.
 * @param[in] soc_clk Input clock frequency in range between (1000000 - 180000000).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_memory_mapped_read_byte(uint16_t address, uint8_t *read_byte, uint32_t soc_clk);

/***************************************************************************/
/**
 * @brief To read 1 byte of data from 32x8 byte eFuse memory (OTP) in FSM mode.
 * 
 * @details The API is used to read 1 byte of data from 32x8 byte eFuse memory (OTP) in FSM mode.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 *      - \ref sl_si91x_efuse_write_bit_v2()
 * 
 * @param[in] address Holds the address from where we are reading the 1 byte of data (max value is 0x31).
 * @param[out] read_byte Pointer that points to the 8-bit data which is stored in the eFuse.
 * @param[in] soc_clk Input clock frequency in range between (1000000 - 180000000).
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_INVALID_PARAMETER  - The parameter is an invalid argument.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on the status documentation, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_efuse_fsm_read_byte(uint16_t address, uint8_t *read_byte, uint32_t soc_clk);

/***************************************************************************/
/**
 * @brief To enable the eFuse by enabling its clock.
 * 
 * @details The API is used to enable the eFuse by enabling its clock.
 * 
 * @pre \ref sl_si91x_efuse_enable_clock()
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_efuse_enable(void)
{
  RSI_EFUSE_Enable(EFUSE);
}

/***************************************************************************/
/**
 * @brief To disable the eFuse by disabling its clock.
 * 
 * @details The API is used to disable the eFuse by disabling its clock.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_efuse_init()
 *      - \ref sl_si91x_efuse_set_address()
 *      - \ref sl_si91x_efuse_write_bit_v2()
 *      - \ref sl_si91x_efuse_memory_mapped_read_byte()
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_efuse_disable(void)
{
  RSI_EFUSE_Disable(EFUSE);
}

/** @} end group EFUSE */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/**  @addtogroup EFUSE eFuse Controller
* @{
*
* @details
*
* @section eFuse_Intro Introduction
*
* Electrically programmable fuses or eFuses are one-time programmable memory elements 
* used to store configuration or data in Integrated Circuits (ICs).
* eFuses use 32-bit addressing with each address holding 8 bits of data.
* The eFuse controller is used to program and read these bits.
* @li Once information is programmed into an eFuse, it is typically irreversible.
* Due to their irreversible nature, eFuses are ideal for storing critical data that cannot be altered.
*
* @image html efuse_timing_diagram.png "eFuse timinig diagram"
*
* @li eFuses find application in various areas, including:
* 1. Supports eFuse programming and read operations.
* 2. Supporting both memory mapped and FSM-based read operations.

* @section eFuse_Config Configuration
* 
* eFuse API's use for reading and writing into the E-fuse 256-bit address are:
* 1. Read:
*  - Read 1 byte of data from 32x8 byte eFuse memory (OTP) in FSM mode using @ref  sl_si91x_efuse_fsm_read_byte.
*  - Read 1 byte of data from 32x8 byte eFuse memory(OTP) in memory-mapped mode using @ref sl_si91x_efuse_memory_mapped_read_byte.
*  - Read the 1 word (16 bits) of data from 32x8 byte eFuse memory (OTP) in memory-mapped mode using @ref sl_si91x_efuse_memory_mapped_read_word.
* 2. Write a bit to a specific 8-bit location in the eFuse at the designated address using @ref  sl_si91x_efuse_write_bit_v2
* 
* @n @section eFuse_Use Usage
*
* The eFuse application used to various read and write operations with 
* OTP (one-time programmable) memory.
* The WRITE_ENABLE macro must be enabled before an application can write to memory.
* By default, only read operations are carried out at the designated memory address. 
* 
* An eFuse can be initialized and configured using the following APIs in default mode:
*
* 1. *Initializes the eFuse:* @ref sl_si91x_efuse_init
* 2. *Set the eFuse address for read and write operations:* @ref sl_si91x_efuse_set_address
* 3. *Get the eFuse address for read and write operations:* @ref sl_si91x_efuse_get_address
* 4. *Write the bit in the position of an 8-bit location in eFuse specified address:* @ref sl_si91x_efuse_write_bit_v2
* 5. *Read 1 byte of data from 32x8 byte eFuse memory (OTP) in FSM mode:* @ref sl_si91x_efuse_fsm_read_byte 
* 6. *Read 1 byte of data from 32x8 byte eFuse memory(OTP) in memory mapped mode:* @ref sl_si91x_efuse_memory_mapped_read_byte
* 7. *Read the 1 word (16 bits) of data from 32x8 byte eFuse memory (OTP) in memory mapped mode:* @ref sl_si91x_efuse_memory_mapped_read_word
* 8. *Un-initializes the eFuse:* @ref sl_si91x_efuse_deinit
*
*/
/** @} end group EFUSE */

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_EFUSE_H
/**************************************************************************************************/
