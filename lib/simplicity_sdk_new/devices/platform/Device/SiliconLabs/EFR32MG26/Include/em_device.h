/**************************************************************************//**
 * @file
 * @brief CMSIS Cortex-M Peripheral Access Layer for Silicon Laboratories
 *        microcontroller devices
 *
 * This is a convenience header file for defining the part number on the
 * build command line, instead of specifying the part specific header file.
 *
 * @verbatim
 * Example: Add "-DEFM32G890F128" to your build options, to define part
 *          Add "#include "em_device.h" to your source files

 *
 * @endverbatim
 ******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories, Inc. www.silabs.com</b>
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
#if defined(EFR32MG26B211F2048IM68)
#include "efr32mg26b211f2048im68.h"

#elif defined(EFR32MG26B211F3200IM48)
#include "efr32mg26b211f3200im48.h"

#elif defined(EFR32MG26B221F2048IM68)
#include "efr32mg26b221f2048im68.h"

#elif defined(EFR32MG26B221F3200IM48)
#include "efr32mg26b221f3200im48.h"

#elif defined(EFR32MG26B311F3200IL136)
#include "efr32mg26b311f3200il136.h"

#elif defined(EFR32MG26B410F3200IM48)
#include "efr32mg26b410f3200im48.h"

#elif defined(EFR32MG26B410F3200IM68)
#include "efr32mg26b410f3200im68.h"

#elif defined(EFR32MG26B411F3200IM48)
#include "efr32mg26b411f3200im48.h"

#elif defined(EFR32MG26B411F3200IM68)
#include "efr32mg26b411f3200im68.h"

#elif defined(EFR32MG26B420F3200IM48)
#include "efr32mg26b420f3200im48.h"

#elif defined(EFR32MG26B420F3200IM68)
#include "efr32mg26b420f3200im68.h"

#elif defined(EFR32MG26B421F3200IM48)
#include "efr32mg26b421f3200im48.h"

#elif defined(EFR32MG26B421F3200IM68)
#include "efr32mg26b421f3200im68.h"

#elif defined(EFR32MG26B510F3200IL136)
#include "efr32mg26b510f3200il136.h"

#elif defined(EFR32MG26B510F3200IM48)
#include "efr32mg26b510f3200im48.h"

#elif defined(EFR32MG26B510F3200IM68)
#include "efr32mg26b510f3200im68.h"

#elif defined(EFR32MG26B511F3200IL136)
#include "efr32mg26b511f3200il136.h"

#elif defined(EFR32MG26B511F3200IM48)
#include "efr32mg26b511f3200im48.h"

#elif defined(EFR32MG26B511F3200IM68)
#include "efr32mg26b511f3200im68.h"

#elif defined(EFR32MG26B520F3200IM48)
#include "efr32mg26b520f3200im48.h"

#elif defined(EFR32MG26B520F3200IM68)
#include "efr32mg26b520f3200im68.h"

#elif defined(EFR32MG26B521F3200IM48)
#include "efr32mg26b521f3200im48.h"

#elif defined(EFR32MG26B521F3200IM68)
#include "efr32mg26b521f3200im68.h"

#elif defined(EFR32MG26B610F3200IM48)
#include "efr32mg26b610f3200im48.h"

#elif defined(EFR32MG26B611F2048IM48)
#include "efr32mg26b611f2048im48.h"

#else
#error "em_device.h: PART NUMBER undefined"
#endif

#if defined(SL_CATALOG_TRUSTZONE_SECURE_CONFIG_PRESENT) && defined(SL_TRUSTZONE_NONSECURE)
#error "Can't define SL_CATALOG_TRUSTZONE_SECURE_CONFIG_PRESENT and SL_TRUSTZONE_NONSECURE MACRO at the same time."
#endif

#if defined(SL_TRUSTZONE_SECURE) && defined(SL_TRUSTZONE_NONSECURE)
#error "Can't define SL_TRUSTZONE_SECURE and SL_TRUSTZONE_NONSECURE MACRO at the same time."
#endif
#endif /* EM_DEVICE_H */
