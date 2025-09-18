#include "BSBCommissionableDataProvider.hpp"

#include <platform/CHIPDeviceConfig.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>

#include "CryptoStorage.hpp"

// Setup code: https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT:YNDA0M.R02-10648G00
// Generated with: $ SetupPayload.py generate -d 1234 -p 20202021 --vendor-id 5514 --product-id 0001 -cf 0 -dm 2

// TODO: Do not hardcode the below values

#define SETUP_DISCRIMINATOR (1234)
#define SETUP_PASSCODE      (20202021)

#define SPAKE2P_ITERATION_COUNT (1000)

namespace chip {
namespace DeviceLayer {

using namespace BSB;

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
    return LoadCryptoStorageItem(FuriHalCryptoKeyTypeMatterSPAKE2Salt, 0, saltBuf);
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pVerifier(
    MutableByteSpan& verifierBuf,
    size_t& outVerifierLen) {
    const auto err =
        LoadCryptoStorageItem(FuriHalCryptoKeyTypeMatterSPAKE2Verifier, 0, verifierBuf);
    outVerifierLen = verifierBuf.size();
    return err;
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
