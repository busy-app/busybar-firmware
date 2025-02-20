#pragma once

typedef struct FuriHalHmac FuriHalHmac;

#define FURI_HAL_HMAC_SHA1_DIGEST_SIZE   20
#define FURI_HAL_HMAC_SHA256_DIGEST_SIZE 32
#define FURI_HAL_HMAC_SHA384_DIGEST_SIZE 48
#define FURI_HAL_HMAC_SHA512_DIGEST_SIZE 64

typedef enum {
    FuriHalHmacShaModeSha1,
    FuriHalHmacShaModeSha256,
    FuriHalHmacShaModeSha384,
    FuriHalHmacShaModeSha512,
} FuriHalHmacShaMode;

typedef enum {
    FuriHalHmacWrappingModeOff,
    FuriHalHmacWrappingModeOn,
} FuriHalHmacWrappingMode;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize HMAC.
 *
 * @param[in] mode HMAC SHA mode.
 *    - FuriHalHmacShaModeSha1
 *    - FuriHalHmacShaModeSha256
 *    - FuriHalHmacShaModeSha384
 *    - FuriHalHmacShaModeSha512
 * @param[in] key Pointer to the key.
 * @param[in] key_size Size of the key.
 * @param[in] wrapping_mode Wrapping mode.
 *
 * @return Handle to the HMAC.
 */
FuriHalHmac* furi_hal_hmac_init(
    FuriHalHmacShaMode mode,
    uint8_t* key,
    size_t key_size,
    FuriHalHmacWrappingMode wrapping_mode);

/**
 * Deinitialize HMAC.
 *
 * @param[in] handle Handle to the HMAC.
 */
void furi_hal_hmac_deinit(FuriHalHmac* handle);

/**
 * Compute HMAC digest.
 *
 * @param[in] handle Handle to the HMAC.
 * @param[in] input Pointer to the input data.
 * @param[in] input_length Length of the input data.
 * @param[out] output Pointer to the output buffer.
 * @param[in] output_length Length of the output buffer.
 *
 * @return True if the HMAC digest is computed successfully, false otherwise.
 */
bool furi_hal_hmac_digest(
    FuriHalHmac* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output,
    size_t output_length);

/**
 * Wrap a key. The key is wrapped using the key wrapping algorithm.
 * 
 * @param[in] key_size Size of the key.
 * @param[in] key Pointer to the key.
 * @param[in] hmac_sha_mode HMAC SHA mode.
 *     - FuriHalHmacShaModeSha1
 *     - FuriHalHmacShaModeSha256
 *     - FuriHalHmacShaModeSha384
 *     - FuriHalHmacShaModeSha512
 * @param[out] wrapped_key Pointer to the wrapped key.
 * @param[out] wrapped_key_size Size of the wrapped key.
 */
void furi_hal_hmac_wrap_key(
    uint32_t key_size,
    uint8_t* key,
    FuriHalHmacShaMode hmac_sha_mode,
    uint8_t* wrapped_key,
    size_t* wrapped_key_size);

#ifdef __cplusplus
}
#endif
