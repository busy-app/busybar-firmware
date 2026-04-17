/***************************************************************************//**
 * @file
 * @brief Application interface to the bootloader.
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

#include "btl_interface.h"
#include "sl_se_manager.h"
#include "core/btl_parse.h"
#include "core/btl_bootload.h"
// -----------------------------------------------------------------------------
// Static variables

// -----------------------------------------------------------------------------
// Enums

typedef enum {
  RESET, INITIALIZED, DEINITIALIZED
} initState_t;

static initState_t bootloader_InitState = RESET;

// -----------------------------------------------------------------------------
// Functions

void bootloader_getInfo(BootloaderInformation_t *info)
{
  if (!bootloader_pointerValid(mainBootloaderTable)) {
    // No bootloader is present (first stage or main stage invalid)
    info->type = NO_BOOTLOADER;
    info->capabilities = 0;
  } else if (mainBootloaderTable->header.type == BOOTLOADER_MAGIC_MAIN) {
    info->type = SL_BOOTLOADER;
    info->version = mainBootloaderTable->header.version;
    info->capabilities = mainBootloaderTable->capabilities;
  } else {
    info->type = NO_BOOTLOADER;
    info->capabilities = 0;
  }
}

int32_t bootloader_init(void)
{
  int32_t retVal;
  sl_status_t status;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal;
  if (bootloader_InitState == RESET || bootloader_InitState == DEINITIALIZED) {
    retVal = btl_osal_init_mutex();
    if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
      return retVal;
    }
  }
#endif

  if (!bootloader_pointerValid(mainBootloaderTable)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
  if (mainBootloaderTable->header.type != BOOTLOADER_MAGIC_MAIN) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  if (bootloader_InitState == RESET || bootloader_InitState == DEINITIALIZED) {
    retVal = mainBootloaderTable->init();
    if (retVal == BOOTLOADER_OK) {
      status = sl_se_init();
      if (status != BOOTLOADER_OK) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
        releaseRetVal = btl_osal_lock_release();
        if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
          return releaseRetVal;
        }
#endif
        return BOOTLOADER_ERROR_SECURITY_INIT_FAILED;
      }
    }
  } else {
    retVal = BOOTLOADER_OK;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  bootloader_InitState = INITIALIZED;
  return retVal;
}

int32_t bootloader_deinit(void)
{
  int32_t retVal;
  sl_status_t status;
  if (!bootloader_pointerValid(mainBootloaderTable)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }
  if (mainBootloaderTable->header.type != BOOTLOADER_MAGIC_MAIN) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal;
  // Acquire lock
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  if (bootloader_InitState == INITIALIZED) {
    retVal = mainBootloaderTable->deinit();
    if (retVal == BOOTLOADER_OK) {
      status = sl_se_deinit();
      if (status != BOOTLOADER_OK) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
        releaseRetVal = btl_osal_lock_release();
        if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
          return releaseRetVal;
        }
        releaseRetVal = btl_osal_free_mutex();
        if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
          return releaseRetVal;
        }
#endif
        return BOOTLOADER_ERROR_SECURITY_DE_INIT_FAILED;
      }
    }
  } else {
    retVal = BOOTLOADER_OK;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
  releaseRetVal = btl_osal_free_mutex();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  bootloader_InitState = DEINITIALIZED;
  return retVal;
}

BootloaderResetCause_t bootloader_getResetReason(void)
{
  volatile BootloaderResetCause_t* resetCause = (BootloaderResetCause_t*) (SRAM_BASE);
  return *resetCause;
}

void bootloader_rebootAndInstall(void)
{
  // Set reset reason to bootloader entry
  BootloaderResetCause_t* resetCause = (BootloaderResetCause_t*) (SRAM_BASE);
  resetCause->reason = BOOTLOADER_RESET_REASON_BOOTLOAD;
  resetCause->signature = BOOTLOADER_RESET_SIGNATURE_VALID;
#if defined(RMU_PRESENT)
  // Clear resetcause
  RMU->CMD = RMU_CMD_RCCLR;
  // Trigger a software system reset
  RMU->CTRL = (RMU->CTRL & ~_RMU_CTRL_SYSRMODE_MASK) | RMU_CTRL_SYSRMODE_FULL;
#endif

  // Disable LFRCO.
  CMU->CLKEN0_SET = CMU_CLKEN0_LFRCO;
  LFRCO->CTRL_SET = LFRCO_CTRL_DISONDEMAND;
  LFRCO->CTRL_CLR = LFRCO_CTRL_FORCEEN;
  while ((LFRCO->STATUS & LFRCO_STATUS_ENS) != 0U) {
  }

  // Disable High Precision Mode to prevent hangs happening
  // in the SE firmware when sending commands to the QSPI.
  LFRCO->CFG_CLR = LFRCO_CFG_HIGHPRECEN;

  // Clear disable on-demand.
  LFRCO->CTRL_CLR = LFRCO_CTRL_DISONDEMAND;
  CMU->CLKEN0_CLR = CMU_CLKEN0_LFRCO;

  NVIC_SystemReset();
}

int32_t bootloader_initParser(BootloaderParserContext_t *context,
                              size_t                    contextSize)
{
  int32_t retVal;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  retVal =  core_init_parser(context, contextSize);

#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_parseBuffer(BootloaderParserContext_t   *context,
                               BootloaderParserCallbacks_t *callbacks,
                               uint8_t                     data[],
                               size_t                      numBytes)
{
  int32_t retVal;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  retVal = core_parse_buffer(context, callbacks, data, numBytes);

#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

int32_t bootloader_parseImageInfo(BootloaderParserContext_t *context,
                                  uint8_t                   data[],
                                  size_t                    numBytes,
                                  ApplicationData_t         *appInfo,
                                  uint32_t                  *bootloaderVersion)
{
  int32_t retVal;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  retVal = core_parse_image_info(context, data, numBytes, appInfo, bootloaderVersion);

#if defined(SL_CATALOG_KERNEL_PRESENT)
  int32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif
  return retVal;
}

uint32_t bootloader_parserContextSize(void)
{
  uint32_t retVal;
#if defined(SL_CATALOG_KERNEL_PRESENT)
  retVal = btl_osal_lock_aquire();
  if (retVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return retVal;
  }
#endif

  retVal = core_parser_context_size();

#if defined(SL_CATALOG_KERNEL_PRESENT)
  uint32_t releaseRetVal = btl_osal_lock_release();
  if (releaseRetVal != BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS) {
    return releaseRetVal;
  }
#endif

  return retVal;
}

bool bootloader_verifyApplication(uint32_t startAddress)
{
  if (!bootloader_pointerValid(mainBootloaderTable)) {
    return false;
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS != btl_osal_lock_aquire()) {
    return false;
  }
#endif

  bool retVal = bootload_verifyApplication(startAddress);

#if defined(SL_CATALOG_KERNEL_PRESENT)
  if (BOOTLOADER_ERROR_RTOS_MUTEX_SUCCESS != btl_osal_lock_release()) {
    return false;
  }
#endif

  return retVal;
}

bool bootloader_secureBootEnforced(void)
{
  BootloaderInformation_t info = { .type = SL_BOOTLOADER, .version = 0U, .capabilities = 0U };
  bootloader_getInfo(&info);

  if (info.capabilities & BOOTLOADER_CAPABILITY_ENFORCE_SECURE_BOOT) {
    return true;
  }
  return false;
}

bool bootloader_getCertificateVersion(uint32_t *version)
{
  // Access word 13 to read sl_app_properties of the bootloader.
  ApplicationProperties_t *blProperties =
    (ApplicationProperties_t *)(*(uint32_t *)(BTL_MAIN_STAGE_BASE + 52UL));

  if (!bootloader_pointerValid(blProperties)) {
    return false;
  }

  // Compatibility check of the application properties struct.
  if (((blProperties->structVersion & APPLICATION_PROPERTIES_VERSION_MAJOR_MASK)
       >> APPLICATION_PROPERTIES_VERSION_MAJOR_SHIFT) < 1UL) {
    return false;
  }
  if (((blProperties->structVersion & APPLICATION_PROPERTIES_VERSION_MINOR_MASK)
       >> APPLICATION_PROPERTIES_VERSION_MINOR_SHIFT) < 1UL) {
    return false;
  }

  if (blProperties->cert == NULL) {
    return false;
  }

  *version = blProperties->cert->version;
  return true;
}
