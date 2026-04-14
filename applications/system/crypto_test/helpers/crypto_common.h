#pragma once
#include <furi.h>
#include "../crypto_test.h"
#include <furi_hal_crypto.h>

void crypto_common_print_buffer_char(const char* tag, const uint8_t* buffer, uint16_t length);

void crypto_common_print_buffer_hex(const char* tag, const uint8_t* buffer, uint16_t length);

void crypto_common_print_key(const char* tag, const FuriHalCryptoKey* key);
