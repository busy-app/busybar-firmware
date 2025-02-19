#include <furi.h>
#include <furi_hal_ecdsa.h>
#include <sl_si91x_ecdsa.h>

#define TAG "ECDSA"

static const sl_si91x_crypto_ecdsa_sha_mode_t furi_hal_ecdsa_sha_mode[] = {
    [FuriHalEcdsaModeSha256] = SL_SI91X_ECDSA_SHA_256,
    [FuriHalEcdsaModeSha384] = SL_SI91X_ECDSA_SHA_384,
    [FuriHalEcdsaModeSha512] = SL_SI91X_ECDSA_SHA_512,
};

struct FuriHalEcdsa {
    sl_si91x_ecdsa_config_t config;
};

FuriHalEcdsa* furi_hal_ecdsa_sign_init(
    FuriHalEcdsaMode mode,
    uint8_t* key,
    uint32_t key_mode,
    FuriHalEcdsaWrappingMode wrapping_mode) {
    FuriHalEcdsa* handle = malloc(sizeof(FuriHalEcdsa));
    furi_check(handle != NULL, "Failed to allocate memory for ECDSA handle");

    handle->config.ecdsa_operation = SL_SI91X_ECDSA_GENERATE_SIGN;
    if(key_mode == FURI_HAL_ECDSA_PRIV_KEY_SIZE_224) {
        handle->config.curve_id = SL_SI91X_ECC_SECP224R1;
    } else if(key_mode == FURI_HAL_ECDSA_PRIV_KEY_SIZE_256) {
        handle->config.curve_id = SL_SI91X_ECC_SECP256R1;
    } else {
        furi_crash("Invalid key size");
    }
    handle->config.sha_mode = furi_hal_ecdsa_sha_mode[mode];
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.private_key = key;
    handle->config.private_key_length = key_mode;
    handle->config.public_key = NULL;
    handle->config.public_key_length = 0;
    handle->config.signature_length = 0;
    if(wrapping_mode != FuriHalEcdsaWrappingModeOn) {
        handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
        handle->config.private_key_length =
            SL_SI91X_ECDSA_PRIV_KEY_SIZE_256; // wrapped key is of fixed output size 32;
        handle->config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    }
    handle->config.key_config.b0.key_size = 0;
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.reserved = 0;

    return handle;
}

FuriHalEcdsa* furi_hal_ecdsa_verify_init(FuriHalEcdsaMode mode, uint8_t* key, uint32_t key_mode) {
    FuriHalEcdsa* handle = malloc(sizeof(FuriHalEcdsa));
    furi_check(handle != NULL, "Failed to allocate memory for ECDSA handle");

    handle->config.ecdsa_operation = SL_SI91X_ECDSA_VERIFY_SIGN;
    if(key_mode == FURI_HAL_ECDSA_PUB_KEY_SIZE_224) {
        handle->config.curve_id = SL_SI91X_ECC_SECP224R1;
    } else if(key_mode == FURI_HAL_ECDSA_PUB_KEY_SIZE_256) {
        handle->config.curve_id = SL_SI91X_ECC_SECP256R1;
    } else {
        furi_crash("Invalid key size");
    }
    handle->config.sha_mode = furi_hal_ecdsa_sha_mode[mode];
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.private_key = NULL;
    handle->config.private_key_length = 0;
    handle->config.public_key = key;
    handle->config.public_key_length = key_mode;
    handle->config.signature_length = 0;
    handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    handle->config.key_config.b0.key_size = 0;
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.reserved = 0;

    return handle;
}

void furi_hal_ecdsa_deinit(FuriHalEcdsa* handle) {
    furi_check(handle);
    free(handle);
}

bool furi_hal_ecdsa_sign(
    FuriHalEcdsa* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output,
    size_t* output_length) {
    furi_check(handle && input && output && output_length);
    furi_check(handle->config.private_key && handle->config.private_key_length);
    handle->config.msg = input;
    handle->config.msg_length = input_length;
    handle->config.signature_length = *output_length;
    sl_status_t status = sl_si91x_ecdsa(&handle->config, output);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to sign data");
        return false;
    }
    *output_length = handle->config.signature_length;
    return true;
}

bool furi_hal_ecdsa_verify(
    FuriHalEcdsa* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* signature,
    uint16_t signature_length) {
    furi_check(handle && input && signature);
    furi_check(handle->config.public_key && handle->config.public_key_length);
    uint8_t* verify = NULL;
    handle->config.msg = input;
    handle->config.msg_length = input_length;
    handle->config.signature = signature;
    handle->config.signature_length = signature_length;
    sl_status_t status = sl_si91x_ecdsa(&handle->config, verify);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to verify data, Error Code : 0x%lX", status);
        return false;
    } else if(*verify != 1) {
        FURI_LOG_D(TAG, "Failed to verify data");
        return false;
    }
    return true;
}
