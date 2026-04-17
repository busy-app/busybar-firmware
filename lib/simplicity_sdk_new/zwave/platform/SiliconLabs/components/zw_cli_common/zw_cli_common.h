/***************************************************************************//**
 * @file zw_cli_common.h
 * @brief zwave cli common component
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef ZW_CLI_COMMON_H
#define ZW_CLI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "events.h"

/*******************************************************************************
 *** DEFINES
 ******************************************************************************/

/*******************************************************************************
 *** MACROS AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 *** PUBLIC FUNCTIONS
 ******************************************************************************/

void cli_log_system_events(EVENT_SYSTEM event);

#ifdef __cplusplus
}
#endif

#endif // ZW_CLI_COMMON_H
