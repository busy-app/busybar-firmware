#pragma once

#include <sl_si91x_aes.h>

typedef struct FuriHalAes FuriHalAes;

#define FuriHalAesIvSize SL_SI91X_IV_SIZE
typedef enum {
    FuriHalAesKeySize128 = SL_SI91X_AES_KEY_SIZE_128,
    FuriHalAesKeySize192 = SL_SI91X_AES_KEY_SIZE_192,
    FuriHalAesKeySize256 = SL_SI91X_AES_KEY_SIZE_256,
} FuriHalAesKeySize;

typedef enum {
    FuriHalAesModeCBC = SL_SI91X_AES_CBC,
    FuriHalAesModeECB = SL_SI91X_AES_ECB,
    FuriHalAesModeCTR = SL_SI91X_AES_CTR,
} FuriHalAesMode;

typedef enum {
    FuriHalAesWrappingModeOff,
    FuriHalAesWrappingModeOn,
} FuriHalAesWrappingMode;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the AES module.
 * 
 * @param key_size Size of the key.
 * @param mode AES mode.
 * @param key Pointer to the key.
 * @param iv Pointer to the initialization vector.
 * @param wrapping_mode Wrapping mode.
 */
FuriHalAes* furi_hal_aes_init(
    FuriHalAesKeySize key_size,
    FuriHalAesMode mode,
    uint8_t* key,
    uint8_t* iv,
    FuriHalAesWrappingMode wrapping_mode);

/**
 * Deinitialize the AES module.
 * 
 * @param handle Pointer to the AES handle.
 */
void furi_hal_aes_deinit(FuriHalAes* handle);

/**
 * Encrypt data using AES.
 * 
 * @param handle Pointer to the AES handle.
 * @param input Pointer to the input data.
 * @param input_length Length of the input data.
 * @param output Pointer to the output buffer.
 * @returns true if the operation was successful, false otherwise.
 */
bool furi_hal_aes_encrypt(
    FuriHalAes* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output);

/**
 * Decrypt data using AES.
 * 
 * @param handle Pointer to the AES handle.
 * @param input Pointer to the input data.
 * @param input_length Length of the input data.
 * @param output Pointer to the output buffer.
 * @returns true if the operation was successful, false otherwise.
 */
bool furi_hal_aes_decrypt(
    FuriHalAes* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output);

#ifdef __cplusplus
}
#endif
