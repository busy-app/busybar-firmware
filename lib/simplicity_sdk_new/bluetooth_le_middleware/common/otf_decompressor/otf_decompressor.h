/***************************************************************************//**
 * @file
 * @brief On-the-fly LZRW1-derivative stream decompressor API declaration
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef OTF_DECOMPRESSOR_H
#define OTF_DECOMPRESSOR_H
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stddef.h>
#include "otf_compressor_config.h"
#include "sl_status.h"

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * Init on-the-fly unpacker.
 *
 * Reset the decompressor by this invocation every time a new unpack is started.
 *****************************************************************************/
void otf_unpack_init(void);

/**************************************************************************//**
 * Unpack single chunk of compressed data.
 *
 * @param *code Pointer to the current incoming compressed data chunk
 * @param chunk_size Size of the incoming data in bytes
 * @param *data Pointer to the target area of unpacking
 * @param data_offset Current write offset from the start of the unpacking area
 * @param data_length Maximum size of the unpacking area
 * @param decompressed_length Decompressed size of last chunk
 * @returns Returns SL_STATUS_OK or SL_STATUS_FAIL depending on the result
 *
 * Unpacks data slices of LZJB-compressed data that has been compressed with
 * compatible settings to the specified target area, respecting the size limit.
 * Can also decompress a whole file if all the compressed data is passed at
 * once, or even if the bytes are passed one at a time. The chunk size can
 * also vary between calls.
 *
 * @note The decompressor does not perform any sanity checks on the incoming
 *       data, so it can easily decompress junk data into more junk. The only
 *       thing it does not allow is writing over the target size.
 *       On chunks smaller than 4 bytes it is sometimes expected to have
 *       a decompressed length of just 0 with a SL_STATUS_OK return value.
 *       Call @ref otf_unpack_init() before each new decompressing session.
 *       The expected end of the compressed stream can be determined e.g. by
 *       negotiating the expected uncompressed size before unpacking, but
 *       this is outside the scope of the decompressor algorithm.
 *****************************************************************************/
sl_status_t otf_unpack_chunk(const void *code,
                             const size_t chunk_size,
                             void *data,
                             size_t data_offset,
                             size_t data_length,
                             size_t* decompressed_length);

#endif // OTF_DECOMPRESSOR_H
