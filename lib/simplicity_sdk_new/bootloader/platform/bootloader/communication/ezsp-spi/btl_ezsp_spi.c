/***************************************************************************//**
 * @file
 * @brief EZSP-SPI communication component for Silicon Labs Bootloader.
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

#include "btl_ezsp_spi.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "driver/btl_driver_delay.h"
#include "core/btl_bootload.h"

// -‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Functions

void communication_init(void)
{
  bootloader_ezsp_communication_init();
}

int32_t communication_start(void)
{
  return bootloader_ezsp_communication_start();
}

int32_t communication_main(void)
{
  int32_t ret = -1;

  const BootloaderParserCallbacks_t parseCb = {
    .context = NULL,
    .applicationCallback = bootload_applicationCallback,
    // Standalone bootloading doesn't care about metadata
    .metadataCallback = NULL,
    .bootloaderCallback = bootload_bootloaderCallback
  };

  ImageProperties_t imageProps = {
    .contents = 0U,
    .instructions = 0xFFU,
    .imageCompleted = false,
    .imageVerified = false,
    .bootloaderVersion = 0,
    .application = { 0 }
  };

  ParserContext_t parserContext;
  DecryptContext_t decryptContext;
  AuthContext_t authContext;
#if defined(_SILICON_LABS_32B_SERIES_3)
  AuthContext_t secondaryAuthContext;
  parser_init(&parserContext,
              &decryptContext,
              &authContext,
              &secondaryAuthContext,
              PARSER_FLAG_PARSE_CUSTOM_TAGS);
#else
  parser_init(&parserContext,
              &decryptContext,
              &authContext,
              PARSER_FLAG_PARSE_CUSTOM_TAGS);
#endif
  ret = bootloader_ezsp_communication_main(&imageProps,
                                           &parserContext,
                                           &parseCb);
  return ret;
}

/***************************************************************************//**
 * Stop communication between the bootloader and external host.
 *
 * @return @ref BOOTLOADER_OK on success
 ******************************************************************************/
void communication_shutdown(void)
{
  bootloader_ezsp_communication_shutdown();
}
