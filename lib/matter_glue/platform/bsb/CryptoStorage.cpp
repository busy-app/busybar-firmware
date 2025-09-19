#include "CryptoStorage.hpp"

#include <furi_hal_crypto.h>

#include <crypto/CHIPCryptoPAL.h>

namespace chip {
namespace DeviceLayer {
namespace BSB {

static CHIP_ERROR TranslateFuriHalCryptoStatus(FuriHalCryptoStatus status) {
    switch(status) {
    case FuriHalCryptoStatusOk:
        return CHIP_NO_ERROR;
    case FuriHalCryptoStatusFail:
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    case FuriHalCryptoStatusFailWrite:
        return CHIP_ERROR_WRITE_FAILED;
    case FuriHalCryptoStatusStorageFull:
        return CHIP_ERROR_NO_MEMORY;
    case FuriHalCryptoStatusDuplicate:
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    case FuriHalCryptoStatusNotFound:
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    case FuriHalCryptoStatusErrorCrc:
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    default:
        return CHIP_ERROR_INTERNAL;
    }
}

MutableByteSpan ToMutableByteSpan(char* buf, size_t bufSize) {
    return {reinterpret_cast<uint8_t*>(buf), bufSize};
}

CHIP_ERROR LoadCryptoStorageKey(
    FuriHalCryptoKeyType key_type,
    uint32_t key_id,
    MutableByteSpan& out_span) {
    CHIP_ERROR err;

    FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);

    do {
        err = TranslateFuriHalCryptoStatus(furi_hal_crypto_storage_read(key, key_type, key_id));

        if(!CHIP_ERROR::IsSuccess(err)) {
            break;
        }

        err = CopySpanToMutableSpan(ByteSpan{key->data, key->header.size}, out_span);

    } while(false);

    furi_hal_crypto_storage_free(key);

    return err;
}

CHIP_ERROR SignWithECDSA256Key(
    FuriHalCryptoKeyType key_type,
    uint32_t key_id,
    const ByteSpan& message,
    MutableByteSpan& out_span) {
    CHIP_ERROR err;

    FuriHalCryptoKey* private_key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);

    do {
        err = TranslateFuriHalCryptoStatus(
            furi_hal_crypto_storage_read(private_key, key_type, key_id));

        if(!CHIP_ERROR::IsSuccess(err)) {
            break;
        }

        const FuriHalCryptoWrappingMode wrap_mode =
            private_key->header.flags & FuriHalCryptoKeyFlagWrap ? FuriHalCryptoWrappingModeOn :
                                                                   FuriHalCryptoWrappingModeOff;

        FuriHalCryptoEcdsa* ecdsa = furi_hal_crypto_ecdsa_sign_init(
            FuriHalCryptoEcdsaModeSha256,
            private_key->data,
            FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256,
            wrap_mode);

        uint8_t asn1_sig[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE] = {0};
        size_t asn1_sig_len = 0;

        const bool sign_success = furi_hal_crypto_ecdsa_sign(
            ecdsa, message.data(), message.size(), asn1_sig, &asn1_sig_len);

        furi_hal_crypto_ecdsa_deinit(ecdsa);

        if(!sign_success) {
            ChipLogError(Crypto, "Failed to sign message with device private key");
            err = CHIP_ERROR_INTERNAL;
            break;
        }

        err = Crypto::EcdsaAsn1SignatureToRaw(
            Crypto::kP256_FE_Length, ByteSpan{asn1_sig, asn1_sig_len}, out_span);

    } while(false);

    furi_hal_crypto_storage_free(private_key);

    return err;
}

} // namespace BSB
} // namespace DeviceLayer
} // namespace Chip
