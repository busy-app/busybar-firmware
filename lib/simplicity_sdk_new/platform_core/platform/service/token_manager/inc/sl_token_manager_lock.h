/***************************************************************************//**
 * @file sl_token_manager_lock.h
 * @brief Token Manager data access lock API definition.
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

#ifndef SL_TOKEN_MANAGER_LOCK_H
#define SL_TOKEN_MANAGER_LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup common_token_manager
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup lock Common Token Manager lock functions
 * @brief This module provides data protection tools for the common token manager.
 * @{
 * @details
 * The functions in this module are the default protection tools for the CTM.
 * The application can substitute the `sl_token_manager_lock_begin` and
 * `sl_token_manager_lock_end` functions, provided that the CTM functions
 * are protected from being called in a re-entrant manner.
 *
 * @note These functions are intended for use by the CTM and should not
 *       be used directly by any applications.
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *  Create a mutex for locking and unlocking a section.
 ******************************************************************************/
void sl_token_manager_lock_create_mutex(void);

/***************************************************************************//**
 * @brief
 *  Begin a lock section.
 ******************************************************************************/
void sl_token_manager_lock_begin(void);

/***************************************************************************//**
 * @brief
 *  End a lock section.
 ******************************************************************************/
void sl_token_manager_lock_end(void);

/** @} (end addtogroup lock) */
/** @} (end addtogroup common_token_manager) */

#ifdef __cplusplus
}
#endif

#endif /* SL_TOKEN_MANAGER_LOCK_H */
