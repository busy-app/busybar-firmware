#include "BSBDeviceInstanceInfoProvider.hpp"

#include "CryptoStorage.hpp"

namespace chip {
namespace DeviceLayer {

namespace KeyId {

enum {
    VendorId,
    ProductId,
    VendorName,
    ProductName,
    PartNumber,
    ProductUrl,
    ProductLabel,
    SerialNumber,
    ManufacturingDate,
    HardwareVersion,
    HardwareVersionString,
};

}; // namespace KeyId

#pragma pack(push, 1)
struct ManufacturingDate {
    uint16_t year;
    uint8_t month;
    uint8_t day;
};
#pragma pack(pop)

static_assert(sizeof(ManufacturingDate) == 4, "Invalid ManufacturingDate size");

using namespace BSB;

class DeviceInstanceInfoProviderImpl : public DeviceInstanceInfoProvider {
public:
    CHIP_ERROR GetVendorName(char* buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t& vendorId) override;
    CHIP_ERROR GetProductName(char* buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t& productId) override;
    CHIP_ERROR GetPartNumber(char* buf, size_t bufSize) override;
    CHIP_ERROR GetProductURL(char* buf, size_t bufSize) override;
    CHIP_ERROR GetProductLabel(char* buf, size_t bufSize) override;
    CHIP_ERROR GetSerialNumber(char* buf, size_t bufSize) override;
    CHIP_ERROR GetManufacturingDate(uint16_t& year, uint8_t& month, uint8_t& day) override;
    CHIP_ERROR GetHardwareVersion(uint16_t& hardwareVersion) override;
    CHIP_ERROR GetHardwareVersionString(char* buf, size_t bufSize) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan& uniqueIdSpan) override;
};

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorName(char* buf, size_t bufSize) {
    auto out_span = ToMutableByteSpan(buf, bufSize);
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::VendorName, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorId(uint16_t& vendorId) {
    auto out_span = ToMutableByteSpan(vendorId);
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::VendorId, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductName(char* buf, size_t bufSize) {
    auto out_span = ToMutableByteSpan(buf, bufSize);
    return LoadCryptoStorageKey(
        FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::ProductName, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductId(uint16_t& productId) {
    auto out_span = ToMutableByteSpan(productId);
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::ProductId, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetPartNumber(char* buf, size_t bufSize) {
    auto out_span = ToMutableByteSpan(buf, bufSize);
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::PartNumber, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductURL(char* buf, size_t bufSize) {
    auto out_span = ToMutableByteSpan(buf, bufSize);
    return LoadCryptoStorageKey(FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::ProductUrl, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductLabel(char* buf, size_t bufSize) {
    auto out_span = ToMutableByteSpan(buf, bufSize);
    return LoadCryptoStorageKey(
        FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::ProductLabel, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetSerialNumber(char* buf, size_t bufSize) {
    auto out_span = ToMutableByteSpan(buf, bufSize);
    return LoadCryptoStorageKey(
        FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::SerialNumber, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetManufacturingDate(
    uint16_t& year,
    uint8_t& month,
    uint8_t& day) {
    ManufacturingDate date;
    auto out_span = ToMutableByteSpan(date);
    const auto err = LoadCryptoStorageKey(
        FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::ManufacturingDate, out_span);

    if(CHIP_ERROR::IsSuccess(err)) {
        year = date.year;
        month = date.month;
        day = date.day;
    }

    return err;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersion(uint16_t& hardwareVersion) {
    auto out_span = ToMutableByteSpan(hardwareVersion);
    return LoadCryptoStorageKey(
        FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::HardwareVersion, out_span);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersionString(char* buf, size_t bufSize) {
    auto out_span = ToMutableByteSpan(buf, bufSize);
    return LoadCryptoStorageKey(
        FuriHalCryptoKeyTypeMatterDeviceInfo, KeyId::HardwareVersionString, out_span);
}

CHIP_ERROR
DeviceInstanceInfoProviderImpl::GetRotatingDeviceIdUniqueId(MutableByteSpan& uniqueIdSpan) {
    uniqueIdSpan.reduce_size(0);
    return CHIP_NO_ERROR;
}

namespace BSB {
DeviceInstanceInfoProvider* GetDeviceInstanceInfoProvider(void) {
    static DeviceInstanceInfoProviderImpl provider;
    return &provider;
}
} // namespace BSB

} // namespace DeviceLayer
} // namespace Chip
