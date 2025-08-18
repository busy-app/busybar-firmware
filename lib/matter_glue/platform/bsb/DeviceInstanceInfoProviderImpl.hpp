#pragma once

#include <platform/DeviceInstanceInfoProvider.h>

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

} // namespace DeviceLayer
} // namespace Chip
