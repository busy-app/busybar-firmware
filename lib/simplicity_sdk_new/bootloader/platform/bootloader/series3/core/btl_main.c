/***************************************************************************//**
 * @file
 * @brief Main file for Main Bootloader.
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

#include "config/btl_config.h"
#include "api/btl_interface.h"

#include "core/btl_core.h"
#include "core/btl_helper.h"
#include "core/btl_reset.h"
#include "core/btl_parse.h"
#include "core/btl_bootload.h"

#include "debug/btl_debug.h"

#ifdef BTL_GPIO_ACTIVATION
#include "gpio/gpio-activation/btl_gpio_activation.h"
#endif

#ifdef BTL_EZSP_GPIO_ACTIVATION
#include "gpio/ezsp-gpio-activation/btl_ezsp_gpio_activation.h"
#endif

#ifdef BOOTLOADER_SUPPORT_STORAGE
#include "storage/btl_storage.h"
#include "storage/bootloadinfo/btl_storage_bootloadinfo.h"
#include "btl_storage_cfg.h"
#endif

#ifdef BOOTLOADER_SUPPORT_COMMUNICATION
#include "communication/btl_communication.h"
#endif

#include "em_device.h"
#include "sl_hal_gpio.h"

#if defined(__GNUC__)
#define ROM_END_SIZE 0
extern const size_t __rom_end__;
#elif defined(__ICCARM__)
#define ROM_END_SIZE 4
const size_t __rom_end__ @ "ROM_SIZE";
#endif

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------

// Vector table relocation for RAM_ALIAS execution
#define TOTAL_INTERRUPTS    (16 + EXT_IRQ_COUNT)

// BootloaderResetCause_t occupies the first 4 bytes (0-3).
#define BTL_RESET_REGION_PC_READ_FLAG_OFFSET  2  // Offset to 2nd uint16_t element (bytes 4-5)
#define BTL_RESET_REGION_QSPI_ERROR_FLAG_OFFSET  3  // Offset to 3rd uint16_t element (bytes 6-7)
#define BTL_RESET_REGION_PC_READ_MAGIC        0xDEAD
#define BTL_RESET_REGION_QSPI_ERROR_MAGIC     0xBEEF

// -----------------------------------------------------------------------------
// External Symbols
// -----------------------------------------------------------------------------

// External linker symbol for bootloader reset region
extern uint32_t __ResetReasonStart__[];

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

// Place RAM vector table in dedicated .ram_vector_table section (regular RAM)
tVectorEntry ramVectorTable[TOTAL_INTERRUPTS]
  __attribute__((section(".ram_vector_table"), aligned(512)));

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------

// Local function declarations
bool enter_bootloader(void);
bool check_qspi_authentication_error(void);
bool check_software_reset_request(void);
bool check_gpio_activation_methods(void);
SL_NORETURN static void boot_to_app(uint32_t);

// Forward declarations for RAM-resident fault handlers
extern void hardfault_ram_handler(void);
extern void busfault_ram_handler(void);
extern void default_ram_handler(void);
/**
 * @brief Enable BusFault exception handling in SCB
 * 
 * @details Enables BusFault exception handler to prevent bus faults
 * from escalating to HardFaults. This allows RAM-resident BusFault_Handler
 * to catch errors, set reset markers, and reboot cleanly for recovery.
 * 
 * @note Must be called early in bootloader initialization.
 */
__STATIC_INLINE void enable_busfault_handler(void)
{
  SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
}

/**
 * @brief Check if application write is complete
 *
 * @details
 *   Reads the entire 64-byte vector table chunk and checks for complete writes
 *   by looking for uninitialized flash (0xFF values). Returns true if no address
 *   contains 0xFF, indicating the app write was complete and successful.
 *
 * @param startOfAppSpace Start address of application space
 * @return true if app write is complete, false if partial/incomplete write detected
 */
__STATIC_INLINE bool is_app_write_complete(uint32_t startOfAppSpace)
{
  // Read entire 64-byte chunk for comprehensive validation
  volatile uint32_t *chunk = (volatile uint32_t *)startOfAppSpace;
  
  // Check if any address is 0xFF (uninitialized flash)
  for (int i = 0; i < 16; i++) {
    if (chunk[i] == 0xFFFFFFFF) {
      return false; // Found uninitialized flash - write incomplete
    }
  }
  
  return true; // No 0xFF found, write complete
}

/**
 * @brief Safely update VTOR register with error handling and rollback
 * 
 * @param vector_table Pointer to the new vector table
 */
__STATIC_INLINE void update_vtor_safely(tVectorEntry *vector_table)
{
    if (!vector_table) {
        return;
    }
       
    // Update VTOR with proper memory barriers
    __asm volatile ("dmb" ::: "memory");
    SCB->VTOR = (uint32_t) vector_table;
    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}

/**
 * @brief Initialize RAM vector table with custom fault handlers
 * 
 * This function initializes the RAM-based vector table with a mix of Flash-based
 * handlers (for Reset_Handler and stack pointer) and RAM_ALIAS-based handlers
 * (for critical fault handlers). This ensures fault recovery even when Flash
 * is corrupted or inaccessible.
 */
__STATIC_INLINE void initialize_ram_vector_table(void)
{
    // External reference to Flash vector table
    extern const tVectorEntry __VECTOR_TABLE[TOTAL_INTERRUPTS];
    
    // Initialize all entries to Default_Handler first
    for (uint32_t i = 0; i < TOTAL_INTERRUPTS; ++i) {
        ramVectorTable[i].VECTOR_TABLE_Type = default_ram_handler;
    }
    
    // Apply specific handler configuration
    // Initial Stack Pointer (index 0) - keep from Flash
    ramVectorTable[0].topOfStack = __VECTOR_TABLE[0].topOfStack;
    
    // Reset_Handler (index 1) - keep from Flash
    ramVectorTable[1].VECTOR_TABLE_Type = __VECTOR_TABLE[1].VECTOR_TABLE_Type;

    // Critical fault handlers - RAM_ALIAS versions for fault recovery
    ramVectorTable[3].VECTOR_TABLE_Type = hardfault_ram_handler;   // HardFault_Handler
    ramVectorTable[5].VECTOR_TABLE_Type = busfault_ram_handler;    // BusFault_Handler
       
    // Update VTOR with proper memory barriers and error handling
    update_vtor_safely(ramVectorTable);
}


/**
 * @brief HardFault exception handler (RAM-resident)
 * 
 * @details Handles HardFault exceptions by writing reset reason to SRAM and triggering
 * system reset. This allows the bootloader to detect the fault on next boot.
 * 
 * @note Placed in RAM to ensure execution when Flash is inaccessible.
 */
__attribute__((section("text_bootloader_critical_ram")))
__attribute__((used))
void hardfault_ram_handler(void)
{
  volatile uint16_t *resetReasonBase = (volatile uint16_t *)&__ResetReasonStart__;
  resetReasonBase[0] = BOOTLOADER_RESET_REASON_FATAL;     // reason
  resetReasonBase[1] = BOOTLOADER_RESET_SIGNATURE_VALID;  // signature

  uint32_t aircr = SCB->AIRCR;
  aircr = (0x5FAUL << SCB_AIRCR_VECTKEY_Pos)
          | (aircr & SCB_AIRCR_PRIGROUP_Msk)
          | SCB_AIRCR_SYSRESETREQ_Msk;
  __asm volatile ("dsb" ::: "memory");
  SCB->AIRCR = aircr;
  __asm volatile ("dsb" ::: "memory");

  // Wait for reset
  for (;;)
  {
    __NOP();
  }
}

/**
 * @brief BusFault exception handler (RAM-resident)
 * 
 * @details Handles BusFault exceptions by writing reset reason to SRAM and triggering
 * system reset. Primarily used for QSPI authentication error recovery in Series3
 * devices with AXiP regions.
 * 
 * @note Placed in RAM to ensure execution when Flash is inaccessible.
 */
__attribute__((section("text_bootloader_critical_ram")))
__attribute__((used))
void busfault_ram_handler(void)
{

  volatile uint16_t *extendedResetRegion = (volatile uint16_t *)&__ResetReasonStart__;

  // If PC read flag is set, this bus fault occurred during application read
  if (extendedResetRegion[BTL_RESET_REGION_PC_READ_FLAG_OFFSET] == BTL_RESET_REGION_PC_READ_MAGIC) {
    // Set QSPI error flag to indicate bus fault occurred during read
    extendedResetRegion[BTL_RESET_REGION_QSPI_ERROR_FLAG_OFFSET] = BTL_RESET_REGION_QSPI_ERROR_MAGIC;
    // Clear PC read flag
    extendedResetRegion[BTL_RESET_REGION_PC_READ_FLAG_OFFSET] = 0;

    uint32_t aircr = SCB->AIRCR;
    aircr = (0x5FAUL << SCB_AIRCR_VECTKEY_Pos)
            | (aircr & SCB_AIRCR_PRIGROUP_Msk)
            | SCB_AIRCR_SYSRESETREQ_Msk;
    __asm volatile ("dsb" ::: "memory");
    SCB->AIRCR = aircr;
    __asm volatile ("dsb" ::: "memory");

    // Wait for reset
    for (;;)
    {
      __NOP();
    }
  }
}

/**
 * @brief Default interrupt handler (RAM-resident)
 * 
 * @details Handles all unhandled interrupts and exceptions. Placed in RAM to ensure
 * execution when Flash is inaccessible.
 * 
 */
__attribute__((section("text_bootloader_critical_ram")))
__attribute__((used))
void default_ram_handler(void)
{
  // wait until reset
  for (;;)
  {
    __NOP();
  }   
}

// -----------------------------------------------------------------------------
// Main Bootloader Implementation
// -----------------------------------------------------------------------------

int main(void)
{
  int32_t ret = BOOTLOADER_ERROR_STORAGE_BOOTLOAD;

  BTL_DEBUG_PRINTLN("BTL entry");

#if defined(EMU_CMD_EM01VSCALE2) && defined(EMU_STATUS_VSCALEBUSY)
  // Device supports voltage scaling, and the bootloader may have been entered
  // with a downscaled voltage. Scale voltage up to allow flash programming.
  if ((EMU->STATUS & EMU_STATUS_VSCALE_VSCALE2) != EMU_STATUS_VSCALE_VSCALE2) {
    EMU->CMD = EMU_CMD_EM01VSCALE2;
    while (EMU->STATUS & EMU_STATUS_VSCALEBUSY) {
      // Do nothing
    }
  }
#endif

  btl_init();

#ifdef BOOTLOADER_SUPPORT_STORAGE
  if (!reset_resetCounterEnabled()) {
    // Storage bootloaders might use part of the reason signature as a counter,
    // so only invalidate the signature when the counter is not in use.
    reset_invalidateResetReason();
  }
#else
  reset_invalidateResetReason();
#endif

#ifdef BOOTLOADER_SUPPORT_STORAGE
  // If the bootloader supports storage, first attempt to apply an existing
  // image from storage.

  ret = storage_main();

  if (ret == BOOTLOADER_OK) {
    // Firmware upgrade from storage successful. Disable the reset counter
    // and return to application
    if (reset_resetCounterEnabled()) {
      reset_disableResetCounter();
    }
    reset_resetWithReason(BOOTLOADER_RESET_REASON_GO);
  } else {
    if (!reset_resetCounterEnabled()) {
      // Start counting the number of consecutive resets after the first reset
      reset_enableResetCounter();
    }

    // Stop after three consecutive resets (the first one counts as 0)
    if (reset_getResetCounter() >= 2) {
      // If the system is not able to recover from a fault like BADAPP or
      // BADIMAGE, wait in a busy loop to ease reflashing and debugging.
      BTL_DEBUG_PRINTLN("Reset loop detected. Stopping...");
      reset_invalidateResetReason();
      while (1) {
        // Wait...
      }
    } else {
      reset_incrementResetCounter();
    }

    // Wait a short while (approx. 500 ms) before continuing.
    // This allows other operations to complete before the reset.
    for (volatile int i = 800000; i > 0; i--) {
      // Do nothing
    }
  }
#endif

#ifdef BOOTLOADER_SUPPORT_COMMUNICATION
  communication_init();

  ret = communication_start();
  if (ret != BOOTLOADER_OK) {
    reset_resetWithReason(BOOTLOADER_RESET_REASON_FATAL);
  }

  ret = communication_main();
  BTL_DEBUG_PRINT("Protocol returned ");
  BTL_DEBUG_PRINT_WORD_HEX(ret);
  BTL_DEBUG_PRINT_LF();

  communication_shutdown();

  if ((ret == BOOTLOADER_OK)
      || (ret == BOOTLOADER_ERROR_COMMUNICATION_DONE)) {
    reset_resetWithReason(BOOTLOADER_RESET_REASON_GO);
  }
#endif // BOOTLOADER_SUPPORT_COMMUNICATION

  // An error occurred in storage or communication, and a firmware upgrade
  // was not performed
  if (0
#ifdef BOOTLOADER_SUPPORT_COMMUNICATION
      || (ret == BOOTLOADER_ERROR_COMMUNICATION_IMAGE_ERROR)
      || (ret == BOOTLOADER_ERROR_COMMUNICATION_TIMEOUT)
#endif
#ifdef BOOTLOADER_SUPPORT_STORAGE
      || (ret == BOOTLOADER_ERROR_STORAGE_BOOTLOAD)
#endif
      ) {
    reset_resetWithReason(BOOTLOADER_RESET_REASON_BADIMAGE);
  } else {
    reset_resetWithReason(BOOTLOADER_RESET_REASON_FATAL);
  }

  return 0;
}

// -----------------------------------------------------------------------------
// Data Structures and Tables
// -----------------------------------------------------------------------------

#ifdef BOOTLOADER_SUPPORT_STORAGE
extern const BootloaderStorageFunctions_t storageFunctions;
#endif

const MainBootloaderTable_t mainStageTable = {
  {
    .type = BOOTLOADER_MAGIC_MAIN,
    .layout = BOOTLOADER_HEADER_VERSION_MAIN,
    .version = BOOTLOADER_VERSION_MAIN
  },
  // Bootloader size is the relative address of the end variable plus 4 for the
  // CRC
  .size = ((uint32_t)&__rom_end__) - BTL_MAIN_STAGE_BASE + ROM_END_SIZE + 4,
  .startOfAppSpace = (BareBootTable_t *)(BTL_APPLICATION_BASE),
  .endOfAppSpace = 0,
  .capabilities = (0
#if defined(BOOTLOADER_ENFORCE_SIGNED_UPGRADE) && (BOOTLOADER_ENFORCE_SIGNED_UPGRADE == 1)
                   | BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_SIGNATURE
#endif
#if defined(BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE) && (BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE == 1)
                   | BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_ENCRYPTION
#endif
#if defined(BOOTLOADER_ENFORCE_SECURE_BOOT) && (BOOTLOADER_ENFORCE_SECURE_BOOT == 1)
                   | BOOTLOADER_CAPABILITY_ENFORCE_SECURE_BOOT
#endif
#if defined(BOOTLOADER_SUPPORT_CERTIFICATES) && (BOOTLOADER_SUPPORT_CERTIFICATES == 1)
                   | BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT
#endif
#if defined(BOOTLOADER_ROLLBACK_PROTECTION) && (BOOTLOADER_ROLLBACK_PROTECTION == 1)
                   | BOOTLOADER_CAPABILITY_ROLLBACK_PROTECTION
#endif
                   | BOOTLOADER_CAPABILITY_BOOTLOADER_UPGRADE
                   | BOOTLOADER_CAPABILITY_GBL
                   | BOOTLOADER_CAPABILITY_GBL_SIGNATURE
#if !defined(BTL_PARSER_NO_SUPPORT_ENCRYPTION)
                   | BOOTLOADER_CAPABILITY_GBL_ENCRYPTION
#endif
#if defined(BOOTLOADER_SUPPORT_STORAGE) && (BOOTLOADER_SUPPORT_STORAGE == 1)
                   | BOOTLOADER_CAPABILITY_STORAGE
#endif
#if defined(BOOTLOADER_SUPPORT_COMMUNICATION) && (BOOTLOADER_SUPPORT_COMMUNICATION == 1)
                   | BOOTLOADER_CAPABILITY_COMMUNICATION
#endif
#if defined(BOOTLOADER_REJECT_DIRECT_SIGNED_IMG) && (BOOTLOADER_REJECT_DIRECT_SIGNED_IMG == 1)
                   | BOOTLOADER_CAPABILITY_REJECT_DIRECT_SIGNED_IMG
#endif
                   ),
  .init = &btl_init,
  .deinit = &btl_deinit,
  .verifyApplication = &bootload_verifyApplication,
  .initParser = &core_init_parser,
  .parseBuffer = &core_parse_buffer,
#if defined(BOOTLOADER_SUPPORT_STORAGE) && (BOOTLOADER_SUPPORT_STORAGE == 1)
  .storage = &storageFunctions,
#else
  .storage = NULL,
#endif
  .parseImageInfo = core_parse_image_info,
  .parserContextSize = core_parser_context_size,
  .remainingApplicationUpgrades = NULL,
  .getPeripheralList = NULL,
  .getUpgradeLocation = NULL
};

#if defined(BOOTLOADER_SUPPORT_CERTIFICATES) && (BOOTLOADER_SUPPORT_CERTIFICATES == 1)
const ApplicationCertificate_t sl_app_certificate = {
  .structVersion = APPLICATION_CERTIFICATE_VERSION,
  .flags = { 0U },
  .key = { 0U },
  .version = 0,
  .signature = { 0U },
};
#endif

const ApplicationProperties_t sl_app_properties = {
  .magic = APPLICATION_PROPERTIES_MAGIC,
  .structVersion = APPLICATION_PROPERTIES_VERSION,
  .signatureType = APPLICATION_SIGNATURE_NONE,
  .signatureLocation = ((uint32_t)&__rom_end__) - BTL_MAIN_STAGE_BASE + ROM_END_SIZE,
  .app = {
    .type = APPLICATION_TYPE_BOOTLOADER,
    .version = BOOTLOADER_VERSION_MAIN,
    .capabilities = 0UL,
    .productId = { 0U },
  },
#if defined(BOOTLOADER_SUPPORT_CERTIFICATES) && (BOOTLOADER_SUPPORT_CERTIFICATES == 1)
  // If certificate based boot chain is enabled, the bootloader binary will be provided with
  // a certificate that does not contain any key.
  // A valid certificate needs to be injected to the bootloader images using Simplicity Commander.
  // Simplicity Commander will replace this certificate.
  .cert = (ApplicationCertificate_t *)&sl_app_certificate,
#else
  .cert = NULL,
#endif
  .longTokenSectionAddress = NULL,
  .decryptKey = { 0u },
};

/**
 * This function gets executed before ANYTHING got initialized.
 * So, no using global variables here!
 */
void SystemInit2(void)
{
  // Initialize debug before first debug print
  BTL_DEBUG_INIT();

  // Initialize RAM vector table with custom fault handlers FIRST to ensure
  // fault handlers are accessible even when Flash has QSPI authentication errors
  initialize_ram_vector_table();

  // Enable BusFault handler EARLY to catch QSPI authentication errors
  enable_busfault_handler();

  // Assumption: We should enter the app
  volatile bool enterApp = true;
  // Assumption: The app should be verified
  volatile bool verifyApp = true;

  if (enter_bootloader()) {
    // We want to enter the bootloader, app doesn't need to be verified
    enterApp = false;
    verifyApp = false;
  }

#if defined(LOCKBIT_SKIP_BOOT_CHECK)
  bool skipLockBitCheck = true;
#else
  bool skipLockBitCheck = false;
#endif

  sl_se_code_region_config_t region_config = { 0 };
  sl_se_command_context_t cmd_ctx = { 0 };
  sl_se_init_command_context(&cmd_ctx);
  sl_se_code_region_get_config(&cmd_ctx, &region_config, 1, 1);

  uint32_t startOfAppSpace = (uint32_t)mainStageTable.startOfAppSpace;

  // Only perform vector table validation if we're planning to enter the app
  // If enter_bootloader() returned true, skip this to go into firmware upgrade mode
  if (enterApp && ((region_config.locked == true) || skipLockBitCheck)) {
    volatile uint16_t *extendedResetRegion = (volatile uint16_t *)&__ResetReasonStart__;
    extendedResetRegion[BTL_RESET_REGION_PC_READ_FLAG_OFFSET] = BTL_RESET_REGION_PC_READ_MAGIC;
    
    // Check application vector table header (may trigger QSPI auth error in AXiP region)
    
    if (!is_app_write_complete(startOfAppSpace)) {
      // Sanity check failed; enter the bootloader
      reset_setResetReason(BOOTLOADER_RESET_REASON_BADAPP);
      enterApp = false;
      verifyApp = false;
    }
    // App should be verified
    if (verifyApp) {
      // If app verification fails, enter bootloader instead
      enterApp = bootload_verifyApplication(startOfAppSpace);
      if (!enterApp) {
        extendedResetRegion[BTL_RESET_REGION_PC_READ_FLAG_OFFSET] = 0;
        BTL_DEBUG_PRINTLN("App verify fail");
        reset_setResetReason(BOOTLOADER_RESET_REASON_BADAPP);
      }
    }
    
    // Clear the PC read flag after application validation operations complete
    extendedResetRegion[BTL_RESET_REGION_PC_READ_FLAG_OFFSET] = 0;
  } else {
    enterApp = false;
  }

  if (enterApp) {
    BTL_DEBUG_PRINTLN("Enter app");
    BTL_DEBUG_PRINT_LF();

#if defined(BOOTLOADER_SUPPORT_STORAGE)
    // Disable the reset counter if we're booting (back) into the application
    if (reset_resetCounterEnabled()) {
      reset_disableResetCounter();
    }
#endif

    // Set vector table to application's table
    SCB->VTOR = startOfAppSpace;

    boot_to_app(startOfAppSpace);
  }
  // Enter bootloader
}

/**
 * @brief Jump to application
 * 
 * @param startOfAppSpace Start address of application space
 */
#if defined(__clang__)
// Clang does not support naked functions which aren't fully inline asm.
// Todo: rewrite function in assembly.
__attribute__ ((noreturn)) static void bootToApp(uint32_t startOfAppSpace)
#else
__attribute__ ((noreturn, naked)) static void boot_to_app(uint32_t startOfAppSpace)
#endif
{
  jump_to_application_routine(startOfAppSpace);
  while (1) {
    // Do nothing
  }
}

// -----------------------------------------------------------------------------
// Helper Functions
// -----------------------------------------------------------------------------

/**
 * @brief Check whether we should enter the bootloader
 * 
 * @details Evaluates various conditions to determine if the bootloader should be entered.
 * Checks for QSPI authentication errors, software reset requests, and GPIO activation
 * methods in priority order to ensure proper recovery and upgrade operations.
 * 
 * @return true if the bootloader should be entered, false otherwise
 */
bool enter_bootloader(void)
{
  // Check for QSPI authentication errors that occurred during previous boot
  // This is the highest priority as it indicates critical system state requiring recovery
  if (check_qspi_authentication_error()) {
    return true;
  }

  // Check if system was reset due to software request
  if (check_software_reset_request()) {
    return true;
  }

  // Check GPIO-based bootloader entry methods
  if (check_gpio_activation_methods()) {
    return true;
  }

  // No bootloader entry conditions met
  return false;
}

/**
 * @brief Check for QSPI authentication error recovery condition
 * 
 * @details Checks if a QSPI authentication error occurred during the previous boot
 * by examining the PC read flag in the extended reset region. If the flag is set,
 * it indicates that a bus fault occurred while reading the application's vector
 * table header, likely due to QSPI authentication failure.
 * 
 * @return true if QSPI authentication error detected, false otherwise
 */
bool check_qspi_authentication_error(void)
{
  volatile uint16_t *extendedResetRegion = (volatile uint16_t *)&__ResetReasonStart__;

  // Check if QSPI error flag indicates bus fault occurred during application read
  if (extendedResetRegion[BTL_RESET_REGION_QSPI_ERROR_FLAG_OFFSET] == BTL_RESET_REGION_QSPI_ERROR_MAGIC) {
    // Clear QSPI error flag to prevent repeated bootloader entry
    extendedResetRegion[BTL_RESET_REGION_QSPI_ERROR_FLAG_OFFSET] = 0;
    return true;  // QSPI authentication error - enter bootloader for recovery
  }
  
  return false;
}

/**
 * @brief Check for software reset request conditions
 * 
 * @details Examines the reset cause register to determine if the system was reset
 * due to a software request. Checks for specific bootloader reset reasons that
 * indicate the bootloader should be entered for recovery or upgrade operations.
 * 
 * @return true if software reset request detected, false otherwise
 */
bool check_software_reset_request(void)
{
// *INDENT-OFF*
#if defined(EMU_RSTCAUSE_SYSREQ)
  if (EMU->RSTCAUSE & EMU_RSTCAUSE_SYSREQ) {
#else
  if (RMU->RSTCAUSE & RMU_RSTCAUSE_SYSREQRST) {
#endif
    // Check specific reset reasons that require bootloader entry
    switch (reset_classifyReset()) {
      case BOOTLOADER_RESET_REASON_BOOTLOAD:
      case BOOTLOADER_RESET_REASON_FORCE:
      case BOOTLOADER_RESET_REASON_UPGRADE:
      case BOOTLOADER_RESET_REASON_BADAPP:
        return true;  // Software requested bootloader entry
      default:
        break;
    }
  }
// *INDENT-ON*
  
  return false;
}

/**
 * @brief Check GPIO-based bootloader activation methods
 * 
 * @details Checks various GPIO-based methods for bootloader activation including
 * standard GPIO pins and EZSP GPIO pins. These methods allow external hardware
 * to signal that the bootloader should be entered.
 * 
 * @return true if GPIO activation detected, false otherwise
 */
bool check_gpio_activation_methods(void)
{
#ifdef BTL_GPIO_ACTIVATION
  if (gpio_enterBootloader()) {
    return true;  // GPIO pin state signals bootloader entry
  }
#endif

#ifdef BTL_EZSP_GPIO_ACTIVATION
  if (ezsp_gpio_enterBootloader()) {
    return true;  // EZSP GPIO pin state signals bootloader entry
  }
#endif

  return false;
}
