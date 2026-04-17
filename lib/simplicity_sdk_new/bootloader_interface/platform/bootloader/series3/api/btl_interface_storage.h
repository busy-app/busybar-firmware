/***************************************************************************//**
 * @file
 * @brief Application interface to the storage component of the bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef BTL_INTERFACE_STORAGE_H
#define BTL_INTERFACE_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Get part series version.
#include "em_device.h"

/***************************************************************************//**
 * @addtogroup Series3 Series 3 APIs
 * @{
 * @addtogroup InterfaceS3 Interface
 * @{
 * @addtogroup StorageInterfaceS3 Application Storage Interface
 * @brief Application interface for interfacing with the bootloader storage.
 * @note These Bootloader APIs are not reentrant and should be wrapped in critical section
 *       where needed.
 * @details The Storage Interface is only available on bootloaders that declare
 *          they support @ref BOOTLOADER_CAPABILITY_STORAGE.
 *
 * @li @ref bootloader_interface_example
 *
 * @n @section bootloader_interface_example Example
 *
 * @brief Snippet for the OTA use case:
 * @verbatim
 * OTA Example
 * Assuming the user has an upgrade image downloaded which will be used to upgrade the application
 *
 * Initialize the bootloader interface
 * bootloader_init();
 *
 * Erase the storage slot in internal/SPI flash memory
 * bootloader_eraseStorageSlot(0);
 *
 * Write the upgrade image (GBL file data) to the slot. blinkGbl  uint8 array holding the GBL data in memory
 * bootloader_writeStorage(0, 0, blinkGbl, sizeof(blinkGbl));
 *
 * Reboot into the bootloader to install the new image
 * bootloader_rebootAndInstall();
 *
 * The general flow for bootloader interface APIs from the application is:
 *
 * General flow
 *
 * Initialize the bootloader interface
 * bootloader_init();
 *
 * Interface API accesses
 * ……………
 * ……………
 * ……………
 *
 * De-initialize the bootloader interface
 * bootloader_deinit();
 * @endverbatim
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Typedefs

/// @ingroup StorageInterfaceS3
/// @brief Possible storage types for Series 3 bootloader storage interface.
/// @typedef BootloaderStorageType_t
typedef enum {
  /// Storage backend is a SPI flash
  EXTERNAL_FLASH,
  /// Storage backend is internal flash
  INTERNAL_FLASH,
  /// Storage backend is custom
  CUSTOM_STORAGE
} BootloaderStorageType_t;

/// Information about a storage slot
typedef struct {
  /// Address of the slot.
  uint32_t address;
  /// Size of the slot.
  uint32_t length;
} BootloaderStorageSlot_t;

/// Information about the bootloader storage \n
/// <b>Note:</b> The <b>flashInfo</b> variable is only usable with
/// Gecko Bootloader version >= 2.0. All previous versions of the
/// Gecko Bootloader do not support the <b>flashInfo</b> data field.
typedef struct {
  /// The version of this data structure
  uint32_t version;
  /// The capabilities of the storage component
  uint32_t capabilities;
  /// Type of storage
  BootloaderStorageType_t storageType;
  /// Number of storage slots
  uint32_t numStorageSlots;
} BootloaderStorageInformation_t;

/// Erase status struct
typedef struct {
  /// Address of the current page to be erased
  uint32_t currentPageAddr;
  /// The size of a single erasable page in bytes
  uint32_t pageSize;
  /// Information about a storage slot
  BootloaderStorageSlot_t storageSlotInfo;
} BootloaderEraseStatus_t;

/// Storage API accessible from the application
typedef struct BootloaderStorageFunctions {
  /// Version of this struct
  uint32_t version;
  /// Get information about the storage -- capabilities, layout, configuration
  void (*getInfo)(BootloaderStorageInformation_t *info);
  /// Get information about storage slot -- size, location
  int32_t (*getSlotInfo)(uint32_t slotId, BootloaderStorageSlot_t *slot);
  /// Read bytes from slot into buffer
  int32_t (*read)(uint32_t slotId,
                  uint32_t offset,
                  uint8_t  *buffer,
                  size_t   length);
  /// Write bytes from buffer into slot
  int32_t (*write)(uint32_t slotId,
                   uint32_t offset,
                   uint8_t  *buffer,
                   size_t   length);
  /// Get application minimum version
  void (*getAppMinVersion)(uint32_t *version);
  // ------------------------------
  /// Mark a list of slots for bootload
  int32_t (*setImagesToBootload)(int32_t *slotIds, size_t length);
  /// Mark a list of slots for bootload
  int32_t (*getImagesToBootload)(int32_t *slotIds, size_t length);
  /// Append a slot to bootload list
  int32_t (*appendImageToBootloadList)(int32_t slotId);
  // ------------------------------
  /// Start image parsing
  int32_t (*initParseImage)(uint32_t                  slotId,
                            BootloaderParserContext_t *context,
                            size_t                    contextSize);
  /// Continue image verification
  int32_t (*verifyImage)(BootloaderParserContext_t  *context,
                         BootloaderParserCallback_t metadataCallback);
  /// Get app and bootloader upgrade information from storage slot
  int32_t (*getImageInfo)(BootloaderParserContext_t *context,
                          ApplicationData_t         *appInfo,
                          uint32_t                  *bootloaderVersion);
  /// Check whether the bootloader storage is busy
  bool (*isBusy)(void);
  /// Read raw bytes from storage
  int32_t (*readRaw)(uint32_t address, uint8_t *buffer, size_t length);
  /// Write bytes to raw storage
  int32_t (*writeRaw)(uint32_t address, uint8_t *buffer, size_t length);
  /// Erase storage
  int32_t (*eraseRaw)(uint32_t address, size_t length);
  /// Get configured DMA channel
  int32_t (*getDMAchannel)(void);
} BootloaderStorageFunctions_t;

// -----------------------------------------------------------------------------
// Defines

/// Context size for bootloader verification context
#define BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE            (3000)

/// Current version of the BootloaderStorageInformation_t struct
#define BOOTLOADER_STORAGE_INFO_VERSION                         (0x30000U)
// -----------------------------------------------------------------------------
// Functions

/***************************************************************************//**
 * Get information about the storage component.
 *
 * @param[out] info Information about the storage component.
 ******************************************************************************/
void bootloader_getStorageInfo(BootloaderStorageInformation_t *info);

/***************************************************************************//**
 * Get information about a storage slot.
 *
 * @param[in]  slotId ID of the slot to get information about
 * @param[out] slot Information about the storage slot
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_getStorageSlotInfo(uint32_t                slotId,
                                      BootloaderStorageSlot_t *slot);

/***************************************************************************//**
 * Read data from a storage slot.
 *
 * @param[in]  slotId  ID of the slot
 * @param[in]  offset Offset into the slot to start reading from
 * @param[out] buffer Buffer to store the data
 * @param[in]  length Amount of data to read
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_readStorage(uint32_t slotId,
                               uint32_t offset,
                               uint8_t  *buffer,
                               size_t   length);

/***************************************************************************//**
 * Write data to a storage slot.
 *
 * @note
 *   If DMA-based MSC write is enabled on the bootloader, writing data from
 *   flash to flash is not supported on Series-1 devices. DMA-based MSC write is
 *   enabled, both offset and buffer should be word aligned. In case the buffer
 *   is not aligned, the normal write procedure is used instead of DMA.
 *
 * @param[in] slotId ID of the slot
 * @param[in] offset Offset into the slot to start writing to
 * @param[in] buffer Buffer to read data to write from
 * @param[in] length Amount of data to write
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_writeStorage(uint32_t slotId,
                                uint32_t offset,
                                uint8_t  *buffer,
                                size_t   length);

/***************************************************************************//**
 * Erase and write data to a storage slot.
 *
 * @note This function automatically erases the following Flash page whenever
 *       the written data crosses a page boundary. In other words, the function
 *       can't be used to perform multiple sequential writes to the same
 *       address range unless the range starts at a page boundary.
 *       For a sequential write, the first call to this function should have
 *       a start address at a page boundary. Otherwise, the corresponding page
 *       of the starting address needs to be erased explicitly. If DMA-based
 *       MSC write is enabled on the bootloader, writing data from flash to
 *       flash is not supported on Series-1 devices.
 *
 * @param[in] slotId ID of the slot
 * @param[in] offset Offset into the slot to start writing to
 * @param[in] buffer Buffer to read data to write from
 * @param[in] length Amount of data to write
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_eraseWriteStorage(uint32_t slotId,
                                     uint32_t offset,
                                     uint8_t  *buffer,
                                     size_t   length);

/***************************************************************************//**
 * Erase all contents of a storage slot.
 *
 * @param[in] slotId ID of the slot
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_eraseStorageSlot(uint32_t slotId);

/***************************************************************************//**
 * Initialize chunked erase of a storage slot.
 *
 * @note This function must be called before calling
 *       @ref bootloader_chunkedEraseStorageSlot in a loop.
 *
 * @param[in] slotId    ID of the slot
 * @param[in] eraseStat Erase status struct
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_initChunkedEraseStorageSlot(uint32_t                slotId,
                                               BootloaderEraseStatus_t *eraseStat);

/***************************************************************************//**
 * Erase one page from a storage slot according to
 * the struct BootloaderEraseStatus_t.
 *
 * @note @ref bootloader_initChunkedEraseStorageSlot must be called
 *       before calling this function, in order to prepare
 *       BootloaderEraseStatus_t.
 *
 * @note This can be called sequentially to, for example, erase all contents
 *       of a storage slot.
 *
 * @param[in] eraseStat Erase status struct
 *
 * @return @ref SL_STATUS_BL_STORAGE_CONTINUE if erasing a page was
 *         successful. Erase can be continued by calling this function again.
 *         @ref BOOTLOADER_OK if the entire slot has been erased,
 *         else error code in @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_chunkedEraseStorageSlot(BootloaderEraseStatus_t *eraseStat);

/***************************************************************************//**
 * Set a prioritized list of images to attempt to bootload. The last call to
 * this function determines which slot will be installed when
 * @ref bootloader_rebootAndInstall is called.
 *
 * @param[in] slotIds Prioritized list of slot IDs to attempt to bootload. The
 *                    first image to pass verification will be bootloaded.
 * @param[in] length  Length of the slotIds array
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_BOOTLOAD_BASE range
 ******************************************************************************/
int32_t bootloader_setImagesToBootload(int32_t *slotIds, size_t length);

/***************************************************************************//**
 * Get the prioritized list of images the bootloader will attempt to bootload.
 *
 * @param[out] slotIds Prioritized list of slot IDs to attempt to bootload. The
 *                     first image to pass verification will be bootloaded.
 * @param[in] length   Length of the slotIds array
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_BOOTLOAD_BASE range
 ******************************************************************************/
int32_t bootloader_getImagesToBootload(int32_t *slotIds, size_t length);

/***************************************************************************//**
 * Append a single image to the list of images to attempt to bootload.
 *
 * @param[in] slotId ID of the slot to append
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_BOOTLOAD_BASE range
 ******************************************************************************/
int32_t bootloader_appendImageToBootloadList(int32_t slotId);

/***************************************************************************//**
 * Set a single image to attempt to bootload.
 *
 * @param[in] slotId ID of the slot
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_BOOTLOAD_BASE range
 ******************************************************************************/
int32_t bootloader_setImageToBootload(int32_t slotId);

/***************************************************************************//**
 * Initialize image verification.
 *
 * Initialize verification of an upgrade image stored in a bootloader storage
 * slot.
 *
 * @note This function must be called before calling
 *       @ref bootloader_continueVerifyImage in a loop.
 *
 * @note The context pointer must point to memory allocated by the caller.
 *       The caller must ensure that the context pointer is 32 bit aligned.
 *       The required size of this context may depend on the version
 *       of the bootloader. The required size for the bootloader associated with
 *       this version of the application interface is given by the define
 *       @ref BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE.
 *
 * @note Instead of calling @ref bootloader_initVerifyImage followed by
 *       @ref bootloader_continueVerifyImage, call
 *       @ref bootloader_verifyImage if no
 *       time-critical tasks are needed and sufficient stack space is
 *       available for the automatically allocated context. The purpose of the
 *       init-and-continue functions is to allow the caller to service system
 *       needs during verification.
 *
 *
 * @param[in] slotId      ID of the slot to check.
 * @param     context     Pointer to memory used to hold the parser context.
 * @param[in] contextSize Size of the context. An error is returned if the
 *                        supplied context is too small.
 *
 * @return @ref BOOTLOADER_OK if the image parser was initialized, else error
 *         code.
 ******************************************************************************/
int32_t bootloader_initVerifyImage(uint32_t slotId,
                                   void     *context,
                                   size_t   contextSize);

/***************************************************************************//**
 * Continue image verification.
 *
 * Continue verification of an upgrade image stored in a bootloader storage
 * slot.
 *
 * @note This function must be called in a loop until anything other than
 *       @ref SL_STATUS_BL_PARSE_CONTINUE is returned.
 *
 * @note @ref bootloader_initVerifyImage must be called before calling this
 *       function to reset the parser.
 *
 * @note Instead of calling @ref bootloader_initVerifyImage followed by
 *       @ref bootloader_continueVerifyImage, call
 *       @ref bootloader_verifyImage if no
 *       time-critical tasks are needed. The purpose of the
 *       init-and-continue functions is to allow the caller to service system
 *       needs during verification.
 *
 *
 * @param     context          Pointer to a context structure that has
 *                             been initialized by calling
 *                             @ref bootloader_initVerifyImage()
 * @param[in] metadataCallback Function pointer, which is called when
 *                             the binary metadata in the image is currently
 *                             verified. Set to NULL if not required.
 *
 * @return @ref SL_STATUS_BL_PARSE_CONTINUE if parsing was successful, and
 *         the parser expects more data. @ref SL_STATUS_BL_PARSE_SUCCESS if
 *         the parser has successfully parsed the image and it passes
 *         verification. Else error code.
 ******************************************************************************/
int32_t bootloader_continueVerifyImage(void                       *context,
                                       BootloaderParserCallback_t metadataCallback);

/***************************************************************************//**
 * Verify that the image in the given storage slot is valid.
 *
 * @param[in] slotId ID of the slot to check
 * @param[in] metadataCallback Function pointer, which is called when
 *                             binary metadata is present in the storage slot.
 *                             Set to NULL if not required.
 *
 * @note This function allocates a context structure of the size given by
 *       @ref BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE on the caller's
 *       stack. To manage memory and allocate the
 *       context elsewhere (on the heap, as a global variable, and so on), use
 *       @ref bootloader_initVerifyImage and @ref bootloader_continueVerifyImage
 *       functions instead.
 *
 * @return @ref BOOTLOADER_OK if the image is valid, else error code.
 ******************************************************************************/
int32_t bootloader_verifyImage(uint32_t                   slotId,
                               BootloaderParserCallback_t metadataCallback);

/***************************************************************************//**
 * Get application and bootloader upgrade metadata from the storage slot.
 *
 * @param[in]  slotId            ID of the slot to check
 * @param[out] appInfo           Pointer to @ref ApplicationData_t struct
 * @param[out] bootloaderVersion Pointer to an integer representing bootloader
 *                               version
 *
 * @return @ref BOOTLOADER_OK if metadata was filled successfully
 ******************************************************************************/
int32_t bootloader_getImageInfo(uint32_t          slotId,
                                ApplicationData_t *appInfo,
                                uint32_t          *bootloaderVersion);

/***************************************************************************//**
 * Check whether the bootloader storage is busy.
 *
 * @return True if the storage is busy
 ******************************************************************************/
bool bootloader_storageIsBusy(void);

/***************************************************************************//**
 * Read raw data from storage.
 *
 * @param[in]  address Address to start reading from
 * @param[out] buffer  Buffer to store the data
 * @param[in]  length  Amount of data to read
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_readRawStorage(uint32_t address,
                                  uint8_t  *buffer,
                                  size_t   length);

/***************************************************************************//**
 * Write data to storage.
 *
 * @note
 *   If DMA-based MSC write is enabled on the bootloader, writing data from
 *   flash to flash is not supported on Series-1 devices.
 *
 * @param[in] address Address to start writing to
 * @param[in] buffer  Buffer to read data to write from
 * @param[in] length  Amount of data to write
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_writeRawStorage(uint32_t address,
                                   uint8_t  *buffer,
                                   size_t   length);

/***************************************************************************//**
 * Erase data from storage.
 *
 * @note Erasing storage must adhere to the limitations of the underlying
 *       storage medium, such as requiring full page erases. Use
 *       @ref bootloader_getStorageInfo to learn about the limitations of the
 *       configured storage medium.
 *
 * @param[in] address Address to start erasing from
 * @param[in] length  Length of data to erase
 *
 * @return @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_STORAGE_BASE range
 ******************************************************************************/
int32_t bootloader_eraseRawStorage(uint32_t address, size_t length);
/***************************************************************************//**
* Get application minimum version
*
* @note Get the application minimum version to validate with application
*       version when ROLLBACK PROTECTION is enabled in bootloader core
*       configuration
*
* @param[in] version Application minimum version
*
*******************************************************************************/
void bootloader_getAppMinVersion(uint32_t *version);

/** @} (end addtogroup StorageInterfaceS3) */
/** @} (end addtogroup Interface3) */
/** @} (end addtogroup Series3) */

#endif // BTL_INTERFACE_STORAGE_H
