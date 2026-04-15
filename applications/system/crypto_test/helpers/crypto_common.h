#pragma once
#include <furi.h>
#include "../crypto_test.h"
#include <furi_hal_crypto.h>

#define CRYPTO_COMMON_CHECK_STATUS(status, tag)                                                 \
    if((status) != FuriHalCryptoStatusOk) {                                                     \
        printf(ANSI_FG_RED "%s operation failed (status = %u)" ANSI_RESET "\r\n", tag, status); \
        break;                                                                                  \
    }

void crypto_common_print_buffer_char(const char* tag, const uint8_t* buffer, uint16_t length);

void crypto_common_print_buffer_hex(const char* tag, const uint8_t* buffer, uint16_t length);

void crypto_common_print_key(const char* tag, const FuriHalCryptoKey* key);
