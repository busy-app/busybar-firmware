#include "crypto_sha.h"
#include "crypto_common.h"

#include <furi_hal_crypto.h>
#include <cli/cli_ansi.h>

#define TAG "SHA"

static const uint8_t message[] =
    "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";

static const uint8_t digest_out_sha1[FURI_HAL_CRYPTO_SHA1_DIGEST_SIZE] = {
    0xa4, 0x9b, 0x24, 0x46, 0xa0, 0x2c, 0x64, 0x5b, 0xf4, 0x19,
    0xf9, 0x95, 0xb6, 0x70, 0x91, 0x25, 0x3a, 0x04, 0xa2, 0x59};
static const uint8_t digest_out_sha256[FURI_HAL_CRYPTO_SHA256_DIGEST_SIZE] = {
    0xCF, 0x5B, 0x16, 0xA7, 0x78, 0xAF, 0x83, 0x80, 0x03, 0x6C, 0xE5,
    0x9E, 0x7B, 0x04, 0x92, 0x37, 0x0B, 0x24, 0x9B, 0x11, 0xE8, 0xF0,
    0x7A, 0x51, 0xAF, 0xAC, 0x45, 0x03, 0x7A, 0xFE, 0xE9, 0xD1};
static const uint8_t digest_out_sha384[FURI_HAL_CRYPTO_SHA384_DIGEST_SIZE] = {
    0x09, 0x33, 0x0C, 0x33, 0xF7, 0x11, 0x47, 0xE8, 0x3D, 0x19, 0x2F, 0xC7,
    0x82, 0xCD, 0x1B, 0x47, 0x53, 0x11, 0x1B, 0x17, 0x3B, 0x3B, 0x05, 0xD2,
    0x2F, 0xA0, 0x80, 0x86, 0xE3, 0xB0, 0xF7, 0x12, 0xFC, 0xC7, 0xC7, 0x1A,
    0x55, 0x7E, 0x2D, 0xB9, 0x66, 0xC3, 0xE9, 0xFA, 0x91, 0x74, 0x60, 0x39};
static const uint8_t digest_out_sha512[FURI_HAL_CRYPTO_SHA512_DIGEST_SIZE] = {
    0x8e, 0x95, 0x9b, 0x75, 0xda, 0xe3, 0x13, 0xda, 0x8c, 0xf4, 0xf7, 0x28, 0x14,
    0xfc, 0x14, 0x3f, 0x8f, 0x77, 0x79, 0xc6, 0xeb, 0x9f, 0x7f, 0xa1, 0x72, 0x99,
    0xae, 0xad, 0xb6, 0x88, 0x90, 0x18, 0x50, 0x1d, 0x28, 0x9e, 0x49, 0x00, 0xf7,
    0xe4, 0x33, 0x1b, 0x99, 0xde, 0xc4, 0xb5, 0x43, 0x3a, 0xc7, 0xd3, 0x29, 0xee,
    0xb6, 0xdd, 0x26, 0x54, 0x5e, 0x96, 0xe5, 0x5b, 0x87, 0x4b, 0xe9, 0x09};
static const uint8_t digest_out_sha244[FURI_HAL_CRYPTO_SHA224_DIGEST_SIZE] = {
    0xC9, 0x7C, 0xA9, 0xA5, 0x59, 0x85, 0x0C, 0xE9, 0x7A, 0x04, 0xA9, 0x6D, 0xEF, 0x6D,
    0x99, 0xA9, 0xE0, 0xE0, 0xE2, 0xAB, 0x14, 0xE6, 0xB8, 0xDF, 0x26, 0x5F, 0xC0, 0xB3};

void crypto_sha_check(
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

void crypto_sha_test_custom_sha_mode(
    FuriHalCryptoShaMode sha_mode) {
    uint8_t digest[FURI_HAL_CRYPTO_SHA512_DIGEST_SIZE] = {0};
    size_t digest_length = 0;

    switch(sha_mode) {
    case FuriHalCryptoShaModeSha1:
        digest_length = FURI_HAL_CRYPTO_SHA1_DIGEST_SIZE;
        break;
    case FuriHalCryptoShaModeSha256:
        digest_length = FURI_HAL_CRYPTO_SHA256_DIGEST_SIZE;
        break;
    case FuriHalCryptoShaModeSha384:
        digest_length = FURI_HAL_CRYPTO_SHA384_DIGEST_SIZE;
        break;
    case FuriHalCryptoShaModeSha512:
        digest_length = FURI_HAL_CRYPTO_SHA512_DIGEST_SIZE;
        break;
    case FuriHalCryptoShaModeSha244:
        digest_length = FURI_HAL_CRYPTO_SHA224_DIGEST_SIZE;
        break;

    default:
        break;
    }

    furi_hal_crypto_sha(sha_mode, (uint8_t*)message, sizeof(message) - 1, digest, digest_length);

    switch(sha_mode) {
    case FuriHalCryptoShaModeSha1:
        crypto_sha_check(
            "SHA1", digest, digest_out_sha1, FURI_HAL_CRYPTO_SHA1_DIGEST_SIZE);
        break;
    case FuriHalCryptoShaModeSha256:
        crypto_sha_check(
            "SHA256", digest, digest_out_sha256, FURI_HAL_CRYPTO_SHA256_DIGEST_SIZE);
        break;
    case FuriHalCryptoShaModeSha384:
        crypto_sha_check(
            "SHA384", digest, digest_out_sha384, FURI_HAL_CRYPTO_SHA384_DIGEST_SIZE);
        break;
    case FuriHalCryptoShaModeSha512:
        crypto_sha_check(
            "SHA512", digest, digest_out_sha512, FURI_HAL_CRYPTO_SHA512_DIGEST_SIZE);
        break;
    case FuriHalCryptoShaModeSha244:
        crypto_sha_check(
            "SHA224", digest, digest_out_sha244, FURI_HAL_CRYPTO_SHA224_DIGEST_SIZE);
        break;

    default:
        break;
    }
}

void crypto_sha_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    crypto_sha_test_custom_sha_mode(FuriHalCryptoShaModeSha1);
    crypto_sha_test_custom_sha_mode(FuriHalCryptoShaModeSha256);
    crypto_sha_test_custom_sha_mode(FuriHalCryptoShaModeSha384);
    crypto_sha_test_custom_sha_mode(FuriHalCryptoShaModeSha512);
    crypto_sha_test_custom_sha_mode(FuriHalCryptoShaModeSha244);
    printf("Crypto SHA done\r\n");
}
