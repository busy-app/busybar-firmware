/***************************************************************************//**
 * @file errno.h
 * @brief errno standard header
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef ERRNO_H
#define ERRNO_H

#include "errno_error_codes.h"

#ifdef __cplusplus
#if defined(__clang__)
#include <version>
#if defined(_LIBCPP_VERSION)
// Pre-include libc++'s header since it expects to own <error.h>
#include "c++/v1/errno.h"
#endif // LIBCPP
#endif // Clang

extern "C" {
#endif

/*******************************************************************************
 * @addtogroup errno Errno
 * @details Errno contains the definition for errno and the defines for
 * the errno values.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * Macro for errno usage.
 * Can be used to retrieve errno value and to assign errno value since function
 * returns address of variable used to store errno value.
 ******************************************************************************/
#define errno (*sl_errno())

/***************************************************************************//**
 * Errno function used with macro errno.
 *
 * @return  Address of global variable used to store errno value
 ******************************************************************************/
int *sl_errno(void);

/** @} (end addtogroup errno) */

#ifdef __cplusplus
}
#endif

#endif // ERRNO_H
