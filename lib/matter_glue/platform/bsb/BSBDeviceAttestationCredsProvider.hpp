#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>

namespace chip {
namespace Credentials {
namespace BSB {

class BSBDACProvider : public DeviceAttestationCredentialsProvider {
private:
    void* m_cd_buffer = NULL;
    size_t m_cd_size = 0;
public:
    void SetCertificationDeclaration(const void* buffer, size_t size);
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan& out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan& out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan& out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan& out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(
        const ByteSpan& message_to_sign,
        MutableByteSpan& out_signature_buffer) override;
};

BSBDACProvider* GetDeviceAttestationCredentialsProvider(void);

} // namespace BSB
} // namespace Credentials
} // namespace chip
