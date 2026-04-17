/***************************************************************************//**
 * @brief ZigBee token management code for host.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

// For ftruncate(2) in glibc.
#define _XOPEN_SOURCE 700

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include "../inc/token.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <sysexits.h>
#include <unistd.h>

uint32_t legacy_token_count = TOKEN_COUNT;

// We use legacy token addresses to detect nonzero sizes, and thus indices in the
// new token system. The new token index system does not register 0-sized tokens,
// so legacy IDs do not match up to new IDs

#define DEFINETOKENS
#define TOKEN_MFG TOKEN_DEF
#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  TOKEN_##name##_ADDRESS,
const uint16_t legacy_token_addresses[] = {
    #include "stack/config/token-stack.h"
};
#undef TOKEN_DEF
#undef TOKEN_MFG
#undef DEFINETOKENS
