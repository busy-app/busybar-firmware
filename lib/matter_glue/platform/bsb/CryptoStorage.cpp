#include "CryptoStorage.hpp"

#include <furi_hal_crypto.h>

#include <crypto/CHIPCryptoPAL.h>

namespace chip {
namespace DeviceLayer {
namespace BSB {

CHIP_ERROR LoadCryptoStorageItem(
    FuriHalCryptoKeyType key_type,
    uint32_t key_id,
    MutableByteSpan& out_buf) {
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;

    FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);

    do {
        FuriHalCryptoStatus status;
        status = furi_hal_crypto_storage_read(key, key_type, key_id);

        if(status != FuriHalCryptoStatusOk) {
            ChipLogError(
                Crypto,
                "Failed to read item with type %d and id %lu: 0x%X",
                key_type,
                key_id,
                status);
            break;
        }

        err = CopySpanToMutableSpan(ByteSpan{key->data, key->header.size}, out_buf);

        if(!CHIP_ERROR::IsSuccess(err)) {
            ChipLogError(
                Crypto,
                "Failed to copy %d bytes to output of %zu bytes",
                key->header.size,
                out_buf.size());
        }

    } while(false);

    furi_hal_crypto_storage_free(key);

    return err;
}

CHIP_ERROR SignWithECDSA256Key(
    FuriHalCryptoKeyType key_type,
    uint32_t key_id,
    const ByteSpan& message,
    MutableByteSpan& out_buf) {
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;

    FuriHalCryptoKey* private_key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);

    do {
        const FuriHalCryptoStatus crypto_status =
            furi_hal_crypto_storage_read(private_key, key_type, key_id);

        if(crypto_status != FuriHalCryptoStatusOk) {
            ChipLogError(Crypto, "Failed to read device private key: 0x%X", crypto_status);
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
            break;
        }

        err = Crypto::EcdsaAsn1SignatureToRaw(
            Crypto::kP256_FE_Length, ByteSpan{asn1_sig, asn1_sig_len}, out_buf);

    } while(false);

    furi_hal_crypto_storage_free(private_key);

    return err;
}

} // namespace BSB
} // namespace DeviceLayer
} // namespace Chip
