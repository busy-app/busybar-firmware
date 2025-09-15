#include "BSBCommissionableDataProvider.hpp"

#include <platform/CHIPDeviceConfig.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>

// TODO: Do not hardcode the below values

// Setup code: https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT:YNDA042C00KA0648G00
// Generated with: $ SetupPayload.py generate -d 3840 -p 20202021 --vendor-id 5514 --product-id 32769 -cf 0 -dm 2

#define SETUP_DISCRIMINATOR (3840)
#define SETUP_PASSCODE      (20202021)

// Generated with: spake2p.py gen-verifier -p 20202021 -s U1BBS0UyUCBLZXkgU2FsdA== -i 1000
#define SPAKE2P_ITERATION_COUNT (1000)
#define SPAKE2P_SALT            "U1BBS0UyUCBLZXkgU2FsdA=="
#define SPAKE2P_VERIFIER \
    "uWFwqugDNGiEck/po7KHwwMwwqZgN10XuyBajPGuyzUEV/iree4lOrao5GuwnlQ65CJzbeUB49s31EH+NEkg0JVI5MGCQGMMT/SRPFNRODm3wH/MBiehuFc6FJ/NH6Rmzw=="

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
    setupDiscriminator = SETUP_DISCRIMINATOR;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::SetSetupDiscriminator(uint16_t setupDiscriminator) {
    UNUSED(setupDiscriminator);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pIterationCount(uint32_t& iterationCount) {
    iterationCount = SPAKE2P_ITERATION_COUNT;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pSalt(MutableByteSpan& saltBuf) {
    static constexpr size_t maxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;

    char saltB64[maxBase64Len] = {0};

    const size_t saltB64Len = strlen(SPAKE2P_SALT);
    ReturnErrorCodeIf(saltB64Len > sizeof(saltB64), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(saltB64, SPAKE2P_SALT, saltB64Len);

    size_t saltLen = chip::Base64Decode32(
        saltB64, static_cast<uint32_t>(saltB64Len), reinterpret_cast<uint8_t*>(saltB64));

    ReturnErrorCodeIf(saltLen > saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), saltB64, saltLen);
    saltBuf.reduce_size(saltLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pVerifier(
    MutableByteSpan& verifierBuf,
    size_t& outVerifierLen) {
    static constexpr size_t maxBase64Len =
        BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_VerifierSerialized_Length) + 1;

    char verifierB64[maxBase64Len] = {0};

    const size_t verifierB64Len = strlen(SPAKE2P_VERIFIER);
    ReturnErrorCodeIf(verifierB64Len > sizeof(verifierB64), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(verifierB64, SPAKE2P_VERIFIER, verifierB64Len);

    outVerifierLen = chip::Base64Decode32(
        verifierB64,
        static_cast<uint32_t>(verifierB64Len),
        reinterpret_cast<uint8_t*>(verifierB64));

    ReturnErrorCodeIf(outVerifierLen > verifierBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), verifierB64, outVerifierLen);
    verifierBuf.reduce_size(outVerifierLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSetupPasscode(uint32_t& setupPasscode) {
    setupPasscode = SETUP_PASSCODE;
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
