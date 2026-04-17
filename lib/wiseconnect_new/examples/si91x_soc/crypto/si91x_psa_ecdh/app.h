/***************************************************************************/ /**
 * @file app.h
 * @brief Top level application functions.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef APP_H
#define APP_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "psa_ecdh.h"
#include "psa_key.h"
#include <string.h>
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SECPR1_SIZE_MAX     (3)
#define KEY_CURVE_MAX       (1)
#define MONTGOMERY_SIZE_MAX (1)

/// Default key usage is none
#define DEFAULT_KEY_USAGE (0)

/// Persistent client key ID
#define CLIENT_KEY_ID PSA_KEY_ID_USER_MIN

/// Persistent server key ID
#define SERVER_KEY_ID (PSA_KEY_ID_USER_MIN + 1)
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/*****************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void);

/***************************************************************************/ /**
 * Application state machine, called infinitely.
 ******************************************************************************/
void app_process_action(void);

#endif // APP_H
