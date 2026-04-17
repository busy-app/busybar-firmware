/*******************************************************************************
 * @file
 * @brief Stub implementation of the Diags class.
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
#include "diags/factory_diags.hpp"

namespace ot {
namespace FactoryDiags {

Diags::Diags(Instance &aInstance)
    : InstanceLocator(aInstance)
    , mOutputCallback(nullptr)
    , mOutputContext(nullptr)
{
}

Error Diags::ProcessLine(const char *aString)
{
    OT_UNUSED_VARIABLE(aString);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessCmd(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

bool Diags::IsEnabled(void)
{
    return false;
}

Error Diags::ParseCmd(char *aString, uint8_t &aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aString);
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessChannel(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessFrame(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessContinuousWave(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessGpio(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessPower(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessRadio(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessRepeat(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessPowerSettings(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessRawPowerSetting(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessSend(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessStart(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessStats(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessStop(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

Error Diags::ProcessStream(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}

#if OPENTHREAD_RADIO && !OPENTHREAD_RADIO_CLI
Error Diags::ProcessEcho(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return ot::kErrorNotImplemented;
}
#endif

Error Diags::GetRawPowerSetting(RawPowerSetting &aRawPowerSetting)
{
    OT_UNUSED_VARIABLE(aRawPowerSetting);

    return ot::kErrorNotImplemented;
}

Error Diags::GetPowerSettings(uint8_t aChannel, PowerSettings &aPowerSettings)
{
    OT_UNUSED_VARIABLE(aChannel);
    OT_UNUSED_VARIABLE(aPowerSettings);

    return ot::kErrorNotImplemented;
}

Error Diags::ParseReceiveConfigFormat(const char *aFormat, ReceiveConfig &aConfig)
{
    OT_UNUSED_VARIABLE(aFormat);
    OT_UNUSED_VARIABLE(aConfig);

    return ot::kErrorNotImplemented;
}

Error Diags::RadioReceive(void)
{
    return ot::kErrorNotImplemented;
}

Error Diags::TransmitPacket(void)
{
    return ot::kErrorNotImplemented;
}

void Diags::OutputReceivedFrame(const otRadioFrame *aFrame)
{
    OT_UNUSED_VARIABLE(aFrame);
}

bool Diags::ShouldHandleReceivedFrame(const otRadioFrame &aFrame) const
{
    OT_UNUSED_VARIABLE(aFrame);

    return false;
}

bool Diags::IsChannelValid(uint8_t aChannel)
{
    OT_UNUSED_VARIABLE(aChannel);
    return false;
}

void Diags::ReceiveDone(otRadioFrame *aFrame, Error aError)
{
    OT_UNUSED_VARIABLE(aFrame);
    OT_UNUSED_VARIABLE(aError);
}

void Diags::TransmitDone(Error aError)
{
    OT_UNUSED_VARIABLE(aError);
}

extern "C" void otPlatDiagAlarmFired(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
}

} // namespace FactoryDiags
} // namespace ot
