#include "BSBDeviceInfoProvider.hpp"

#include "DeviceInfoProviderImpl.h"

namespace chip {
namespace DeviceLayer {
namespace BSB {

DeviceInfoProvider* GetDeviceInfoProvider(void) {
    static DeviceInfoProviderImpl provider;
    return &provider;
}

} // namespace BSB
} // namespace DeviceLayer
} // namespace Chip
