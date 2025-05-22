#include "crypto_hmac.h"
#include "crypto_common.h"

#include <furi_hal_crypto.h>

#define TAG "HMAC"

static const uint8_t key_hmac[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                   0xaa, 0xaa, 0xaa, 0xaa};

static const uint8_t message[] = {0x54, 0x65, 0x73, 0x74, 0x20, 0x55, 0x73, 0x69, 0x6e, 0x67, 0x20,
                                  0x4c, 0x61, 0x72, 0x67, 0x65, 0x72, 0x20, 0x54, 0x68, 0x61, 0x6e,
                                  0x20, 0x42, 0x6c, 0x6f, 0x63, 0x6b, 0x2d, 0x53, 0x69, 0x7a, 0x65,
                                  0x20, 0x4b, 0x65, 0x79, 0x20, 0x2d, 0x20, 0x48, 0x61, 0x73, 0x68,
                                  0x20, 0x4b, 0x65, 0x79, 0x20, 0x46, 0x69, 0x72, 0x73, 0x74};

// Expected output for the given input from HMAC engine
static const uint8_t digest_output_sha1[FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE] = {
    0x07, 0x0a, 0x98, 0x99, 0x2c, 0x4c, 0x1a, 0x83, 0x47, 0x4c,
    0xb7, 0x80, 0xfc, 0x56, 0x46, 0x08, 0xdf, 0x3c, 0xf5, 0x03};
static const uint8_t digest_output_sha256[FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE] = {
    0x84, 0x33, 0x2a, 0x75, 0x80, 0xed, 0x3c, 0xf7, 0x5d, 0xe8, 0x3c,
    0x64, 0x4c, 0x8d, 0x2c, 0x1c, 0x26, 0x2a, 0xd9, 0x0e, 0x01, 0x90,
    0xe5, 0xc5, 0xae, 0x4b, 0x82, 0xb2, 0x10, 0x2e, 0x8e, 0x75};

static const uint8_t digest_output_sha384[FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE] = {
    0x30, 0x67, 0x92, 0xdd, 0x87, 0xe9, 0x9a, 0x24, 0xd7, 0xfd, 0x2f, 0x8f,
    0xee, 0x29, 0x50, 0xf9, 0x72, 0x5f, 0x6b, 0xb1, 0xc7, 0x63, 0x63, 0xdc,
    0x02, 0x77, 0x65, 0xab, 0x8e, 0xcf, 0x0e, 0x22, 0x49, 0x8c, 0xbc, 0x4d,
    0x61, 0x8f, 0x8b, 0x7f, 0x1e, 0xde, 0xe0, 0x8c, 0x37, 0x52, 0x2c, 0xc7};

static const uint8_t digest_output_sha512[FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE] = {
    0xC8, 0x75, 0xFD, 0x3B, 0xDA, 0xED, 0xF8, 0x41, 0xDB, 0xA9, 0x29, 0x22, 0x3C,
    0xFE, 0x23, 0xEF, 0xCB, 0x3A, 0x3C, 0xC8, 0x6C, 0x07, 0x91, 0x58, 0x90, 0x2B,
    0xAF, 0xE2, 0x81, 0x29, 0x07, 0x5C, 0xC4, 0x32, 0x65, 0x10, 0x04, 0xE4, 0xCC,
    0x5D, 0x17, 0x54, 0x26, 0x51, 0x67, 0x4D, 0xFC, 0xE0, 0xFC, 0x29, 0xBF, 0x0A,
    0x01, 0x06, 0x28, 0xD9, 0x9A, 0x62, 0xC3, 0x1B, 0x0C, 0x91, 0x54, 0x98};

void crypto_hmac_chek(
    CryptoTestApp* app,
    FuriString* msg,
    char* tag,
    const uint8_t* digest,
    const uint8_t* digest_out,
    size_t digest_length) {
    crypto_common_print_buffer_hex(app, msg, "Message =\t\t", (uint8_t*)message, sizeof(message));
    crypto_common_print_buffer_hex(app, msg, "Digest =\t\t", digest, digest_length);
    crypto_common_print_buffer_hex(app, msg, "Expected =\t", digest_out, digest_length);
    if(memcmp(digest, digest_out, digest_length) != 0) {
        furi_string_printf(msg, "\033[0;31m %s mode failed\033[0m\r\n", tag);
        crypto_test_app_send_text(app, msg);
    } else {
        furi_string_printf(msg, "\033[0;32m %s mode success\033[0m\r\n", tag);
        crypto_test_app_send_text(app, msg);
    }
}

void crypto_hmac_test_custom_sha_mode(
    CryptoTestApp* app,
    FuriString* msg,
    FuriHalCryptoHmacShaMode hmac_sha_mode) {
    // Buffer to store response
    uint8_t digest[FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE] = {0};
    size_t digest_length = 0;

    switch(hmac_sha_mode) {
    case FuriHalCryptoHmacShaModeSha1:
        digest_length = FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE;
        break;
    case FuriHalCryptoHmacShaModeSha256:
        digest_length = FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE;
        break;
    case FuriHalCryptoHmacShaModeSha384:
        digest_length = FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE;
        break;
    case FuriHalCryptoHmacShaModeSha512:
        digest_length = FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE;
        break;

    default:
        break;
    }

    // Initialize HMAC
    FuriHalCryptoHmac* handle = furi_hal_crypto_hmac_init(
        hmac_sha_mode, (uint8_t*)key_hmac, sizeof(key_hmac), FuriHalCryptoWrappingModeOff);
    // Compute HMAC digest
    furi_hal_crypto_hmac_digest(handle, (uint8_t*)message, sizeof(message), digest, digest_length);
    // Deinitialize HMAC
    furi_hal_crypto_hmac_deinit(handle);

    switch(hmac_sha_mode) {
    case FuriHalCryptoHmacShaModeSha1:
        crypto_hmac_chek(
            app,
            msg,
            "HMAC_SHA1",
            digest,
            digest_output_sha1,
            FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha256:
        crypto_hmac_chek(
            app,
            msg,
            "HMAC_SHA256",
            digest,
            digest_output_sha256,
            FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha384:
        crypto_hmac_chek(
            app,
            msg,
            "HMAC_SHA384",
            digest,
            digest_output_sha384,
            FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha512:
        crypto_hmac_chek(
            app,
            msg,
            "HMAC_SHA512",
            digest,
            digest_output_sha512,
            FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE);
        break;
    default:
        break;
    }
}

void crypto_hmac_test_custom_sha_mode_wrap_on(
    CryptoTestApp* app,
    FuriString* msg,
    FuriHalCryptoHmacShaMode hmac_sha_mode) {
    // Buffer to store response
    uint8_t digest[FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE] = {0};
    size_t digest_length = 0;

    switch(hmac_sha_mode) {
    case FuriHalCryptoHmacShaModeSha1:
        digest_length = FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE;
        break;
    case FuriHalCryptoHmacShaModeSha256:
        digest_length = FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE;
        break;
    case FuriHalCryptoHmacShaModeSha384:
        digest_length = FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE;
        break;
    case FuriHalCryptoHmacShaModeSha512:
        digest_length = FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE;
        break;

    default:
        break;
    }

    size_t wrap_key_size = 256;
    uint8_t wrapped_key[256] = {0};
    furi_hal_crypto_hmac_wrap_key(
        sizeof(key_hmac), (uint8_t*)key_hmac, hmac_sha_mode, wrapped_key, &wrap_key_size);
    crypto_common_print_buffer_hex(app, msg, "Key =\t\t", key_hmac, sizeof(key_hmac));
    furi_string_printf(msg, "Wrapped key size = %d\r\n", wrap_key_size);
    crypto_test_app_send_text(app, msg);
    crypto_common_print_buffer_hex(app, msg, "Wrapped key =\t", wrapped_key, wrap_key_size);

    // Initialize HMAC
    FuriHalCryptoHmac* handle = furi_hal_crypto_hmac_init(
        hmac_sha_mode, wrapped_key, wrap_key_size, FuriHalCryptoWrappingModeOn);
    // Compute HMAC digest
    furi_hal_crypto_hmac_digest(handle, (uint8_t*)message, sizeof(message), digest, digest_length);
    // Deinitialize HMAC
    furi_hal_crypto_hmac_deinit(handle);

    switch(hmac_sha_mode) {
    case FuriHalCryptoHmacShaModeSha1:
        crypto_hmac_chek(
            app,
            msg,
            "HMAC_SHA1",
            digest,
            digest_output_sha1,
            FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha256:
        crypto_hmac_chek(
            app,
            msg,
            "HMAC_SHA256",
            digest,
            digest_output_sha256,
            FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha384:
        crypto_hmac_chek(
            app,
            msg,
            "HMAC_SHA384",
            digest,
            digest_output_sha384,
            FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha512:
        crypto_hmac_chek(
            app,
            msg,
            "HMAC_SHA512",
            digest,
            digest_output_sha512,
            FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE);
        break;
    default:
        break;
    }
}

void crypto_hmac_test(CryptoTestApp* app, FuriString* msg) {
    furi_string_printf(msg, "\033[0;33mHMAC key wrap off test\033[0m\r\n");
    crypto_test_app_send_text(app, msg);
    crypto_hmac_test_custom_sha_mode(app, msg, FuriHalCryptoHmacShaModeSha1);
    crypto_hmac_test_custom_sha_mode(app, msg, FuriHalCryptoHmacShaModeSha256);
    crypto_hmac_test_custom_sha_mode(app, msg, FuriHalCryptoHmacShaModeSha384);
    crypto_hmac_test_custom_sha_mode(app, msg, FuriHalCryptoHmacShaModeSha512);

    furi_string_printf(msg, "\033[0;33mHMAC key wrap on test\033[0m\r\n");
    crypto_test_app_send_text(app, msg);
    crypto_hmac_test_custom_sha_mode_wrap_on(app, msg, FuriHalCryptoHmacShaModeSha1);
    crypto_hmac_test_custom_sha_mode_wrap_on(app, msg, FuriHalCryptoHmacShaModeSha256);
    crypto_hmac_test_custom_sha_mode_wrap_on(app, msg, FuriHalCryptoHmacShaModeSha384);
    crypto_hmac_test_custom_sha_mode_wrap_on(app, msg, FuriHalCryptoHmacShaModeSha512);

    furi_string_printf(msg, "Crypto HMAC done\r\n");
    crypto_test_app_send_text(app, msg);
}
