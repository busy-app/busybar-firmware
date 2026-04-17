/***************************************************************************//**
 * @file sl_custom_token_header.h
 * @brief Custom token header
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

/***************************************************************************//**
 * Custom Tokens Usage Examples
 *
 * User can create custom nvm3 tokens as shown below. Indexed tokens should be handled by application.
 *
 * #define TOKEN_BASICTOKEN1      SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_USER | 0x0001), 0)
 * #define TOKEN_BASICTOKEN2      SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_USER | 0x0002), 0)
 * #define TOKEN_COUNTERTOKEN1    SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_USER | 0x0010), 1)
 * #define TOKEN_COUNTERTOKEN2    SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_USER | 0x0020), 1)
 *
 * Refer sl_token_manager_defines.h file for SL_TOKEN_GET_DYNAMIC_TOKEN define
 * and MACRO's for NVM3 Default Instance Key Space Regions.
 *
 * User can create a token size as shown below.
 *
 * typedef uint16_t tokTypeBasicToken1;
 * typedef struct {
 *   uint8_t basicToken2Array[3];
 *   uint16_t basicToken2VarA;
 * } tokTypeBasicToken2;
 *
 * typedef uint32_t tokTypeCounterToken1;
 * typedef uint32_t tokTypeCounterToken2;
 *
 * #define TOKEN_BASICTOKEN1_SIZE       (sizeof(tokTypeBasicToken1))
 * #define TOKEN_BASICTOKEN2_SIZE       (sizeof(tokTypeBasicToken2))
 * #define TOKEN_COUNTERTOKEN1_SIZE     (sizeof(tokTypeCounterToken1))
 * #define TOKEN_COUNTERTOKEN2_SIZE     (sizeof(tokTypeCounterToken2))
 *
 * This code will read the token data:
 * Refer sl_token_manager_api.h file, for sl_token_manager_get_data() API usage
 *     uint8_t data[TOKEN_BASICTOKEN1_SIZE];
 *     sl_token_manager_get_data(TOKEN_BASICTOKEN1, &data, sizeof(data));
 *
 *********************************************************************************
 * For backward compatibility, the legacy token header file is still available, for
 * silicon labs series 2 devices.
 *
 * This format is deprecated and will be removed in the future.
 *
 * Custom Tokens Usage Examples
 *
 * #define BASICTOKEN1_DEFAULT                                            0xA5F0
 * #define BASICTOKEN2_DEFAULT                  { { 0xAA, 0xBB, 0xCC }, 0xDDDD }
 * #define BASICTOKEN3_DEFAULT                                         { { 0 } }
 * #define COUNTERTOKEN1_DEFAULT                                               0
 * #define COUNTERTOKEN2_DEFAULT                                      0xCCCCCCCC
 * #define INDEXEDTOKEN1_DEFAULT                                               0
 * #define INDEXEDTOKEN2_DEFAULT                                      { 0xDDDD }
 * #define INDEXEDTOKEN3_DEFAULT    { 0xAA, 0xBBBB, { 0x00, 0x11, 0x22 }, 0xCC }
 *
 * #ifdef DEFINETYPES
 * typedef uint16_t tokTypeBasicToken1;
 * typedef struct {
 *   uint8_t basicToken2Array[3];
 *   uint16_t basicToken2VarA;
 * } tokTypeBasicToken2;
 *
 * typedef struct {
 *   uint8_t basicToken3Array[254];
 * } tokTypeBasicToken3;
 *
 * typedef uint32_t tokTypeCounterToken1;
 * typedef uint32_t tokTypeCounterToken2;
 *
 * typedef uint8_t  tokTypeIndexedToken1Element;
 * typedef uint16_t tokTypeIndexedToken2Element;
 *
 * typedef struct {
 *   uint8_t indexedToken3VarA;
 *   uint16_t indexedToken3VarB;
 *   uint8_t indexedToken3Array[3];
 *   int8_t indexedToken3VarC;
 * } tokTypeIndexedToken3Element;
 *
 * #endif
 *
 * #ifdef DEFINETOKENS
 * DEFINE_BASIC_TOKEN(BASICTOKEN1,
 *                    tokTypeBasicToken1,
 *                    BASICTOKEN1_DEFAULT)
 * DEFINE_BASIC_TOKEN(BASICTOKEN2,
 *                    tokTypeBasicToken2,
 *                    BASICTOKEN2_DEFAULT)
 * DEFINE_BASIC_TOKEN(BASICTOKEN3,
 *                    tokTypeBasicToken3,
 *                    BASICTOKEN3_DEFAULT)
 * DEFINE_COUNTER_TOKEN(COUNTERTOKEN1,
 *                      tokTypeCounterToken1,
 *                      COUNTERTOKEN1_DEFAULT)
 * DEFINE_COUNTER_TOKEN(COUNTERTOKEN2,
 *                      tokTypeCounterToken2,
 *                      COUNTERTOKEN2_DEFAULT)
 * DEFINE_INDEXED_TOKEN(INDEXEDTOKEN1,
 *                      tokTypeIndexedToken1Element,
 *                      INDEXEDTOKEN1_ELEMENTS,
 *                      INDEXEDTOKEN1_DEFAULT)
 * DEFINE_INDEXED_TOKEN(INDEXEDTOKEN2,
 *                      tokTypeIndexedToken2Element,
 *                      INDEXEDTOKEN2_ELEMENTS,
 *                      INDEXEDTOKEN2_DEFAULT)
 * DEFINE_INDEXED_TOKEN(INDEXEDTOKEN3,
 *                      tokTypeIndexedToken3Element,
 *                      INDEXEDTOKEN3_ELEMENTS,
 *                      INDEXEDTOKEN3_DEFAULT)
 * #endif
 *
 ******************************************************************************/
