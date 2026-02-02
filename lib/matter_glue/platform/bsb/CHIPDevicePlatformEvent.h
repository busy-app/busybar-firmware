/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the Chip
 *          Device Layer on Silabs platforms using the Silicon Labs SDK.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates Silabs platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes {
    // No public platform specific events
};

/**
 * Enumerates Silabs platform-specific event types that are internal to the Chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes {
    kDummySystemEvent = kRange_InternalPlatformSpecific,
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Silicon Labs platforms.
 */

struct ChipDevicePlatformEvent final {
    union {
        uint32_t dummy;
    };
};
}; // namespace DeviceLayer
} // namespace chip
