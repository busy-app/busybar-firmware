#include "CommissionableDataProviderImpl.hpp"

// Setup code: https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT:SQ4R285W01VVV110000
// Generated with: SetupPayload.py generate -d 2025 -p 4269 -vid 51966 -pid 51914 -cf 0

static constexpr uint16_t sSetupDiscriminatorDummy = 2025;
static constexpr uint16_t sSetupPasscodeDummy = 4269;

// Generated with: spake2p.py gen-verifier -p 4269 -i 1000 -s SGVsbG8gdGhlcmUhIEdlbmVyYWwgS2Vub2JpIQ==
// Output Base64 verifier: uGTWf8lEcjn25E1cSjLjt8dt2RY21vkh5RmVghj8U74E7B1DWoFyrNiCBxujeoo2Qi+tX+AT7Dyf6NiReBLu/woruQATnjs4sNNvZ14iMpl1RcHlayGozifhO/0wz7/3rA==

static constexpr uint32_t sSpake2pIterationCountDummy = 1000;
static constexpr char sSpake2pSaltDummy[] = "Hello there! General Kenobi!";
static constexpr uint8_t sSpake2pVerifierDummy[] = {
    0xb8, 0x64, 0xd6, 0x7f, 0xc9, 0x44, 0x72, 0x39, 0xf6, 0xe4, 0x4d, 0x5c, 0x4a, 0x32,
    0xe3, 0xb7, 0xc7, 0x6d, 0xd9, 0x16, 0x36, 0xd6, 0xf9, 0x21, 0xe5, 0x19, 0x95, 0x82,
    0x18, 0xfc, 0x53, 0xbe, 0x04, 0xec, 0x1d, 0x43, 0x5a, 0x81, 0x72, 0xac, 0xd8, 0x82,
    0x07, 0x1b, 0xa3, 0x7a, 0x8a, 0x36, 0x42, 0x2f, 0xad, 0x5f, 0xe0, 0x13, 0xec, 0x3c,
    0x9f, 0xe8, 0xd8, 0x91, 0x78, 0x12, 0xee, 0xff, 0x0a, 0x2b, 0xb9, 0x00, 0x13, 0x9e,
    0x3b, 0x38, 0xb0, 0xd3, 0x6f, 0x67, 0x5e, 0x22, 0x32, 0x99, 0x75, 0x45, 0xc1, 0xe5,
    0x6b, 0x21, 0xa8, 0xce, 0x27, 0xe1, 0x3b, 0xfd, 0x30, 0xcf, 0xbf, 0xf7, 0xac,
};

namespace chip {
namespace DeviceLayer {

class CommissionableDataProviderImpl : public CommissionableDataProvider {
public:
    CHIP_ERROR GetSetupDiscriminator(uint16_t& setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t& iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan& saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan& verifierBuf, size_t& outVerifierLen) override;
    CHIP_ERROR GetSetupPasscode(uint32_t& setupPasscode) override;
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override;
};

CHIP_ERROR CommissionableDataProviderImpl::GetSetupDiscriminator(uint16_t& setupDiscriminator) {
    setupDiscriminator = sSetupDiscriminatorDummy;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::SetSetupDiscriminator(uint16_t setupDiscriminator) {
    UNUSED(setupDiscriminator);
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pIterationCount(uint32_t& iterationCount) {
    iterationCount = sSpake2pIterationCountDummy;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pSalt(MutableByteSpan& saltBuf) {
    const ByteSpan tmp(
        reinterpret_cast<const uint8_t*>(sSpake2pSaltDummy), strlen(sSpake2pSaltDummy));
    return CopySpanToMutableSpan(tmp, saltBuf);
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pVerifier(
    MutableByteSpan& verifierBuf,
    size_t& outVerifierLen) {
    const ByteSpan tmp(sSpake2pVerifierDummy, sizeof(sSpake2pVerifierDummy));

    const CHIP_ERROR err = CopySpanToMutableSpan(tmp, verifierBuf);
    if(err == CHIP_NO_ERROR) {
        outVerifierLen = tmp.size();
    }

    return err;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSetupPasscode(uint32_t& setupPasscode) {
    setupPasscode = sSetupPasscodeDummy;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::SetSetupPasscode(uint32_t setupPasscode) {
    UNUSED(setupPasscode);
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

namespace BSB {
CommissionableDataProvider* GetCommissionableDataProvider(void) {
    static CommissionableDataProviderImpl provider;
    return &provider;
}
} // namespace BSB

} // namespace DeviceLayer
} // namespace Chip
