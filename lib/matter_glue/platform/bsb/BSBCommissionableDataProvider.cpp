#include "BSBCommissionableDataProvider.hpp"

#include "CryptoStorage.hpp"

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
    auto out_buf = MutableByteSpan{
        reinterpret_cast<uint8_t*>(&setupDiscriminator), sizeof(setupDiscriminator)};
    return LoadCryptoStorageItem(FuriHalCryptoKeyTypeMatterDiscriminator, 0, out_buf);
}

CHIP_ERROR CommissionableDataProviderImpl::SetSetupDiscriminator(uint16_t setupDiscriminator) {
    UNUSED(setupDiscriminator);
    return CHIP_ERROR_NOT_IMPLEMENTED;
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
    auto out_buf =
        MutableByteSpan{reinterpret_cast<uint8_t*>(&setupPasscode), sizeof(setupPasscode)};
    return LoadCryptoStorageItem(FuriHalCryptoKeyTypeMatterPasscode, 0, out_buf);
}

CHIP_ERROR CommissionableDataProviderImpl::SetSetupPasscode(uint32_t setupPasscode) {
    UNUSED(setupPasscode);
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
