#include "CommissionableDataProviderImpl.hpp"

namespace chip {
namespace DeviceLayer {

CHIP_ERROR CommissionableDataProviderImpl::GetSetupDiscriminator(uint16_t& setupDiscriminator) {
    setupDiscriminator = 0;
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::SetSetupDiscriminator(uint16_t setupDiscriminator) {
    UNUSED(setupDiscriminator);
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pIterationCount(uint32_t& iterationCount) {
    iterationCount = 0;
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pSalt(MutableByteSpan& saltBuf) {
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSpake2pVerifier(
    MutableByteSpan& verifierBuf,
    size_t& outVerifierLen) {
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    outVerifierLen = verifierBuf.size();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableDataProviderImpl::GetSetupPasscode(uint32_t& setupPasscode) {
    UNUSED(setupPasscode);
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR CommissionableDataProviderImpl::SetSetupPasscode(uint32_t setupPasscode) {
    UNUSED(setupPasscode);
    ChipLogDetail(DeviceLayer, "%s", __PRETTY_FUNCTION__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace DeviceLayer
} // namespace Chip
