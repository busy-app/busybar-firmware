/***************************************************************************//**
 * @file
 * @brief Application interface to the bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef BTL_INTERFACE_H
#define BTL_INTERFACE_H

#include <stddef.h>

#include "btl_errorcode.h"
#include "btl_reset_info.h"
#include "application_properties.h"

// Include component-specific interfaces
#include "btl_interface_parser.h"
#include "btl_interface_storage.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "btl_interface_lock.h"
#endif

// Get flash page size
#include "em_device.h"

/***************************************************************************//**
 * @addtogroup Series3 Series 3 APIs
 * @{
 * @addtogroup InterfaceS3 Application Interface
 * @brief ApplicationS3 interface to the bootloader
 * @details
 *   The application interface consists of functions that can be included
 *   in the customer application that and will communicate with the
 *   bootloader through the @ref MainBootloaderTable_t. This table
 *   contains function pointers to the bootloader. The 10th word of the
 *   bootloader contains a pointer to this struct, allowing any application to
 *   easily locate it.
 *   To access the bootloader table, use wrapper functions. Avoid
 *   accessing the bootloader table directly.
 *
 * @{
 * @addtogroup CommonInterfaceS3 Common Application Interface
 * @brief Generic application interface available on all versions of the
 *        bootloader, regardless of the available components.
 * @note These Bootloader APIs are not reentrant and should be wrapped in critical section
 *       where needed.
 * @details
 * @{
 ******************************************************************************/

/// Bare boot table. Can be mapped on top of vector table to access contents.
typedef struct {
  /// Pointer to top of stack
  uint32_t *stackTop;
  /// Pointer to reset vector
  void     (*resetVector)(void);
  /// Reserved pointers to fault handlers
  uint32_t reserved0[5];
  /// Reserved pointers to RESERVED fields
  uint32_t reserved1[3];
  /// Pointer to bootloader table
  void     *table;
  /// Reserved pointers to SVC and DebugMon interrupts
  uint32_t reserved2[2];
  /// Pointer to application signature
  void     *signature;
} BareBootTable_t;

// --------------------------------
// Bootloader defines

/// Bootloader version major version shift value
#define BOOTLOADER_VERSION_MAJOR_SHIFT (24U)
/// Bootloader version minor version shift value
#define BOOTLOADER_VERSION_MINOR_SHIFT (16U)
/// Bootloader version major version mask
#define BOOTLOADER_VERSION_MAJOR_MASK (0xFF000000U)
/// Bootloader version minor version mask
#define BOOTLOADER_VERSION_MINOR_MASK (0x00FF0000U)

// --------------------------------
// Bootloader information typedefs
/// Type of bootloader
typedef enum {
  /// No bootloader present.
  NO_BOOTLOADER = 0,
  /// Bootloader is a Silicon Labs bootloader.
  SL_BOOTLOADER = 1
} BootloaderType_t;

/// Information about the current bootloader
typedef struct {
  /// The type of bootloader.
  BootloaderType_t type;
  /// Version number of the bootloader
  uint32_t version;
  /// Capability mask for the bootloader.
  uint32_t capabilities;
} BootloaderInformation_t;

/// Common header for bootloader tables
typedef struct {
  /// Type of image
  uint32_t type;
  /// Version number of the bootloader/application table
  uint32_t layout;
  /// Version number of the image
  uint32_t version;
} BootloaderHeader_t;

/// Address table for the First Stage Bootloader
typedef struct {
  /// Header of the First Stage Bootloader table
  BootloaderHeader_t header;
  /// Start address of the Main Bootloader
  BareBootTable_t *mainBootloader;
  /// Location of the Main Bootloader upgrade image
  BareBootTable_t *upgradeLocation;
} FirstBootloaderTable_t;

/// Address table for the Main Bootloader
typedef struct {
  /// Header of the Main Bootloader table
  BootloaderHeader_t header;
  /// Size of the Main Bootloader
  uint32_t size;
  /// Start address of the application
  BareBootTable_t *startOfAppSpace;
  /// End address of the allocated application space
  uint32_t *endOfAppSpace;
  /// Capabilities of the bootloader
  uint32_t capabilities;
  // ------------------------------
  /// Initialize bootloader for use from application
  int32_t (*init)(void);
  /// Deinitialize bootloader after use from application
  int32_t (*deinit)(void);
  // ------------------------------
  /// Verify application
  bool (*verifyApplication)(uint32_t startAddress);
  // ------------------------------
  /// Initialize parser
  int32_t (*initParser)(BootloaderParserContext_t *context, size_t contextSize);
  /// Parse a buffer
  int32_t (*parseBuffer)(BootloaderParserContext_t         *context,
                         const BootloaderParserCallbacks_t *callbacks,
                         uint8_t                           data[],
                         size_t                            numBytes);
  // ------------------------------
  /// Function table for storage component
  const BootloaderStorageFunctions_t *storage;
  // ------------------------------
  /// Parse a buffer and get application and bootloader upgrade metadata from the buffer.
  int32_t (*parseImageInfo)(BootloaderParserContext_t *context,
                            uint8_t                    data[],
                            size_t                     numBytes,
                            ApplicationData_t          *appInfo,
                            uint32_t                   *bootloaderVersion);
  // ------------------------------
  /// Size of context buffer used by bootloader image parser to store parser state
  uint32_t (*parserContextSize)(void);
  // ------------------------------
  /// Remaining number of application upgrades
  uint32_t (*remainingApplicationUpgrades)(void);
  // ------------------------------
  /// Get the list of the peripheral that is used by the bootloader
  void (*getPeripheralList)(uint32_t *ppusatd0, uint32_t *ppusatd1);
  // ------------------------------
  /// Get base address of bootloader upgrade image
  uint32_t (*getUpgradeLocation)(void);
} MainBootloaderTable_t;

// --------------------------------
// Bootloader capabilities

/// Bootloader enforces signed application upgrade images
#define BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_SIGNATURE        (1 << 0)
/// Bootloader enforces encrypted application upgrade images
#define BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_ENCRYPTION       (1 << 1)
/// @brief Bootloader enforces signature verification of the application image
///        before every boot
#define BOOTLOADER_CAPABILITY_ENFORCE_SECURE_BOOT              (1 << 2)

/// Bootloader has the capability of being upgraded
#define BOOTLOADER_CAPABILITY_BOOTLOADER_UPGRADE               (1 << 4)

/// Bootloader has the capability of parsing GBL files
#define BOOTLOADER_CAPABILITY_GBL                              (1 << 5)
/// Bootloader has the capability of parsing signed GBL files
#define BOOTLOADER_CAPABILITY_GBL_SIGNATURE                    (1 << 6)
/// Bootloader has the capability of parsing encrypted GBL files
#define BOOTLOADER_CAPABILITY_GBL_ENCRYPTION                   (1 << 7)
/// @brief Bootloader enforces signature verification of the application image
///        before every boot using certificate
#define BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT  (1 << 8)
/// Bootloader has the capability of application rollback protection
#define BOOTLOADER_CAPABILITY_ROLLBACK_PROTECTION              (1 << 9)
/// Bootloader has the capability to check the peripherals in use
#define BOOTLOADER_CAPABILITY_PERIPHERAL_LIST                  (1 << 10)

/// Bootloader rejects directly signed images without certificates
#define BOOTLOADER_CAPABILITY_REJECT_DIRECT_SIGNED_IMG         (1 << 11)

/// @brief Bootloader has the capability of storing data in an internal or
/// external storage medium
#define BOOTLOADER_CAPABILITY_STORAGE                     (1 << 16)
/// @brief Bootloader has the capability of communicating with host processors
/// using a communication interface
#define BOOTLOADER_CAPABILITY_COMMUNICATION               (1 << 20)

// --------------------------------
// Magic constants for bootloader tables

/// Magic word indicating first stage bootloader table
#define BOOTLOADER_MAGIC_FIRST_STAGE                      (0xB00710ADUL)
/// Magic word indicating main bootloader table
#define BOOTLOADER_MAGIC_MAIN                             (0x5ECDB007UL)

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN
#define BOOTLOADER_HEADER_VERSION_FIRST_STAGE             (0x00000001UL)
#define BOOTLOADER_HEADER_VERSION_MAIN                    (0x00000002UL)
/// @endcond

// --------------------------------
// Bootloader table access

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

// No bootloader area: Place the bootloader in main flash
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00008000UL)
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE \
                                           - (FLASH_BASE))

#define BTL_MAIN_STAGE_BASE               FLASH_BASE
#define BTL_MAIN_BOOTLOADER_TABLE_BASE    (BTL_MAIN_STAGE_BASE \
                                           + offsetof(BareBootTable_t, table))

/// @endcond // DO_NOT_INCLUDE_WITH_DOXYGEN

#if defined(MAIN_BOOTLOADER_TEST)
extern MainBootloaderTable_t *mainBootloaderTable;
#else
/// Pointer to main bootloader table
#define mainBootloaderTable               (*(MainBootloaderTable_t **) \
                                           (BTL_MAIN_BOOTLOADER_TABLE_BASE))
#endif // MAIN_BOOTLOADER_TEST
// --------------------------------
// Functions

/***************************************************************************//**
 * Get information about the bootloader on this device.
 *
 * The returned information is fetched from the main bootloader
 * information table.
 *
 * @param[out] info Pointer to the bootloader information struct.
 ******************************************************************************/
void bootloader_getInfo(BootloaderInformation_t *info);

/***************************************************************************//**
 * Initialize components of the bootloader
 * so the app can use the interface. This typically includes initializing
 * serial peripherals for communication with external SPI flashes, and so on.
 *
 * @return Error code. @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_INIT_BASE range.
 ******************************************************************************/
int32_t bootloader_init(void);

/***************************************************************************//**
 * De-initialize components of the bootloader that were previously initialized.
 * This typically includes powering down external SPI flashes and
 * de-initializing the serial peripheral used for communication with the
 * external flash.
 *
 * @return Error code. @ref BOOTLOADER_OK on success, else error code in
 *         @ref BOOTLOADER_ERROR_INIT_BASE range.
 ******************************************************************************/
int32_t bootloader_deinit(void);

/***************************************************************************//**
 * Reboot into the bootloader to perform an install.
 *
 * If there is a storage component and a slot is marked for bootload, install
 * the image in that slot after verifying it.
 *
 * If a communication component is present, open the communication channel and
 * receive an image to be installed.
 ******************************************************************************/
void bootloader_rebootAndInstall(void);

/***************************************************************************//**
 * Verify the application image stored in the Flash memory starting at
 * the address startAddress.
 *
 * If the secure boot is enforced, the function will only return true if the
 * cryptographic signature of the application is valid. Otherwise, the
 * application is verified according to the signature type defined in the
 * ApplicationProperties_t structure embedded in the application. Silicon Labs
 * wireless stacks include a declaration of this structure. However,
 * applications not using a full wireless stack may need to instantiate
 * the structure.
 *
 * Examples of results when the secure boot is not enforced:
 * - App has no signature: Valid if initial stack pointer and program counter
 *                         have reasonable values.
 * - App has CRC checksum: Valid if checksum is valid.
 * - App has ECDSA signature: Valid if ECDSA signature is valid.
 *
 * When secure boot is enforced, only ECDSA signed applications with
 * a valid signature are considered valid.
 *
 *
 * @param[in] startAddress Starting address of the application.
 *
 * @return True if the application is valid, else false.
 ******************************************************************************/
bool bootloader_verifyApplication(uint32_t startAddress);

/***************************************************************************//**
 * Check whether signature verification on the application image in internal flash
 * is enforced before every boot.
 *
 * @return True if signature verification is enforced, else false.
 ******************************************************************************/
bool bootloader_secureBootEnforced(void);

/***************************************************************************//**
 * Get bootloader certificate version.
 *
 * @param[out] version           Bootloader certificate version
 *
 * @return True if a valid certificate version is found.
 ******************************************************************************/
bool bootloader_getCertificateVersion(uint32_t *version);

/***************************************************************************//**
 * Get reset cause of the bootloader.
 *
 * @return Reset cause of the bootloader.
 ******************************************************************************/
BootloaderResetCause_t bootloader_getResetReason(void);

/***************************************************************************//**
 * Check if a pointer is valid and if it points to the bootloader main stage.
 *
 * This function checks pointers to bootloader
 * jump tables.
 *
 *
 * @param[in] ptr The pointer to check
 *
 * @return True if the pointer points into the bootloader main stage,
 *         false if not.
 ******************************************************************************/
__STATIC_INLINE bool bootloader_pointerValid(const void *ptr);
__STATIC_INLINE bool bootloader_pointerValid(const void *ptr)
{
#if BTL_MAIN_STAGE_BASE > 0U
  if (((size_t)ptr >= BTL_MAIN_STAGE_BASE)
      && ((size_t)ptr < (BTL_MAIN_STAGE_BASE + BTL_MAIN_STAGE_MAX_SIZE))) {
    return true;
  } else {
    return false;
  }
#else
  if ((size_t)ptr < (BTL_MAIN_STAGE_BASE + BTL_MAIN_STAGE_MAX_SIZE)) {
    return true;
  } else {
    return false;
  }
#endif
}

/***************************************************************************//**
 * Get Aliased flash address
 *
 * This function gets aliased flash address
 *
 * @param[in] address to be converted based on secure/nonsecure
 *
 * @return Aliased address
 ******************************************************************************/
__STATIC_INLINE uint32_t bootloader_GetAliasedAddr(uint32_t address);
__STATIC_INLINE uint32_t bootloader_GetAliasedAddr(uint32_t address)
{
#ifdef SL_TRUSTZONE_SECURE
  return (address | 0x10000000);
#else
  return (address & ~0x10000000);
#endif
}
/** @} (end addtogroup CommonInterface) */
/** @} (end addtogroup Interface) */
/** @} (end addtogroup Series3) */

#endif // BTL_INTERFACE_H
