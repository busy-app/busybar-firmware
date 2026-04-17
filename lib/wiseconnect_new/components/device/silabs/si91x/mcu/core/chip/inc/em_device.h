/**************************************************************************/ /**
 * @file em_device.h
 * @brief Device-specific header file inclusion based on the defined part number.
 *
 * This section of the code includes the appropriate device-specific header files
 * based on the defined part number. If the part number is not defined, it generates
 * a preprocessor error.
 *
 * @verbatim
 * Example: Add "-DEFM32G890F128" to your build options, to define part
 *          Add "#include "em_device.h" to your source files

 *
 * @endverbatim
 ******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
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
 *****************************************************************************/

#ifndef EM_DEVICE_H
#define EM_DEVICE_H

#define EXT_IRQ_COUNT SI91X_EXT_IRQ_COUNT

#if defined(SLI_SI917)
#include "si91x_device.h"
#include "RTE_Device_917.h"
#else
#error "em_device.h: PART NUMBER undefined"
#endif /* SLI_SI917  */

#endif /* EM_DEVICE_H */
