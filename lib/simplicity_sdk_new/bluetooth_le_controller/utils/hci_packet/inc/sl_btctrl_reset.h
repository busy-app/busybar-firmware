/******************************************************************************
*
* @file  sl_btctrl_reset.h
* @brief Internal helper functions for reset and getting the reset reason,
* these are called internally by the HCI layer.
*
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

#ifndef SL_BTCTRL_RESET_H
#define SL_BTCTRL_RESET_H

#include <stdbool.h>

// Custom reset reason.
#define SL_BTCTRL_RESET_CUSTOM_REASON_NA               0x00000000UL
#define SL_BTCTRL_RESET_CUSTOM_REASON_HCI_COMMAND      (0x1UL << 0)
#define SL_BTCTRL_RESET_CUSTOM_REASON_UNKNOWN          (0x1UL << 31)

#define SL_BTCTRL_RESET_VALID_MASK                     ((0x1UL << 1) - 1)

/**
 * Reset the device
 */
void sl_btctrl_hci_reset(void);

/**
 * @brief Set the custom reset reason that can be read by sl_btctrl_reset_get_custom_reason()
 * API after a system reset.
 * @param[in] reason Custom reset reason.
 */
void sl_btctrl_reset_set_custom_reason(uint32_t reason);

/**
 * @brief Read the custom reset reason that was set by sl_btctrl_reset_set_custom_reason()
 * API before a system reset.
 * @return Custom reset reason.
 */
uint32_t sl_btctrl_reset_get_custom_reason(void);

/**
 * @brief Clear the custom reset reason.
 */
void sl_btctrl_reset_clear_custom_reason(void);

#endif // SL_BTCTRL_RESET_H
