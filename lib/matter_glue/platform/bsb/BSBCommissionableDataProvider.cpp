#include "BSBCommissionableDataProvider.hpp"

#include "CryptoStorage.hpp"

namespace chip {
namespace DeviceLayer {

namespace KeyId {

enum {
    SPAKE2PSalt,
    SPAKE2PVerifier,
    SPAKE2PIterCount,
    SetupDiscriminator,
    SetupPasscode,
};

}; // namespace KeyId

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
    auto out_span = ToMutableByteSpan(setupDiscriminator);
    return LoadCryptoStorageKey(
        FuriHalCryptoKeyTypeMatterSetup, KeyId::SetupDiscriminator, out_span);
}

CHIP_ERROR CommissionableDataProviderImpl::SetSetupDiscriminator(uint16_t setupDiscriminator) {
    UNUSED(setupDiscriminator);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pIterationCount(uint32_t& iterationCount) {
    auto out_span = ToMutableByteSpan(iterationCount);
    return LoadCryptoStorageKey(
        FuriHalCryptoKeyTypeMatterSetup, KeyId::SPAKE2PIterCount, out_span);
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pSalt(MutableByteSpan& saltBuf) {
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterSetup, KeyId::SPAKE2PSalt, saltBuf);
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pVerifier(
    MutableByteSpan& verifierBuf,
    size_t& outVerifierLen) {
    const auto err =
        LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterSetup, KeyId::SPAKE2PVerifier, verifierBuf);
    outVerifierLen = verifierBuf.size();
    return err;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSetupPasscode(uint32_t& setupPasscode) {
    auto out_span = ToMutableByteSpan(setupPasscode);
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterSetup, KeyId::SetupPasscode, out_span);
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
