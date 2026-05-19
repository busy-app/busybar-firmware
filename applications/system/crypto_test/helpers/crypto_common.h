#pragma once
#include <furi.h>
#include "../crypto_test.h"
#include <furi_hal_crypto.h>

typedef enum {
    CryptoCommonTestResultOk,
    CryptoCommonTestResultOkNoWrap,
    CryptoCommonTestResultFail,
} CryptoCommonTestResult;

#define CRYPTO_COMMON_CHECK_STATUS_EQ(expected, actual, tag)                                    \
    if((expected) != (actual)) {                                                                \
        printf(                                                                                 \
            ANSI_FG_RED "%s: unexpected status (expected = %u, actual = %u)" ANSI_RESET "\r\n", \
            tag,                                                                                \
            expected,                                                                           \
            actual);                                                                            \
        result = false;                                                                         \
        break;                                                                                  \
    }

#define CRYPTO_COMMON_CHECK_STATUS(status, tag)                                                 \
    if((status) != FuriHalCryptoStatusOk) {                                                     \
        printf(ANSI_FG_RED "%s operation failed (status = %u)" ANSI_RESET "\r\n", tag, status); \
        result = false;                                                                         \
        break;                                                                                  \
    }

void crypto_common_print_buffer_char(const char* tag, const uint8_t* buffer, uint16_t length);

void crypto_common_print_buffer_hex(const char* tag, const uint8_t* buffer, uint16_t length);

void crypto_common_print_key(const char* tag, const FuriHalCryptoKey* key);

CryptoCommonTestResult
    crypto_common_test_result_compose(CryptoCommonTestResult a, CryptoCommonTestResult b);
