/***************************************************************************//**
 * @file sli_wisun_coap_mem.h
 * @brief Wi-SUN CoAP memory handler module
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_WISUN_COAP_MEM_H
#define SLI_WISUN_COAP_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>
#include "sl_wisun_coap_config.h"
#include "sl_component_catalog.h"
#include "sl_common.h"

/**************************************************************************//**
 * @defgroup SL_WISUN_COAP_MEMORY Memory Handler
 * @ingroup SL_WISUN_COAP_API
 * @{
 *****************************************************************************/

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Memory pool setup
#if SL_WISUN_COAP_MEM_USE_STATIC_MEMORY_POOL

/// Count of static memory pool options
#define WISUN_COAP_MEMORY_OPTION_COUNT     (4U)

/// Very low memory request ID
#define WISUN_COAP_MEMORY_VERY_LOW_ID      (1000U)
/// Very low memory request size
#define WISUN_COAP_MEMORY_VERY_LOW_SIZE    (32U)
/// Very low memory request max count
#define WISUN_COAP_MEMORY_VERY_LOW_COUNT   (30U)

/// Low memory request ID
#define WISUN_COAP_MEMORY_LOW_ID           (1001U)
/// Low memory requests size
#define WISUN_COAP_MEMORY_LOW_SIZE         (64U)
/// Low memory requests max count
#define WISUN_COAP_MEMORY_LOW_COUNT        (15U)

/// Medium memory request ID
#define WISUN_COAP_MEMORY_MEDIUM_ID        (1002U)
/// Medium memory request size
#define WISUN_COAP_MEMORY_MEDIUM_SIZE      (128U)
/// Medium memory request max count
#define WISUN_COAP_MEMORY_MEDIUM_COUNT     (5U)

/// High memory requests ID
#define WISUN_COAP_MEMORY_HIGH_ID          (1003U)
/// High memory requests size
#define WISUN_COAP_MEMORY_HIGH_SIZE        (256U)
/// High memory requests max count
#define WISUN_COAP_MEMORY_HIGH_COUNT       (2U)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Initialize CoAP component memory management
 * @details
 * @deprecated As sli_wisun_coap_mem_malloc and sli_wisun_coap_mem_free are not used anymore,
 *             sli_wisun_coap_mem_init is also deprecated
 *****************************************************************************/
void sli_wisun_coap_mem_init(void) SL_DEPRECATED_API_SDK_2025_6;

/**************************************************************************//**
 * @brief Wi-SUN CoAP malloc for internal usage
 * @details Not thread safe. Only used in sl_wisun_coap.c
 * @param size size to alloc
 * @return void* ptr to the allocated memory, on error NULL
 * @deprecated The only usage of the function is in sl_wisun_coap_malloc, which is deprecated.
 *****************************************************************************/
void *sli_wisun_coap_mem_malloc(size_t size) SL_DEPRECATED_API_SDK_2025_6;

/**************************************************************************//**
 * @brief Wi-SUN CoAP free memory for internal usage
 * @details Not thread safe. Only used in sl_wisun_coap.c
 * @param addr address
 * @deprecated The only usage of the function is in sl_wisun_coap_free, which is deprecated.
 *****************************************************************************/
void sli_wisun_coap_mem_free(void *addr) SL_DEPRECATED_API_SDK_2025_6;

/** @}*/

#ifdef __cplusplus
}
#endif

#endif // SLI_WISUN_COAP_MEM_H
