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
    if(!strlen(m_hw_version_str)) return CHIP_ERROR_INCORRECT_STATE;
    hardwareVersion = m_hw_version;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersionString(char* buf, size_t bufSize) {
    if(!strlen(m_hw_version_str)) return CHIP_ERROR_INCORRECT_STATE;
    strncpy(buf, m_hw_version_str, bufSize - 1);
    return CHIP_NO_ERROR;
}

CHIP_ERROR
DeviceInstanceInfoProviderImpl::SetHardwareVersion(uint16_t number, const char* string) {
    m_hw_version = number;
    strncpy(m_hw_version_str, string, sizeof(m_hw_version_str) - 1);
    return CHIP_NO_ERROR;
}

CHIP_ERROR
DeviceInstanceInfoProviderImpl::GetRotatingDeviceIdUniqueId(MutableByteSpan& uniqueIdSpan) {
    uniqueIdSpan.reduce_size(0);
    return CHIP_NO_ERROR;
}

namespace BSB {
DeviceInstanceInfoProviderImpl* GetDeviceInstanceInfoProvider(void) {
    static DeviceInstanceInfoProviderImpl provider;
    return &provider;
}
} // namespace BSB

} // namespace DeviceLayer
} // namespace Chip
