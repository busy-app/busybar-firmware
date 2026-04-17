/***************************************************************************/ /**
 * @file
 * @brief SL SI91X Crypto Utility header file
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#pragma once

#include <stdint.h>
#include <string.h>

/******************************************************
 *                Function Declarations
*******************************************************/
/**
 * @addtogroup CRYPTO_UTILITY_FUNCTIONS 
 * @{ 
 */

/***************************************************************************/
/**
 * @brief 
 *   To perform PKCS #7 padding on the given data.
 * @param[in]  data
 *   Pointer to the data to be padded.
 * @param[in]  data_length
 *   Length of the data in bytes.
 * @param[in]  block_size
 *   Size of the block in bytes (typically 16 for AES).
 * @param[out] padded_data_length        
 *   Pointer to store the length of the padded data.
 * @return
 *   Returns a pointer to the padded data.
 ******************************************************************************/
uint8_t *pkcs7_padding(const uint8_t *data, size_t data_length, size_t block_size, size_t *padded_data_length);

/***************************************************************************/
/**
 * @brief
 *   To unpad a PKCS #7 padded data array in-place, and return the length of the unpadded data. This function performs PKCS #7 unpadding on the input data array. It checks the padding bytes at the end of the array, removes them if they are valid, and returns the length of the unpadded data.   
 * @param[in]  data 
 *   Pointer to the data array containing PKCS #7 padded data.
 * @param[in]  data_length 
 *   Length of the data array.
 * @return
 *   The length of the unpadded data, or -1, if the padding is invalid or data is empty.
 ******************************************************************************/
int pkcs7_unpad(const unsigned char *data, size_t data_length);

/** @} */
