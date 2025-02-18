#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Wrap a key. The key is wrapped using the key wrapping algorithm.
 *
 * @param uint32_t key_size Size of the key.
 * @param uint8_t* key Pointer to the key.
 * @param uint8_t* wrapped_key Pointer to the wrapped key.
 */
void furi_hal_wrap_key(uint32_t key_size, uint8_t* key, uint8_t* wrapped_key);

#ifdef __cplusplus
}
#endif
