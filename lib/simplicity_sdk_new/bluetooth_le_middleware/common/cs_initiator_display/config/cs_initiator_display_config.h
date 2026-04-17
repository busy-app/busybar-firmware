/***************************************************************************/ /**
 * @file
 * @brief CS Initiator display configuration.
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
#ifndef CS_INITIATOR_DISPLAY_CONFIG_H
#define CS_INITIATOR_DISPLAY_CONFIG_H

/***********************************************************************************************//**
 * @addtogroup cs_initiator_display
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <q CS_INITIATOR_DISPLAY_LOG> Enable initiator display log
// <i> Default: 1
// <i> Enable Initiator event buffer managers own log
#ifndef CS_INITIATOR_DISPLAY_LOG
#define CS_INITIATOR_DISPLAY_LOG              (1)
#endif

// <o CS_INITIATOR_DISPLAY_GLOBAL_ALIGNMENT> Global display alignment
// <CS_INITIATOR_DISPLAY_ALIGNMENT_LEFT=> Left
// <CS_INITIATOR_DISPLAY_ALIGNMENT_CENTER=> Centered
// <CS_INITIATOR_DISPLAY_ALIGNMENT_RIGHT=> Right
// <i> Default: CS_INITIATOR_DISPLAY_ALIGNMENT_CENTER
#ifndef CS_INITIATOR_DISPLAY_GLOBAL_ALIGNMENT
#define CS_INITIATOR_DISPLAY_GLOBAL_ALIGNMENT CS_INITIATOR_DISPLAY_ALIGNMENT_LEFT
#endif

// <<< end of configuration section >>>

/** @} (end addtogroup cs_initiator_display) */
#endif // CS_INITIATOR_DISPLAY_CONFIG_H
