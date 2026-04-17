/***************************************************************************//**
 * @file token-stack.h
 * @brief Definitions for stack tokens. This file is a stub for the actual
 * token-stack.h file. It is included by common_token_manager_compatibility
 * component when the application does not provide its own token-stack.h file.
 * @version 1.0.0
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
/**
 * @name Convenience Macros
 * @brief The following convenience macros are used to simplify the definition
 * process for commonly specified parameters to the basic TOKEN_DEF macro.
 * See hal/micro/token.h for a more detailed explanation.
 *@{
 */
#define DEFINE_BASIC_TOKEN(name, type, ...) \
  TOKEN_DEF(name, CREATOR_##name, 0, 0, type, 1, __VA_ARGS__)

#define DEFINE_COUNTER_TOKEN(name, type, ...) \
  TOKEN_DEF(name, CREATOR_##name, 1, 0, type, 1, __VA_ARGS__)

/** @} END Convenience Macros */

/**
 * @name Creator Codes
 * @brief The CREATOR is used as a distinct identifier tag for the
 * token.
 *
 * The CREATOR is necessary because the token name is defined
 * differently depending on the hardware platform. Therefore, the CREATOR
 * ensures that token definitions and data stay tagged and known. The only
 * requirement is that each creator definition must be unique.
 * See hal/micro/token.h for a more detailed explanation.
 *@{
 */

// STACK CREATORS
#define CREATOR_USER_TEST_TOKEN_01           0x0002
#define CREATOR_USER_TEST_TOKEN_COUNTER      0x0020

/** @} END Creator Codes  */

/**
 * @name NVM3 Object Keys
 * @brief The NVM3 object key is used as a distinct identifier tag for a
 * token stored in NVM3.
 *
 * Every token must have a defined NVM3 object key and the object key
 * must be unique. The object key defined must be in the following
 * format:
 * NVM3KEY_tokenname
 *
 * where tokenname is the name of the token without NVM3KEY_ or TOKEN_
 * prefix.

 *@{
 */

// NVM3KEY domain base keys
#define NVM3KEY_DOMAIN_USER    0x00000U

// STACK KEYS
#define NVM3KEY_USER_TEST_TOKEN_01      (NVM3KEY_DOMAIN_USER | 0x0002)
#define NVM3KEY_USER_TEST_TOKEN_COUNTER (NVM3KEY_DOMAIN_USER | 0x0020)
/** @} END NVM3 Object Keys  */

#ifdef DEFINETYPES
typedef uint32_t tokTypeTestToken01;
typedef uint32_t tokTypeTestCounter;
#endif //DEFINETYPES

#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(USER_TEST_TOKEN_01,
                   tokTypeTestToken01,
                   0x0000)
DEFINE_COUNTER_TOKEN(USER_TEST_TOKEN_COUNTER,
                     tokTypeTestCounter,
                     0)
#endif //DEFINETOKENS
///////////////////////////////////////////////////////////////////////////////
// APPLICATION DATA

#ifdef APPLICATION_TOKEN_HEADER
  #include APPLICATION_TOKEN_HEADER
#endif
