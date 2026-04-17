/*******************************************************************************
 * @file
 * @brief
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

#include "sli_buffer_manager_fake_function.h"

DEFINE_FFF_GLOBALS

DEFINE_FAKE_VALUE_FUNC(uint32_t, CORE_EnterAtomic);
DEFINE_FAKE_VOID_FUNC1(CORE_ExitAtomic, uint32_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, osKernelGetTickCount);
DEFINE_FAKE_VOID_FUNC5(sli_mem_pool_create, sli_mem_pool_handle_t *, uint32_t, uint32_t, void *, uint32_t);
DEFINE_FAKE_VALUE_FUNC1(void *, sli_mem_pool_alloc, sli_mem_pool_handle_t *);
DEFINE_FAKE_VOID_FUNC2(sli_mem_pool_free, sli_mem_pool_handle_t *, void *);
