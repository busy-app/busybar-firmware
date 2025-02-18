

#include <furi.h>
#include <furi_hal_aes.h>
#include <sl_si91x_aes.h>

#define TAG "AES"

struct FuriHalAes {
    sl_si91x_aes_config_t config;
};

const sl_si91x_aes_mode_t furi_hal_aes_mode[] = {
    [FuriHalAesModeCBC] = SL_SI91X_AES_CBC,
    [FuriHalAesModeECB] = SL_SI91X_AES_ECB,
    [FuriHalAesModeCTR] = SL_SI91X_AES_CTR};

FuriHalAes* furi_hal_aes_init(
    FuriHalAesMode mode,
    uint8_t* key,
    size_t key_size,
    uint8_t* iv,
    FuriHalAesWrappingMode wrapping_mode) {
    if(mode != FuriHalAesModeECB) {
        furi_check(iv);
    }

    FuriHalAes* handle = malloc(sizeof(FuriHalAes));
    furi_check(handle != NULL, "Failed to allocate memory for AES handle");

    handle->config.aes_mode = furi_hal_aes_mode[mode];
    handle->config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.iv = iv;
    switch(key_size) {
    case SL_SI91X_AES_KEY_SIZE_128:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_128;
        break;
    case SL_SI91X_AES_KEY_SIZE_192:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_192;
        break;
    case SL_SI91X_AES_KEY_SIZE_256:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_256;
        break;

    default:
        furi_crash("Invalid key size");
        break;
    }
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    memcpy(handle->config.key_config.b0.key_buffer, key, handle->config.key_config.b0.key_size);

    if(wrapping_mode == FuriHalAesWrappingModeOff) {
        handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
        //for 128 bits key, wrap key size is 128 bits,
        //for 192 and 256 bits keys, wrap key size is 256 bits
        if(handle->config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_128) {
            memcpy(handle->config.key_config.b0.key_buffer, key, SL_SI91X_AES_KEY_SIZE_128);
        } else if(
            handle->config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_192 ||
            handle->config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_256) {
            memcpy(handle->config.key_config.b0.key_buffer, key, SL_SI91X_AES_KEY_SIZE_256);
        } else {
            furi_crash("Invalid key size");
        }
    }

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
    furi_assert(input_length <= SL_SI91X_MAX_DATA_SIZE_IN_BYTES);

    handle->config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
    handle->config.msg = input;
    handle->config.msg_length = input_length;

    sl_status_t status = sl_si91x_aes(&handle->config, output);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "AES encryption failed, Error Code : 0x%lX", status);
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
    furi_assert(input_length <= SL_SI91X_MAX_DATA_SIZE_IN_BYTES);

    handle->config.encrypt_decrypt = SL_SI91X_AES_DECRYPT;
    handle->config.msg = input;
    handle->config.msg_length = input_length;

    sl_status_t status = sl_si91x_aes(&handle->config, output);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "AES decryption failed, Error Code : 0x%lX", status);
        return false;
    }
    return true;
}
