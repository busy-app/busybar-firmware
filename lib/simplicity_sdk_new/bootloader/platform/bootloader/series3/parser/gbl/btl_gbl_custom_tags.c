/***************************************************************************//**
 * @file
 * @brief Custom tags for Gecko Bootloader
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
#include "parser/gbl/btl_gbl_custom_tags.h"

#if defined(BTL_PARSER_SUPPORT_LZ4)
#include "parser/compression/btl_decompress_lz4.h"
#endif

#if defined(BTL_PARSER_SUPPORT_LZMA)
#include "parser/compression/btl_decompress_lzma.h"
#endif

#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
const GblCustomTag_t gblCustomTags[] = {
#if defined(BTL_PARSER_SUPPORT_LZ4)
  {
    .compressionId = COMPRESSION_LZ4,
    .enterTag = gbl_lz4EnterProgTag,
    .parseTag = gbl_lz4ParseProgTag,
    .exitTag = gbl_lz4ExitProgTag,
    .numBytesRequired = gbl_lz4NumBytesRequired
  },
#endif
#if defined(BTL_PARSER_SUPPORT_LZMA)
  {
    .compressionId = COMPRESSION_LZMA,
    .enterTag = gbl_lzmaEnterProgTag,
    .parseTag = gbl_lzmaParseProgTag,
    .exitTag = gbl_lzmaExitProgTag,
    .numBytesRequired = gbl_lzmaNumBytesRequired
  },
#endif
};
#endif

bool gbl_isCustomTag(uint8_t compressionId)
{
#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
  for (size_t i = 0; i < sizeof(gblCustomTags) / sizeof(GblCustomTag_t); i++) {
    if (gblCustomTags[i].compressionId == compressionId) {
      return true;
    }
  }
#else
  (void) compressionId;
#endif
  return false;
}

const GblCustomTag_t * gbl_getCustomTagProperties(uint8_t compressionId)
{
#if defined(BTL_PARSER_SUPPORT_CUSTOM_TAGS)
  for (size_t i = 0; i < sizeof(gblCustomTags) / sizeof(GblCustomTag_t); i++) {
    if (gblCustomTags[i].compressionId == compressionId) {
      return &gblCustomTags[i];
    }
  }
#else
  (void) compressionId;
#endif
  return NULL;
}
