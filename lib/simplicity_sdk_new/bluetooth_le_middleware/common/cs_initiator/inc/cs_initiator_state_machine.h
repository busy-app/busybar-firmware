/***************************************************************************//**
 * @file
 * @brief CS initiator - state machine header
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
#ifndef CS_INITIATOR_STATE_MACHINE_H
#define CS_INITIATOR_STATE_MACHINE_H

// -----------------------------------------------------------------------------
// Includes

#include "sl_status.h"
#include "cs_initiator_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Event / callback definitions

/**************************************************************************//**
 * Event handler to be called to execute the initiator state machine.
 * @param[in] initiator initiator instance reference
 * @param[in] event event to be processed
 * @param[in] data pointer to the event data
 *
 * @return status of the operation.
 *****************************************************************************/
sl_status_t initiator_state_machine_event_handler(cs_initiator_t *initiator,
                                                  state_machine_event_t      event,
                                                  state_machine_event_data_t *data);

#ifdef __cplusplus
}
#endif

#endif // CS_INITIATOR_STATE_MACHINE_H
