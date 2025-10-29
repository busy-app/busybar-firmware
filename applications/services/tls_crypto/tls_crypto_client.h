#pragma once

#include <furi.h>

bool tls_crypto_client_sign(
    uint8_t key_slot,
    const uint8_t* hash,
    size_t hash_len,
    uint8_t* sign_buf,
    size_t sign_buf_size,
    size_t* sign_len);
