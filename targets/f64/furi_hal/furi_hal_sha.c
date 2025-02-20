#include <furi.h>
#include <furi_hal_sha.h>
#include <sl_si91x_sha.h>

#define TAG "SHA"

static const sl_si91x_crypto_sha_mode_t furi_hal_sha_mode[] = {
    [FuriHalShaModeSha1] = SL_SI91X_SHA_1,
    [FuriHalShaModeSha256] = SL_SI91X_SHA_256,
    [FuriHalShaModeSha384] = SL_SI91X_SHA_384,
    [FuriHalShaModeSha512] = SL_SI91X_SHA_512,
    [FuriHalShaModeSha244] = SL_SI91X_SHA_224,
};

bool furi_hal_sha(
    FuriHalShaMode sha_mode,
    uint8_t* msg,
    uint16_t msg_length,
    uint8_t* digest,
    size_t digest_length) {
    furi_check(sha_mode < FuriHalShaModeMAX, "Invalid SHA mode");
    furi_assert(msg && msg_length && digest);
    furi_assert(
        (sha_mode == FuriHalShaModeSha1 && digest_length == FURI_HAL_SHA1_DIGEST_SIZE) ||
        (sha_mode == FuriHalShaModeSha256 && digest_length == FURI_HAL_SHA256_DIGEST_SIZE) ||
        (sha_mode == FuriHalShaModeSha384 && digest_length == FURI_HAL_SHA384_DIGEST_SIZE) ||
        (sha_mode == FuriHalShaModeSha512 && digest_length == FURI_HAL_SHA512_DIGEST_SIZE) ||
        (sha_mode == FuriHalShaModeSha244 && digest_length == FURI_HAL_SHA224_DIGEST_SIZE));
    sl_status_t status = sl_si91x_sha(furi_hal_sha_mode[sha_mode], msg, msg_length, digest);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed, Error Code : 0x%lX", status);
        return false;
    }
    return true;
}
