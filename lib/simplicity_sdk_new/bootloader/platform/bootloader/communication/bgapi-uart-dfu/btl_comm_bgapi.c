/***************************************************************************//**
 * @file
 * @brief Communication component implementing the BGAPI UART DFU protocol
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

// -----------------------------------------------------------------------------
// Includes

#include "btl_comm_bgapi.h"
#include "communication/btl_communication.h"
#include "core/btl_bootload.h"

// -----------------------------------------------------------------------------
// Functions

void communication_init(void)
{
  bootloader_bgapi_communication_init();
}

int32_t communication_start(void)
{
  return bootloader_bgapi_communication_start();
}

int32_t communication_main(void)
{
  int32_t ret = BOOTLOADER_OK;

  const BootloaderParserCallbacks_t parseCb = {
    .context = NULL,
    .applicationCallback = bootload_applicationCallback,
    .metadataCallback = NULL,
    .bootloaderCallback = bootload_bootloaderCallback
  };

  ImageProperties_t imageProps = {
    .contents = 0U,
  #if defined(_SILICON_LABS_32B_SERIES_3)
    .instructions = 0xFFFFFFFFFFFFFFFFU,
  #else
    .instructions = 0xFFU,
  #endif
    .imageCompleted = false,
    .imageVerified = false,
    .bootloaderVersion = 0,
    .application = { 0 },
#if defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT)
    .seUpgradeVersion = 0
#endif
  };

  ParserContext_t parserContext = { 0 };
  DecryptContext_t decryptContext = { 0 };
  AuthContext_t primaryAuthContext = { 0 };
#if defined (_SILICON_LABS_32B_SERIES_3)
  AuthContext_t secondaryAuthContext = { 0 };
  parser_init(&parserContext,
              &decryptContext,
              &primaryAuthContext,
              &secondaryAuthContext,
              PARSER_FLAG_PARSE_CUSTOM_TAGS);
#else
  parser_init(&parserContext,
              &decryptContext,
              &primaryAuthContext,
              PARSER_FLAG_PARSE_CUSTOM_TAGS);
#endif
  ret = bootloader_bgapi_communication_main(&imageProps,
                                            &parserContext,
                                            &parseCb);
  return ret;
}

void communication_shutdown(void)
{
  // Do nothing
}
