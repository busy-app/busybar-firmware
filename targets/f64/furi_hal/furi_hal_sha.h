#pragma once

typedef enum {
    FuriHalShaModeSha1,
    FuriHalShaModeSha256,
    FuriHalShaModeSha384,
    FuriHalShaModeSha512,
    FuriHalShaModeSha244,
    FuriHalShaModeMAX,
} FuriHalShaMode;

typedef enum {
    FuriHalShaLengthSha1 = 20, // Digest length for SHA 1
    FuriHalShaLengthSha244 = 28, // Digest length for SHA 224
    FuriHalShaLengthSha256 = 32, // Digest length for SHA 256
    FuriHalShaLengthSha384 = 48, // Digest length for SHA 384
    FuriHalShaLengthSha512 = 64, // Digest length for SHA 512
} FuriHalShaLength;

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
 * @return 
 *   true - Success
 *   false - Failure
 */
bool furi_hal_sha(FuriHalShaMode sha_mode, uint8_t* msg, uint16_t msg_length, uint8_t* digest);
#ifdef __cplusplus
}
#endif
