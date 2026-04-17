/***************************************************************************//**
 * @file
 * @brief Bluetooth security header.
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

#include "sl_bt_api.h"
#include "sl_status.h"

// -----------------------------------------------------------------------------
// Enums

enum security_send_confirmation {
  SECURITY_DENY_CONFIRMATION = 0,
  SECURITY_ALLOW_CONFIRMATION,
  SECURITY_PROCESS_STARTED,
  SECURITY_PROCESS_NOT_STARTED
};

// -----------------------------------------------------------------------------
// Public function declarations

/******************************************************************************
 * Bluetooth security manager event handler.
 *
 * @param[in] evt Pointer to the Bluetooth event message.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t on_event_security(const sl_bt_msg_t *evt);

/******************************************************************************
 * Set security configuration flags.
 * This will be used to configure the security manager.
 *****************************************************************************/
void security_set_config_flags(void);

/******************************************************************************
 * Send confirmation for passkey.
 * @return Status of the operation.
 *****************************************************************************/
sl_status_t security_send_confirmation(void);

/******************************************************************************
 * Check if a confirmation is in progress.
 * @return true if confirmation is in progress, false otherwise.
 *****************************************************************************/
bool security_is_confirmation_in_progress(void);

/******************************************************************************
 * Set the confirmation state.
 * @param confirmation Confirmation state to set.
 *****************************************************************************/
void security_set_confirmation(enum security_send_confirmation confirmation);
