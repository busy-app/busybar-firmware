/*******************************************************************************
 * @file
 * @brief This file contains implementations for functions to interact with the
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

#include "posix_vendor_spinel_interface.hpp"
#include "openthread-posix-config.h"
#include "vendor_spinel.hpp"
#include "lib/spinel/radio_spinel.hpp"
#include "posix/platform/radio.hpp"

#if OPENTHREAD_POSIX_CONFIG_SPINEL_HDLC_INTERFACE_ENABLE // CMake: OT_POSIX_RCP_HDLC_BUS
#include "hdlc_interface.hpp"
using InterfaceType = ot::Posix::HdlcInterface;

#elif OPENTHREAD_POSIX_CONFIG_SPINEL_SPI_INTERFACE_ENABLE // CMake: OT_POSIX_RCP_SPI_BUS
#include "spi_interface.hpp"
using InterfaceType = ot::Posix::SpiInterface;
#elif OPENTHREAD_POSIX_CONFIG_SPINEL_VENDOR_INTERFACE_ENABLE // CMake: OT_POSIX_RCP_VENDOR_BUS
#include "vendor_interface.hpp"
using InterfaceType = ot::Posix::VendorInterface;
#else
#error "You must define OT_POSIX_RCP_HDLC_BUS, OT_POSIX_RCP_SPI_BUS or OT_POSIX_RCP_VENDOR_BUS!"
#endif

namespace VendorCmd      = ot::Vendor;
namespace AntennaCmd     = VendorCmd::Antenna;
namespace CoexCmd        = VendorCmd::Coex;
namespace TestCmd        = VendorCmd::Test;
namespace Efr32Cmd       = VendorCmd::Efr32;
namespace MemoryUsageCmd = VendorCmd::MemoryUsage;

extern ot::Spinel::RadioSpinel &GetRadioSpinel(void);

namespace ot {
namespace Spinel {
namespace Vendor {

static void LogIfFail(const char *aText, otError aError)
{
    if (aError != OT_ERROR_NONE && aError != OT_ERROR_NO_ACK)
    {
        otLogWarnPlat("%s: %s", aText, otThreadErrorToString(aError));
    }
}

namespace Antenna {
otError getTxAntennaMode(uint8_t &aMode)
{
    static const uint8_t command = AntennaCmd::ANT_TX_MODE_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_ANTENNA,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT8_S,
                                                  &aMode);
    LogIfFail("Get TX Antenna Mode failed", error);
    return error;
}

otError setTxAntennaMode(uint8_t aMode)
{
    static const uint8_t command = AntennaCmd::ANT_TX_MODE_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_ANTENNA,
                                         SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S,
                                         command,
                                         aMode);
    LogIfFail("Set TX Antenna Mode failed", error);
    return error;
}

otError getRxAntennaMode(uint8_t &aMode)
{
    static const uint8_t command = AntennaCmd::ANT_RX_MODE_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_ANTENNA,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT8_S,
                                                  &aMode);
    LogIfFail("Get RX Antenna Mode failed", error);
    return error;
}

otError setRxAntennaMode(uint8_t aMode)
{
    static const uint8_t command = AntennaCmd::ANT_RX_MODE_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_ANTENNA,
                                         SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S,
                                         command,
                                         aMode);
    LogIfFail("Set RX Antenna Mode failed", error);
    return error;
}

otError getActivePhy(uint8_t &aActivePhy)
{
    static const uint8_t command = AntennaCmd::ANT_ACTIVE_PHY_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_ANTENNA,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT8_S,
                                                  &aActivePhy);
    LogIfFail("Get Active Phy failed", error);
    return error;
}
} // namespace Antenna

namespace Coex {
otError getDpState(uint8_t &dpPulse)
{
    static const uint8_t command = CoexCmd::COEX_DP_STATE_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT8_S,
                                                  &dpPulse);
    LogIfFail("Get DP State failed", error);
    return error;
}

otError setDpState(uint8_t dpPulse)
{
    static const uint8_t command = CoexCmd::COEX_DP_STATE_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_COEX,
                                         SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S,
                                         command,
                                         dpPulse);
    LogIfFail("Set DP State failed", error);
    return error;
}

otError getGpioInputOverride(uint8_t &enabled)
{
    static const uint8_t command = CoexCmd::COEX_GPIO_INPUT_OVERRIDE_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT8_S,
                                                  &enabled);
    LogIfFail("Get GPIO Input Override failed", error);
    return error;
}

otError setGpioInputOverride(uint8_t gpioIndex, bool enabled)
{
    static const uint8_t command = CoexCmd::COEX_GPIO_INPUT_OVERRIDE_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_COEX,
                                         SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_BOOL_S,
                                         command,
                                         gpioIndex,
                                         enabled);
    LogIfFail("Set GPIO Input Override failed", error);
    return error;
}

otError getActiveRadio(uint8_t &activePhy)
{
    static const uint8_t command = CoexCmd::COEX_ACTIVE_RADIO_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT8_S,
                                                  &activePhy);
    LogIfFail("Get Active Radio failed", error);
    return error;
}

otError getPhySelectTimeout(uint8_t &timeout)
{
    static const uint8_t command = CoexCmd::COEX_PHY_SELECT_TIMEOUT_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT8_S,
                                                  &timeout);
    LogIfFail("Get Phy Select Timeout failed", error);
    return error;
}

otError setPhySelectTimeout(uint8_t timeout)
{
    static const uint8_t command = CoexCmd::COEX_PHY_SELECT_TIMEOUT_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_COEX,
                                         SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S,
                                         command,
                                         timeout);
    LogIfFail("Set Phy Select Timeout failed", error);
    return error;
}

otError getOptions(uint32_t &ptaOptions)
{
    static const uint8_t command = CoexCmd::COEX_PTA_OPTIONS_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT32_S,
                                                  &ptaOptions);
    LogIfFail("Get PTA Options failed", error);
    return error;
}

otError setOptions(uint32_t ptaOptions)
{
    static const uint8_t command = CoexCmd::COEX_PTA_OPTIONS_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_COEX,
                                         SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT32_S,
                                         command,
                                         ptaOptions);
    LogIfFail("Set PTA Options failed", error);
    return error;
}

otError getConstantOptions(uint32_t &ptaOptions)
{
    static const uint8_t command = CoexCmd::COEX_CONSTANT_OPTIONS_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT32_S,
                                                  &ptaOptions);
    LogIfFail("Get Constant Options failed", error);
    return error;
}

otError isEnabled(bool &ptaState)
{
    static const uint8_t command = CoexCmd::COEX_PTA_STATE_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_BOOL_S,
                                                  &ptaState);
    LogIfFail("Is Enabled failed", error);
    return error;
}

otError setEnable(bool ptaState)
{
    static const uint8_t command = CoexCmd::COEX_PTA_STATE_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_COEX,
                                         SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_BOOL_S,
                                         command,
                                         ptaState);
    LogIfFail("Set Enable failed", error);
    return error;
}

otError getRequestPwmArgs(uint8_t &pwmReq, uint8_t &pwmDutyCycle, uint8_t &pwmPeriodHalfMs)
{
    static const uint8_t command = CoexCmd::COEX_PWM_STATE_COMMAND;
    otError              error =
        GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                      &command,
                                      sizeof(command),
                                      SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S,
                                      &pwmReq,
                                      &pwmDutyCycle,
                                      &pwmPeriodHalfMs);
    LogIfFail("Get Request PWM Args failed", error);
    return error;
}

otError setRequestPwmArgs(uint8_t pwmReq, uint8_t pwmDutyCycle, uint8_t pwmPeriodHalfMs)
{
    static const uint8_t command = CoexCmd::COEX_PWM_STATE_COMMAND;
    otError              error   = GetRadioSpinel().Set(
        SPINEL_PROP_VENDOR_COEX,
        SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S,
        command,
        pwmReq,
        pwmDutyCycle,
        pwmPeriodHalfMs);
    LogIfFail("Set Request PWM Args failed", error);
    return error;
}

otError clearCoexCounters()
{
    static const uint8_t command = CoexCmd::COEX_COUNTERS_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_COEX, SPINEL_DATATYPE_UINT8_S, command);
    LogIfFail("Clear Coex Counters failed", error);
    return error;
}

otError getCoexCounters(uint32_t coexCounters[])
{
    static const uint8_t command = CoexCmd::COEX_COUNTERS_COMMAND;
    otError              error =
        GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_COEX,
                                      &command,
                                      sizeof(command),
                                      SPINEL_DATATYPE_UINT32_S SPINEL_DATATYPE_UINT32_S SPINEL_DATATYPE_UINT32_S
                                          SPINEL_DATATYPE_UINT32_S SPINEL_DATATYPE_UINT32_S SPINEL_DATATYPE_UINT32_S,
                                      &coexCounters[0],
                                      &coexCounters[1],
                                      &coexCounters[2],
                                      &coexCounters[3],
                                      &coexCounters[4],
                                      &coexCounters[5]);
    LogIfFail("Get Coex Counters failed", error);
    return error;
}

otError setRadioHoldoff(bool enabled)
{
    static const uint8_t command = CoexCmd::COEX_RADIO_HOLDOFF_COMMAND;
    otError              error =
        GetRadioSpinel().Set(SPINEL_PROP_VENDOR_COEX, SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_BOOL_S, command, enabled);
    LogIfFail("Set Radio Holdoff failed", error);
    return error;
}

} // namespace Coex

namespace Test {
otError getPtiRadioConfig(uint16_t &aRadioConfig)
{
    static const uint8_t command = TestCmd::GEN_PTI_RADIO_CONFIG_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_TEST,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT16_S,
                                                  &aRadioConfig);
    LogIfFail("Get PTI Radio Config failed", error);
    return error;
}

otError setCcaMode(uint8_t aMode)
{
    static const uint8_t command = TestCmd::GEN_CCA_MODE_COMMAND;
    otError              error =
        GetRadioSpinel().Set(SPINEL_PROP_VENDOR_TEST, SPINEL_DATATYPE_UINT8_S SPINEL_DATATYPE_UINT8_S, command, aMode);
    LogIfFail("Set CCA Mode failed", error);
    return error;
}
} // namespace Test

namespace Efr32 {
otError getRadioCounters(efr32RadioCounters &aCounters)
{
    static const uint8_t command       = Efr32Cmd::EFR32_RADIO_COUNTERS_COMMAND;
    size_t               read_data_len = sizeof(efr32RadioCounters);
    otError              error         = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_EFR32,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_DATA_WLEN_S,
                                                  &aCounters,
                                                  &read_data_len);
    LogIfFail("Get Radio Counters failed", error);
    return error;
}

otError clearRadioCounters(void)
{
    static const uint8_t command = Efr32Cmd::EFR32_RADIO_COUNTERS_COMMAND;
    otError              error   = GetRadioSpinel().Set(SPINEL_PROP_VENDOR_EFR32, SPINEL_DATATYPE_UINT8_S, command);
    LogIfFail("Clear Radio Counters failed", error);
    return error;
}
} // namespace Efr32

namespace MemoryUsage {
otError getUsedHeap(uint32_t &aUsedHeap)
{
    static const uint8_t command = MemoryUsageCmd::MEMORY_USAGE_USED_HEAP_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_MEMORY_USAGE,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT32_S,
                                                  &aUsedHeap);
    LogIfFail("Get Current Heap Usage failed", error);
    return error;
}
otError getHighWaterMark(uint32_t &aheapHighWatermark)
{
    static const uint8_t command = MemoryUsageCmd::MEMORY_USAGE_HIGH_WATERMARK_COMMAND;
    otError              error   = GetRadioSpinel().GetWithParam(SPINEL_PROP_VENDOR_MEMORY_USAGE,
                                                  &command,
                                                  sizeof(command),
                                                  SPINEL_DATATYPE_UINT32_S,
                                                  &aheapHighWatermark);
    LogIfFail("Get High Watermark failed", error);
    return error;
}
} // namespace MemoryUsage
} // namespace Vendor
} // namespace Spinel
} // namespace ot
