/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <platform/ConnectivityManager.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#include <app/server/Server.h>

#include <network/network.h>

#include <wifi/wifi_common_i.h>

namespace chip {
namespace DeviceLayer {

/** Singleton instance of the ConnectivityManager implementation object.
 */
ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

using WiFiStationMode = ConnectivityManagerImpl::WiFiStationMode;

void ConnectivityManagerImpl::WifiEvent(const void* message, void* context) {
    auto state = *static_cast<const WifiBackendState*>(message);
    auto* self = static_cast<ConnectivityManagerImpl*>(context);

    ChipLogDetail(DeviceLayer, "ConnectivityManagerImpl::WifiEvent(%d)", state);

    StackLock lock;
    ChipDeviceEvent event;
    ConnectivityChange change = (state == WifiBackendStateConnected) ? kConnectivity_Established :
                                                                       kConnectivity_Lost;
    self->mIsConnected = state == WifiBackendStateConnected;

    event.Type = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = change;
    PlatformMgr().PostEventOrDie(&event);

    event.Type = DeviceEventType::kInternetConnectivityChange;
    event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
    event.InternetConnectivityChange.IPv6 = change;
    // event.InternetConnectivityChange.ipAddress is only used for debug logging, no need to fill it in
    PlatformMgr().PostEventOrDie(&event);

    event.Type = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = (state == WifiBackendStateConnected) ?
                                               InterfaceIpChangeType::kIpV6_Assigned :
                                               InterfaceIpChangeType::kIpV6_Lost;
    PlatformMgr().PostEventOrDie(&event);

    event.Type = DeviceEventType::kDnssdRestartNeeded;
    PlatformMgr().PostEventOrDie(&event);
}

CHIP_ERROR ConnectivityManagerImpl::_Init(void) {
    auto* network = static_cast<Network*>(furi_record_open(RECORD_NETWORK));
    network_init_current_thread(network);

    mIsConnected = false;
    mWifiPubSub = static_cast<FuriPubSub*>(furi_record_open(RECORD_WIFI));
    mPubSubSub = furi_pubsub_subscribe(mWifiPubSub, ConnectivityManagerImpl::WifiEvent, this);

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent* event) {
    ChipLogDetail(DeviceLayer, "Platform event of type %hu", event->Type);
}

} // namespace DeviceLayer
} // namespace chip
