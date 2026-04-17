/***************************************************************************/ /**
 * @file
 * @brief WPS Push Button Configuration (PBC) Example Application Header
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

#ifndef APP_H
#define APP_H

/******************************************************
 *                 Macro Definitions
 ******************************************************/

// WPS Configuration Parameters
#define WPS_TIMEOUT_MS         120000 // 2 minutes timeout for WPS
#define WPS_CONNECTION_TIMEOUT 30000  // 30 seconds for final connection
#define APP_STATUS_DELAY_MS    5000   // Status update interval

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * @brief Initialize the WPS PBC application
 * 
 * This function is called by the system to initialize the application.
 * It creates the main application thread.
 */
void app_init(void);

#endif // APP_H