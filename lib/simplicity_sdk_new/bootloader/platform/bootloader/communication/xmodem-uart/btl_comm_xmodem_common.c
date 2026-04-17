/***************************************************************************//**
 * @file
 * @brief Implements the communication core functionality
 *        for the UART XMODEM protocol
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "btl_comm_xmodem.h"
#include "btl_xmodem_config.h"
#include "driver/btl_serial_driver.h"
#include "driver/btl_driver_delay.h"

#if defined(BOOTLOADER_NONSECURE)
// NS headers
  #include "core/btl_reset_ns.h"
  #include "core/btl_bootload_ns.h"
#else
  #include "core/btl_reset.h"
  #include "core/btl_bootload.h"
#endif

// Debug
#include "debug/btl_debug.h"

#include <string.h>

// -----------------------------------------------------------------------------
// Defines

#ifndef BTL_XMODEM_IDLE_TIMEOUT
#error  "BTL_XMODEM_IDLE_TIMEOUT undefined."
#endif

// -----------------------------------------------------------------------------
// Static consts

static const uint8_t transferInitStr[] = "\r\nbegin upload\r\n";
static const uint8_t transferCompleteStr[] = "\r\nSerial upload complete\r\n";
static const uint8_t transferAbortedStr[] = "\r\nSerial upload aborted\r\n";
static const uint8_t xmodemError[] = "\r\nblock error 0x";
static const uint8_t fileError[] = "\r\nfile error 0x";
static const uint8_t bootError[] = "\r\nFailed to boot\r\n";

// -----------------------------------------------------------------------------
// Static local functions

static int32_t sendPacket(uint8_t packet)
{
  int32_t ret;
  ret = uart_sendByte(packet);

  if (packet == XMODEM_CMD_CAN) {
    // If packet is CAN, send three times
    uart_sendByte(packet);
    uart_sendByte(packet);
  }

  return ret;
}

static int32_t receivePacket(XmodemPacket_t *packet)
{
  int32_t ret = BOOTLOADER_OK;
  size_t requestedBytes;
  size_t receivedBytes;
  uint8_t *buf = (uint8_t *)packet;

  // Wait for bytes to be available in RX buffer
  delay_milliseconds(3000, false);
  while (uart_getRxAvailableBytes() == 0) {
    if (delay_expired()) {
      return BOOTLOADER_ERROR_COMMUNICATION_ERROR;
    }
  }

  // Read the first byte
  requestedBytes = 1;
  uart_receiveBuffer(buf,
                     requestedBytes,
                     &receivedBytes,
                     true,
                     1000);

  if (packet->header != XMODEM_CMD_SOH) {
    // All packets except XMODEM_CMD_SOH are single-byte
    return BOOTLOADER_OK;
  }

  requestedBytes = sizeof(XmodemPacket_t) - 1;
  ret = uart_receiveBuffer(buf + 1,
                           requestedBytes,
                           &receivedBytes,
                           true,
                           3000);

  if (receivedBytes != requestedBytes) {
    BTL_DEBUG_PRINT("Recvd ");
    BTL_DEBUG_PRINT_WORD_HEX(receivedBytes);
    BTL_DEBUG_PRINT_LF();
    // Didn't receive entire packet within 3000 ms; bail
    return BOOTLOADER_ERROR_COMMUNICATION_ERROR;
  }

  return ret;
}

static XmodemState_t getAction(void)
{
  uint8_t c;
  XmodemState_t state;
  int ret = uart_receiveByteTimeout(&c, 1000UL);

  if (ret != BOOTLOADER_OK) {
    return IDLE;
  }

  switch (c) {
    case '1':
      state = INIT_TRANSFER;
      break;
    case '2':
      state = BOOT;
      break;
    case '3':
      state = MENU;
      break;
    default:
      state = MENU;
      break;
  }
  return state;
}

__STATIC_INLINE uint8_t nibbleToHex(uint8_t nibble)
{
  return (nibble > 9) ? (nibble - 10 + 'A') : (nibble + '0');
}

// Helper function to handle SE upgrade
static bool handleSeUpgrade(const ImageProperties_t *imageProps, const ParserContext_t *parserContext)
{
#if defined(_SILICON_LABS_32B_SERIES_3)
  if ((imageProps->contents & BTL_IMAGE_CONTENT_SE)
      && bootload_checkSeUpgradeVersion(imageProps->seUpgradeVersion)) {
    // Install SE upgrade
    void *address = NULL;
    size_t size;
    sl_se_command_context_t cmd_ctx = { 0u };
    parserContext->seCmdCtxInterface.init(&cmd_ctx);
    parserContext->flashDataRegionInterface.data_region_get_location(&cmd_ctx, &address, &size);
    bootload_commitSeUpgrade((uint32_t)address);
    return true; // SE upgrade handled
    // If we get here, the SE upgrade failed
    // Return to menu
  }
#else
  (void)parserContext;
#if defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT)
  if ((imageProps->contents & BTL_IMAGE_CONTENT_SE)
      && bootload_checkSeUpgradeVersion(imageProps->seUpgradeVersion)) {
    // Install SE upgrade
#if defined(BOOTLOADER_NONSECURE)
    bootload_commitSeUpgrade();
#else
    bootload_commitSeUpgrade(BTL_UPGRADE_LOCATION);
#endif
    // If we get here, the SE upgrade failed
    return true; // SE upgrade handled
  }
#endif
#endif
  return false; // No SE upgrade
}

// Helper function to handle bootloader upgrade
static bool handleBootloaderUpgrade(const ImageProperties_t *imageProps, const ParserContext_t *parserContext)
{
#if defined(_SILICON_LABS_32B_SERIES_3)
  if ((imageProps->contents & BTL_IMAGE_CONTENT_MEM_SEC_1) \
      && (imageProps->bootloaderVersion > bootload_getBootloaderVersion())) {
    // Install bootloader upgrade
    bootload_commitBootloaderUpgrade(parserContext->plainBootloaderAddress,
                                     imageProps->bootloaderUpgradeSize);
    return true; // Bootloader upgrade handled
  }
#else
  (void)parserContext;
  if ((imageProps->contents & BTL_IMAGE_CONTENT_BOOTLOADER)
      && (imageProps->bootloaderVersion > bootload_getBootloaderVersion())) {
#if defined(BOOTLOADER_NONSECURE)
    bootload_commitBootloaderUpgrade(imageProps->bootloaderUpgradeSize);
#else
    bootload_commitBootloaderUpgrade(BTL_UPGRADE_LOCATION, imageProps->bootloaderUpgradeSize);
#endif
    return true; // Bootloader upgrade handled
  }
#endif
  return false; // No bootloader upgrade
}

// Handle BOOT state logic
static void handleBootState(const ImageProperties_t *imageProps, const ParserContext_t *parserContext)
{
  if (imageProps->imageCompleted && imageProps->imageVerified) {
    if (handleSeUpgrade(imageProps, parserContext)) {
      return; // SE upgrade handled
    }
    if (handleBootloaderUpgrade(imageProps, parserContext)) {
      return; // Bootloader upgrade handled
    } else {
      // Enter application if no upgrades are required
      reset_resetWithReason(BOOTLOADER_RESET_REASON_GO);
    }
    // If we get here, the bootloader upgrade or reboot failed
    uart_sendBuffer(bootError, sizeof(bootError), true);
  } else {
    // No valid image or verification failed
    reset_resetWithReason(BOOTLOADER_RESET_REASON_BADIMAGE);
  }
}

// -----------------------------------------------------------------------------
// Global Functions

void bootloader_xmodem_communication_init(void)
{
  uart_init();
}

int32_t bootloader_xmodem_communication_start(void)
{
  int32_t ret = BOOTLOADER_OK;
  char str[] = "\r\nGecko Bootloader vX.YY.ZZ\r\n"
               "1. upload gbl\r\n"
               "2. run\r\n"
               "3. ebl info\r\n"
               "BL > ";

  uint32_t version = bootload_getBootloaderVersion();
  str[20] = nibbleToHex((version >> 24) & 0x0F);
  str[22] = (((version >> 16) & 0x0F) / 10 + '0');
  str[23] = (((version >> 16) & 0x0F) % 10 + '0');
  str[25] = ((version & 0x0F) / 10 + '0');
  str[26] = ((version & 0x0F) % 10 + '0');

  uart_sendBuffer((uint8_t *)str, sizeof(str), true);
  return ret;
}

int32_t bootloader_xmodem_communication_main(ImageProperties_t *imageProps,
                                             const BootloaderParserCallbacks_t *parseCb)
{
  int32_t ret = -1;

  XmodemState_t state = IDLE;
  XmodemReceiveBuffer_t buf;
  uint8_t response = 0;
  int packetTimeout = 60;
#if BTL_XMODEM_IDLE_TIMEOUT > 0
  int idleTimeout = BTL_XMODEM_IDLE_TIMEOUT;
#endif

#if !defined(BOOTLOADER_NONSECURE)
  ParserContext_t parserContext = { 0 };
  DecryptContext_t decryptContext = { 0 };
  AuthContext_t primaryAuthContext = { 0 };
#if defined (_SILICON_LABS_32B_SERIES_3)
  AuthContext_t secondaryAuthContext = { 0 };
#endif
#endif

  delay_init();
  while (1) {
    switch (state) {
      case MENU:
        // Print menu
        bootloader_xmodem_communication_start();
        BTL_DEBUG_PRINTLN("Menu");
        state = IDLE;
        break;

      case IDLE:
        // Get user input
        state = getAction();

#if BTL_XMODEM_IDLE_TIMEOUT > 0
        if (state == IDLE) {
          if (--idleTimeout == 0) {
            reset_resetWithReason(BOOTLOADER_RESET_REASON_TIMEOUT);
          }
        } else {
          idleTimeout = BTL_XMODEM_IDLE_TIMEOUT;
        }
#endif
        packetTimeout = 60;
        break;

      case INIT_TRANSFER:
        uart_sendBuffer(transferInitStr, sizeof(transferInitStr), true);
        memset(imageProps, 0, sizeof(ImageProperties_t));
#if defined(BOOTLOADER_NONSECURE)
        parser_init(PARSER_FLAG_PARSE_CUSTOM_TAGS);
#else
#if defined(_SILICON_LABS_32B_SERIES_3)
        parser_init(&parserContext,
                    &decryptContext,
                    &primaryAuthContext,
                    &secondaryAuthContext,
                    PARSER_FLAG_PARSE_CUSTOM_TAGS);
        imageProps->instructions = 0xFFFFFFFFFFFFFFFFU;
#else
        parser_init(&parserContext,
                    &decryptContext,
                    &primaryAuthContext,
                    PARSER_FLAG_PARSE_CUSTOM_TAGS);
        imageProps->instructions = 0xFFU;
#endif
#endif
        imageProps->imageCompleted = false;
        imageProps->imageVerified = false;

        // Wait 5ms and see if we got any premature input; discard it
        delay_milliseconds(5, true);
        if (uart_getRxAvailableBytes()) {
          uart_flush(false, true);
        }

        // Initialize XMODEM parser
        xmodem_reset();

        state = WAIT_FOR_DATA;
        break;

      case WAIT_FOR_DATA:
        // Send 'C'
        sendPacket(XMODEM_CMD_C);
        delay_milliseconds(1000, false);
        while (uart_getRxAvailableBytes() == 0 && !delay_expired()) {
          // Do nothing
        }

        if (uart_getRxAvailableBytes()) {
          // We got a response; move to receive state
          state = RECEIVE_DATA;
        } else if (--packetTimeout == 0) {
          // No response within 1 second; tick towards timeout
          sendPacket(XMODEM_CMD_CAN);
          state = MENU;
        }
        break;

      case RECEIVE_DATA:
        // Wait for a full XMODEM packet
        memset(&(buf.packet), 0, sizeof(XmodemPacket_t));
        ret = receivePacket(&(buf.packet));

        if (ret != BOOTLOADER_OK) {
          response = XMODEM_CMD_NAK;
          sendPacket(response);
          break;
        }

        ret = xmodem_parsePacket(&(buf.packet), &response);
        if (ret == BOOTLOADER_ERROR_XMODEM_DONE) {
          // XMODEM receive complete; return to menu
          state = COMPLETE;

          // Send CAN rather than ACK if the image verification failed
          if (!imageProps->imageCompleted || !imageProps->imageVerified) {
            BTL_DEBUG_PRINTLN("Checksum fail");
            response = XMODEM_CMD_CAN;
          }
        }

        if ((ret == BOOTLOADER_OK) && (buf.packet.header == XMODEM_CMD_SOH)) {
          // Packet is OK, parse contents
#if defined(BOOTLOADER_NONSECURE)
          (void)parseCb;
          ret = parser_parse(buf.packet.data,
                             XMODEM_DATA_SIZE,
                             imageProps);
#else
          ret = parser_parse(&parserContext,
                             imageProps,
                             buf.packet.data,
                             XMODEM_DATA_SIZE,
                             parseCb);
#endif
          if (ret != BOOTLOADER_OK) {
            // Parsing file failed; cancel transfer and return to menu
            BTL_DEBUG_PRINT("Parse fail ");
            BTL_DEBUG_PRINT_WORD_HEX(ret);
            BTL_DEBUG_PRINT_LF();
            response = XMODEM_CMD_CAN;
          }
        }

        if (response == XMODEM_CMD_CAN) {
          // Parsing packet failed; return to main menu
          state = COMPLETE;
        }

        // Send response
        sendPacket(response);
        break;

      case COMPLETE:
        BTL_DEBUG_PRINT("Complete ");
        BTL_DEBUG_PRINT_WORD_HEX(ret);
        BTL_DEBUG_PRINT_LF();
        uart_flush(false, true);

        delay_milliseconds(10, true);
        uart_sendBuffer((response == XMODEM_CMD_ACK) ? transferCompleteStr : transferAbortedStr,
                        (response == XMODEM_CMD_ACK) ? sizeof(transferCompleteStr) : sizeof(transferAbortedStr),
                        true);

        if (response != XMODEM_CMD_ACK) {
          if ((ret >= BOOTLOADER_ERROR_XMODEM_BASE)
              && (ret < BOOTLOADER_ERROR_PARSER_BASE)) {
            uart_sendBuffer(xmodemError,
                            sizeof(xmodemError),
                            true);
          } else {
            uart_sendBuffer(fileError,
                            sizeof(fileError),
                            true);
          }

          switch (ret) {
            case BOOTLOADER_ERROR_XMODEM_NO_SOH:
              response = 0x21;
              break;
            case BOOTLOADER_ERROR_XMODEM_PKTNUM:
              response = 0x22;
              break;
            case BOOTLOADER_ERROR_XMODEM_CRCL:
              response = 0x23;
              break;
            case BOOTLOADER_ERROR_XMODEM_CRCH:
              response = 0x24;
              break;
            case BOOTLOADER_ERROR_XMODEM_PKTSEQ:
              response = 0x25;
              break;
            case BOOTLOADER_ERROR_XMODEM_PKTDUP:
              response = 0x27;
              break;

            case BOOTLOADER_ERROR_PARSER_VERSION:
              response = 0x41; // BL_ERR_HEADER_EXP
              break;
            case BOOTLOADER_ERROR_PARSER_CRC:
              response = 0x43; // BL_ERR_CRC
              break;
            case BOOTLOADER_ERROR_PARSER_UNKNOWN_TAG:
              response = 0x44; // BL_ERR_UNKNOWN_TAG
              break;
            case BOOTLOADER_ERROR_PARSER_SIGNATURE:
              response = 0x45; // BL_ERR_SIG
              break;
            case BOOTLOADER_ERROR_PARSER_FILETYPE:
            case BOOTLOADER_ERROR_PARSER_UNEXPECTED:
            case BOOTLOADER_ERROR_PARSER_EOF:
              response = 0x4F; // BL_ERR_TAGBUF
              break;
            case BOOTLOADER_ERROR_PARSER_KEYERROR:
              response = 0x50; // BL_ERR_INV_KEY
              break;
            default:
              break;
          }

          uart_sendByte(nibbleToHex(response >> 4));
          uart_sendByte(nibbleToHex(response & 0x0F));
          uart_sendByte('\r');
          uart_sendByte('\n');
        }
        state = MENU;
        break;

      case BOOT:
        state = MENU;
#if defined(BOOTLOADER_NONSECURE)
        handleBootState(imageProps, NULL);
#else
        handleBootState(imageProps, &parserContext);
#endif
        break;
    }
  }
}
