#include "crypto_hmac.h"
#include "crypto_common.h"

#include <furi_hal_crypto.h>
#include <cli/cli_ansi.h>

#define TAG "HMAC"

static const uint8_t key_hmac[] = {
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};

static const uint8_t message[] = {0x54, 0x65, 0x73, 0x74, 0x20, 0x55, 0x73, 0x69, 0x6e, 0x67, 0x20,
                                  0x4c, 0x61, 0x72, 0x67, 0x65, 0x72, 0x20, 0x54, 0x68, 0x61, 0x6e,
                                  0x20, 0x42, 0x6c, 0x6f, 0x63, 0x6b, 0x2d, 0x53, 0x69, 0x7a, 0x65,
                                  0x20, 0x4b, 0x65, 0x79, 0x20, 0x2d, 0x20, 0x48, 0x61, 0x73, 0x68,
                                  0x20, 0x4b, 0x65, 0x79, 0x20, 0x46, 0x69, 0x72, 0x73, 0x74};

// Expected output for the given input from HMAC engine
static const uint8_t digest_output_sha1[FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE] = {
    0x90, 0xD0, 0xDA, 0xCE, 0x1C, 0x1B, 0xDC, 0x95, 0x73, 0x39,
    0x30, 0x78, 0x03, 0x16, 0x03, 0x35, 0xBD, 0xE6, 0xDF, 0x2B};
static const uint8_t digest_output_sha256[FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE] = {
    0x60, 0xe4, 0x31, 0x59, 0x1e, 0xe0, 0xb6, 0x7f, 0x0d, 0x8a, 0x26,
    0xaa, 0xcb, 0xf5, 0xb7, 0x7f, 0x8e, 0x0b, 0xc6, 0x21, 0x37, 0x28,
    0xc5, 0x14, 0x05, 0x46, 0x04, 0x0f, 0x0e, 0xe3, 0x7f, 0x54};
static const uint8_t digest_output_sha384[FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE] = {
    0x4e, 0xce, 0x08, 0x44, 0x85, 0x81, 0x3e, 0x90, 0x88, 0xd2, 0xc6, 0x3a,
    0x04, 0x1b, 0xc5, 0xb4, 0x4f, 0x9e, 0xf1, 0x01, 0x2a, 0x2b, 0x58, 0x8f,
    0x3c, 0xd1, 0x1f, 0x05, 0x03, 0x3a, 0xc4, 0xc6, 0x0c, 0x2e, 0xf6, 0xab,
    0x40, 0x30, 0xfe, 0x82, 0x96, 0x24, 0x8d, 0xf1, 0x63, 0xf4, 0x49, 0x52};
static const uint8_t digest_output_sha512[FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE] = {
    0x80, 0xb2, 0x42, 0x63, 0xc7, 0xc1, 0xa3, 0xeb, 0xb7, 0x14, 0x93, 0xc1, 0xdd,
    0x7b, 0xe8, 0xb4, 0x9b, 0x46, 0xd1, 0xf4, 0x1b, 0x4a, 0xee, 0xc1, 0x12, 0x1b,
    0x01, 0x37, 0x83, 0xf8, 0xf3, 0x52, 0x6b, 0x56, 0xd0, 0x37, 0xe0, 0x5f, 0x25,
    0x98, 0xbd, 0x0f, 0xd2, 0x21, 0x5d, 0x6a, 0x1e, 0x52, 0x95, 0xe6, 0x4f, 0x73,
    0xf6, 0x3f, 0x0a, 0xec, 0x8b, 0x91, 0x5a, 0x98, 0x5d, 0x78, 0x65, 0x98};

void crypto_hmac_check(
    char* tag,
    const uint8_t* digest,
    const uint8_t* digest_out,
    size_t digest_length) {
    crypto_common_print_buffer_hex("Message =\t\t", (uint8_t*)message, sizeof(message));
    crypto_common_print_buffer_hex("Digest =\t\t", digest, digest_length);
    crypto_common_print_buffer_hex("Expected =\t", digest_out, digest_length);
    if(memcmp(digest, digest_out, digest_length) != 0) {
        printf(ANSI_FG_RED "%s mode failed" ANSI_RESET "\r\n", tag);
    } else {
        printf(ANSI_FG_GREEN "%s mode success" ANSI_RESET "\r\n", tag);
    }
}

void crypto_hmac_test_custom_sha_mode(FuriHalCryptoHmacShaMode hmac_sha_mode) {
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
        crypto_hmac_check(
            "HMAC_SHA1", digest, digest_output_sha1, FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha256:
        crypto_hmac_check(
            "HMAC_SHA256", digest, digest_output_sha256, FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha384:
        crypto_hmac_check(
            "HMAC_SHA384", digest, digest_output_sha384, FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha512:
        crypto_hmac_check(
            "HMAC_SHA512", digest, digest_output_sha512, FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE);
        break;
    default:
        break;
    }
}

void crypto_hmac_test_custom_sha_mode_wrap_on(FuriHalCryptoHmacShaMode hmac_sha_mode) {
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
    crypto_common_print_buffer_hex("Key =\t\t", key_hmac, sizeof(key_hmac));
    printf("Wrapped key size = %d\r\n", wrap_key_size);
    crypto_common_print_buffer_hex("Wrapped key =\t", wrapped_key, wrap_key_size);

    // Initialize HMAC
    FuriHalCryptoHmac* handle = furi_hal_crypto_hmac_init(
        hmac_sha_mode, wrapped_key, wrap_key_size, FuriHalCryptoWrappingModeOn);
    // Compute HMAC digest
    furi_hal_crypto_hmac_digest(handle, (uint8_t*)message, sizeof(message), digest, digest_length);
    // Deinitialize HMAC
    furi_hal_crypto_hmac_deinit(handle);

    switch(hmac_sha_mode) {
    case FuriHalCryptoHmacShaModeSha1:
        crypto_hmac_check(
            "HMAC_SHA1", digest, digest_output_sha1, FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha256:
        crypto_hmac_check(
            "HMAC_SHA256", digest, digest_output_sha256, FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha384:
        crypto_hmac_check(
            "HMAC_SHA384", digest, digest_output_sha384, FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE);
        break;
    case FuriHalCryptoHmacShaModeSha512:
        crypto_hmac_check(
            "HMAC_SHA512", digest, digest_output_sha512, FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE);
        break;
    default:
        break;
    }
}

void crypto_hmac_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    printf(ANSI_FG_YELLOW "HMAC key wrap off test" ANSI_RESET "\r\n");
    crypto_hmac_test_custom_sha_mode(FuriHalCryptoHmacShaModeSha1);
    crypto_hmac_test_custom_sha_mode(FuriHalCryptoHmacShaModeSha256);
    crypto_hmac_test_custom_sha_mode(FuriHalCryptoHmacShaModeSha384);
    crypto_hmac_test_custom_sha_mode(FuriHalCryptoHmacShaModeSha512);

    printf(ANSI_FG_YELLOW "HMAC key wrap on test" ANSI_RESET "\r\n");
    crypto_hmac_test_custom_sha_mode_wrap_on(FuriHalCryptoHmacShaModeSha1);
    crypto_hmac_test_custom_sha_mode_wrap_on(FuriHalCryptoHmacShaModeSha256);
    crypto_hmac_test_custom_sha_mode_wrap_on(FuriHalCryptoHmacShaModeSha384);
    crypto_hmac_test_custom_sha_mode_wrap_on(FuriHalCryptoHmacShaModeSha512);

    printf("Crypto HMAC done\r\n");
}
