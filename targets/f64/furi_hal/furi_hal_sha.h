#pragma once

#define FURI_HAL_SHA1_DIGEST_SIZE   20
#define FURI_HAL_SHA256_DIGEST_SIZE 32
#define FURI_HAL_SHA384_DIGEST_SIZE 48
#define FURI_HAL_SHA512_DIGEST_SIZE 64
#define FURI_HAL_SHA224_DIGEST_SIZE 28

typedef enum {
    FuriHalShaModeSha1,
    FuriHalShaModeSha256,
    FuriHalShaModeSha384,
    FuriHalShaModeSha512,
    FuriHalShaModeSha244,
    FuriHalShaModeMAX,
} FuriHalShaMode;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief 
 *   To provide the SHA output for the given configuration. This is a blocking API.
 * @param[in] sha_mode 
 *   FuriHalShaModeSha1 – For SHA1 
 *   FuriHalShaModeSha256 – For SHA256 
 *   FuriHalShaModeSha384 – For SHA384 
 *   FuriHalShaModeSha512 – For SHA512 
 *   FuriHalShaModeSha244 – For SHA224 
 * @param[in] msg Pointer to the message buffer
 * @param[in] msg_length Length of the message buffer
 * @param[out] digest Pointer to the digest buffer
 * @param[in] digest_length Length of the digest buffer
 * @return 
 *   true - Success
 *   false - Failure
 */
bool furi_hal_sha(
    FuriHalShaMode sha_mode,
    uint8_t* msg,
    uint16_t msg_length,
    uint8_t* digest,
    size_t digest_length);
#ifdef __cplusplus
}
#endif
