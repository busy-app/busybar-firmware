/***************************************************************************//**
 * @file
 * @brief Application interface to the bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "em_emu.h"
#include "em_cmu.h"

// Include component-specific interfaces
#include "btl_interface_parser.h"
#include "btl_interface_storage.h"

// Get flash page size
#include "em_device.h"

#if defined(SL_TRUSTZONE_NONSECURE)
#include "sli_tz_ns_interface.h"
#endif

#if defined(BURTC_PRESENT)
#include "em_burtc.h"
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
/***************************************************************************//**
 * @addtogroup Interface Application Interface
 * @brief Application interface to the bootloader
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
 * @addtogroup CommonInterface Common Application Interface
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

/// Bootloader interface APIs are trust zone aware
#if defined(BOOTLOADER_SECURE)
#define BOOTLOADER_INTERFACE_TRUSTZONE_AWARE
#elif defined(_SILICON_LABS_32B_SERIES_2) && !defined(BOOTLOADER_APPLOADER)
// The bootloader with AppLoader as the communication interface will not
// re-configure the SMU since it is using the NS peripherals by default.
#define BOOTLOADER_INTERFACE_TRUSTZONE_AWARE
#endif

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
  #if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)
  void (*getPeripheralList)(uint32_t *ppusatd0, uint32_t *ppusatd1, uint32_t *ppusatd2);
  #else
  void (*getPeripheralList)(uint32_t *ppusatd0, uint32_t *ppusatd1);
  #endif
  // ------------------------------
  /// Get base address of bootloader upgrade image
  uint32_t (*getUpgradeLocation)(void);
} MainBootloaderTable_t;

#if defined(BOOTLOADER_INTERFACE_TRUSTZONE_AWARE)
/// Struct that represents the state of the PPUSATDn, PPUPATDn and CLKENn registers
typedef struct {
  uint32_t PPUSATD0;
  uint32_t PPUSATD1;
  #if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)
  uint32_t PPUSATD2;
  #endif
  uint32_t BMPUSATD0;
#if defined(SMU_NS_CFGNS_BASE)
  uint32_t PPUPATD0;
  uint32_t PPUPATD1;
  #if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)
  uint32_t PPUPATD2;
  #endif
#endif
#if defined(_CMU_CLKEN0_MASK)
  uint32_t CLKEN0;
  uint32_t CLKEN1;
#endif
  uint32_t SMU_STATUS;
} Bootloader_PPUSATDnCLKENnState_t;

/// Struct containing function arguments
typedef struct Bootloader_inOutVec {
  void *base;   /// the start address of the memory buffer
  size_t len;   /// the size in bytes
} Bootloader_inOutVec_t;

#endif // BOOTLOADER_INTERFACE_TRUSTZONE_AWARE

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

/// @brief Bootloader has the capability of storing data in an internal or
/// external storage medium
#define BOOTLOADER_CAPABILITY_STORAGE                     (1 << 16)
/// @brief Bootloader has the capability of communicating with host processors
/// using a communication interface
#define BOOTLOADER_CAPABILITY_COMMUNICATION               (1 << 20)

/// The bootloader supports EM4 GPIO retention
#define BOOTLOADER_CAPABILITY_EM4_GPIO_RETENTION   (1 << 21)

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

#if defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT) || defined(MAIN_BOOTLOADER_TEST)
// No first stage on devices with SE
#define BTL_FIRST_STAGE_SIZE              (0UL)
#else
// First stage takes a single flash page
#define BTL_FIRST_STAGE_SIZE              (FLASH_PAGE_SIZE)
#define BOOTLOADER_HAS_FIRST_STAGE
#endif

#if defined(_SILICON_LABS_GECKO_INTERNAL_SDID_80)
// No writeable bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              0x00000000UL
#define BTL_APPLICATION_BASE              0x00004000UL
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE \
                                           - BTL_FIRST_STAGE_SIZE)
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_84)
// Dedicated bootloader area of 38k
// Place the bootloader in the dedicated bootloader area of the
// information block
#define BTL_FIRST_STAGE_BASE              0x0FE10000UL
#define BTL_APPLICATION_BASE              0x00000000UL
#define BTL_MAIN_STAGE_MAX_SIZE           (0x00009800UL - BTL_FIRST_STAGE_SIZE)
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_89)
#define BTL_FIRST_STAGE_BASE              0x0FE10000UL
#if defined(MAIN_BOOTLOADER_IN_MAIN_FLASH)
#define BTL_APPLICATION_BASE              0x00004800UL
#define BTL_MAIN_STAGE_MAX_SIZE           BTL_APPLICATION_BASE
#else
// Dedicated bootloader area of 16k
// Place the bootloader in the dedicated bootloader area of the
// information block
#define BTL_APPLICATION_BASE              0x00000000UL
#define BTL_MAIN_STAGE_MAX_SIZE           (0x00004000UL - BTL_FIRST_STAGE_SIZE)
#endif
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_95)
#define BTL_FIRST_STAGE_BASE              0x0FE10000UL
#if defined(MAIN_BOOTLOADER_IN_MAIN_FLASH)
#define BTL_APPLICATION_BASE              0x00004800UL
#define BTL_MAIN_STAGE_MAX_SIZE           BTL_APPLICATION_BASE
#else
// Dedicated bootloader area of 18k
// Place the bootloader in the dedicated bootloader area of the
// information block
#define BTL_APPLICATION_BASE              0x00000000UL
#define BTL_MAIN_STAGE_MAX_SIZE           (0x00004800UL - BTL_FIRST_STAGE_SIZE)
#endif
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_100) || defined(_SILICON_LABS_GECKO_INTERNAL_SDID_106)
// Dedicated bootloader area of 32k
// Place the bootloader in the dedicated bootloader area of the
// information block
#define BTL_FIRST_STAGE_BASE              0x0FE10000UL
#define BTL_APPLICATION_BASE              0x00000000UL
#define BTL_MAIN_STAGE_MAX_SIZE           (0x00008000UL - BTL_FIRST_STAGE_SIZE)
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_103)
// Dedicated bootloader area of 18k
// Place the bootloader in the dedicated bootloader area of the
// information block
#define BTL_FIRST_STAGE_BASE              0x0FE10000UL
#define BTL_APPLICATION_BASE              0x00000000UL
#define BTL_MAIN_STAGE_MAX_SIZE           (0x00004800UL - BTL_FIRST_STAGE_SIZE)
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_200)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#elif defined(BOOTLOADER_SECURE) && defined(BOOTLOADER_SUPPORT_COMMUNICATION)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00004000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE \
                                           - BTL_FIRST_STAGE_SIZE)
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_205)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE \
                                           - BTL_FIRST_STAGE_SIZE)
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_210)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#elif defined(BOOTLOADER_CUSTOM_SIZE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00004000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE    \
                                           - (BTL_FIRST_STAGE_BASE \
                                              + BTL_FIRST_STAGE_SIZE))
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_215)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE    \
                                           - (BTL_FIRST_STAGE_BASE \
                                              + BTL_FIRST_STAGE_SIZE))
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_220)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE    \
                                           - (BTL_FIRST_STAGE_BASE \
                                              + BTL_FIRST_STAGE_SIZE))
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_225)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE    \
                                           - (BTL_FIRST_STAGE_BASE \
                                              + BTL_FIRST_STAGE_SIZE))
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_230)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE    \
                                           - (BTL_FIRST_STAGE_BASE \
                                              + BTL_FIRST_STAGE_SIZE))
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_235)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE    \
                                           - (BTL_FIRST_STAGE_BASE \
                                              + BTL_FIRST_STAGE_SIZE))
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_240)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE    \
                                           - (BTL_FIRST_STAGE_BASE \
                                              + BTL_FIRST_STAGE_SIZE))
#elif defined(_SILICON_LABS_GECKO_INTERNAL_SDID_260)
// No bootloader area: Place the bootloader in main flash
#define BTL_FIRST_STAGE_BASE              FLASH_BASE
#if defined(BOOTLOADER_APPLOADER)
#if defined(BOOTLOADER_SECURE)
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00014000UL)
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00012000UL)
#endif // BOOTLOADER_SECURE
#else
#define BTL_APPLICATION_BASE              (FLASH_BASE + 0x00006000UL)
#endif // BOOTLOADER_APPLOADER
#define BTL_MAIN_STAGE_MAX_SIZE           (BTL_APPLICATION_BASE    \
                                           - (BTL_FIRST_STAGE_BASE \
                                              + BTL_FIRST_STAGE_SIZE))
#elif defined(MAIN_BOOTLOADER_TEST)
#else
#error "This part is not supported in this bootloader version."
#endif

#if defined(MAIN_BOOTLOADER_TEST) || defined(MAIN_BOOTLOADER_IN_MAIN_FLASH)
#define BTL_MAIN_STAGE_BASE               (FLASH_BASE)
#else
#define BTL_MAIN_STAGE_BASE               (BTL_FIRST_STAGE_BASE \
                                           + BTL_FIRST_STAGE_SIZE)
#endif

#if defined(BOOTLOADER_HAS_FIRST_STAGE)
#define BTL_FIRST_BOOTLOADER_TABLE_BASE   (BTL_FIRST_STAGE_BASE \
                                           + offsetof(BareBootTable_t, table))
#endif
#define BTL_MAIN_BOOTLOADER_TABLE_BASE    (BTL_MAIN_STAGE_BASE \
                                           + offsetof(BareBootTable_t, table))

/// @endcond // DO_NOT_INCLUDE_WITH_DOXYGEN

#if defined(MAIN_BOOTLOADER_TEST)
#if defined(BOOTLOADER_HAS_FIRST_STAGE)
extern FirstBootloaderTable_t * firstBootloaderTable;
#endif
extern MainBootloaderTable_t *mainBootloaderTable;
#else
#if defined(BOOTLOADER_HAS_FIRST_STAGE)
/// Pointer to first stage bootloader table
#define firstBootloaderTable              (*(FirstBootloaderTable_t **) \
                                           (BTL_FIRST_BOOTLOADER_TABLE_BASE))
#endif // BOOTLOADER_HAS_FIRST_STAGE
/// Pointer to main bootloader table
#define mainBootloaderTable               (*(MainBootloaderTable_t **) \
                                           (BTL_MAIN_BOOTLOADER_TABLE_BASE))
#endif // MAIN_BOOTLOADER_TEST
// --------------------------------
// -----------------------------------------------------------------------------

// EM4 GPIO Retention constants
#define MAX_EM4_RETAINED_PINS                   16
#define BURTC_WAKEUP_CYCLES                     2
#define BURAM_SIGNATURE_VALID                   0x5

#define BURAM_RESET_COUNTER_SIGNATURE           0xA
#define BURAM_RESET_COUNTER_MAX                 3    // Maximum consecutive failed wakeups

// BURAM Reset Reason Definitions (4-bit values for reset_reason field)

/// Normal application boot
#define BURAM_RESET_REASON_GO                 0x1u
/// Application requested that bootloader runs
#define BURAM_RESET_REASON_BOOTLOAD           0x2u
/// Bootloader detected bad  upgrade image
#define BURAM_RESET_REASON_BADIMAGE           0x3u
/// Bootloader detected bad application
#define BURAM_RESET_REASON_BADAPP             0x4u
/// Bootloader detected a fatal error
#define BURAM_RESET_REASON_FATAL              0x5u

#pragma pack(push, 1)
// EM4 GPIO Retention data structures
typedef union {
  struct {
    uint8_t signature : 4;      // 4-bit signature
    uint8_t reset_reason : 4;   // Reset reason (0-15)
    uint8_t reset_counter : 4;  // Reset counter (0-15, max 3 for detection)
    uint8_t num_pins : 4;       // Number of pins to retain (0-15)
  } fields;
  uint16_t packed;              // 2 bytes
} gpio_em4_header_t;

// Pin configuration structure (8 bits)
typedef union {
  struct {
    uint8_t port     : 3;     // Port (supports upto 8 ports)
    uint8_t pin      : 4;     // Pin number (0-15)
    uint8_t dout     : 1;     // Output value (0/1)
  } fields;
  uint8_t packed;             // 1 byte
} gpio_em4_retention_t;

// Complete configuration structure
typedef struct {
  gpio_em4_header_t header;                        // Header (2 bytes)
  gpio_em4_retention_t pins[MAX_EM4_RETAINED_PINS];// Pin configurations (16 bytes)
} gpio_em4_retention_config_t;
#pragma pack(pop)
// -----------------------------------------------------------------------------
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
 * Get base address of the bootloader upgrade image.
 *
 * @param[out] location the base address of bootloader upgrade image.
 *
 * @return Returns true if the location was found.
 ******************************************************************************/
bool bootloader_getUpgradeLocation(uint32_t *location);

#if defined(BOOTLOADER_INTERFACE_TRUSTZONE_AWARE)
/***************************************************************************//**
 * Get the list of the peripheral that is used by the bootloader.
 *
 * @param[out] ppusatd0      Word containing all the peripherals used by the
 *                           bootloader. Each bit represents a peripheral,
 *                           which is ordered after the PPUSATD0 register bit
 *                           fields.
 *
 * @param[out] ppusatd1      Word containing all the peripherals used by the
 *                           bootloader. Each bit represents a peripheral,
 *                           which is ordered after the PPUSATD1 register bit
 *                           fields.
 ******************************************************************************/
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)
void bootloader_getPeripheralList(uint32_t *ppusatd0, uint32_t *ppusatd1, uint32_t *ppusatd2);
#else
void bootloader_getPeripheralList(uint32_t *ppusatd0, uint32_t *ppusatd1);
#endif

/***************************************************************************//**
 * Save PPUSATDn state in RAM.
 * Configure the peripheral attributes before calling into the bootloader.
 *
 * @note Enters ATOMIC section.
 *
 * @param[out] ctx      Context struct to save register state into
 *
 * @return True if a valid certificate version is found.
 ******************************************************************************/
void bootloader_ppusatdnSaveReconfigureState(Bootloader_PPUSATDnCLKENnState_t *ctx);

/***************************************************************************//**
 * Restore PPUSATDn state from RAM.
 * Store the USART used by the bootloader if this is unknown.
 *
 * @note Exits ATOMIC section.
 *
 * @param[in] ctx      Context struct to restore register state from
 *
 * @return True if a valid certificate version is found.
 ******************************************************************************/
void bootloader_ppusatdnRestoreState(Bootloader_PPUSATDnCLKENnState_t *ctx);

/***************************************************************************//**
 * Called before the bootloader is initialized.
 *
 * This function implementation does not perform anything, but it is __weak
 * so that it can be implemented by another application.
 ******************************************************************************/
void sli_bootloader_preHook(void);

/***************************************************************************//**
 * Called after the bootloader is de-initialized.
 *
 * This function implementation does not perform anything, but it is __weak
 * so that it can be implemented by another application.
 ******************************************************************************/
void sli_bootloader_postHook(void);
#endif

#if !defined(_SILICON_LABS_GECKO_INTERNAL_SDID_80)
/***************************************************************************//**
 * Count the total remaining number of application upgrades.
 *
 * @return remaining number of application upgrades.
 ******************************************************************************/
uint32_t bootloader_remainingApplicationUpgrades(void);
#endif

#if defined(_SILICON_LABS_32B_SERIES_2)
/***************************************************************************//**
 * Get bootloader certificate version.
 *
 * @param[out] version           Bootloader certificate version
 *
 * @return True if a valid certificate version is found.
 ******************************************************************************/
bool bootloader_getCertificateVersion(uint32_t *version);
#endif // _SILICON_LABS_32B_SERIES_2

/***************************************************************************//**
 * Get reset cause of the bootloader.
 *
 * @return Reset cause of the bootloader.
 ******************************************************************************/
BootloaderResetCause_t bootloader_getResetReason(void);

#if defined(BOOTLOADER_HAS_FIRST_STAGE)
/***************************************************************************//**
 * Check if a pointer is valid and if it points to the bootloader first stage.
 *
 * This function checks pointers to bootloader
 * jump tables.
 *
 *
 * @param[in] ptr The pointer to check
 *
 * @return True if the pointer points to the bootloader first stage,
 *         false if not.
 ******************************************************************************/
__STATIC_INLINE bool bootloader_pointerToFirstStageValid(const void *ptr);
__STATIC_INLINE bool bootloader_pointerToFirstStageValid(const void *ptr)
{
#if defined(MAIN_BOOTLOADER_TEST)
  // In main bootloader tests, no first stage is present
  (void) ptr;
  return false;
#elif BTL_FIRST_STAGE_BASE > 0U
  if (((size_t)(ptr) >= BTL_FIRST_STAGE_BASE)
      && ((size_t)(ptr) < (BTL_FIRST_STAGE_BASE + BTL_FIRST_STAGE_SIZE))) {
    return true;
  } else {
    return false;
  }
#else
  // First stage starts at address 0, don't need to check lower bound
  if ((size_t)(ptr) < (BTL_FIRST_STAGE_BASE + BTL_FIRST_STAGE_SIZE)) {
    return true;
  } else {
    return false;
  }
#endif
}
#endif // BOOTLOADER_HAS_FIRST_STAGE

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
#if defined(MAIN_BOOTLOADER_TEST)
  // In main bootloader tests, all of memory is considered part of the bootloader
  (void) ptr;
  return true;
#elif BTL_MAIN_STAGE_BASE > 0U
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
 * @brief Checks if a memory region is within the valid flash address range.
 *
 * This function verifies that the specified address and length are within the
 * bounds of the device's flash memory region, defined by FLASH_BASE and FLASH_SIZE.
 *
 * @param address The starting address of the memory region to check.
 * @param length  The length (in bytes) of the memory region to check.
 * @return true if the region is fully within the flash memory range, false otherwise.
 ******************************************************************************/
__STATIC_INLINE bool verifyAddressRange(uint32_t address,
                                        uint32_t length);
__STATIC_INLINE bool verifyAddressRange(uint32_t address,
                                        uint32_t length)
{
  // Flash starts at FLASH_BASE, and is FLASH_SIZE large
  if ((length > FLASH_SIZE)
#if (FLASH_BASE > 0x0UL)
      || (address < FLASH_BASE)
#endif
      || (address > FLASH_BASE + FLASH_SIZE)) {
    return false;
  }

  return ((address + length) <= FLASH_BASE + FLASH_SIZE);
}

/***************************************************************************//**
 * @brief Checks if a memory region is erased (all bits set to 1).
 *
 * This function iterates over the specified memory region and checks if every
 * 32-bit word is equal to 0xFFFFFFFF, which indicates an erased state in flash memory.
 *
 * @param address The starting address of the memory region to check.
 * @param length  The length (in bytes) of the memory region to check. Must be a multiple of 4.
 * @return true if all 32-bit words in the region are erased (0xFFFFFFFF), false otherwise.
 ******************************************************************************/
__STATIC_INLINE bool verifyErased(uint32_t address,
                                  uint32_t length);
__STATIC_INLINE bool verifyErased(uint32_t address,
                                  uint32_t length)
{
  for (uint32_t i = 0; i < length; i += 4) {
    if (*(uint32_t *)(address + i) != 0xFFFFFFFF) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Reboots the system with GPIO retention and enters EM4 sleep mode
 *
 * This function implements a complete EM4 GPIO retention workflow by:
 * 1. Validating the provided GPIO configuration parameters
 * 2. Persisting GPIO configurations to BURAM for retention across EM4 cycles
 * 3. Entering EM4 sleep mode with configurable wakeup behavior
 *
 * The wakeup mechanism is controlled by the auto_configure_wakeup parameter:
 * - When true: BURTC is configured for scheduled wakeup before entering EM4
 * - When false: EM4 is entered directly, allowing the application to handle wakeup configuration
 *
 * @param[in] pins                  Pointer to array of GPIO pin configurations to retain
 * @param[in] num_pins              Number of pins to retain (range: 1 to MAX_EM4_RETAINED_PINS)
 * @param[in] auto_configure_wakeup Wakeup configuration flag (true: BURTC wakeup, false: direct EM4 entry)
 *
 * @return On success, this function transitions the system to EM4 sleep mode and does not return
 *         - BOOTLOADER_ERROR_GPIO_RETENTION_INVALID: Invalid parameters or EM4 GPIO retention not available
 *         - Other error codes from btl_em4GpioRetentionConfigSave() on GPIO retention save failure
 */
int32_t bootloader_rebootAndInstallWithGpioRetention(const gpio_em4_retention_t* pins, uint8_t num_pins, bool auto_configure_wakeup);

#if defined(BURTC_PRESENT)
/***************************************************************************//**
 * @brief Configure BURTC and enter EM4 sleep mode.
 *
 * This function configures BURTC for EM4 wakeup and enters EM4 sleep mode.
 * Used when EM4 GPIO retention component is installed.
 *
 * @note This function does not return as it puts the system into EM4 sleep mode.
 ******************************************************************************/
__STATIC_INLINE void configureBurtcAndEnterEM4(void);
__STATIC_INLINE void configureBurtcAndEnterEM4(void)
{
  // Configure BURTC for EM4 wakeup
  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
  burtcInit.start = true;
  burtcInit.debugRun = false;
  burtcInit.clkDiv = burtcClkDiv_1;
  burtcInit.compare0Top = true;
  burtcInit.em4comp = true;
  burtcInit.em4overflow = false;

  // Enable BURTC clock
  CMU_ClockEnable(cmuClock_BURTC, true);

  // Initialize BURTC
  BURTC_Init(&burtcInit);

  // Set compare value for wakeup (configurable via BURTC_WAKEUP_CYCLES)
  BURTC_CompareSet(0, BURTC_WAKEUP_CYCLES);

  // Clear and enable interrupts
  BURTC_IntClear(BURTC_IF_COMP | BURTC_IF_OF);
  BURTC_IntEnable(BURTC_IEN_COMP | BURTC_IEN_OF);
  NVIC_EnableIRQ(BURTC_IRQn);

  // Enter EM4
  EMU_EnterEM4();
}
#endif // BURTC_PRESENT

/** @} (end addtogroup CommonInterface) */
/** @} (end addtogroup Interface) */
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif // BTL_INTERFACE_H
