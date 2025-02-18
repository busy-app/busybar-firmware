

#include <furi.h>
#include <furi_hal_wrap_key.h>
#include <sl_si91x_wrap.h>

#define TAG "Wrap_Key"

void furi_hal_wrap_key(uint32_t key_size, uint8_t* key, uint8_t* wrapped_key) {
    sl_si91x_wrap_config_t wrap_config = {0};
    wrap_config.key_type = SL_SI91X_TRANSPARENT_KEY;
    wrap_config.key_size = key_size;
    wrap_config.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    wrap_config.padding = 0;
    memcpy(wrap_config.key_buffer, key, wrap_config.key_size);
    sl_status_t status = sl_si91x_wrap(&wrap_config, wrapped_key);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to wrap key: %ld", status);
        furi_crash("Failed to wrap key");
    }
}
