/***************************************************************************//**
 * @file
 * @brief ESL Tag core WEAK implementations of abstract image functionalities.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "sl_common.h"
#include "esl_tag_log.h"
#include "esl_tag_core.h"
#include "esl_tag_image_core.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_OTF_DECOMPRESSOR_PRESENT
#include "otf_decompressor.h"
#endif // SL_CATALOG_OTF_DECOMPRESSOR_PRESENT

SL_WEAK void esl_image_init(void)
{
}

SL_WEAK void esl_image_characteristic_update(void)
{
}

SL_WEAK sl_status_t esl_image_get_data(uint8_t image_index, uint16_t* offset,
                                       uint16_t buf_size, uint8_t *target_buf)
{
  (void)image_index;
  (void)buf_size;
  (void)offset;
  (void)target_buf;

  esl_core_set_last_error(ESL_ERROR_IMAGE_NOT_AVAILABLE);

  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK uint8_t esl_image_get_count(void)
{
  return 0;
}

SL_WEAK void esl_image_reset_storage(void)
{
}

void esl_image_unpack_init(void)
{
#ifdef SL_CATALOG_OTF_DECOMPRESSOR_PRESENT
  // (re)initialize decompressor on start
  otf_unpack_init();
#endif // SL_CATALOG_OTF_DECOMPRESSOR_PRESENT
}

sl_status_t esl_image_unpack_chunk(const void *code,
                                   const size_t chunk_size,
                                   void *data,
                                   size_t data_offset,
                                   size_t data_length,
                                   size_t* decompressed_length)
{
#ifdef SL_CATALOG_OTF_DECOMPRESSOR_PRESENT
  return otf_unpack_chunk(code, chunk_size,
                          data, data_offset, data_length,
                          decompressed_length);
#else
  // check for overflow condition
  if ((data_offset + chunk_size) > data_length) {
    sl_bt_esl_log(ESL_LOG_COMPONENT_RAM_IMAGE,
                  ESL_LOG_LEVEL_ERROR,
                  "Image size overflow!");
    return SL_STATUS_WOULD_OVERFLOW;
  } else {
    // throw an usage assert - only if ESL CLI Test harness component is present
    sl_bt_esl_assert(decompressed_length != NULL);
    // store the data, otherwise
    memcpy(((uint8_t *)data + data_offset), code, chunk_size);
    *decompressed_length = chunk_size;
    return SL_STATUS_OK;
  }
#endif // SL_CATALOG_OTF_DECOMPRESSOR_PRESENT
}
