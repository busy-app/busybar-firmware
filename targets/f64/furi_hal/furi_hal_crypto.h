#pragma once

#include <stdint.h>
#include <stddef.h>

#define FURI_HAL_CRYPTO_DATA_SIZE_MAX (996UL) // Maximum data size for keys

typedef enum {
    FuriHalCryptoWrappingModeOff,
    FuriHalCryptoWrappingModeOn,
} FuriHalCryptoWrappingMode;

//#################### AES ####################
typedef struct FuriHalCryptoAes FuriHalCryptoAes;

#define FURI_HAL_CRYPTO_AES_IV_SIZE      16
#define FURI_HAL_CRYPTO_AES_KEY_SIZE_128 16
#define FURI_HAL_CRYPTO_AES_KEY_SIZE_192 24
#define FURI_HAL_CRYPTO_AES_KEY_SIZE_256 32

typedef enum {
    FuriHalCryptoAesModeCBC,
    FuriHalCryptoAesModeECB,
    FuriHalCryptoAesModeCTR,
} FuriHalCryptoAesMode;

//#################### ECDSA ####################
typedef struct FuriHalCryptoEcdsa FuriHalCryptoEcdsa;

#define FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224      28
#define FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256      32
#define FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_224       57
#define FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256       65
#define FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE     72
#define FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224_BITS 224
#define FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256_BITS 256

typedef enum {
    FuriHalCryptoEcdsaModeSha256,
    FuriHalCryptoEcdsaModeSha384,
    FuriHalCryptoEcdsaModeSha512,
} FuriHalCryptoEcdsaMode;

//#################### HMAC ####################
typedef struct FuriHalCryptoHmac FuriHalCryptoHmac;

#define FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE   20
#define FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE 32
#define FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE 48
#define FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE 64

typedef enum {
    FuriHalCryptoHmacShaModeSha1,
    FuriHalCryptoHmacShaModeSha256,
    FuriHalCryptoHmacShaModeSha384,
    FuriHalCryptoHmacShaModeSha512,
} FuriHalCryptoHmacShaMode;

//#################### SHA ####################
#define FURI_HAL_CRYPTO_SHA1_DIGEST_SIZE   20
#define FURI_HAL_CRYPTO_SHA256_DIGEST_SIZE 32
#define FURI_HAL_CRYPTO_SHA384_DIGEST_SIZE 48
#define FURI_HAL_CRYPTO_SHA512_DIGEST_SIZE 64
#define FURI_HAL_CRYPTO_SHA224_DIGEST_SIZE 28

typedef enum {
    FuriHalCryptoShaModeSha1,
    FuriHalCryptoShaModeSha256,
    FuriHalCryptoShaModeSha384,
    FuriHalCryptoShaModeSha512,
    FuriHalCryptoShaModeSha244,
    FuriHalCryptoShaModeMAX,
} FuriHalCryptoShaMode;

typedef enum {
    FuriHalCryptoKeyTypeAes128,
    FuriHalCryptoKeyTypeAes192,
    FuriHalCryptoKeyTypeAes256,
    FuriHalCryptoKeyTypeHmacSha1,
    FuriHalCryptoKeyTypeHmacSha256,
    FuriHalCryptoKeyTypeHmacSha384,
    FuriHalCryptoKeyTypeHmacSha512,
    FuriHalCryptoKeyTypeEcdsaPriv224,
    FuriHalCryptoKeyTypeEcdsaPriv256,
    FuriHalCryptoKeyTypeEcdsaPub224,
    FuriHalCryptoKeyTypeEcdsaPub256,

    FuriHalCryptoKeyTypeCsrDerEcdsa256,
    FuriHalCryptoKeyTypeCrtDerEcdsa256,

    FuriHalCryptoKeyTypeMatterAttestation,
    FuriHalCryptoKeyTypeMatterSetup,
    FuriHalCryptoKeyTypeMatterDeviceInfo,

    FuriHalCryptoKeyTypeNone = 0xFFFFFFFF,
} FuriHalCryptoKeyType;
_Static_assert(sizeof(FuriHalCryptoKeyType) == 4, "Size check for 'FuriHalCryptoKeyType' failed.");

typedef enum {
    FuriHalCryptoKeyFlagWrap = (1 << 0UL),
    FuriHalCryptoKeyFlagNone = 0xFFFFFFFF,
} FuriHalCryptoKeyFlag;
_Static_assert(sizeof(FuriHalCryptoKeyFlag) == 4, "Size check for 'FuriHalCryptoKeyFlag' failed.");

typedef struct FuriHalCryptoKey {
    FuriHalCryptoKeyType type;
    FuriHalCryptoKeyFlag flags;
    uint16_t length;
    uint8_t data[FURI_HAL_CRYPTO_DATA_SIZE_MAX];
} FuriHalCryptoKey;

typedef enum {
    FuriHalCryptoStatusOk,
    FuriHalCryptoStatusFail,
    FuriHalCryptoStatusFailWrite,
    FuriHalCryptoStatusStorageFull,
    FuriHalCryptoStatusDuplicate,
    FuriHalCryptoStatusNotFound,
    FuriHalCryptoStatusErrorCrc,
    FuriHalCryptoStatusWrongType,
    FuriHalCryptoStatusUnavailable,
} FuriHalCryptoStatus;

#ifdef __cplusplus
extern "C" {
#endif

FuriHalCryptoKey* furi_hal_crypto_key_alloc(void);

void furi_hal_crypto_key_free(FuriHalCryptoKey* key);

//#################### AES ####################
/**
 * Initialize the AES module.
 * 
 * @param[in] mode AES mode.
 *     - FuriHalCryptoAesModeCBC
 *     - FuriHalCryptoAesModeECB
 *     - FuriHalCryptoAesModeCTR
 * @param[in] key Pointer to the key.
 * @param[in] key_size Size of the key.
 *     - FURI_HAL_CRYPTO_AES_KEY_SIZE_128
 *     - FURI_HAL_CRYPTO_AES_KEY_SIZE_192
 *     - FURI_HAL_CRYPTO_AES_KEY_SIZE_256
 * @param[in] wrapping_mode Wrapping mode.
 *     - FuriHalCryptoWrappingModeOff
 *     - FuriHalCryptoWrappingModeOn
 * @return status of the operation
 *    - FuriHalCryptoStatusOk on success
 *    - FuriHalCryptoStatusWrongType if supplied key is of a wrong type
 */
FuriHalCryptoStatus furi_hal_crypto_aes_init(
    FuriHalCryptoAes** handle,
    FuriHalCryptoAesMode mode,
    const FuriHalCryptoKey* key);

/**
 * Deinitialize the AES module.
 * 
 * @param handle Pointer to the AES handle.
 */
void furi_hal_crypto_aes_deinit(FuriHalCryptoAes* handle);

/**
 * Encrypt data using AES.
 * 
 * @param[in] handle Pointer to the AES handle.
 * @param[in] iv Pointer to the initialization vector.
 * @param[in] input Pointer to the input data.
 * @param[in] input_length Length of the input data.
 * @param[out] output Pointer to the output buffer.
 * @returns true if the operation was successful, false otherwise.
 */
bool furi_hal_crypto_aes_encrypt(
    FuriHalCryptoAes* handle,
    uint8_t* iv,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output);

/**
 * Decrypt data using AES.
 * 
 * @param[in] handle Pointer to the AES handle.
 * @param[in] iv Pointer to the initialization vector.
 * @param[in] input Pointer to the input data.
 * @param[in] input_length Length of the input data.
 * @param[out] output Pointer to the output buffer.
 * @returns true if the operation was successful, false otherwise.
 */
bool furi_hal_crypto_aes_decrypt(
    FuriHalCryptoAes* handle,
    uint8_t* iv,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output);

//#################### ECDSA ####################
/**
 * @brief Initialize ECDSA signing
 *
 * @param[out] handle ECDSA handle
 * @param[in] mode ECDSA mode
 *    - FuriHalCryptoEcdsaModeSha256
 *    - FuriHalCryptoEcdsaModeSha384
 *    - FuriHalCryptoEcdsaModeSha512
 * @param[in] key Pointer to private key
 * @param[in] key_size Key size
 *    - FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224
 *    - FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256
 * @param[in] wrapping_mode Wrapping mode
 *    - FuriHalCryptoWrappingModeOff
 *    - FuriHalCryptoWrappingModeOn
 * @return status of the operation
 *    - FuriHalCryptoStatusOk on success
 *    - FuriHalCryptoStatusWrongType if supplied key is of a wrong type
 */
FuriHalCryptoStatus furi_hal_crypto_ecdsa_sign_init(
    FuriHalCryptoEcdsa** handle,
    FuriHalCryptoEcdsaMode mode,
    const FuriHalCryptoKey* key);

/**
 * @brief Initialize ECDSA verification
 * 
 * @param[out] handle ECDSA handle
 * @param[in] mode ECDSA mode
 *   - FuriHalCryptoEcdsaModeSha256
 *   - FuriHalCryptoEcdsaModeSha384
 *   - FuriHalCryptoEcdsaModeSha512
 * @param[in] key Pointer to public key
 * @param[in] key_size Key size
 *   - FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_224
 *   - FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256
 * @return status of the operation
 *    - FuriHalCryptoStatusOk on success
 *    - FuriHalCryptoStatusWrongType if supplied key is of a wrong type
 */
FuriHalCryptoStatus furi_hal_crypto_ecdsa_verify_init(
    FuriHalCryptoEcdsa** handle,
    FuriHalCryptoEcdsaMode mode,
    const FuriHalCryptoKey* key);

/**
 * @brief Deinitialize ECDSA handle
 * 
 * @param handle Pointer to ECDSA handle
 */
void furi_hal_crypto_ecdsa_deinit(FuriHalCryptoEcdsa* handle);

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
bool furi_hal_crypto_ecdsa_sign(
    FuriHalCryptoEcdsa* handle,
    const uint8_t* input,
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
bool furi_hal_crypto_ecdsa_verify(
    FuriHalCryptoEcdsa* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* signature,
    uint16_t signature_length);

//#################### HMAC ####################
/**
 * Initialize HMAC.
 *
 * @param[in] mode HMAC SHA mode.
 *    - FuriHalCryptoHmacShaModeSha1
 *    - FuriHalCryptoHmacShaModeSha256
 *    - FuriHalCryptoHmacShaModeSha384
 *    - FuriHalCryptoHmacShaModeSha512
 * @param[in] key Pointer to the key.
 * @param[in] key_size Size of the key.
 *    - FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE
 *    - FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE
 *    - FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE
 *    - FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE
 * @param[in] wrapping_mode Wrapping mode.
 *    - FuriHalCryptoWrappingModeOff
 *    - FuriHalCryptoWrappingModeOn
 * @return Handle to the HMAC.
 */
FuriHalCryptoHmac* furi_hal_crypto_hmac_init(
    FuriHalCryptoHmacShaMode mode,
    uint8_t* key,
    size_t key_size,
    FuriHalCryptoWrappingMode wrapping_mode);

/**
 * Deinitialize HMAC.
 *
 * @param[in] handle Handle to the HMAC.
 */
void furi_hal_crypto_hmac_deinit(FuriHalCryptoHmac* handle);

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
bool furi_hal_crypto_hmac_digest(
    FuriHalCryptoHmac* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output,
    size_t output_length);

//#################### SHA ####################
/**
 * @brief 
 *   To provide the SHA output for the given configuration. This is a blocking API.
 * @param[in] sha_mode 
 *   FuriHalCryptoShaModeSha1 – For SHA1 
 *   FuriHalCryptoShaModeSha256 – For SHA256 
 *   FuriHalCryptoShaModeSha384 – For SHA384 
 *   FuriHalCryptoShaModeSha512 – For SHA512 
 *   FuriHalCryptoShaModeSha244 – For SHA224 
 * @param[in] msg Pointer to the message buffer
 * @param[in] msg_length Length of the message buffer
 * @param[out] digest Pointer to the digest buffer
 * @param[in] digest_length Length of the digest buffer
 * @return 
 *   true - Success
 *   false - Failure
 */
bool furi_hal_crypto_sha(
    FuriHalCryptoShaMode sha_mode,
    uint8_t* msg,
    uint16_t msg_length,
    uint8_t* digest,
    size_t digest_length);

//#################### Wrap Key ####################
/**
 * Wrap a key. The key is wrapped using the key wrapping algorithm.
 *
 * @param[in] uint8_t* key Pointer to the key.
 * @param[out] uint8_t* wrapped_key Pointer to the wrapped key.
 */
FuriHalCryptoStatus
    furi_hal_crypto_wrap_key(const FuriHalCryptoKey* key, FuriHalCryptoKey* wrapped_key);

FuriHalCryptoStatus
    furi_hal_crypto_wrap_raw_key(size_t size, const uint8_t* src_buf, uint8_t* dst_buf);

//#################### Key generation ##############

/** Generate a random buffer of the specified size.
* @param[out] buf Pointer to the buffer to fill with random data.
* @param[in] size Size of the buffer to fill.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_gen_random_buf(uint8_t* buf, size_t size);

/** Generate a random key.
 * @param[in] type key type to generate.
 */
FuriHalCryptoStatus furi_hal_crypto_gen_random_key(
    FuriHalCryptoKey* key,
    FuriHalCryptoKeyType type,
    FuriHalCryptoKeyFlag flags);

/** Generate an asymmetric public key from a private key.
* @param[in] priv_key Pointer to the private key.
* @param[out] pub_key Pointer to the public key.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_gen_asymmetric_pub_key(
    const FuriHalCryptoKey* priv_key,
    FuriHalCryptoKey* pub_key);

/** Generate a CSR in DER format for ECDSA 256.
* @param[in] priv_key Pointer to the private key.
* @param[out] csr_der_key Pointer to the CSR DER key.
* @param[in] subject_name Subject name for the CSR.
* @return FuriHalCryptoStatus indicating the result of the operation.
*/
FuriHalCryptoStatus furi_hal_crypto_gen_csr_der_ecdsa256(
    const FuriHalCryptoKey* priv_key,
    FuriHalCryptoKey* csr_der_key,
    const char* subject_name);

//#################### Util ##############
/** Get human-readable key type name */
const char* furi_hal_crypto_get_key_type_name(FuriHalCryptoKeyType type);

#ifdef __cplusplus
}
#endif
