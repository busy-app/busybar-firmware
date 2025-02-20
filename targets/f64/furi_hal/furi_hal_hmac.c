#include <furi.h>
#include <furi_hal_hmac.h>
#include <sl_si91x_hmac.h>
#include <sl_si91x_wrap.h>

#define TAG "HMAC"

struct FuriHalHmac {
    sl_si91x_hmac_config_t config;
};

static const sl_si91x_hmac_mode_t furi_hal_hmac_sha_mode[] = {
    [FuriHalHmacShaModeSha1] = SL_SI91X_HMAC_SHA_1,
    [FuriHalHmacShaModeSha256] = SL_SI91X_HMAC_SHA_256,
    [FuriHalHmacShaModeSha384] = SL_SI91X_HMAC_SHA_384,
    [FuriHalHmacShaModeSha512] = SL_SI91X_HMAC_SHA_512,
};

FuriHalHmac* furi_hal_hmac_init(
    FuriHalHmacShaMode mode,
    uint8_t* key,
    size_t key_size,
    FuriHalHmacWrappingMode wrapping_mode) {
    FuriHalHmac* handle = malloc(sizeof(FuriHalHmac));
    furi_check(handle != NULL, "Failed to allocate memory for HMAC handle");

    handle->config.hmac_mode = furi_hal_hmac_sha_mode[mode];
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    if(wrapping_mode != FuriHalHmacWrappingModeOn) {
        handle->config.key_config.B0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.B0.key_type = SL_SI91X_WRAPPED_KEY;
        handle->config.key_config.B0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    }
    handle->config.key_config.B0.key_size = key_size;
    handle->config.key_config.B0.key = key;

    return handle;
}

void furi_hal_hmac_deinit(FuriHalHmac* handle) {
    furi_check(handle);
    free(handle);
}

bool furi_hal_hmac_digest(
    FuriHalHmac* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output,
    size_t output_length) {
    furi_check(handle && input && output);
    furi_check(
        (handle->config.hmac_mode == SL_SI91X_HMAC_SHA_1 &&
         output_length == SL_SI91X_HMAC_SHA_1_DIGEST_LEN) ||
        (handle->config.hmac_mode == SL_SI91X_HMAC_SHA_256 &&
         output_length == SL_SI91X_HMAC_SHA_256_DIGEST_LEN) ||
        (handle->config.hmac_mode == SL_SI91X_HMAC_SHA_384 &&
         output_length == SL_SI91X_HMAC_SHA_384_DIGEST_LEN) ||
        (handle->config.hmac_mode == SL_SI91X_HMAC_SHA_512 &&
         output_length == SL_SI91X_HMAC_SHA_512_DIGEST_LEN));

    handle->config.msg = input;
    handle->config.msg_length = input_length;

    sl_status_t status = sl_si91x_hmac(&handle->config, output);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to compute HMAC");
        return false;
    }
    return true;
}

void furi_hal_hmac_wrap_key(
    uint32_t key_size,
    uint8_t* key,
    FuriHalHmacShaMode hmac_sha_mode,
    uint8_t* wrapped_key,
    size_t* wrapped_key_size) {
    furi_assert(key);
    furi_assert(wrapped_key);
    furi_check(key_size <= SL_SI91X_WRAP_KEY_BUFFER_SIZE);
    //sl_si91x_wrap_config_t - size 1432 bytes
    sl_si91x_wrap_config_t wrap_config = {0};
    wrap_config.key_type = SL_SI91X_TRANSPARENT_KEY;
    wrap_config.key_size = key_size;
    wrap_config.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    wrap_config.padding = (1<<0); //SL_SI91X_HMAC_PADDING;
    wrap_config.hmac_sha_mode = furi_hal_hmac_sha_mode[hmac_sha_mode];

    //memset(wrapped_key, 0, *wrapped_key_size);
    memcpy(wrap_config.key_buffer, key, wrap_config.key_size);

    sl_status_t status = sl_si91x_wrap(&wrap_config, wrapped_key);
    *wrapped_key_size = wrap_config.key_size;

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to wrap key: %ld", status);
        furi_crash("Failed to wrap key");
    }
}
