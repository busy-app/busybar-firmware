/***************************************************************************//**
 * @file
 * @brief LZ4 decompression functionality for Gecko Bootloader
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
#include "btl_decompress_lz4.h"

#include "api/btl_errorcode.h"
#include "debug/btl_debug.h"

#include <string.h>

static Lz4ParserContext_t lz4ParserContext;

#define DECOMPRESS_BUFFER_SIZE 32UL

// -----------------------------------------------------------------------------
// LZ4 parser implementation

int32_t lz4_init(Lz4Context_t *ctx, Lz4DataRead_t readFunction)
{
  BTL_ASSERT(ctx != NULL);
  if (ctx == NULL) {
    return BOOTLOADER_ERROR_COMPRESSION_INIT;
  }

  memset(ctx, 0UL, sizeof(Lz4Context_t));

  ctx->state = LZ4_STATE_TOKEN;
  ctx->readFunction = readFunction;

  return BOOTLOADER_OK;
}

int32_t lz4_decompress(Lz4Context_t *ctx,
                       void *inputData,
                       size_t inputLength,
                       Lz4DataWrite_t writeFunction)
{
  size_t inputPosition = 0;
  uint8_t *data = (uint8_t *)inputData;
  size_t length = 0;
  uint8_t backtrackBuffer[DECOMPRESS_BUFFER_SIZE];
  int32_t retval = BOOTLOADER_OK;

  while (inputPosition < inputLength) {
    switch (ctx->state) {
      case LZ4_STATE_TOKEN:
        // Get literal and match lengths from header token
        ctx->literalLength = data[inputPosition] >> 4U;
        ctx->matchLength = 4 + (data[inputPosition] & 0xFU);
        ctx->backtrackOffset = 0;
        if (ctx->literalLength == 15) {
          // More than 15 literals, prepare for extended length
          ctx->state = LZ4_STATE_LITERAL_LENGTH;
        } else if (ctx->literalLength == 0) {
          // No literals, move on to backtrack offset
          ctx->state = LZ4_STATE_OFFSET_LSB;
        } else {
          // 1-14 literals, prepare to receive literals
          ctx->state = LZ4_STATE_LITERAL_VALUE;
        }
        break;

      case LZ4_STATE_LITERAL_LENGTH:
        ctx->literalLength += data[inputPosition];
        if (data[inputPosition] != 255) {
          // Last literal length byte, move on to literals
          ctx->state = LZ4_STATE_LITERAL_VALUE;
        }
        break;

      case LZ4_STATE_LITERAL_VALUE:
        // Call write function with the max number of bytes we have available
        length = SL_MIN(inputLength - inputPosition, ctx->literalLength);
        retval = writeFunction(&data[inputPosition], length);
        ctx->literalLength -= length;
        // Advance parser, since we handled more than a single byte this iteration
        inputPosition += length - 1;
        if (ctx->literalLength == 0) {
          // No literals left, move on to backtrack offset
          ctx->state = LZ4_STATE_OFFSET_LSB;
        }
        break;

      case LZ4_STATE_OFFSET_LSB:
        ctx->backtrackOffset = data[inputPosition];
        ctx->state = LZ4_STATE_OFFSET_MSB;
        break;

      case LZ4_STATE_OFFSET_MSB:
        ctx->backtrackOffset |= (uint16_t)data[inputPosition] << 8U;
        if (ctx->matchLength == 19) {
          // More that 19 bytes match, prepare for extended length
          ctx->state = LZ4_STATE_MATCH_LENGTH;
        } else {
          // No extra match length, go straight to backtracking
          ctx->state = LZ4_STATE_BACKTRACKING;
        }
        break;

      case LZ4_STATE_MATCH_LENGTH:
        ctx->matchLength += data[inputPosition];
        if (data[inputPosition] != 255) {
          // Last match length byte, move on to backtracking
          ctx->state = LZ4_STATE_BACKTRACKING;
        }
        break;

      default:
        BTL_ASSERT(0);
        return BOOTLOADER_ERROR_COMPRESSION_STATE;
    }

    if ((retval == BOOTLOADER_OK) && (ctx->state == LZ4_STATE_BACKTRACKING)) {
      size_t backtrackBytes;
      BTL_DEBUG_PRINT_WORD_HEX(lz4ParserContext.parserContext->programmingAddress);
      BTL_DEBUG_PRINT(" Backtrack by ");
      BTL_DEBUG_PRINT_WORD_HEX(ctx->backtrackOffset);
      BTL_DEBUG_PRINT(" bytes, copy ");
      BTL_DEBUG_PRINT_WORD_HEX(ctx->matchLength);
      BTL_DEBUG_PRINTLN(" bytes");
      while ((ctx->matchLength > 0) && (retval == BOOTLOADER_OK)) {
        // If the backtracking is forward matching, clamp to already written data
        // This is to keep forward-matching logic inside the parser
        backtrackBytes = SL_MIN(ctx->matchLength, ctx->backtrackOffset);
        // Clamp to the buffer size
        backtrackBytes = SL_MIN(DECOMPRESS_BUFFER_SIZE, backtrackBytes);
        // Read bytes using function registered at init
        ctx->readFunction(ctx->backtrackOffset, backtrackBuffer, backtrackBytes);
        // Write bytes using function
        retval = writeFunction(backtrackBuffer, backtrackBytes);
        ctx->matchLength -= backtrackBytes;
      }
      // Done with LZ4 block, move on to next block
      ctx->state = LZ4_STATE_TOKEN;
    }

    if (retval != BOOTLOADER_OK) {
      break;
    }

    // Single byte parsed, move parser forward
    inputPosition++;
  }

  return retval;
}

int32_t lz4_finish(Lz4Context_t *ctx)
{
  // The last LZ4 block should end with literals, so the parser should end
  // in the LSB offset state. Else, something went wrong during decompression.
  if (ctx->state != LZ4_STATE_OFFSET_LSB) {
    return BOOTLOADER_ERROR_COMPRESSION_STATE;
  } else {
    return BOOTLOADER_OK;
  }
}

// -----------------------------------------------------------------------------
// GBL tag implementation

int32_t gbl_lz4ReadMemory(size_t backtrackOffset, uint8_t *data, size_t length)
{
  uint32_t dataOffset = 0;
  uint32_t programmingAddress = lz4ParserContext.parserContext->programmingAddress
                                + lz4ParserContext.outputOffset - backtrackOffset;

  // Backtracking further than the word alignment buffer, get data from flash
  if (backtrackOffset > lz4ParserContext.outputOffset) {
    uint32_t programOffset = backtrackOffset - lz4ParserContext.outputOffset;
    uint32_t copyLength = SL_MIN(length, programOffset);

    if ( lz4ParserContext.parserContext->regionId == 0) {
      // Copy data from flash to read buffer
      (void) memcpy(data,
                    (void *)programmingAddress,
                    copyLength);
    } else {
      sl_se_code_region_config_t region_config = { 0 };
      sl_se_command_context_t cmd_ctx = { 0 };
      sl_se_init_command_context(&cmd_ctx);
      sl_se_code_region_get_config(&cmd_ctx, &region_config, 1, 1);
      uint32_t startOfAppSpace = (uint32_t) mainBootloaderTable->startOfAppSpace;
      uint32_t appPc = (uint32_t)lz4ParserContext.parserContext->withheldApplicationVectors[4U];
      bool isInSecureRange = (appPc >= FLASH_S_BASE + 0x8000) && (appPc <= FLASH_S_BASE + 0x8000 + region_config.region_size);
      if (isInSecureRange) {
        startOfAppSpace = (startOfAppSpace | 0x10000000);
      } else {
        startOfAppSpace = (startOfAppSpace & ~0x10000000);
      }
      // Some of the data read from flash may have been withheld by the parser
      // due to overlapping with the application vector table
      if ((programmingAddress + copyLength >= startOfAppSpace)
          && (programmingAddress < startOfAppSpace + 64UL)) {
        // Absolute start address of region to withhold
        uint32_t startWithheld = SL_MAX(programmingAddress, startOfAppSpace);
        // Absolute end address of region to withhold
        uint32_t endWithheld = SL_MIN(programmingAddress + copyLength,
                                      startOfAppSpace + 64UL);

        // Offset into withhold buffer of region to extract
        uint32_t withheldSrcOffset = startWithheld - startOfAppSpace;
        // Offset into output buffer to place withheld region
        uint32_t withheldDstOffset = startWithheld - programmingAddress;

        // Replace with withheld bytes
        (void) memcpy(&data[withheldDstOffset],
                      &lz4ParserContext.parserContext->withheldApplicationVectors[withheldSrcOffset],
                      endWithheld - startWithheld);
      } else {
        // Copy data from flash to read buffer
        (void) memcpy(data,
                      (void *)programmingAddress,
                      copyLength);
      }
    }
    dataOffset = copyLength;
    backtrackOffset -= copyLength;
  }

  // Backtracking into the word alignment buffer
  if (dataOffset < length) {
    // Current backtracking position should be inside alignment buffer
    BTL_ASSERT(backtrackOffset <= lz4ParserContext.outputOffset);

    size_t bufferLength = length - dataOffset;

    // Offset into alignment buffer is given by how much to backtrack
    size_t bufferOffset = lz4ParserContext.outputOffset - backtrackOffset;

    // The remaining data to get should be less than available data in
    // alignment buffer
    BTL_ASSERT(bufferLength <= (lz4ParserContext.outputOffset - bufferOffset));

    // Copy data from word alignment buffer to read buffer
    (void) memcpy(&data[dataOffset],
                  &lz4ParserContext.outputBuffer[bufferOffset],
                  bufferLength);
  }

  return BOOTLOADER_OK;
}

int32_t gbl_lz4WriteMemory(uint8_t *data, size_t length)
{
  size_t offset = 0UL;
  // We have some unaligned data from a previous iteration
  if (lz4ParserContext.outputOffset != 0UL) {
    // Set offset to the first word-aligned index into the write buffer,
    // and copy any data before that into the alignment buffer
    offset = SL_MIN(ALIGNMENT_SIZE - lz4ParserContext.outputOffset, length);
    memcpy(&lz4ParserContext.outputBuffer[lz4ParserContext.outputOffset], data, offset);
    lz4ParserContext.outputOffset += offset;

    // Filled the alignment buffer, write it out
    if (lz4ParserContext.outputOffset == ALIGNMENT_SIZE) {
      if ((lz4ParserContext.parserContext->regionId == 0) && (lz4ParserContext.parserCallbacks->bootloaderCallback != NULL)) {
        lz4ParserContext.parserCallbacks->bootloaderCallback(lz4ParserContext.parserContext->programmingAddress,
                                                             lz4ParserContext.outputBuffer,
                                                             ALIGNMENT_SIZE,
                                                             0,
                                                             lz4ParserContext.parserContext);
        if (lz4ParserContext.parserContext->retCode != BOOTLOADER_OK) {
          return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
        }
      } else if ((lz4ParserContext.parserContext->regionId != 0) && (lz4ParserContext.parserCallbacks->applicationCallback != NULL)) {
        uint32_t addOffset;
        addOffset = lz4ParserContext.parserContext->programmingAddress - lz4ParserContext.parserContext->currentUpdateMemorySection.updateMemorySection.targetAddr;
        lz4ParserContext.parserCallbacks->applicationCallback(addOffset,
                                                              lz4ParserContext.outputBuffer,
                                                              ALIGNMENT_SIZE,
                                                              lz4ParserContext.parserContext->regionId,
                                                              lz4ParserContext.parserContext);
        if (lz4ParserContext.parserContext->retCode != BOOTLOADER_OK) {
          return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
        }
      }
      lz4ParserContext.parserContext->programmingAddress += ALIGNMENT_SIZE;
      lz4ParserContext.outputOffset = 0UL;
    }
  }

  // Calculate the length of remaining word-aligned data
  size_t alignedLength = (length - offset) & ~(ALIGNMENT_SIZE - 1);

  // We have some data destined for an aligned address, write it out
  if (alignedLength > 0U) {
    uint8_t alignedData[OUTPUT_BUFFER_SIZE];
    memset(alignedData, 0UL, sizeof(alignedData));

    while (alignedLength > 0U) {
      size_t currentLength = SL_MIN(alignedLength, (size_t)OUTPUT_BUFFER_SIZE);
      memcpy(alignedData, &data[offset], currentLength);
      if ((lz4ParserContext.parserContext->regionId == 0) && (lz4ParserContext.parserCallbacks->bootloaderCallback != NULL)) {
        lz4ParserContext.parserCallbacks->bootloaderCallback(lz4ParserContext.parserContext->programmingAddress,
                                                             alignedData,
                                                             currentLength,
                                                             0,
                                                             lz4ParserContext.parserContext);
        if (lz4ParserContext.parserContext->retCode != BOOTLOADER_OK) {
          return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
        }
      } else if ((lz4ParserContext.parserContext->regionId != 0) && (lz4ParserContext.parserCallbacks->applicationCallback != NULL)) {
        uint32_t offset;
        offset = lz4ParserContext.parserContext->programmingAddress - lz4ParserContext.parserContext->currentUpdateMemorySection.updateMemorySection.targetAddr;
        lz4ParserContext.parserCallbacks->applicationCallback(offset,
                                                              alignedData,
                                                              currentLength,
                                                              lz4ParserContext.parserContext->regionId,
                                                              lz4ParserContext.parserContext);
        if (lz4ParserContext.parserContext->retCode != BOOTLOADER_OK) {
          return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
        }
      }
      lz4ParserContext.parserContext->programmingAddress += currentLength;
      offset += currentLength;
      alignedLength -= currentLength;
    }
  }

  // We have a leftover partial word
  if (offset < length) {
    // Clear alignment buffer and save the partial word for later
    memset(lz4ParserContext.outputBuffer, 0, OUTPUT_BUFFER_SIZE);
    memcpy(lz4ParserContext.outputBuffer, &data[offset], length - offset);
    lz4ParserContext.outputOffset = length - offset;
  }
  return BOOTLOADER_OK;
}

int32_t gbl_lz4EnterProgTag(ParserContext_t *ctx)
{
  BTL_DEBUG_PRINTLN("Enter compressed tag");

  // Initialize LZ4 parser context
  memset(&lz4ParserContext, 0, sizeof(Lz4ParserContext_t));
  lz4ParserContext.parserContext = ctx;
  lz4ParserContext.firstCall = true;
  return lz4_init(&lz4ParserContext.lz4Context, gbl_lz4ReadMemory);
}

int32_t gbl_lz4ParseProgTag(ParserContext_t *ctx,
                            void *data,
                            size_t length,
                            const BootloaderParserCallbacks_t *callbacks)
{
  uint8_t *dataArray = (uint8_t *)data;
  size_t dataOffset = 0UL;
  (void)ctx;

  if ((callbacks->applicationCallback == NULL) && (callbacks->bootloaderCallback == NULL)) {
    // Nothing to do
    return BOOTLOADER_OK;
  }

  lz4ParserContext.parserCallbacks = callbacks;

  // Attempt to decompress this chunk of data
  int32_t retval = lz4_decompress(&lz4ParserContext.lz4Context,
                                  &dataArray[dataOffset],
                                  length - dataOffset,
                                  gbl_lz4WriteMemory);

  return retval;
}

size_t gbl_lz4NumBytesRequired(ParserContext_t *ctx)
{
  // If this is the first data in the tag, we need a full word to
  // set the programming address correctly
  if (ctx->offsetInTLV == 0) {
    return 4UL;
  } else {
    return 1UL;
  }
}

int32_t gbl_lz4ExitProgTag(ParserContext_t *ctx,
                           const BootloaderParserCallbacks_t *callbacks)
{
  (void) ctx;
  (void) callbacks;
  BTL_DEBUG_PRINTLN("Exit compressed tag");

  // We have cached some unaligned data, pad with 0xFF and push it out
  if (lz4ParserContext.outputOffset != 0UL) {
    size_t paddedSize = (lz4ParserContext.outputOffset + (PAD_SIZE - 1)) & ~(PAD_SIZE - 1);
    for (size_t i = lz4ParserContext.outputOffset; i < paddedSize; i++) {
      lz4ParserContext.outputBuffer[i] = 0xFFU;
    }
    if ((lz4ParserContext.parserContext->regionId == 0) && (callbacks->bootloaderCallback != NULL)) {
      callbacks->bootloaderCallback(lz4ParserContext.parserContext->programmingAddress,
                                    lz4ParserContext.outputBuffer,
                                    paddedSize,
                                    0,
                                    ctx);
      if (lz4ParserContext.parserContext->retCode != BOOTLOADER_OK) {
        return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
      }
    } else if ((lz4ParserContext.parserContext->regionId != 0) && (callbacks->applicationCallback != NULL)) {
      uint32_t addOffset;
      addOffset = lz4ParserContext.parserContext->programmingAddress - lz4ParserContext.parserContext->currentUpdateMemorySection.updateMemorySection.targetAddr;
      callbacks->applicationCallback(addOffset,
                                     lz4ParserContext.outputBuffer,
                                     paddedSize,
                                     lz4ParserContext.parserContext->regionId,
                                     lz4ParserContext.parserContext);
      if (lz4ParserContext.parserContext->retCode != BOOTLOADER_OK) {
        return BOOTLOADER_ERROR_PARSER_UNEXPECTED;
      }
    }
    lz4ParserContext.outputOffset = 0UL;
  }

  return lz4_finish(&lz4ParserContext.lz4Context);
}
