

#include <furi.h>
#include <furi_hal_aes.h>

#define TAG_AES "AES"
//#include <sl_si91x_aes.h>

struct FuriHalAes {
    sl_si91x_aes_config_t config;
};

FuriHalAes* furi_hal_aes_init(
    FuriHalAesKeySize key_size,
    FuriHalAesMode mode,
    uint8_t* key,
    uint8_t* iv,
    FuriHalAesWrappingMode wrapping_mode) {
    if(mode != FuriHalAesModeECB) {
        furi_check(iv);
    }

    FuriHalAes* handle = malloc(sizeof(FuriHalAes));
    furi_check(handle != NULL, "Failed to allocate memory for AES handle");
    handle->config.aes_mode = (sl_si91x_aes_mode_t)mode;
    handle->config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.iv = iv;
    handle->config.key_config.b0.key_size = (sl_si91x_aes_key_size_t)key_size;
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;

    if(wrapping_mode == FuriHalAesWrappingModeOff) {
        handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
        if(mode == 16) handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_128;
        if(mode == 24) handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_192;
        if(mode == 32) handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_256;
    }
    memcpy(handle->config.key_config.b0.key_buffer, key, handle->config.key_config.b0.key_size);

    return handle;
}

void furi_hal_aes_deinit(FuriHalAes* handle) {
    free(handle);
}

bool furi_hal_aes_encrypt(
    FuriHalAes* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output) {
    furi_assert(handle);
    furi_assert(input);
    furi_assert(input_length % 16 == 0);

    handle->config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
    handle->config.msg = input;
    handle->config.msg_length = input_length;

    sl_status_t status = sl_si91x_aes(&handle->config, output);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG_AES, "AES encryption failed, Error Code : 0x%lX", status);
        return false;
    }

    return true;
}

bool furi_hal_aes_decrypt(
    FuriHalAes* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output) {
    furi_assert(handle);
    furi_assert(input);
    furi_assert(input_length % 16 == 0);

    handle->config.encrypt_decrypt = SL_SI91X_AES_DECRYPT;
    handle->config.msg = input;
    handle->config.msg_length = input_length;

    sl_status_t status = sl_si91x_aes(&handle->config, output);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG_AES, "AES decryption failed, Error Code : 0x%lX", status);
        return false;
    }

    return true;
}
