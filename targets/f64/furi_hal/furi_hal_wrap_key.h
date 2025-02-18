#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_wrap_key(uint32_t key_size, uint8_t* key, uint8_t* wrapped_key);

#ifdef __cplusplus
}
#endif
