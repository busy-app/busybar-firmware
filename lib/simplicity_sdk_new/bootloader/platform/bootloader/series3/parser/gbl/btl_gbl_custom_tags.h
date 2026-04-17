/***************************************************************************//**
 * @file
 * @brief Custom tags for Gecko Bootloader GBL parser
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
#ifndef BTL_GBL_CUSTOM_TAGS_H
#define BTL_GBL_CUSTOM_TAGS_H

#include "parser/gbl/btl_gbl_parser.h"
#include "parser/gbl/btl_gbl_format.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @addtogroup ComponentsS3 Component
 * @{
 * @addtogroup ImageParserS3
 * @{
 * @addtogroup GblParserS3
 * @{
 * @addtogroup CustomTagsS3 Custom GBL Tags
 * @brief Handle custom GBL tags for added functionality in the GBL parser
 * @details
 * @{
 */

// -----------------------------------------------------------------------------
// Structs

/// Custom tag descriptor
typedef struct {
  /// GBL compression ID of the custom tag
  uint8_t compressionId;
  /// Function to call upon entering the tag
  int32_t (*enterTag)(ParserContext_t *ctx);
  /// Function to call while parsing the tag
  int32_t (*parseTag)(ParserContext_t *ctx,
                      void *data,
                      size_t length,
                      const BootloaderParserCallbacks_t *callbacks);
  /// Function to call upon exiting the tag
  int32_t (*exitTag)(ParserContext_t *ctx,
                     const BootloaderParserCallbacks_t *callbacks);
  /// @brief Function returning how many bytes should be collected before
  ///        calling parseTag the next time
  size_t (*numBytesRequired)(ParserContext_t *ctx);
} GblCustomTag_t;

// -----------------------------------------------------------------------------
// Functions

/***************************************************************************//**
 * Indicate whether the compression ID is a custom tag.
 * @param compressionId Id of compression scheme
 *
 * @return True if the id is a custom tag, else false
 ******************************************************************************/
bool gbl_isCustomTag(uint8_t compressionId);

/***************************************************************************//**
 * Get properties for a custom GBL tag.
 * @param @param compressionId Id of compression scheme
 *
 * @return Pointer to the custom tag descriptor
 ******************************************************************************/
const GblCustomTag_t * gbl_getCustomTagProperties(uint8_t compressionId);

/** @} addtogroup CustomTags */
/** @} addtogroup GblParser */
/** @} addtogroup ImageParser */
/** @} addtogroup Components */

#endif // BTL_GBL_CUSTOM_TAGS_H
