/***************************************************************************//**
 * @file
 * @brief Sleep Timer configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_MAIN_START_TASK_CONFIG_H
#define SL_MAIN_START_TASK_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> sl_main start task settings

// <o SL_MAIN_START_TASK_STACK_SIZE_BYTES> Size of the start task stack
// <i> Default: 4096
#define SL_MAIN_START_TASK_STACK_SIZE_BYTES  4096

// <e> Enable start task priority change after initialization
// <i> This is useful for applications that keep the start task during the lifetime of the application.
// <i> Note that the priority change occurs at the end of 'sl_main_second_stage_init()' before 'app_init()' is called.
// <i> Default: Disabled
#define SL_MAIN_ENABLE_START_TASK_PRIORITY_CHANGE    0

// <o SL_MAIN_START_TASK_PRIORITY> Priority that the start task will take after initialization is done.
// <osPriorityLow           => Priority: low
// <osPriorityLow1          => Priority: low + 1
// <osPriorityLow2          => Priority: low + 2
// <osPriorityLow3          => Priority: low + 3
// <osPriorityLow4          => Priority: low + 4
// <osPriorityLow5          => Priority: low + 5
// <osPriorityLow6          => Priority: low + 6
// <osPriorityLow7          => Priority: low + 7
// <osPriorityBelowNormal   => Priority: below normal
// <osPriorityBelowNormal1  => Priority: below normal + 1
// <osPriorityBelowNormal2  => Priority: below normal + 2
// <osPriorityBelowNormal3  => Priority: below normal + 3
// <osPriorityBelowNormal4  => Priority: below normal + 4
// <osPriorityBelowNormal5  => Priority: below normal + 5
// <osPriorityBelowNormal6  => Priority: below normal + 6
// <osPriorityBelowNormal7  => Priority: below normal + 7
// <osPriorityNormal        => Priority: normal
// <osPriorityNormal1       => Priority: normal + 1
// <osPriorityNormal2       => Priority: normal + 2
// <osPriorityNormal3       => Priority: normal + 3
// <osPriorityNormal4       => Priority: normal + 4
// <osPriorityNormal5       => Priority: normal + 5
// <osPriorityNormal6       => Priority: normal + 6
// <osPriorityNormal7       => Priority: normal + 7
// <osPriorityAboveNormal   => Priority: above normal
// <osPriorityAboveNormal1  => Priority: above normal + 1
// <osPriorityAboveNormal2  => Priority: above normal + 2
// <osPriorityAboveNormal3  => Priority: above normal + 3
// <osPriorityAboveNormal4  => Priority: above normal + 4
// <osPriorityAboveNormal5  => Priority: above normal + 5
// <osPriorityAboveNormal6  => Priority: above normal + 6
// <osPriorityAboveNormal7  => Priority: above normal + 7
// <osPriorityHigh          => Priority: high
// <osPriorityHigh1         => Priority: high + 1
// <osPriorityHigh2         => Priority: high + 2
// <osPriorityHigh3         => Priority: high + 3
// <osPriorityHigh4         => Priority: high + 4
// <osPriorityHigh5         => Priority: high + 5
// <osPriorityHigh6         => Priority: high + 6
// <osPriorityHigh7         => Priority: high + 7
// <osPriorityRealtime      => Priority: realtime
// <osPriorityRealtime1     => Priority: realtime + 1
// <osPriorityRealtime2     => Priority: realtime + 2
// <osPriorityRealtime3     => Priority: realtime + 3
// <osPriorityRealtime4     => Priority: realtime + 4
// <osPriorityRealtime5     => Priority: realtime + 5
// <osPriorityRealtime6     => Priority: realtime + 6
// <osPriorityRealtime7     => Priority: realtime + 7
// <i> Default: osPriorityNormal
#define SL_MAIN_START_TASK_PRIORITY osPriorityNormal

//</e>
//</h>

#endif /* SL_MAIN_START_TASK_CONFIG_H */

// <<< end of configuration section >>>
