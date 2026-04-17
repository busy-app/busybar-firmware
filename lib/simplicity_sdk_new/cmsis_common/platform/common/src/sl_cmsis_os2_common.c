/***************************************************************************//**
 * @file
 * @brief CMSIS OS2 Common
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

#include <stddef.h>
#include "sl_assert.h"
#include "sl_status.h"
#include "cmsis_os2.h"

/***************************************************************************//**
 * Convert OsStatus from CMSIS-RTOS2 to sl_status type.
 ******************************************************************************/
sl_status_t sl_cmsis_os_convert_status(osStatus_t os_status)
{
  switch (os_status) {
    case osOK:
      return SL_STATUS_OK;
    case osError:
      return SL_STATUS_FAIL;
    case osErrorTimeout:
      return SL_STATUS_TIMEOUT;
    case osErrorResource:
      return SL_STATUS_NOT_AVAILABLE;
    case osErrorParameter:
      return SL_STATUS_INVALID_PARAMETER;
    case osErrorNoMemory:
      return SL_STATUS_NO_MORE_RESOURCE;
    case osErrorISR:
      return SL_STATUS_ISR;
    case osStatusReserved:
    default:
      EFM_ASSERT(0);
      return SL_STATUS_FAIL;
  }
}
