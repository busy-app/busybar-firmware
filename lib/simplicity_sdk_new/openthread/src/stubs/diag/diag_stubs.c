/*******************************************************************************
 * @file
 * @brief Stub implementation of the otPlatDiag interface.
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
#include <openthread/platform/diag.h>
#include <openthread/platform/toolchain.h>

void otPlatDiagSetOutputCallback(otInstance *aInstance, otPlatDiagOutputCallback aCallback, void *aContext)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aCallback);
    OT_UNUSED_VARIABLE(aContext);
}

otError otPlatDiagProcess(otInstance *aInstance, uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);
    return OT_ERROR_INVALID_COMMAND;
}

void otPlatDiagModeSet(bool aMode)
{
    OT_UNUSED_VARIABLE(aMode);
}

bool otPlatDiagModeGet(void)
{
    return false;
}

void otPlatDiagChannelSet(uint8_t aChannel)
{
    OT_UNUSED_VARIABLE(aChannel);
}

void otPlatDiagTxPowerSet(int8_t aTxPower)
{
    OT_UNUSED_VARIABLE(aTxPower);
}

void otPlatDiagRadioReceived(otInstance *aInstance, otRadioFrame *aFrame, otError aError)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aFrame);
    OT_UNUSED_VARIABLE(aError);
}

void otPlatDiagAlarmCallback(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
}

otError otPlatDiagGpioSet(uint32_t aGpio, bool aValue)
{
    OT_UNUSED_VARIABLE(aGpio);
    OT_UNUSED_VARIABLE(aValue);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagGpioGet(uint32_t aGpio, bool *aValue)
{
    OT_UNUSED_VARIABLE(aGpio);
    OT_UNUSED_VARIABLE(aValue);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagGpioSetMode(uint32_t aGpio, otGpioMode aMode)
{
    OT_UNUSED_VARIABLE(aGpio);
    OT_UNUSED_VARIABLE(aMode);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagGpioGetMode(uint32_t aGpio, otGpioMode *aMode)
{
    OT_UNUSED_VARIABLE(aGpio);
    OT_UNUSED_VARIABLE(aMode);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagRadioSetRawPowerSetting(otInstance    *aInstance,
                                          const uint8_t *aRawPowerSetting,
                                          uint16_t       aRawPowerSettingLength)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aRawPowerSetting);
    OT_UNUSED_VARIABLE(aRawPowerSettingLength);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagRadioGetRawPowerSetting(otInstance *aInstance,
                                          uint8_t    *aRawPowerSetting,
                                          uint16_t   *aRawPowerSettingLength)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aRawPowerSetting);
    OT_UNUSED_VARIABLE(aRawPowerSettingLength);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagRadioRawPowerSettingEnable(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aEnable);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagRadioTransmitCarrier(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aEnable);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagRadioTransmitStream(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aEnable);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatDiagRadioGetPowerSettings(otInstance *aInstance,
                                        uint8_t     aChannel,
                                        int16_t    *aTargetPower,
                                        int16_t    *aActualPower,
                                        uint8_t    *aRawPowerSetting,
                                        uint16_t   *aRawPowerSettingLength)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aChannel);
    OT_UNUSED_VARIABLE(aTargetPower);
    OT_UNUSED_VARIABLE(aActualPower);
    OT_UNUSED_VARIABLE(aRawPowerSetting);
    OT_UNUSED_VARIABLE(aRawPowerSettingLength);

    return OT_ERROR_NOT_IMPLEMENTED;
}
