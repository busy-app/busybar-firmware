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
};

}; // namespace KeyId

using namespace DeviceLayer::BSB;

void BSBDACProvider::SetCertificationDeclaration(const void* buffer, size_t size) {
    furi_check(buffer);
    furi_check(!m_cd_buffer);
    furi_check(!m_cd_size);

    if(!size) return;

    m_cd_buffer = malloc(size);
    m_cd_size = size;
    memcpy(m_cd_buffer, buffer, size);
}

CHIP_ERROR BSBDACProvider::GetCertificationDeclaration(MutableByteSpan& out_cd_buffer) {
    if(!m_cd_buffer) return CHIP_ERROR_CERT_NOT_FOUND;

    size_t to_copy = MIN(m_cd_size, out_cd_buffer.size());
    memcpy(out_cd_buffer.begin(), m_cd_buffer, to_copy);

    return CHIP_NO_ERROR;
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

BSBDACProvider* GetDeviceAttestationCredentialsProvider(void) {
    static BSBDACProvider provider;
    return &provider;
}

} // namespace BSB
} // namespace Credentials
} // namespace chip
