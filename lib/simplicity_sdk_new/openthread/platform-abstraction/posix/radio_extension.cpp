/*******************************************************************************
 * @file
 * @brief Defines the Silicon Labs extension to the openthread platform radio interface.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source version‰s must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "radio_extension.h"
#include "posix_vendor_spinel_interface.hpp"

#include "common/code_utils.hpp"

namespace VendorSpinel = ot::Spinel::Vendor;

/**
 * @file
 *   This file implements the OpenThread platform abstraction for vendor additions to radio communication.
 *
 */

namespace AntennaSpinel = VendorSpinel::Antenna;

otError otPlatRadioExtensionGetTxAntennaMode(uint8_t *aMode)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aMode != nullptr);
    error = AntennaSpinel::getTxAntennaMode(*aMode);
exit:
    return error;
}

otError otPlatRadioExtensionSetTxAntennaMode(uint8_t aMode)
{
    return AntennaSpinel::setTxAntennaMode(aMode);
}

otError otPlatRadioExtensionGetRxAntennaMode(uint8_t *aMode)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aMode != nullptr);
    error = AntennaSpinel::getRxAntennaMode(*aMode);
exit:
    return error;
}

otError otPlatRadioExtensionSetRxAntennaMode(uint8_t aMode)
{
    return AntennaSpinel::setRxAntennaMode(aMode);
}

otError otPlatRadioExtensionGetActivePhy(uint8_t *aActivePhy)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aActivePhy != nullptr);
    error = AntennaSpinel::getActivePhy(*aActivePhy);
exit:
    return error;
}

namespace CoexSpinel = VendorSpinel::Coex;

otError otPlatRadioExtensionGetDpState(uint8_t *aDpPulse)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aDpPulse != nullptr);
    error = CoexSpinel::getDpState(*aDpPulse);
exit:
    return error;
}

otError otPlatRadioExtensionSetDpState(uint8_t aDpPulse)
{
    return CoexSpinel::setDpState(aDpPulse);
}

otError otPlatRadioExtensionGetGpioInputOverride(uint8_t aGpioIndex, bool *aEnabled)
{
    otError error = OT_ERROR_INVALID_ARGS;
    uint8_t gpioStates;

    VerifyOrExit(aEnabled != nullptr);
    SuccessOrExit(error = CoexSpinel::getGpioInputOverride(gpioStates));

    *aEnabled = (bool)((gpioStates >> aGpioIndex) & 0x01);
exit:
    return error;
}

otError otPlatRadioExtensionSetGpioInputOverride(uint8_t aGpioIndex, bool aEnabled)
{
    return CoexSpinel::setGpioInputOverride(aGpioIndex, aEnabled);
}

otError otPlatRadioExtensionGetActiveRadio(uint8_t *aActivePhy)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aActivePhy != nullptr);
    error = CoexSpinel::getActiveRadio(*aActivePhy);
exit:
    return error;
}

otError otPlatRadioExtensionGetPhySelectTimeout(uint8_t *aTimeout)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aTimeout != nullptr);
    error = CoexSpinel::getPhySelectTimeout(*aTimeout);
exit:
    return error;
}

otError otPlatRadioExtensionSetPhySelectTimeout(uint8_t aTimeout)
{
    return CoexSpinel::setPhySelectTimeout(aTimeout);
}

otError otPlatRadioExtensionGetCoexOptions(uint32_t *aPtaOptions)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aPtaOptions != nullptr);
    error = CoexSpinel::getOptions(*aPtaOptions);
exit:
    return error;
}

otError otPlatRadioExtensionSetCoexOptions(uint32_t aPtaOptions)
{
    return CoexSpinel::setOptions(aPtaOptions);
}

otError otPlatRadioExtensionGetCoexConstantOptions(uint32_t *aPtaOptions)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aPtaOptions != nullptr);
    error = CoexSpinel::getConstantOptions(*aPtaOptions);
exit:
    return error;
}

otError otPlatRadioExtensionIsCoexEnabled(bool *aPtaState)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aPtaState != nullptr);
    error = CoexSpinel::isEnabled(*aPtaState);
exit:
    return error;
}

otError otPlatRadioExtensionSetCoexEnable(bool aPtaState)
{
    return CoexSpinel::setEnable(aPtaState);
}

otError otPlatRadioExtensionGetRequestPwmArgs(uint8_t *aPwmReq, uint8_t *aPwmDutyCycle, uint8_t *aPwmPeriodHalfMs)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aPwmReq != nullptr && aPwmDutyCycle != nullptr && aPwmPeriodHalfMs != nullptr);
    error = CoexSpinel::getRequestPwmArgs(*aPwmReq, *aPwmDutyCycle, *aPwmPeriodHalfMs);
exit:
    return error;
}

otError otPlatRadioExtensionSetRequestPwmArgs(uint8_t aPwmReq, uint8_t aPwmDutyCycle, uint8_t aPwmPeriodHalfMs)
{
    return CoexSpinel::setRequestPwmArgs(aPwmReq, aPwmDutyCycle, aPwmPeriodHalfMs);
}

otError otPlatRadioExtensionClearCoexCounters(void)
{
    return CoexSpinel::clearCoexCounters();
}

otError otPlatRadioExtensionGetCoexCounters(uint8_t aNumEntries, uint32_t aCoexCounters[])
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aNumEntries == OT_PLAT_RADIO_EXTENSION_COEX_EVENT_COUNT);
    VerifyOrExit(aCoexCounters != nullptr);
    error = CoexSpinel::getCoexCounters(aCoexCounters);
exit:
    return error;
}

otError otPlatRadioExtensionSetRadioHoldoff(bool aEnabled)
{
    return CoexSpinel::setRadioHoldoff(aEnabled);
}

namespace TestSpinel = VendorSpinel::Test;

otError otPlatRadioExtensionGetPtiRadioConfig(uint16_t *radioConfig)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(radioConfig != nullptr);
    error = TestSpinel::getPtiRadioConfig(*radioConfig);
exit:
    return error;
}

otError otPlatRadioExtensionSetCcaMode(uint8_t aMode)
{
    return TestSpinel::setCcaMode(aMode);
}

namespace Efr32Spinel = VendorSpinel::Efr32;

otError otPlatRadioExtensionGetRadioCounters(efr32RadioCounters *aCounters)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aCounters != nullptr);
    error = Efr32Spinel::getRadioCounters(*aCounters);
exit:
    return error;
}

otError otPlatRadioExtensionClearRadioCounters(void)
{
    return Efr32Spinel::clearRadioCounters();
}

namespace MemoryUsageSpinel = VendorSpinel::MemoryUsage;

otError otPlatRadioExtensionGetUsedHeap(uint32_t *aUsedHeap)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aUsedHeap != nullptr);
    error = MemoryUsageSpinel::getUsedHeap(*aUsedHeap);
exit:
    return error;
}

otError otPlatRadioExtensionGetHighWaterMark(uint32_t *aheapHighWaterMark)
{
    otError error = OT_ERROR_INVALID_ARGS;
    VerifyOrExit(aheapHighWaterMark != nullptr);
    error = MemoryUsageSpinel::getHighWaterMark(*aheapHighWaterMark);
exit:
    return error;
}
