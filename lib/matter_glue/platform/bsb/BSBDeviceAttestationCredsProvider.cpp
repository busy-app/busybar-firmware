#include "BSBDeviceAttestationCredsProvider.hpp"

#include "CryptoStorage.hpp"

namespace chip {
namespace Credentials {
namespace BSB {

namespace KeyId {

enum {
    PK,
    DAC,
    PAI,
    CD,
};

}; // namespace KeyId

using namespace DeviceLayer::BSB;

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

CHIP_ERROR BSBDACProvider::GetCertificationDeclaration(MutableByteSpan& out_cd_buffer) {
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterAttestation, KeyId::CD, out_cd_buffer);
}

CHIP_ERROR BSBDACProvider::GetFirmwareInformation(MutableByteSpan& out_firmware_info_buffer) {
    // TODO: Figure out firmware information
    out_firmware_info_buffer.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BSBDACProvider::GetDeviceAttestationCert(MutableByteSpan& out_dac_buffer) {
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterAttestation, KeyId::DAC, out_dac_buffer);
}

CHIP_ERROR BSBDACProvider::GetProductAttestationIntermediateCert(MutableByteSpan& out_pai_buffer) {
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterAttestation, KeyId::PAI, out_pai_buffer);
}

CHIP_ERROR BSBDACProvider::SignWithDeviceAttestationKey(
    const ByteSpan& message_to_sign,
    MutableByteSpan& out_signature_buffer) {
    return SignWithECDSA256Key(
        FuriHalCryptoKeyTypeMatterAttestation, KeyId::PK, message_to_sign, out_signature_buffer);
}

DeviceAttestationCredentialsProvider* GetDeviceAttestationCredentialsProvider(void) {
    static BSBDACProvider provider;
    return &provider;
}

} // namespace BSB
} // namespace Credentials
} // namespace chip
