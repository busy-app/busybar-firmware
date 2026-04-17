/***************************************************************************//**
 * @file
 * @brief Bootloading functionality for the Silicon Labs bootloader
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

#include "btl_bootload.h"
#include "core/btl_reset.h"
#include "btl_util.h"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

MISRAC_DISABLE
#include "sli_se_manager_mailbox.h"
#include "sli_se_manager_internal.h"
MISRAC_ENABLE

// Interface
#include "api/btl_interface.h"
#include "api/application_properties.h"

// Image parser
#include "parser/gbl/btl_gbl_parser.h"
#include "parser/gbl/btl_gbl_format.h"

// Security algorithms
#include "security/btl_security_types.h"
#include "security/btl_security_ecdsa.h"
#include "security/btl_crc32.h"

// Get memcpy
#include <string.h>

#include <stdlib.h>

#ifdef __ICCARM__
// Silence MISRA warning disallowing statements without side effects
#pragma diag_suppress=Pm049
// Silence MISRA warning disallowing access to volatile object in right-hand operand of || operator
#pragma diag_suppress=Pm026
#endif

//
// Option validation
//
#if defined(BOOTLOADER_ROLLBACK_PROTECTION) && (BOOTLOADER_ROLLBACK_PROTECTION == 1)
#if defined(MAIN_BOOTLOADER_IN_MAIN_FLASH)
#error "Rollback protection not supported"
#endif
#endif // defined(BOOTLOADER_ROLLBACK_PROTECTION)

// --------------------------------
// Local type declarations
static bool bootload_verifySecureBoot(uint32_t startAddress);
static bool getSignatureX(ApplicationProperties_t *appProperties,
                          uint32_t *appSignatureX);

// --------------------------------
// Defines

// --------------------------------
// Local functions

static bool getSignatureX(ApplicationProperties_t *appProperties, uint32_t *appSignatureX)
{
  // Check if app properties struct or legacy direct signature pointer
  if (bootload_checkApplicationPropertiesMagic(appProperties)) {
    if (appProperties->signatureType != APPLICATION_SIGNATURE_ECDSA_P256) {
      // Application signature isn't ECDSA, fail early
      return false;
    }
    // Compatibility check of the application properties struct.
    if (!bootload_checkApplicationPropertiesVersion(appProperties)) {
      return false;
    }
    *appSignatureX = appProperties->signatureLocation;
  } else {
    *appSignatureX = (uint32_t)appProperties;
  }
  return true;
}

static bool bootload_verifySecureBoot(uint32_t startAddress)
{
  volatile int32_t retVal = BOOTLOADER_ERROR_SECURITY_REJECTED;
  Sha256Context_t shaState;

  BareBootTable_t *appStart = (BareBootTable_t *)startAddress;
  uint32_t appProps = (uint32_t)appStart->signature;
  uint32_t appSignatureX;
  uint32_t appSignatureY;
  ApplicationProperties_t *appProperties =
    (ApplicationProperties_t *)(appProps);

  if (!bootload_checkApplicationPropertiesMagic(appProperties)) {
    return false;
  }
  if (!bootload_checkApplicationPropertiesVersion(appProperties)) {
    return false;
  }

  if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ROLLBACK_PROTECTION) {
    if (!bootload_verifyApplicationVersion(appProperties->app.version)) {
      return false;
    }
  }

  // Access word 13 to read sl_app_properties of the bootloader.
  ApplicationProperties_t *blProperties =
    (ApplicationProperties_t *)(*(uint32_t *)(BTL_MAIN_STAGE_BASE + 52UL));
  if (!bootload_checkApplicationPropertiesMagic(blProperties)) {
    return false;
  }

  if ((uint32_t)blProperties > ((uint32_t)mainBootloaderTable->startOfAppSpace - sizeof(ApplicationProperties_t))) {
    // Make sure that this pointer is within the bootloader space.
    return false;
  }

  bool gotCert = false;
  if (!bootload_verifyApplicationCertificate(appProperties, &gotCert)) {
    return false;
  }

  if (!getSignatureX(appProperties, &appSignatureX)) {
    return false;
  }

  sl_se_code_region_config_t region_config = { 0 };
  sl_se_command_context_t cmd_ctx = { 0 };
  sl_se_init_command_context(&cmd_ctx);
  sl_se_code_region_get_config(&cmd_ctx, &region_config, 1, 1);

  // Check that signature is in application flash
  if ((appSignatureX < (uint32_t)(mainBootloaderTable->startOfAppSpace))
      || (appSignatureX < startAddress)
      || (appSignatureX > (startAddress + region_config.region_size))) {
    return false;
  }

  // SHA-256 of the entire application (startAddress until signature)
  retVal = sl_se_hash(&cmd_ctx,
                      SL_SE_HASH_SHA256,
                      (const uint8_t *)startAddress,
                      appSignatureX - startAddress,
                      shaState.sha,
                      32U);
  sl_se_deinit_command_context(&cmd_ctx);
  if (retVal != SL_STATUS_OK) {
    return false;
  }
  appSignatureY = appSignatureX + 32UL;
  if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT) {
    if (gotCert) {
      // Application certificate is already verified to be valid at this point.
      // Authenticate the application using the certificate of the application.
      retVal = btl_verifyEcdsaP256r1(shaState.sha,
                                     (uint8_t*)appSignatureX,
                                     (uint8_t*)appSignatureY,
                                     &(appProperties->cert->key[0]),
                                     &(appProperties->cert->key[32]));
    } else {
      // Application is directly signed + cert on app is not required.
      // Authenticate the application using the certificate of the bootloader.
      retVal = btl_verifyEcdsaP256r1(shaState.sha,
                                     (uint8_t*)appSignatureX,
                                     (uint8_t*)appSignatureY,
                                     &(blProperties->cert->key[0]),
                                     &(blProperties->cert->key[32]));
    }
  } else {
    retVal = btl_verifyEcdsaP256r1(shaState.sha,
                                   (uint8_t*)appSignatureX,
                                   (uint8_t*)appSignatureY,
                                   NULL,
                                   NULL);
  }
  if (retVal == BOOTLOADER_OK) {
    return true;
  } else {
    return false;
  }
}

// --------------------------------
// Global functions

// Callbacks
SL_WEAK void bootload_applicationCallback(uint32_t offset,
                                          uint8_t  *data,
                                          size_t   length,
                                          unsigned int region_idx,
                                          void     *context)
{
  ParserContext_t *ctx = (ParserContext_t*)context;
  sl_se_command_context_t cmd_ctx = { 0u };
  sl_se_init_command_context(&cmd_ctx);

  if (offset == 0) {
    memcpy(ctx->withheldApplicationVectors, data, WITHHELD_VECTORS_SIZE);
    data += WITHHELD_VECTORS_SIZE;
    offset += WITHHELD_VECTORS_SIZE;
    length -= WITHHELD_VECTORS_SIZE;
  }

  if (length > 0) {
    size_t paddedSize = (length + (PAD_SIZE - 1)) & ~(PAD_SIZE - 1);
    uint8_t *finalData = data;
    uint8_t *paddedData = NULL;

    if (length % PAD_SIZE != 0) {
      // Allocate a separate buffer for padding to ensure the input buffer remains intact
      paddedData = (uint8_t *)malloc(paddedSize);
      if (paddedData == NULL) {
        return;
      }
      memcpy(paddedData, data, length);

      for (size_t i = length; i < paddedSize; i++) {
        paddedData[i] = 0xFF;
      }
      finalData = paddedData;
    }

    ctx->retCode = sl_se_code_region_write(&cmd_ctx,
                                           region_idx,
                                           offset,
                                           finalData,
                                           paddedSize,
                                           NULL,
                                           NULL);

    if (paddedData != NULL) {
      free(paddedData);
    }
  }
}

SL_WEAK void bootload_bootloaderCallback(uint32_t address,
                                         uint8_t  *data,
                                         size_t   length,
                                         unsigned int region_idx,
                                         void     *context)
{
  (void) region_idx;
  ParserContext_t *ctx = (ParserContext_t*)context;
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);

  size_t paddedSize = (length + (PAD_SIZE - 1)) & ~(PAD_SIZE - 1);
  uint8_t *finalData = data;
  uint8_t *paddedData = NULL;

  if (length % PAD_SIZE != 0) {
    // Allocate a separate buffer for padding to ensure the input buffer remains intact
    paddedData = (uint8_t *)malloc(paddedSize);
    if (paddedData == NULL) {
      return;
    }
    memcpy(paddedData, data, length);

    for (size_t i = length; i < paddedSize; i++) {
      paddedData[i] = 0xFF;
    }
    finalData = paddedData;
  }

  ctx->retCode = sl_se_data_region_write(&cmd_ctx,
                                         (void *)address,
                                         finalData,
                                         paddedSize);

  if (paddedData != NULL) {
    free(paddedData);
  }
}

bool bootload_checkApplicationPropertiesMagic(void *appProperties)
{
  if ((appProperties == NULL) || ((uint32_t) appProperties == 0xFFFFFFFFUL)) {
    return false;
  }

#if (FLASH_BASE > 0x0UL)
  if ((uint32_t)appProperties < FLASH_BASE) {
    return false;
  }
#endif

  uint8_t magicRev[16U] = APPLICATION_PROPERTIES_REVERSED;
  const uint8_t *magic = (uint8_t *)appProperties;

  for (size_t i = 0U; i < 16U; i++) {
    if (magicRev[15U - i] != magic[i]) {
      return false;
    }
  }

  return true;
}

bool bootload_checkApplicationPropertiesVersion(void *appProperties)
{
  const ApplicationProperties_t *appProp = (ApplicationProperties_t *)appProperties;
  // Compatibility check of the application properties struct.
  if (((appProp->structVersion & APPLICATION_PROPERTIES_VERSION_MAJOR_MASK)
       >> APPLICATION_PROPERTIES_VERSION_MAJOR_SHIFT)
      > (uint32_t)APPLICATION_PROPERTIES_VERSION_MAJOR) {
    return false;
  }
  return true;
}

bool bootload_verifyApplication(uint32_t startAddress)
{
  BareBootTable_t *appStart = (BareBootTable_t *)startAddress;
  uint32_t appSp = (uint32_t)appStart->stackTop;
  uint32_t appPc = (uint32_t)appStart->resetVector;
  uint32_t appProps = (uint32_t)appStart->signature;

  // Check that SP points to RAM
  if ((appSp < SRAM_BASE) || (appSp > (SRAM_BASE + SRAM_SIZE))) {
    return false;
  }

  sl_se_code_region_config_t region_config = { 0 };
  sl_se_command_context_t cmd_ctx = { 0 };
  sl_se_init_command_context(&cmd_ctx);
  sl_se_code_region_get_config(&cmd_ctx, &region_config, 1, 1);

  appPc = bootloader_GetAliasedAddr(appPc);
  // Check that PC points to application flash
  if ((appPc < (uint32_t)mainBootloaderTable->startOfAppSpace)
      || (appPc > ((uint32_t)mainBootloaderTable->startOfAppSpace + region_config.region_size))) {
    return false;
  }

  ApplicationProperties_t *appProperties =
    (ApplicationProperties_t *)(appProps);

  // Application properties pointer can take on many possible values:
  //
  // 0xFFFFFFFF - Likely unused part of AAT
  //   - Pointer to Reset_Handler (app without app properties)
  //   - Pointer to ApplicationProperties_t
  //   - Pointer to ECDSA signature (legacy direct solution)
  appProps = bootloader_GetAliasedAddr(appProps);
  bool isAppRequireAuth = (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_SECURE_BOOT);
  if ((appProps < ((uint32_t)mainBootloaderTable->startOfAppSpace + 64UL))
      || (appProps > ((uint32_t)mainBootloaderTable->startOfAppSpace + region_config.region_size))) {
    // Application properties pointer does not point inside flash (is not valid)
    if (isAppRequireAuth) {
      // Secure boot is enforced. Either an app properties struct or a direct
      // pointer to the signature is required. An address outside main flash
      // is not valid for either case.
      return false;
    } else {
      // Secure boot is not enforced, we have to assume app is valid
      return true;
    }
  } else if (isAppRequireAuth) {
    // Secure boot is enforced, attempt to verify secure boot signature
    return bootload_verifySecureBoot(startAddress);
  } else if (bootload_checkApplicationPropertiesMagic(appProperties)) {
    if (!bootload_checkApplicationPropertiesVersion(appProperties)) {
      return false;
    }
    // Application properties pointer is valid, decide what action to take
    // based on signature type
    if (appProperties->signatureType == APPLICATION_SIGNATURE_NONE) {
      // No signature, app has to be assumed valid
      return true;
    } else {
      // Default to secure boot check
      return bootload_verifySecureBoot(startAddress);
    }
  } else {
    // Application properties pointer points into flash, but doesn't point to
    // an application properties struct.
    // Secure boot is not enforced (checked above), assume that this is a
    // pointer to the Reset_Handler and that the app is valid
    return true;
  }
}

bool bootload_verifyApplicationVersion(uint32_t appVersion)
{
  if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ROLLBACK_PROTECTION) {
    // Check for the minimum application version that should be allowed.
#ifdef BOOTLOADER_ENABLE
#if defined(BOOTLOADER_ROLLBACK_PROTECTION_MINIMUM_VERSION) && (BOOTLOADER_ROLLBACK_PROTECTION_MINIMUM_VERSION != 0)
    if (PARSER_APPLICATION_MINIMUM_VERSION_VALID > appVersion) {
      return false;
    }
#endif
#else
    uint32_t minVersion;
    bootloader_getAppMinVersion(&minVersion);
    if (minVersion > appVersion) {
      return false;
    }
#endif

    uint32_t runningAppVersion;
    sl_se_command_context_t cmd_ctx = { 0u };
    sl_status_t status;
    sl_se_init_command_context(&cmd_ctx);
    status = sl_se_code_region_get_version(&cmd_ctx, 1U, &runningAppVersion);
    if (status == BOOTLOADER_OK) {
      if (runningAppVersion > appVersion) {
        return false;
      }
      return true;
    } else if (status == SL_STATUS_NOT_INITIALIZED) {
      return true;
    }
    return false;
  } else {
    (void)appVersion;
    return false;
  }
}

bool bootload_gotCertificate(void *appProp)
{
  bool isCertAuthRequired = (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT);

  if (isCertAuthRequired) {
    if (appProp == NULL) {
      return false;
    }

    ApplicationProperties_t *appProperties = (ApplicationProperties_t *)(appProp);
    // Compatibility check of the application properties struct.
    // The application properties struct with the major version 0
    // does not contain the certificate struct.
    if (((appProperties->structVersion & APPLICATION_PROPERTIES_VERSION_MAJOR_MASK)
         >> APPLICATION_PROPERTIES_VERSION_MAJOR_SHIFT) < 1UL) {
      return false;
    }

    if (((appProperties->structVersion & APPLICATION_PROPERTIES_VERSION_MINOR_MASK)
         >> APPLICATION_PROPERTIES_VERSION_MINOR_SHIFT) < 1UL) {
      return false;
    }

    if (appProperties->cert == NULL) {
      return false;
    }

    return true;
  } else {
    (void)appProp;
    return false;
  }
}

bool bootload_verifyCertificate(void *cert)
{
  bool isCertAuthRequired = (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT);
  if (isCertAuthRequired) {
    if (cert == NULL) {
      return false;
    }
    ApplicationCertificate_t *appCertificate = (ApplicationCertificate_t *)(cert);

    volatile int32_t retVal = BOOTLOADER_ERROR_SECURITY_REJECTED;
    Sha256Context_t shaState;

    // Access word 13 to read sl_app_properties of the bootloader.
    ApplicationProperties_t *blProperties =
      (ApplicationProperties_t *)(*(uint32_t *)(BTL_MAIN_STAGE_BASE + 52UL));
    if (!bootload_checkApplicationPropertiesMagic(blProperties)) {
      return false;
    }
#if !defined(MAIN_BOOTLOADER_TEST)
    if ((uint32_t)blProperties > (BTL_APPLICATION_BASE - sizeof(ApplicationProperties_t))) {
      // Make sure that this pointer is within the bootloader space.
      return false;
    }
#endif

    // Application cert version need to be higher or equal than
    // the running bootloader version.
    if (blProperties->cert->version > appCertificate->version) {
      return false;
    } else {
      sl_se_command_context_t cmd_ctx = { 0 };
      sl_se_init_command_context(&cmd_ctx);
      // Check ECDSA signing.
      retVal = sl_se_hash(&cmd_ctx, SL_SE_HASH_SHA256, (const uint8_t *)appCertificate, 72U, shaState.sha, 32U);
      // Use the public key stored in bootloader certificate
      // to verify the certificate. The bootloader cerfiticate
      // has been validated by SE.
      sl_se_deinit_command_context(&cmd_ctx);
      if (retVal != SL_STATUS_OK) {
        return false;
      }
      retVal = btl_verifyEcdsaP256r1(shaState.sha,
                                     &(appCertificate->signature[0]),
                                     &(appCertificate->signature[32]),
                                     &(blProperties->cert->key[0]),
                                     &(blProperties->cert->key[32]));
      if (retVal != BOOTLOADER_OK) {
        return false;
      }
      return true;
    }
  } else {
    (void)cert;
    return false;
  }
}

bool bootload_verifyApplicationCertificate(void *appProp, void *gotCert)
{
  bool isCertAuthRequired = (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT);
  if (isCertAuthRequired) {
    ApplicationProperties_t *appProperties = (ApplicationProperties_t *)(appProp);
    bool *gotCertificate = (bool *)gotCert;
    *gotCertificate = bootload_gotCertificate(appProperties);
    if (*gotCertificate) {
      // Validate Cert
      if (!bootload_verifyCertificate(appProperties->cert)) {
        // Cert found, but it contains an invalid signature.
        return false;
      }
    } else if (mainBootloaderTable->capabilities & BOOTLOADER_CAPABILITY_REJECT_DIRECT_SIGNED_IMG) {
      return false;
    }
    return true;
  } else {
    (void)appProp;
    (void)gotCert;
    return true;
  }
}

// --------------------------------
// Secure Element functions

SL_WEAK bool bootload_commitBootloaderUpgrade(uint32_t upgradeAddress, uint32_t size)
{
  // Check CRC32 checksum on the bootloader image.
  uint32_t crc = btl_crc32Stream((void *)upgradeAddress, (size_t)size, BTL_CRC32_START);
  if (crc != BTL_CRC32_END) {
    // CRC32 check failed. Return early.
    return false;
  }

  // The CRC32 checksum has been appended to the image and it has already been checked above
  // so just disregard it. This give us the correct image size.
  size = size - 4u;

  sl_status_t ret = 0;
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  // Verify upgrade image
  ret = sl_se_check_host_image(&cmd_ctx, (void *)upgradeAddress, size);
  if (ret != SL_STATUS_OK) {
    return false;
  }

  // Set reset code for when we get back
  reset_setResetReason(BOOTLOADER_RESET_REASON_BOOTLOAD);

  sl_se_init_command_context(&cmd_ctx);
  // Apply upgrade image
  ret = sl_se_apply_host_image(&cmd_ctx, (void *)upgradeAddress, size);
  if (ret != SL_STATUS_OK) {
    return false;
  }

  // Should never get here
  return false;
}

SL_WEAK bool bootload_checkSeUpgradeVersion(uint32_t upgradeVersion)
{
  uint32_t runningVersion = 0xFFFFFFFFUL;

  if (!bootload_getSeVersion(&runningVersion)) {
    return false; // Could not retrieve SE version
  }

  // Only allow upgrade if it is higher than the running version
  if (runningVersion < upgradeVersion) {
    return true;
  } else {
    return false;
  }
}

SL_WEAK bool bootload_commitSeUpgrade(uint32_t upgradeAddress)
{
  sl_status_t ret = 0;
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  ret = sl_se_check_se_image(&cmd_ctx, (void *)upgradeAddress);
  if (ret != SL_STATUS_OK) {
    return false;
  }

  // Set reset code for when we get back
  reset_setResetReason(BOOTLOADER_RESET_REASON_BOOTLOAD);

  sl_se_init_command_context(&cmd_ctx);
  ret = sl_se_apply_se_image(&cmd_ctx, (void *)upgradeAddress);
  if (ret != SL_STATUS_OK) {
    return false;
  }
  // Should never get here
  return false;
}

uint32_t bootload_getBootloaderVersion(void)
{
  return mainBootloaderTable->header.version;
}

bool bootload_getSeVersion(uint32_t *version)
{
  sl_status_t ret = 0;
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  ret = sl_se_get_se_version(&cmd_ctx, version);
  if (ret != SL_STATUS_OK) {
    return false; // error getting SE version
  }
  return true;
}
