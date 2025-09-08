/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for Silabs platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DiagnosticDataProvider.h>
#include "DiagnosticDataProviderImpl.h"

#include <inet/InetInterface.h>
#include <lib/support/CHIPMemString.h>

using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl& DiagnosticDataProviderImpl::GetDefaultInstance() {
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

// Software Diagnostics Getters
/*
 * The following Heap stats are compiled values done by the sl_memory_manager.
 * It keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation.
 */
CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t& currentHeapFree) {
    // TODO: Implementation
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t& currentHeapUsed) {
    // TODO: Implementation
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR
DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t& currentHeapHighWatermark) {
    // TODO: Implementation
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWatermarks() {
    // TODO: Implementation
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics** threadMetricsOut) {
    // TODO: Implement thread metrics
    *threadMetricsOut = nullptr;
    return CHIP_NO_ERROR;
}

void DiagnosticDataProviderImpl::ReleaseThreadMetrics(ThreadMetrics* threadMetrics) {
    while(threadMetrics) {
        ThreadMetrics* del = threadMetrics;
        threadMetrics = threadMetrics->Next;
        delete del;
    }
}

// General Diagnostics Getters

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t& rebootCount) {
    uint32_t count = 0;
    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);

    if(err == CHIP_NO_ERROR) {
        VerifyOrReturnError(count <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        rebootCount = static_cast<uint16_t>(count);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType& bootReason) {
    uint32_t reason = 0;
    CHIP_ERROR err = ConfigurationMgr().GetBootReason(reason);

    if(err == CHIP_NO_ERROR) {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<BootReasonType>(reason);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetUpTime(uint64_t& upTime) {
    // TODO: Implementation
    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetTotalOperationalHours(uint32_t& totalOperationalHours) {
    return ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveHardwareFaults(
    GeneralFaults<kMaxHardwareFaults>& hardwareFaults) {
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(hardwareFaults.add(to_underlying(HardwareFaultEnum::kRadio)));
    ReturnErrorOnFailure(hardwareFaults.add(to_underlying(HardwareFaultEnum::kSensor)));
    ReturnErrorOnFailure(hardwareFaults.add(to_underlying(HardwareFaultEnum::kPowerSource)));
    ReturnErrorOnFailure(
        hardwareFaults.add(to_underlying(HardwareFaultEnum::kUserInterfaceFault)));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DiagnosticDataProviderImpl::GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults>& radioFaults) {
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(radioFaults.add(to_underlying(RadioFaultEnum::kThreadFault)));
    ReturnErrorOnFailure(radioFaults.add(to_underlying(RadioFaultEnum::kBLEFault)));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveNetworkFaults(
    GeneralFaults<kMaxNetworkFaults>& networkFaults) {
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(networkFaults.add(to_underlying(NetworkFaultEnum::kHardwareFailure)));
    ReturnErrorOnFailure(networkFaults.add(to_underlying(NetworkFaultEnum::kNetworkJammed)));
    ReturnErrorOnFailure(networkFaults.add(to_underlying(NetworkFaultEnum::kConnectionFailed)));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface** netifpp) {
    NetworkInterface* ifp = new NetworkInterface();
    NetworkInterface* head = NULL;

    for(Inet::InterfaceIterator interfaceIterator; interfaceIterator.HasCurrent();
        interfaceIterator.Next()) {
        interfaceIterator.GetInterfaceName(ifp->Name, Inet::InterfaceId::kMaxIfNameLength);
        ifp->name = CharSpan::fromCharString(ifp->Name);
        ifp->isOperational = true;
        Inet::InterfaceType interfaceType;
        CHIP_ERROR err = interfaceIterator.GetInterfaceType(interfaceType);
        if(err == CHIP_NO_ERROR || err == CHIP_ERROR_NOT_IMPLEMENTED) {
            switch(interfaceType) {
            case Inet::InterfaceType::Unknown:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kUnspecified;
                break;
            case Inet::InterfaceType::WiFi:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kWiFi;
                break;
            case Inet::InterfaceType::Ethernet:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kEthernet;
                break;
            case Inet::InterfaceType::Thread:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kThread;
                break;
            case Inet::InterfaceType::Cellular:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kCellular;
                break;
            default:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kWiFi;
                break;
            }
        } else {
            ChipLogError(DeviceLayer, "Failed to get interface type");
        }

        ifp->offPremiseServicesReachableIPv4.SetNull();
        ifp->offPremiseServicesReachableIPv6.SetNull();

        uint8_t addressSize;
        if(interfaceIterator.GetHardwareAddress(
               ifp->MacAddress, addressSize, sizeof(ifp->MacAddress)) != CHIP_NO_ERROR) {
            ChipLogError(DeviceLayer, "Failed to get network hardware address");
        } else {
            ifp->hardwareAddress = ByteSpan(ifp->MacAddress, addressSize);
        }

        // Assuming IPv6-only support
        Inet::InterfaceAddressIterator interfaceAddressIterator;
        uint8_t ipv6AddressesCount = 0;
        while(interfaceAddressIterator.HasCurrent() && ipv6AddressesCount < kMaxIPv6AddrCount) {
            if(interfaceAddressIterator.GetInterfaceId() == interfaceIterator.GetInterfaceId()) {
                chip::Inet::IPAddress ipv6Address;
                if(interfaceAddressIterator.GetAddress(ipv6Address) == CHIP_NO_ERROR) {
                    memcpy(
                        ifp->Ipv6AddressesBuffer[ipv6AddressesCount],
                        ipv6Address.Addr,
                        kMaxIPv6AddrSize);
                    ifp->Ipv6AddressSpans[ipv6AddressesCount] =
                        ByteSpan(ifp->Ipv6AddressesBuffer[ipv6AddressesCount]);
                    ipv6AddressesCount++;
                }
            }
            interfaceAddressIterator.Next();
        }

        ifp->IPv6Addresses =
            chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressesCount);
        head = ifp;
    }
    *netifpp = head;

    return CHIP_NO_ERROR;
}

void DiagnosticDataProviderImpl::ReleaseNetworkInterfaces(NetworkInterface* netifp) {
    while(netifp) {
        NetworkInterface* del = netifp;
        netifp = netifp->Next;
        delete del;
    }
}

DiagnosticDataProvider& GetDiagnosticDataProviderImpl() {
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
