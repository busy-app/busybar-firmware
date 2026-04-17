/*******************************************************************************
 * @file
 * @brief Crash handler definitions
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: LicenseRef-MSLA
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#include "micro.h"

// For halPrintCrash*() functions
#ifdef CORTEXM3
#include "cortexm3/diagnostic.h"
#endif

void sl_ot_crash_handler_init(void);

#endif // CRASH_HANDLER_H
