#pragma once

typedef struct FuriHalAes FuriHalAes;

#define FURI_HAL_AES_IV_SIZE 16
#define FURI_HAL_AES_KEY_SIZE_128 16
#define FURI_HAL_AES_KEY_SIZE_192 24
#define FURI_HAL_AES_KEY_SIZE_256 32

typedef enum {
    FuriHalAesModeCBC,
    FuriHalAesModeECB,
    FuriHalAesModeCTR,
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
 * @param[in] mode AES mode.
 * @param[in] key Pointer to the key.
 * @param[in] key_size Size of the key.
 * @param[in] iv Pointer to the initialization vector.
 * @param[in] wrapping_mode Wrapping mode.
 */
FuriHalAes* furi_hal_aes_init(
    FuriHalAesMode mode,
    uint8_t* key,
    size_t key_size,
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
 * @param[in] handle Pointer to the AES handle.
 * @param[in] input Pointer to the input data.
 * @param[in] input_length Length of the input data.
 * @param[out] output Pointer to the output buffer.
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
 * @param[in] handle Pointer to the AES handle.
 * @param[in] input Pointer to the input data.
 * @param[in] input_length Length of the input data.
 * @param[out] output Pointer to the output buffer.
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
