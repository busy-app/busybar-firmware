#pragma once
#include <furi.h>
#include "crypto_test_app.h"

void crypto_common_print_buffer_char(
    CryptoTestApp* app,
    FuriString* msg,
    char* tag,
    uint8_t* buffer,
    uint16_t length);

void crypto_common_print_buffer_hex(
    CryptoTestApp* app,
    FuriString* msg,
    char* tag,
    const uint8_t* buffer,
    uint16_t length);
