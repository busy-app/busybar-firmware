/******************************************************************************
* @file  aux_reference_volt_config.h
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

// Includes Files

#include "rsi_error.h"
#include "base_types.h"

#ifndef RSI_AUX_REF_VOLT_CONFIG_H
#define RSI_AUX_REF_VOLT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

rsi_error_t RSI_AUX_RefVoltageConfig(float verf_val, float chip_vltg);
#ifdef __cplusplus
}
#endif

#endif // RSI_AUX_REF_VOLT_CONFIG_H
