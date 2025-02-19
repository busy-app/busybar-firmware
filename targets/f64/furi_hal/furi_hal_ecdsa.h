#pragma once

typedef struct FuriHalEcdsa FuriHalEcdsa;

#define FURI_HAL_ECDSA_PRIV_KEY_SIZE_224 28
#define FURI_HAL_ECDSA_PRIV_KEY_SIZE_256 32
#define FURI_HAL_ECDSA_PUB_KEY_SIZE_224  57
#define FURI_HAL_ECDSA_PUB_KEY_SIZE_256  65
#define FURI_HAL_ECDSA_MAX_SIGNATURE_SIZE 72

typedef enum {
    FuriHalEcdsaModeSha256,
    FuriHalEcdsaModeSha384,
    FuriHalEcdsaModeSha512,
} FuriHalEcdsaMode;

typedef enum {
    FuriHalEcdsaWrappingModeOff,
    FuriHalEcdsaWrappingModeOn,
} FuriHalEcdsaWrappingMode;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize ECDSA signing
 *
 * @param[in] mode ECDSA mode
 *    - FuriHalEcdsaModeSha256
 *    - FuriHalEcdsaModeSha384
 *    - FuriHalEcdsaModeSha512
 * @param[in] key Pointer to private key
 * @param[in] key_mode Key mode
 *    - FURI_HAL_ECDSA_PRIV_KEY_SIZE_224
 *    - FURI_HAL_ECDSA_PRIV_KEY_SIZE_256
 * @param[in] wrapping_mode Wrapping mode
 *    - FuriHalEcdsaWrappingModeOff
 *    - FuriHalEcdsaWrappingModeOn
 * @return FuriHalEcdsa* ECDSA handle
 */
FuriHalEcdsa* furi_hal_ecdsa_sign_init(
    FuriHalEcdsaMode mode,
    uint8_t* key,
    uint32_t key_mode,
    FuriHalEcdsaWrappingMode wrapping_mode);

/**
 * @brief Initialize ECDSA verification
 * 
 * @param[in] mode ECDSA mode
 *   - FuriHalEcdsaModeSha256
 *   - FuriHalEcdsaModeSha384
 *   - FuriHalEcdsaModeSha512
 * @param[in] key Pointer to public key
 * @param[in] key_mode Key mode
 *   - FURI_HAL_ECDSA_PUB_KEY_SIZE_224
 *   - FURI_HAL_ECDSA_PUB_KEY_SIZE_256
 * @return FuriHalEcdsa* ECDSA handle
 */
FuriHalEcdsa* furi_hal_ecdsa_verify_init(FuriHalEcdsaMode mode, uint8_t* key, uint32_t key_mode);

/**
 * @brief Deinitialize ECDSA handle
 * 
 * @param handle Pointer to ECDSA handle
 */
void furi_hal_ecdsa_deinit(FuriHalEcdsa* handle);

/**
 * @brief Sign data using ECDSA
 * 
 * @param handle Pointer to ECDSA handle
 * @param input Pointer to input data
 * @param input_length Length of input data
 * @param output Pointer to output buffer
 * @param output_length Length of output buffer
 * @return true if the operation was successful, false otherwise
 */
bool furi_hal_ecdsa_sign(
    FuriHalEcdsa* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output,
    size_t* output_length);

/**
 * @brief Verify ECDSA signature
 * 
 * @param handle Pointer to ECDSA handle
 * @param input Pointer to input data
 * @param input_length Length of input data
 * @param signature Pointer to signature
 * @param signature_length Length of signature
 * @return true if the operation was successful, false otherwise
 */
bool furi_hal_ecdsa_verify(
    FuriHalEcdsa* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* signature,
    uint16_t signature_length);
#ifdef __cplusplus
}
#endif
