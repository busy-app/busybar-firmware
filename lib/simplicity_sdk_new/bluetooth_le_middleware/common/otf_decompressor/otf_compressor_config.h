/***************************************************************************//**
 * @file
 * @brief On-the-fly LZRW1-derivative stream compressor configuretion
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
#ifndef OTF_COMPRESSOR_CONFIG_H
#define OTF_COMPRESSOR_CONFIG_H
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// <<< Use Configuration Wizard in Context Menu >>>

// <h> ESL Tag Image in-air compressor config

// <o ESL_LIB_COMPRESSION_LEVEL> Compression level [exponent]<2-17>
// <i> This is an exponent for the Lempel window size: the higher it is the smaller the compressed result can be.
// <i> Each subsequent value doubles the amount of RAM used for compression and make the process slightly slower.
// <i> For ESLs the compression is done at the AP if the component is added to the ESL project.
// <i> Decompression performance does not depend on level: it doesn't affect embedded RAM usage or CPU load.
// <i> Default: 12
#define OTF_CFG_COMPRESSION_LEVEL     12

// </e>

// </h>

// <<< end of configuration section >>>

// Bit count helper definition for bytes
#define OTF_CFG_BYTE_WIDTH            8
// Bit count helper definition for words
#define OTF_CFG_WORD_WIDTH            (2 * OTF_CFG_BYTE_WIDTH)

// On-the-fly decompressor Lempel-Ziv parameters for compatible compressor - do not edit these macros below!
// To create a decompressor-compatible stream, use these values in the compressor.
// For a reference implementation of a compatible compressor algorithm written in Python, see the AirCompressor class
// in the air_compressor.py file of the ESL AP Python example.
#define OTF_CFG_PEER_BITS             (OTF_CFG_BYTE_WIDTH - 3)
#define OTF_CFG_PEER_MIN              (OTF_CFG_BYTE_WIDTH - 5)
#define OTF_CFG_PEER_MAX              ((1 << OTF_CFG_PEER_BITS) + (OTF_CFG_PEER_MIN - 1))
#define OTF_CFG_OFFSET_MASK           ((1 << (OTF_CFG_WORD_WIDTH - OTF_CFG_PEER_BITS)) - 1)
#define OTF_CFG_PATTERN_MASK_INIT     (1 << (OTF_CFG_BYTE_WIDTH - 1))
#define OTF_CFG_PATTERN_MASK_LIMIT    (1 << OTF_CFG_BYTE_WIDTH)
#define OTF_CFG_PATTERN_SHIFT_COUNT   (OTF_CFG_BYTE_WIDTH - OTF_CFG_PEER_BITS)
#define OTF_CFG_HASH_WINDOW_SIZE      (1 << OTF_CFG_COMPRESSION_LEVEL)

#endif // OTF_COMPRESSOR_CONFIG_H
