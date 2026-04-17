/***************************************************************************//**
 * @file
 * @brief CS initiator - error handler header
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
#ifndef CS_INITIATOR_ERROR_H
#define CS_INITIATOR_ERROR_H

// -----------------------------------------------------------------------------
// Includes

#include "sl_status.h"
#include "cs_initiator_client.h"
#include "cs_initiator_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Function declarations

/******************************************************************************
 * Call user error callback
 *
 * @param cs_initiator_t instance reference
 * @param cs_error_event_t initiator error event
 * @param sl_status_t status code
 *****************************************************************************/
void on_error(cs_initiator_t *initiator,
              cs_error_event_t evt,
              sl_status_t      sc);

/******************************************************************************
 * Start error timer of the selected instance
 *
 * @param cs_initiator_t instance reference
 *****************************************************************************/
void start_error_timer(cs_initiator_t *initiator);

/******************************************************************************
 * Stop error timer of the selected instance
 *
 * @param cs_initiator_t instance reference
 *****************************************************************************/
void stop_error_timer(cs_initiator_t *initiator);

#ifdef __cplusplus
}
#endif

#endif // CS_INITIATOR_ERROR_H
