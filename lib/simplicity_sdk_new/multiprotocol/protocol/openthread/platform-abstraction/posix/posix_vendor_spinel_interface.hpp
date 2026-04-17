/*******************************************************************************
 * @file
 * @brief This file contains definitions for functions to interact with the
 *        posix spinel interface to build vendor specific spinel frames.
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
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef _POSIX_VENDOR_SPINEL_INTERFACE_HPP
#define _POSIX_VENDOR_SPINEL_INTERFACE_HPP

#include <cstdint>
#include <openthread/error.h>

#include "radio_counters.h"

namespace ot {
namespace Spinel {
namespace Vendor {

namespace Antenna {
otError getTxAntennaMode(uint8_t &aMode);
otError setTxAntennaMode(uint8_t aMode);
otError getRxAntennaMode(uint8_t &aMode);
otError setRxAntennaMode(uint8_t aMode);
otError getActivePhy(uint8_t &aActivePhy);
} // namespace Antenna

namespace Coex {
otError getDpState(uint8_t &dpPulse);
otError setDpState(uint8_t dpPulse);
otError getGpioInputOverride(uint8_t &enabled);
otError setGpioInputOverride(uint8_t gpioIndex, bool enabled);
otError getActiveRadio(uint8_t &activePhy);
otError getPhySelectTimeout(uint8_t &timeout);
otError setPhySelectTimeout(uint8_t timeout);
otError getOptions(uint32_t &ptaOptions);
otError setOptions(uint32_t ptaOptions);
otError getConstantOptions(uint32_t &ptaOptions);
otError isEnabled(bool &ptaState);
otError setEnable(bool ptaState);
otError getRequestPwmArgs(uint8_t &pwmReq, uint8_t &pwmDutyCycle, uint8_t &pwmPeriodHalfMs);
otError setRequestPwmArgs(uint8_t pwmReq, uint8_t pwmDutyCycle, uint8_t pwmPeriodHalfMs);
otError clearCoexCounters();
otError getCoexCounters(uint32_t coexCounters[]);
otError setRadioHoldoff(bool enabled);
} // namespace Coex

namespace Test {
otError getPtiRadioConfig(uint16_t &aRadioConfig);
otError setCcaMode(uint8_t aMode);
} // namespace Test

namespace Efr32 {
otError getRadioCounters(efr32RadioCounters &aCounters);
otError clearRadioCounters(void);
} // namespace Efr32

namespace MemoryUsage {
otError getUsedHeap(uint32_t &aUsedHeap);
otError getHighWaterMark(uint32_t &aheapHighWaterMark);
} // namespace MemoryUsage

} // namespace Vendor
} // namespace Spinel
} // namespace ot
#endif // _POSIX_VENDOR_SPINEL_INTERFACE_HPP
