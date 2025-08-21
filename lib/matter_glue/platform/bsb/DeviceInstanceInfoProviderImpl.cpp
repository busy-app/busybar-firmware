#include "DeviceInstanceInfoProviderImpl.hpp"

namespace chip {
namespace DeviceLayer {

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
    static const char* const vendorName = "Flipper Devices";
    strncpy(buf, vendorName, bufSize);

    return (bufSize > strlen(vendorName)) ? CHIP_NO_ERROR : CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorId(uint16_t& vendorId) {
    vendorId = 0xCAFE;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductName(char* buf, size_t bufSize) {
    static const char* const productName = "Busy Statusbar";
    strncpy(buf, productName, bufSize);

    return (bufSize > strlen(productName)) ? CHIP_NO_ERROR : CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductId(uint16_t& productId) {
    productId = 0xCACA;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetPartNumber(char* buf, size_t bufSize) {
    static const char* const partNumber = "BSB0001";
    strncpy(buf, partNumber, bufSize - 1);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductURL(char* buf, size_t bufSize) {
    static const char* const productUrl = "https://busy.bar";
    strncpy(buf, productUrl, bufSize - 1);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductLabel(char* buf, size_t bufSize) {
    static const char* const productLabel = "No idea";
    strncpy(buf, productLabel, bufSize - 1);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetSerialNumber(char* buf, size_t bufSize) {
    static const char* const serialNumber = "1234567890";
    strncpy(buf, serialNumber, bufSize);

    return (bufSize > strlen(serialNumber)) ? CHIP_NO_ERROR : CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetManufacturingDate(
    uint16_t& year,
    uint8_t& month,
    uint8_t& day) {
    year = 2025;
    month = 8;
    day = 18;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersion(uint16_t& hardwareVersion) {
    hardwareVersion = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersionString(char* buf, size_t bufSize) {
    static const char* const versionString = "4269";
    strncpy(buf, versionString, bufSize);

    return (bufSize > strlen(versionString)) ? CHIP_NO_ERROR : CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR
DeviceInstanceInfoProviderImpl::GetRotatingDeviceIdUniqueId(MutableByteSpan& uniqueIdSpan) {
    UNUSED(uniqueIdSpan);
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
