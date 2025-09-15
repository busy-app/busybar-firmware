#include "BSBDeviceAttestationCredsProvider.hpp"

#include <lib/support/Span.h>

#include <crypto/CHIPCryptoPAL.h>

#include <furi_hal_crypto.h>
#include <furi_hal_crypto_storage.h>

namespace chip {
namespace Credentials {
namespace BSB {

class BSBDACProvider : public DeviceAttestationCredentialsProvider {
public:
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan& out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan& out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan& out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan& out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(
        const ByteSpan& message_to_sign,
        MutableByteSpan& out_signature_buffer) override;
};

static CHIP_ERROR LoadCryptoStorageItem(
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

CHIP_ERROR BSBDACProvider::GetCertificationDeclaration(MutableByteSpan& out_cd_buffer) {
    return LoadCryptoStorageItem(FuriHalCryptoKeyTypeMatterCD, 0, out_cd_buffer);
}

CHIP_ERROR BSBDACProvider::GetFirmwareInformation(MutableByteSpan& out_firmware_info_buffer) {
    // TODO: Figure out firmware information
    out_firmware_info_buffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BSBDACProvider::GetDeviceAttestationCert(MutableByteSpan& out_dac_buffer) {
    return LoadCryptoStorageItem(FuriHalCryptoKeyTypeMatterDAC, 0, out_dac_buffer);
}

CHIP_ERROR BSBDACProvider::GetProductAttestationIntermediateCert(MutableByteSpan& out_pai_buffer) {
    return LoadCryptoStorageItem(FuriHalCryptoKeyTypeMatterPAI, 0, out_pai_buffer);
}

CHIP_ERROR BSBDACProvider::SignWithDeviceAttestationKey(
    const ByteSpan& message_to_sign,
    MutableByteSpan& out_signature_buffer) {
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;

    FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);

    do {
        const FuriHalCryptoStatus status =
            furi_hal_crypto_storage_read(key, FuriHalCryptoKeyTypeEcdsaPriv256, 0);

        if(status != FuriHalCryptoStatusOk) {
            ChipLogError(Crypto, "Failed to read device private key: 0x%X", status);
            break;
        }

        uint8_t signature[FURI_HAL_CRYPTO_ECDSA_MAX_SIGNATURE_SIZE] = {0};
        size_t signature_length = 0;

        FuriHalCryptoEcdsa* handle = furi_hal_crypto_ecdsa_sign_init(
            FuriHalCryptoEcdsaModeSha256,
            key->data,
            FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256,
            FuriHalCryptoWrappingModeOff);

        const bool sign_success = furi_hal_crypto_ecdsa_sign(
            handle, message_to_sign.data(), message_to_sign.size(), signature, &signature_length);

        furi_hal_crypto_ecdsa_deinit(handle);

        if(!sign_success) {
            ChipLogError(Crypto, "Failed to sign with device attestation key");
            break;
        }

        err = chip::Crypto::EcdsaAsn1SignatureToRaw(
            chip::Crypto::kP256_FE_Length,
            ByteSpan{signature, signature_length},
            out_signature_buffer);

    } while(false);

    furi_hal_crypto_storage_free(key);

    return err;
}

DeviceAttestationCredentialsProvider* GetDeviceAttestationCredentialsProvider(void) {
    static BSBDACProvider provider;
    return &provider;
}

} // namespace BSB
} // namespace Credentials
} // namespace chip
