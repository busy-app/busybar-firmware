#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>

namespace chip {
namespace Credentials {
namespace BSB {

DeviceAttestationCredentialsProvider* GetDeviceAttestationCredentialsProvider(void);

} // namespace BSB
} // namespace Credentials
} // namespace chip
