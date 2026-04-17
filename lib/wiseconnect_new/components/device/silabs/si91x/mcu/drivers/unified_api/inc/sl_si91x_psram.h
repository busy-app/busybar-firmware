/******************************************************************************
* @file  sl_si91x_psram.h
* @brief PSRAM Driver API implementation
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
#ifndef __SL_SI91X_PSRAM_H_
#define __SL_SI91X_PSRAM_H_

#include <stdbool.h>
#include <stdint.h>
#include "rsi_qspi_proto.h"
#if (SL_SI91X_D_CACHE_ENABLE == 1)
#include "rsi_d_cache.h"
#endif

/***************************************************************************/
/**
 * @addtogroup PSRAM PSRAM Driver
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 *
 ******************************************************************************/

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/

/** @defgroup PSRAM_COMMANDS PSRAM Commands
 * @{
 */

/** @brief Read ID command */
#define PSRAM_READ_ID (0x9F)

/** @brief Enter QPI interface mode command */
#define PSRAM_ENTER_QPI (0x35)

/** @brief Exit QPI interface mode command */
#define PSRAM_EXIT_QPI (0xF5)

/** @brief Reset Enable command */
#define PSRAM_RESET_EN (0x66)

/** @brief Reset command */
#define PSRAM_RESET (0x99)

/** @brief Burst Length Toggle command */
#define PSRAM_BURST_LEN (0xC0)

/** @brief Mode Register Read command */
#define PSRAM_MODE_REG_READ (0xB5)

/** @brief Mode Register Write command */
#define PSRAM_MODE_REG_WRITE (0xB1)

/** @brief Sleep Entry command */
#define PSRAM_HALF_SLEEP (0xC0)

/** @} */ // end of PSRAM_COMMANDS

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/** @brief PSRAM Read Write Page Size */
#define PSRAM_READ_WRITE_PAGE_SIZE (16)

/** @brief Types of Read Write */
#define NORMAL_RW 1
#define FAST_RW   2
#define QUAD_RW   3

/** @brief Macro to convert MHz to Hz */
#define MHz_(a) (a * 1000000)

/** @brief Macro to convert Mbit to bits */
#define Mbit_(a) (a * 1000000)

/** @brief Enable D-Cache */

#if (SL_SI91X_D_CACHE_ENABLE == 1)

/** @brief M4SS D-Cache Base Address */
#define M4SS_DCACHE_BASE_ADDR (0x44040000)

/** @brief D-Cache Control Register */
#define DCACHE_REG_CTRL (*((uint32_t volatile *)(M4SS_DCACHE_BASE_ADDR + 0x010)))

/** @brief D-Cache Maintenance Status Register */
#define DCACHE_REG_MAINT_STATUS (*((uint32_t volatile *)(M4SS_DCACHE_BASE_ADDR + 0x028)))

#endif

/** @endcond */

/** @defgroup PSRAM_TIMINGS PSRAM Timings
 * @{
 */

/** @brief Sleep Exit chip select low pulse width in us */
#define tXPHS_US (12)

/** @brief Sleep Exit chip select low to CLK setup in us */
#define tXHS_US (160)

/** @brief Minimum sleep duration in us */
#define tHS_US (8)

/** @} */ // end of PSRAM_TIMINGS

/*******************************************************************************
 *****************************   DATA TYPES   *********************************
 ******************************************************************************/

/**
 * @brief PSRAM return error code
 */
typedef enum {
  PSRAM_SUCCESS,                ///< No error.
  PSRAM_FAILURE,                ///< Generic error
  PSRAM_UNKNOWN,                ///< Unknown request
  PSRAM_UNKNOWN_DEVICE,         ///< Unknown PSRAM Device
  PSRAM_CLOCK_INIT_FAILURE,     ///< Clock init failure
  PSRAM_NOT_INITIALIZED,        ///< PSRAM not initialized
  PSRAM_SUPPORTED_DEVICE,       ///< PSRAM Device supported
  PSRAM_DEVICE_MISMATCH,        ///< PSRAM Device mismatch
  PSRAM_INVALID_HSIZE,          ///< Invalid size of element
  PSRAM_NULL_ADDRESS,           ///< Null address
  PSRAM_INVALID_ADDRESS_LENGTH, ///< Invalid address length
  PSRAM_AUTO_MODE,              ///< PSRAM Auto mode
  PSRAM_MANUAL_MODE,            ///< PSRAM Manual mode
  PSRAM_UNSUPPORTED_SECURITY,   ///< Unsupported security
  PSRAM_MAX_SEC_SEGMENT_REACH   ///< Maximum secure segment reached
} sl_psram_return_type_t;

/**
 * @brief Wrap burst size enum
 */
typedef enum {
  _WRAP16, ///< Burst wrap size 16 Bytes
  _WRAP32, ///< Burst wrap size 32 Bytes
  _WRAP64, ///< Burst wrap size 64 Bytes
  _WRAP512 ///< Burst wrap size 512 Bytes
} sl_psram_burst_size_type_t;

/**
 * @brief PSRAM DMA status enum
 */
typedef enum {
  DMA_NONE, ///< DMA status default
  DMA_DONE, ///< DMA transfer completed
  DMA_FAIL  ///< DMA transfer failed
} sl_psram_dma_status_type_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/**
 * @brief PSRAM Read-Write Type
 */
typedef enum {
  normal_readwrite = 1, ///< Normal read/write operation
  fast_readwrite,       ///< Fast read/write operation
  quad_readwrite,       ///< Quad read/write operation
} PSRAMRWType;

/**
 * @brief PSRAM supported device list
 */
typedef enum {
  APS1604M_SQR,               ///< APS1604M_SQR device
  APS6404L_SQRH,              ///< APS6404L_SQRH device
  APS6404L_SQH,               ///< APS6404L_SQH device
  MAX_SUPPORTED_PSRAM_DEVICES ///< Maximum supported PSRAM devices
} PSRAMDevType;

/**
 * @brief PSRAM device initialization state enum
 */
typedef enum {
  initialised,   ///< PSRAM is initialized
  uninitialised, ///< PSRAM is uninitialized
  unknown        ///< PSRAM state is unknown
} PSRAMStateType;

/** @endcond */

/**
 * @brief Read ID structure
 */
typedef struct {
  uint8_t MFID;   ///< Manufacturer ID
  uint8_t KGD;    ///< Kodak Group Designator
  uint8_t EID[6]; ///< Extended ID
} sl_psram_id_type_t;

/**
 * @brief PSRAM configuration handle structure
 */
struct sl_psram_info_type_t {
  PSRAMDevType deviceName;         ///< PSRAM Device enum value
  sl_psram_id_type_t deviceID;     ///< Device ID struct
  uint32_t devDensity;             ///< Device Density in bits
  uint32_t normalReadMAXFrequency; ///< Max frequency for normal read
  uint32_t fastReadMAXFrequency;   ///< Max frequency for fast read
  PSRAMRWType rwType;              ///< Read-Write type configuration
  spi_config_t spi_config;         ///< SPI Config for QSPI interface
  uint16_t defaultBurstWrapSize;   ///< Default burst wrap size
  uint16_t toggleBurstWrapSize;    ///< Toggle Burst Wrap size
};

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/**
 * @brief PSRAM status context
 */
struct PSRAMStatusType {
  PSRAMStateType state;        ///< PSRAM state enum value
  uint8_t interfaceMode;       ///< QSPI Interface mode for PSRAM
  bool secureModeEnable;       ///< Secure mode enable status
  uint8_t secureSegmentNumber; ///< Secure segment number
  uint16_t burstSize;          ///< Burst Size
};

/**
 * @brief PSRAM pin configuration structure
 */
struct PSRAMPinConfigType {
  uint8_t port; ///< GPIO port number
  uint8_t pin;  ///< GPIO pin number
  uint8_t mux;  ///< GPIO mux setting
  uint8_t pad;  ///< GPIO pad setting
};

/**
 * @brief DMA transfer status enum
 */
typedef enum {
  IDLE,       ///< DMA is idle
  TX_RUNNING, ///< DMA transmission is running
  RX_RUNNING, ///< DMA reception is running
  FAILED      ///< DMA transfer failed
} XferStatusType;

/**
 * @brief DMA transfer context
 */
struct xferContextType {
  XferStatusType xferStatus;        ///< Current transfer status
  uint32_t xferNextAddress;         ///< Next address for transfer
  void *xferNextSourceAddress;      ///< Next source address for transfer
  uint32_t xferRemLength;           ///< Remaining length of transfer
  uint8_t xferHsize;                ///< Transfer size
  sl_psram_dma_status_type_t *done; ///< Pointer to DMA status
};

/**
 * @brief PSRAM secure segment configuration
 */
struct PSRAMSecureSegmentType {
  uint8_t segmentEnable;   ///< Segment enable flag
  uint32_t lowerBoundary;  ///< Lower boundary of the segment
  uint32_t higherBoundary; ///< Higher boundary of the segment
#define MAX_SEC_SEGMENTS 4 ///< Maximum number of secure segments
};

/** @endcond */

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************/
/**
 * @brief To configure and initialize the PSRAM Device 
 *
 * @return      
 *   Status code indicating the result.   
 *   - PSRAM_SUCCESS: Initialization successful
 *   - PSRAM_FAILURE: Initialization failed
 * 
 * @note The configurations for initialization are taken from the PSRAM Device 
 * config header file. The selection of the configuration file happens implicitly 
 * based on the radio board selection.         
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_init(void);

/***************************************************************************/
/**
 * @brief To uninitialize the PSRAM Device
 *
 * @return      
 *   Status code indicating the result.
 *   - PSRAM_SUCCESS: Uninitialization successful
 *   - PSRAM_FAILURE: Uninitialization failed
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_uninit(void);

/***************************************************************************/
/**
 * @brief To reset the PSRAM Device
 * 
 * @return Status code indicating the result.
 *   - PSRAM_SUCCESS: Reset successful
 *   - PSRAM_FAILURE: Reset failed
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_reset(void);

/***************************************************************************/
/**
 * @brief To write data to PSRAM in manual mode 
 * 
 * @param[in] addr
 *   PSRAM address for the write operation
 * @param[in] SourceBuf
 *   Reference to the source buffer
 * @param[in] hSize
 *   Size of each element
 * @param[in] length
 *   Number of elements for the write operation                        
 * 
 * @return Status code indicating the result.
 *   - PSRAM_SUCCESS: Write operation successful
 *   - PSRAM_FAILURE: Write operation failed
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_manual_write_in_blocking_mode(uint32_t addr,
                                                                    void *SourceBuf,
                                                                    uint8_t hSize,
                                                                    uint32_t length);

/***************************************************************************/
/**
 * @brief To read data from PSRAM in manual mode 
 * 
 * @param[in] addr
 *   PSRAM address for the read operation
 * @param[in] hSize
 *   Size of each element
 * @param[in] length
 *   Number of elements for the read operation
 * @param[out] DestBuf
 *   Reference to the destination buffer                       
 * 
 * @return Status code indicating the result.
 *   - PSRAM_SUCCESS: Read operation successful
 *   - PSRAM_FAILURE: Read operation failed
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_manual_read_in_blocking_mode(uint32_t addr,
                                                                   void *DestBuf,
                                                                   uint8_t hSize,
                                                                   uint32_t length);

/***************************************************************************/
/**
 * @brief To write data to PSRAM in manual mode using DMA
 * 
 * @param[in] addr
 *   PSRAM address for the write operation
 * @param[in] SourceBuf
 *   Reference to the source buffer
 * @param[in] hSize
 *   Size of each element
 * @param[in] length
 *   Number of elements for the write operation
 * @param[out] dmaStatus
 *   DMA operation completion status           
 * 
 * @return Status code indicating the result.
 *   - PSRAM_SUCCESS: Write operation successful
 *   - PSRAM_FAILURE: Write operation failed
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_manual_write_in_dma_mode(uint32_t addr,
                                                               void *SourceBuf,
                                                               uint8_t hSize,
                                                               uint32_t length,
                                                               sl_psram_dma_status_type_t *dmaStatus);

/***************************************************************************/
/**
 * @brief To read data from PSRAM in manual mode using DMA
 * 
 * @param[in] addr
 *   PSRAM address for the read operation
 * @param[out] DestBuf
 *   Reference to the destination buffer
 * @param[in] hSize
 *   Size of each element
 * @param[in] length
 *   Number of elements for the read operation
 * @param[out] dmaStatus
 *   DMA operation completion status                            
 * 
 * @return Status code indicating the result.
 *   - PSRAM_SUCCESS: Read operation successful
 *   - PSRAM_FAILURE: Read operation failed
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_manual_read_in_dma_mode(uint32_t addr,
                                                              void *DestBuf,
                                                              uint8_t hSize,
                                                              uint32_t length,
                                                              sl_psram_dma_status_type_t *dmaStatus);

/***************************************************************************/
/**
 * @brief To put the PSRAM device in sleep mode
 * 
 * @details Sleep mode is a feature that puts the PSRAM device in an ultra-low power state, 
 * while retaining the stored data.  
 * 
 * @return Status code indicating the result.
 *   - PSRAM_SUCCESS: Sleep mode enabled successfully
 *   - PSRAM_FAILURE: Failed to enable sleep mode
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_sleep(void);

/***************************************************************************/
/**
 * @brief To exit the PSRAM device from sleep mode
 * 
 * @return Status code indicating the result.
 *   - PSRAM_SUCCESS: Wakeup successful
 *   - PSRAM_FAILURE: Wakeup failed
 * 
 * @note This function call re-initializes the PSRAM device after the wakeup sequence.
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_wakeup(void);

/***************************************************************************/
/**
 * @brief To enable CTR encryption-decryption on PSRAM
 * 
 * @param[in] keySize
 *   Pass 128/256-bit size                                           
 * 
 * @return Status code indicating the result.
 *   - PSRAM_SUCCESS: Encryption-decryption enabled successfully
 *   - PSRAM_FAILURE: Failed to enable encryption-decryption
 ******************************************************************************/
sl_psram_return_type_t sl_si91x_psram_enable_encry_decry(uint16_t keySize);

/// @} end group PSRAM ********************************************************/

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** @addtogroup PSRAM PSRAM Driver
 * @{
 *
 *   @details
 *
 *
 *   @n @section PSRAM_Intro Introduction
 *
 *    PSRAM (Pseudo Static Random Access Memory) is a random-access memory whose
 *    internal structure is based on dynamic memory with refresh control signals
 *    generated internally, in the standby mode, so that it can mimic the functionality
 *    of a static memory. It combines the high density of DRAM with the ease-of-use
 *    of true SRAM. The M4 core communicates with the PSRAM via Quad SPI interface.
 * 
 *    @image html psram_ahb_interconnect_block_diagram.png  "Memory Architecture Diagram"
 *
 *    To enhance performance, PSRAM access is optimized through a cache memory system. 
 *    This "Modified Harvard Architecture" allows the core to access instructions and data through separate 
 *    caches (Data cache and Instruction cache), which then access the common PSRAM. When data is already 
 *    in the cache ("Cache Hit"), it’s quickly provided to the core. If not ("Cache Miss"), the cache 
 *    retrieves the data from PSRAM, improving overall performance.
 *
 *    The architecture is designed to ensure efficient memory access, with the data path supported by a 
 *    16KB, 4-way set associative cache with a 32-byte cache line. The instruction path leverages the existing 
 *    instruction cache shared between flash and PSRAM.
 *
 *    @image html psram_m4_qspi_controllers.png "PSRAM Accessibility"
 * 
 *   @n @section PSRAM_Config Configuration
 *
 *   ### PSRAM Device Configuration
 *
 *    The PSRAM Driver offers configuring the following:
 *    - Read-Write type
 *     - Normal: This is supported only in SPI interface mode. Supports a maximum
 *       frequency of 33MHz. Uses normal read and normal write commands.
 *     - Fast: Supported in SPI and QPI mode. Uses fast read and normal write.
 *     - Quad IO: Supported in SPI and QPI mode. Uses fast quad read and write.
 *    - Interface mode
 *     - SPI Mode (Serial IO)
 *     - QPI Mode (Quad IO)
 *
 *   ### Linker configurations
 *
 *    The text segment, data segment, bss, heap, and stack can be placed in PSRAM by
 *    installing the respective components present under "PSRAM Linker Configurations"
 *    from the "SOFTWARE COMPONENTS" GUI. Since PSRAM is already initialized in the bootloader,
 *    these components can be installed and the respective segments can be placed in
 *    PSRAM without installing the "PSRAM Core" component and without initializing PSRAM
 *    from the application.
 *
 *   - For more information on configuring available parameters, see the respective peripheral example readme document.
 *
 *   @n @section PSRAM_Usage Usage
 *
 *    The PSRAM Driver and QSPI are initialized by the bootloader with Quad IO read-write type
 *    and QPI interface. The application is not required to reinitialize the PSRAM device
 *    and QSPI unless the configurations required are different from the default set
 *    by the bootloader.
 *
 *    The PSRAM device handle "PSRAM_Device" of type @ref sl_psram_info_type_t is defined
 *    in "sl_si91x_psram_handle.c".
 *
 *    @ref sl_si91x_psram_uninit assumes that PSRAM was initialized with QPI mode and exits
 *    QPI mode within the definition. If the PSRAM configuration in the boot code has SPI mode enabled,
 *    the user is expected to comment out the exit QPI mode function call in @ref sl_si91x_psram_uninit.
 *
 *    To reconfigure and initialize PSRAM, set the required configurations from the PSRAM Core component
 *    and call @ref sl_si91x_psram_init within the application.
 *
 * @} end group PSRAM ********************************************************/

#endif //__SL_SI91X_PSRAM_H_