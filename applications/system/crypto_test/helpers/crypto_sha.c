#include "crypto_sha.h"
#include "crypto_common.h"

#include <furi_hal_sha.h>

#define TAG "SHA"

static const uint8_t message[] =
    "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";

static const uint8_t digest_out_sha1[FURI_HAL_SHA1_DIGEST_SIZE] = {0xa4, 0x9b, 0x24, 0x46, 0xa0,
                                                                   0x2c, 0x64, 0x5b, 0xf4, 0x19,
                                                                   0xf9, 0x95, 0xb6, 0x70, 0x91,
                                                                   0x25, 0x3a, 0x04, 0xa2, 0x59};
static const uint8_t digest_out_sha256[FURI_HAL_SHA256_DIGEST_SIZE] = {
    0xCF, 0x5B, 0x16, 0xA7, 0x78, 0xAF, 0x83, 0x80, 0x03, 0x6C, 0xE5,
    0x9E, 0x7B, 0x04, 0x92, 0x37, 0x0B, 0x24, 0x9B, 0x11, 0xE8, 0xF0,
    0x7A, 0x51, 0xAF, 0xAC, 0x45, 0x03, 0x7A, 0xFE, 0xE9, 0xD1};
static const uint8_t digest_out_sha384[FURI_HAL_SHA384_DIGEST_SIZE] = {
    0x09, 0x33, 0x0C, 0x33, 0xF7, 0x11, 0x47, 0xE8, 0x3D, 0x19, 0x2F, 0xC7,
    0x82, 0xCD, 0x1B, 0x47, 0x53, 0x11, 0x1B, 0x17, 0x3B, 0x3B, 0x05, 0xD2,
    0x2F, 0xA0, 0x80, 0x86, 0xE3, 0xB0, 0xF7, 0x12, 0xFC, 0xC7, 0xC7, 0x1A,
    0x55, 0x7E, 0x2D, 0xB9, 0x66, 0xC3, 0xE9, 0xFA, 0x91, 0x74, 0x60, 0x39};
static const uint8_t digest_out_sha512[FURI_HAL_SHA512_DIGEST_SIZE] = {
    0x8e, 0x95, 0x9b, 0x75, 0xda, 0xe3, 0x13, 0xda, 0x8c, 0xf4, 0xf7, 0x28, 0x14,
    0xfc, 0x14, 0x3f, 0x8f, 0x77, 0x79, 0xc6, 0xeb, 0x9f, 0x7f, 0xa1, 0x72, 0x99,
    0xae, 0xad, 0xb6, 0x88, 0x90, 0x18, 0x50, 0x1d, 0x28, 0x9e, 0x49, 0x00, 0xf7,
    0xe4, 0x33, 0x1b, 0x99, 0xde, 0xc4, 0xb5, 0x43, 0x3a, 0xc7, 0xd3, 0x29, 0xee,
    0xb6, 0xdd, 0x26, 0x54, 0x5e, 0x96, 0xe5, 0x5b, 0x87, 0x4b, 0xe9, 0x09};
static const uint8_t digest_out_sha244[FURI_HAL_SHA224_DIGEST_SIZE] = {
    0xC9, 0x7C, 0xA9, 0xA5, 0x59, 0x85, 0x0C, 0xE9, 0x7A, 0x04, 0xA9, 0x6D, 0xEF, 0x6D,
    0x99, 0xA9, 0xE0, 0xE0, 0xE2, 0xAB, 0x14, 0xE6, 0xB8, 0xDF, 0x26, 0x5F, 0xC0, 0xB3};

void crypto_sha_chek(
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

void crypto_sha_test_custom_sha_mode(CryptoTestApp* app, FuriString* msg, FuriHalShaMode sha_mode) {
    uint8_t digest[FURI_HAL_SHA512_DIGEST_SIZE] = {0};
    size_t digest_length = 0;

    switch(sha_mode) {
    case FuriHalShaModeSha1:
        digest_length = FURI_HAL_SHA1_DIGEST_SIZE;
        break;
    case FuriHalShaModeSha256:
        digest_length = FURI_HAL_SHA256_DIGEST_SIZE;
        break;
    case FuriHalShaModeSha384:
        digest_length = FURI_HAL_SHA384_DIGEST_SIZE;
        break;
    case FuriHalShaModeSha512:
        digest_length = FURI_HAL_SHA512_DIGEST_SIZE;
        break;
    case FuriHalShaModeSha244:
        digest_length = FURI_HAL_SHA224_DIGEST_SIZE;
        break;

    default:
        break;
    }

    furi_hal_sha(sha_mode, (uint8_t*)message, sizeof(message) - 1, digest, digest_length);

    switch(sha_mode) {
    case FuriHalShaModeSha1:
        crypto_sha_chek(app, msg, "SHA1", digest, digest_out_sha1, FURI_HAL_SHA1_DIGEST_SIZE);
        break;
    case FuriHalShaModeSha256:
        crypto_sha_chek(
            app, msg, "SHA256", digest, digest_out_sha256, FURI_HAL_SHA256_DIGEST_SIZE);
        break;
    case FuriHalShaModeSha384:
        crypto_sha_chek(
            app, msg, "SHA384", digest, digest_out_sha384, FURI_HAL_SHA384_DIGEST_SIZE);
        break;
    case FuriHalShaModeSha512:
        crypto_sha_chek(
            app, msg, "SHA512", digest, digest_out_sha512, FURI_HAL_SHA512_DIGEST_SIZE);
        break;
    case FuriHalShaModeSha244:
        crypto_sha_chek(
            app, msg, "SHA224", digest, digest_out_sha244, FURI_HAL_SHA224_DIGEST_SIZE);
        break;

    default:
        break;
    }
}

void crypto_sha_test(CryptoTestApp* app, FuriString* msg) {
    crypto_sha_test_custom_sha_mode(app, msg, FuriHalShaModeSha1);
    crypto_sha_test_custom_sha_mode(app, msg, FuriHalShaModeSha256);
    crypto_sha_test_custom_sha_mode(app, msg, FuriHalShaModeSha384);
    crypto_sha_test_custom_sha_mode(app, msg, FuriHalShaModeSha512);
    crypto_sha_test_custom_sha_mode(app, msg, FuriHalShaModeSha244);
    furi_string_printf(msg, "Crypto SHA done\r\n");
    crypto_test_app_send_text(app, msg);
}
