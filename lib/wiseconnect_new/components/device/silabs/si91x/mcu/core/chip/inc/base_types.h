/******************************************************************************
* @file  base_types.h
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

#ifndef __BASE_TYPES_H__
#define __BASE_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <stddef.h>
#include "stdint.h"
#include <stdbool.h>
#include <string.h>
/*****************************************************************************/
/* Global pre-processor symbols/macros                                       */
/*****************************************************************************/
/** @brief
 *    Macro for marking deprecated functions
 *
 *  @details
 *    SL_DEPRECATED_API_WISECONNECT_<RELEASE> is used to mark functions that are
 *    deprecated and should not be used from a given version of WiSeConnect SDK.
 *    The accompanying SL_SUPPRESS_DEPRECATION_WARNINGS_WISECONNECT_<RELEASE>
 *    define can be set to suppress warnings generated when using
 *    deprecated APIs.
 */
#ifndef SL_DEPRECATED_API_WISECONNECT_3_5
#ifdef SL_SUPPRESS_DEPRECATION_WARNINGS_WISECONNECT_3_5
#define SL_DEPRECATED_API_WISECONNECT_3_5
#else
#define SL_DEPRECATED_API_WISECONNECT_3_5 __attribute__((deprecated))
#endif
#endif

/**
 * Macro for marking deprecated functions from WiSeConnect SDK 4.0
 */
#ifndef SL_DEPRECATED_API_WISECONNECT_4_0
#ifdef SL_SUPPRESS_DEPRECATION_WARNINGS_WISECONNECT_4_0
#define SL_DEPRECATED_API_WISECONNECT_4_0
#else
#define SL_DEPRECATED_API_WISECONNECT_4_0 __attribute__((deprecated))
#endif
#endif

#ifndef TRUE
/** Value is true (boolean_t type) */
#define TRUE 1
#endif

#ifndef FALSE
/** Value is false (boolean_t type) */
#define FALSE 0
#endif

#ifndef MIN
/** Returns the minimum value out of two values */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
/** Returns the maximum value out of two values */
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif
/** Returns the dimension of an array */
#define DIM(X) (sizeof(X) / sizeof(X[0]))

#ifndef BIT
#define BIT(x) ((uint32_t)1U << (x))
#endif
typedef enum en { Enable = 1, Disable = 0 } en_t;
/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/** logical datatype (only values are TRUE and FALSE) */
typedef uint8_t boolean_t;

/** single precision floating point number (4 byte) */
typedef float float32_t;

/** double precision floating point number (8 byte) */
typedef double float64_t;

/** ASCCI character for string generation (8 bit) */
typedef char char_t;

/** function pointer type to void/void function */
typedef void (*func_ptr_t)(void);

/** function pointer type to void/uint8_t function */
typedef void (*func_ptr_arg1_t)(uint8_t);

#define RSI_DRIVER_VERSION_MAJOR_MINOR(major, minor) (((major) << 8) | (minor))

/**
\brief Driver Version
 */
typedef struct _RSI_DRIVER_VERSION {
  uint16_t api; ///< API version
  uint16_t drv; ///< Driver version
} RSI_DRIVER_VERSION_M4;

/* General return codes */
#define RSI_DRIVER_OK                0  ///< Operation succeeded
#define RSI_DRIVER_ERROR             -1 ///< Unspecified error
#define RSI_DRIVER_ERROR_BUSY        -2 ///< Driver is busy
#define RSI_DRIVER_ERROR_TIMEOUT     -3 ///< Timeout occurred
#define RSI_DRIVER_ERROR_UNSUPPORTED -4 ///< Operation not supported
#define RSI_DRIVER_ERROR_PARAMETER   -5 ///< Parameter error
#define RSI_DRIVER_ERROR_SPECIFIC    -6 ///< Start of driver specific errors

#define SET_BIT(n) BIT(n)  //((uint32_t)1 << n)
#define CLR_BIT(n) ~BIT(n) //(~((uint32_t)1 << n))

#ifndef STATIC
#define STATIC static
#endif

#ifndef INLINE
#ifdef __CC_ARM
#define INLINE __inline
#else
#define INLINE inline
#endif
#endif

#define ENABLE  1
#define DISABLE 0

#ifdef __cplusplus
}
#endif

#endif /* __BASE_TYPES_H__ */

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/
