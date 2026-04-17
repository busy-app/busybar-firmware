/***************************************************************************/ /**
 * @file psa_macro.h
 * @brief PSA Crypto macros.
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
#ifndef PSA_MACRO_H
#define PSA_MACRO_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "psa/crypto.h"
#include <inttypes.h>
#include <stdio.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define print_error_cycle(CODE)     \
  do {                              \
    psa_status_t ret;               \
                                    \
    ret = CODE;                     \
    if (ret == PSA_SUCCESS) {       \
      printf("OK\n");               \
    } else {                        \
      printf("Failed: %ld\n", ret); \
    }                               \
    return ret;                     \
  } while (0)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

#endif // PSA_MACRO_H
