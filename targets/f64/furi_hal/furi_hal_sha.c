#include <furi.h>
#include <furi_hal_sha.h>
#include <sl_si91x_sha.h>

#define TAG "SHA"

const sl_si91x_crypto_sha_mode_t furi_hal_sha_mode[] = {
    [FuriHalShaModeSha1] = SL_SI91X_SHA_1,
    [FuriHalShaModeSha256] = SL_SI91X_SHA_256,
    [FuriHalShaModeSha384] = SL_SI91X_SHA_384,
    [FuriHalShaModeSha512] = SL_SI91X_SHA_512,
    [FuriHalShaModeSha244] = SL_SI91X_SHA_224,
};

bool furi_hal_sha(FuriHalShaMode sha_mode, uint8_t* msg, uint16_t msg_length, uint8_t* digest) {
    furi_check(sha_mode < FuriHalShaModeMAX, "Invalid SHA mode");
    furi_assert(msg);
    furi_assert(msg_length);
    furi_assert(digest);
    sl_status_t status = sl_si91x_sha(furi_hal_sha_mode[sha_mode], msg, msg_length, digest);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed, Error Code : 0x%lX", status);
        return false;
    }
    return true;
}
