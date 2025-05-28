#include "crypto_ecdsa.h"
#include "crypto_common.h"

#include <furi_hal_crypto.h>
#include <cli/cli_ansi.h>

#define TAG "ECDSA"

#define INPUT_MSG_SIZE 32
static const uint8_t input_data[INPUT_MSG_SIZE] = {0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
                                                   0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                                                   0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
                                                   0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};

static const uint8_t private_key[FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256] = {
    0x41, 0x9c, 0x9c, 0x80, 0x33, 0x6c, 0x40, 0x1f, 0xd2, 0x06, 0x49,
    0x59, 0x8b, 0xb6, 0x5f, 0xb3, 0xd8, 0xd8, 0xef, 0xd5, 0xeb, 0x4a,
    0xe1, 0xe8, 0x8a, 0x63, 0x36, 0x81, 0xcb, 0x0a, 0x21, 0x07};

static const uint8_t public_key[FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256] = {
    0x04, 0xb6, 0xcd, 0x40, 0x84, 0x9a, 0xf6, 0xc4, 0xc2, 0x2b, 0x57, 0x99, 0x86,
    0xa7, 0x7d, 0xfa, 0x19, 0x61, 0xaa, 0xe2, 0x6e, 0x60, 0xe6, 0x83, 0x82, 0x11,
    0xeb, 0xe5, 0xd1, 0x40, 0x79, 0x22, 0x25, 0xe4, 0x12, 0x40, 0xfe, 0x30, 0xec,
    0x63, 0x88, 0xab, 0x35, 0xaf, 0xb6, 0x34, 0xd8, 0x76, 0x03, 0xef, 0x81, 0xb8,
    0x11, 0x7d, 0x90, 0x43, 0xf6, 0x7e, 0x0a, 0x73, 0x01, 0xbd, 0x48, 0x5e, 0x7f};

uint8_t signature_test[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE] = {
    0x30, 0x44, 0x02, 0x20, 0x6A, 0x4B, 0x71, 0x62, 0xBE, 0x09, 0xDA, 0x78, 0x90, 0xF7, 0xC2,
    0x1F, 0x11, 0x6D, 0x69, 0x08, 0x94, 0x32, 0x50, 0x5B, 0x91, 0xD5, 0xE0, 0xFC, 0x7A, 0x5C,
    0x0D, 0xBC, 0x39, 0x16, 0x44, 0xB4, 0x02, 0x20, 0x5B, 0xE3, 0xF8, 0x84, 0xEF, 0xE0, 0x08,
    0x7E, 0x21, 0x4D, 0xF9, 0x93, 0x5D, 0xF7, 0xD6, 0x89, 0x01, 0xF4, 0x67, 0xED, 0x2A, 0x00,
    0x75, 0x71, 0x82, 0x9C, 0xC5, 0xA6, 0x49, 0x8D, 0xCD, 0xF2, 0x00, 0x00};

static const uint8_t private_key_224[FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224] = {
    0x96, 0xCD, 0x3A, 0x36, 0x25, 0xD6, 0xF6, 0x06, 0xBD, 0xC8, 0x64, 0x77, 0x8D, 0x4A,
    0xA6, 0x50, 0xC2, 0xD7, 0x9A, 0x05, 0x94, 0xDD, 0x10, 0xCF, 0x4C, 0x47, 0x4B, 0x83};

static const uint8_t public_key_224[FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_224] = {
    0x04, 0xCE, 0xAD, 0x4B, 0x29, 0x0B, 0xC2, 0xDD, 0xCF, 0x70, 0x6A, 0x37, 0x80, 0x6B, 0x81,
    0x24, 0x8D, 0xA4, 0x8C, 0xAD, 0x4F, 0x48, 0x3A, 0x3D, 0xA0, 0x2C, 0xAE, 0x99, 0x99, 0x3F,
    0x97, 0x2C, 0xBE, 0xAA, 0x1E, 0x06, 0xEB, 0xC8, 0xE1, 0xDD, 0x69, 0xB7, 0x1C, 0xEF, 0xD5,
    0x78, 0xF2, 0xEE, 0xD5, 0x5D, 0x92, 0x2C, 0x71, 0x35, 0xD9, 0xE7, 0xE1};

void crypto_ecdsa_test_wrap_off_custom_sha_mode(
    FuriHalCryptoEcdsaMode mode) {
    switch(mode) {
    case FuriHalCryptoEcdsaModeSha256:
        printf("ECDSA SHA256 mode\r\n");
        break;
    case FuriHalCryptoEcdsaModeSha384:
        printf("ECDSA SHA384 mode\r\n");
        break;
    case FuriHalCryptoEcdsaModeSha512:
        printf("ECDSA SHA512 mode\r\n");
        break;

    default:
        break;
    }

    uint8_t signature[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE] = {0};
    size_t signature_length = FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE;
    FuriHalCryptoEcdsa* handle = furi_hal_crypto_ecdsa_sign_init(
        mode,
        (uint8_t*)private_key,
        FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256,
        FuriHalCryptoWrappingModeOff);
    furi_hal_crypto_ecdsa_sign(
        handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature, &signature_length);
    furi_hal_crypto_ecdsa_deinit(handle);

    crypto_common_print_buffer_hex("Signature =\t", signature, signature_length);

    handle = furi_hal_crypto_ecdsa_verify_init(
        mode, (uint8_t*)public_key, FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256);
    if(furi_hal_crypto_ecdsa_verify(
           handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature, signature_length)) {
        printf(ANSI_FG_GREEN "ECDSA mode success\r\n" ANSI_RESET);
        
    } else {
        printf(ANSI_FG_RED " ECDSA mode failed" ANSI_RESET "\r\n");
        
    }
    furi_hal_crypto_ecdsa_deinit(handle);
}

void crypto_ecdsa_test_wrap_on_custom_sha_mode(
    FuriHalCryptoEcdsaMode mode,
    uint8_t* private_key_wrap) {
    switch(mode) {
    case FuriHalCryptoEcdsaModeSha256:
        printf("ECDSA SHA256 mode\r\n");
        break;
    case FuriHalCryptoEcdsaModeSha384:
        printf("ECDSA SHA384 mode\r\n");
        break;
    case FuriHalCryptoEcdsaModeSha512:
        printf("ECDSA SHA512 mode\r\n");
        break;

    default:
        break;
    }

    uint8_t signature[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE] = {0};
    size_t signature_length = FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE;
    FuriHalCryptoEcdsa* handle = furi_hal_crypto_ecdsa_sign_init(
        mode,
        private_key_wrap,
        FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256,
        FuriHalCryptoWrappingModeOn);
    furi_hal_crypto_ecdsa_sign(
        handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature, &signature_length);
    furi_hal_crypto_ecdsa_deinit(handle);

    crypto_common_print_buffer_hex("Signature =\t", signature, signature_length);

    handle = furi_hal_crypto_ecdsa_verify_init(
        mode, (uint8_t*)public_key, FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256);
    if(furi_hal_crypto_ecdsa_verify(
           handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature, signature_length)) {
        printf(ANSI_FG_GREEN " ECDSA mode success" ANSI_RESET "\r\n");
        
    } else {
        printf(ANSI_FG_RED " ECDSA mode failed" ANSI_RESET "\r\n");
        
    }
    furi_hal_crypto_ecdsa_deinit(handle);
}

void crypto_ecdsa_224_test_wrap_off_custom_sha_mode(
    FuriHalCryptoEcdsaMode mode) {
    switch(mode) {
    case FuriHalCryptoEcdsaModeSha256:
        printf("ECDSA SHA256 mode\r\n");
        break;
    case FuriHalCryptoEcdsaModeSha384:
        printf("ECDSA SHA384 mode\r\n");
        break;
    case FuriHalCryptoEcdsaModeSha512:
        printf("ECDSA SHA512 mode\r\n");
        break;

    default:
        break;
    }

    uint8_t signature[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE] = {0};
    size_t signature_length = FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE;
    FuriHalCryptoEcdsa* handle = furi_hal_crypto_ecdsa_sign_init(
        mode,
        (uint8_t*)private_key_224,
        FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224,
        FuriHalCryptoWrappingModeOff);
    furi_hal_crypto_ecdsa_sign(
        handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature, &signature_length);
    furi_hal_crypto_ecdsa_deinit(handle);

    crypto_common_print_buffer_hex("Signature =\t", signature, signature_length);

    handle = furi_hal_crypto_ecdsa_verify_init(
        mode, (uint8_t*)public_key_224, FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_224);
    if(furi_hal_crypto_ecdsa_verify(
           handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature, signature_length)) {
        printf(ANSI_FG_GREEN " ECDSA mode success" ANSI_RESET "\r\n");
        
    } else {
        printf(ANSI_FG_RED " ECDSA mode failed" ANSI_RESET "\r\n");
        
    }
    furi_hal_crypto_ecdsa_deinit(handle);
}

void crypto_ecdsa_224_test_wrap_on_custom_sha_mode(
    FuriHalCryptoEcdsaMode mode,
    uint8_t* private_key_wrap) {
    switch(mode) {
    case FuriHalCryptoEcdsaModeSha256:
        printf("ECDSA SHA256 mode\r\n");
        break;
    case FuriHalCryptoEcdsaModeSha384:
        printf("ECDSA SHA384 mode\r\n");
        break;
    case FuriHalCryptoEcdsaModeSha512:
        printf("ECDSA SHA512 mode\r\n");
        break;

    default:
        break;
    }

    uint8_t signature[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE] = {0};
    size_t signature_length = FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE;
    FuriHalCryptoEcdsa* handle = furi_hal_crypto_ecdsa_sign_init(
        mode,
        private_key_wrap,
        FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224,
        FuriHalCryptoWrappingModeOn);
    furi_hal_crypto_ecdsa_sign(
        handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature, &signature_length);
    furi_hal_crypto_ecdsa_deinit(handle);

    crypto_common_print_buffer_hex("Signature =\t", signature, signature_length);

    handle = furi_hal_crypto_ecdsa_verify_init(
        mode, (uint8_t*)public_key_224, FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_224);
    if(furi_hal_crypto_ecdsa_verify(
           handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature, signature_length)) {
        printf(ANSI_FG_GREEN " ECDSA mode success" ANSI_RESET "\r\n");
        
    } else {
        printf(ANSI_FG_RED " ECDSA mode failed" ANSI_RESET "\r\n");
        
    }
    furi_hal_crypto_ecdsa_deinit(handle);
}

void crypto_ecdsa_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    crypto_common_print_buffer_hex("message =\t\t", (uint8_t*)input_data, INPUT_MSG_SIZE);

    FuriHalCryptoEcdsa* handle = furi_hal_crypto_ecdsa_verify_init(
        FuriHalCryptoEcdsaModeSha256,
        (uint8_t*)public_key,
        FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256);
    if(furi_hal_crypto_ecdsa_verify(
           handle, (uint8_t*)input_data, INPUT_MSG_SIZE, signature_test, sizeof(signature_test))) {
        printf(ANSI_FG_GREEN " ECDSA signature_test success" ANSI_RESET "\r\n");
    } else {
        printf(ANSI_FG_RED " ECDSA signature_test failed" ANSI_RESET "\r\n");
    }
    furi_hal_crypto_ecdsa_deinit(handle);

    printf(ANSI_FG_YELLOW "ECDSA SECP256R1 key wrap off test" ANSI_RESET "\r\n");
    
    crypto_ecdsa_test_wrap_off_custom_sha_mode(FuriHalCryptoEcdsaModeSha256);
    crypto_ecdsa_test_wrap_off_custom_sha_mode(FuriHalCryptoEcdsaModeSha384);
    crypto_ecdsa_test_wrap_off_custom_sha_mode(FuriHalCryptoEcdsaModeSha512);

    printf(ANSI_FG_YELLOW "ECDSA SECP256R1 key wrap on test" ANSI_RESET "\r\n");
    
    uint8_t private_key_wrap[FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256] = {0};
    furi_hal_crypto_wrap_key(
        FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256, (uint8_t*)private_key, private_key_wrap);
    crypto_common_print_buffer_hex(
        "Key =\t\t", (uint8_t*)private_key, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256);
    crypto_common_print_buffer_hex(
        "Wrapped key =\t", private_key_wrap, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256);
    crypto_ecdsa_test_wrap_on_custom_sha_mode(FuriHalCryptoEcdsaModeSha256, private_key_wrap);
    crypto_ecdsa_test_wrap_on_custom_sha_mode(FuriHalCryptoEcdsaModeSha384, private_key_wrap);
    crypto_ecdsa_test_wrap_on_custom_sha_mode(FuriHalCryptoEcdsaModeSha512, private_key_wrap);

    printf(ANSI_FG_YELLOW "ECDSA SECP224R1 key wrap off test" ANSI_RESET "\r\n");
    
    crypto_ecdsa_224_test_wrap_off_custom_sha_mode(FuriHalCryptoEcdsaModeSha256);
    crypto_ecdsa_224_test_wrap_off_custom_sha_mode(FuriHalCryptoEcdsaModeSha384);
    crypto_ecdsa_224_test_wrap_off_custom_sha_mode(FuriHalCryptoEcdsaModeSha512);

    printf(ANSI_FG_YELLOW "ECDSA SECP256R1 key wrap on test" ANSI_RESET "\r\n");
    
    uint8_t private_key_wrap_224[FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224] = {0};
    furi_hal_crypto_wrap_key(
        FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224, (uint8_t*)private_key_224, private_key_wrap_224);
    crypto_common_print_buffer_hex(
        "Key =\t\t", (uint8_t*)private_key_224, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224);
    crypto_common_print_buffer_hex(
        "Wrapped key =\t", private_key_wrap_224, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224);
    crypto_ecdsa_224_test_wrap_on_custom_sha_mode(FuriHalCryptoEcdsaModeSha256, private_key_wrap_224);
    crypto_ecdsa_224_test_wrap_on_custom_sha_mode(FuriHalCryptoEcdsaModeSha384, private_key_wrap_224);
    crypto_ecdsa_224_test_wrap_on_custom_sha_mode(FuriHalCryptoEcdsaModeSha512, private_key_wrap_224);

    printf("Crypto ECDSA done\r\n");
    
}
