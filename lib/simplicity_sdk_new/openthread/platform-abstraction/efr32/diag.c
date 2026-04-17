/*
 *  Copyright (c) 2023, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the OpenThread platform abstraction for the diagnostics.
 *
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#include <openthread-core-config.h>
#include <utils/code_utils.h>
#include <openthread/cli.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>
#include <openthread/platform/radio.h>
#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/logging.hpp"

#include "diag.h"

#include "sl_gpio.h"
#include "sl_hal_gpio.h"

#include "platform-band.h"
#include "radio_interface.h"
#include "sl_rail_ieee802154.h"

#include "sl_status.h"

#if OPENTHREAD_CONFIG_DIAG_ENABLE

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
#include "sl_rail_util_ant_div.h"
#endif

#define GPIO_PIN_BITMASK 0xFFFFUL
#define GPIO_PORT_BITMASK (0xFFFFUL << 16)
#define GET_GPIO_PIN(x) (x & GPIO_PIN_BITMASK)
#define GET_GPIO_PORT(x) ((x & GPIO_PORT_BITMASK) >> 16)

// To cache the transmit power, so that we don't override it while loading the
// channel config or setting the channel.
static int8_t sTxPower = OPENTHREAD_CONFIG_DEFAULT_TRANSMIT_POWER;

struct PlatformDiagCommand
{
    const char *mName;
    otError (*mCommand)(const otInstance *aInstance, uint8_t aArgsLength, char *aArgs[]);
};

// Diagnostics mode variables.
static bool sDiagMode = false;

static otPlatDiagOutputCallback sDiagOutputCallback  = NULL;
static void                    *sDiagCallbackContext = NULL;

static void diagOutput(const char *aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    if (sDiagOutputCallback != NULL)
    {
        sDiagOutputCallback(aFormat, args, sDiagCallbackContext);
    }

    va_end(args);
}

static void appendErrorResult(otError aError)
{
    if (aError != OT_ERROR_NONE)
    {
        diagOutput("failed\r\nstatus %#x\r\n", aError);
    }
}

// *****************************************************************************
// CLI functions
// *****************************************************************************
static otError processAddressMatch(const otInstance *aInstance, uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_INVALID_ARGS;

    VerifyOrExit(otPlatDiagModeGet(), error = OT_ERROR_INVALID_STATE);
    VerifyOrExit(aArgsLength > 0, error = OT_ERROR_INVALID_ARGS);

    if (strcmp(aArgs[0], "enable") == 0)
    {
        error = otPlatDiagRadioAddressMatch(true);
    }
    else if (strcmp(aArgs[0], "disable") == 0)
    {
        error = otPlatDiagRadioAddressMatch(false);
    }

exit:
    appendErrorResult(error);
    return error;
}

static otError processAutoAck(const otInstance *aInstance, uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_INVALID_ARGS;

    VerifyOrExit(otPlatDiagModeGet(), error = OT_ERROR_INVALID_STATE);
    VerifyOrExit(aArgsLength > 0, error = OT_ERROR_INVALID_ARGS);

    if (strcmp(aArgs[0], "enable") == 0)
    {
        error = otPlatDiagRadioAutoAck(true);
    }
    else if (strcmp(aArgs[0], "disable") == 0)
    {
        error = otPlatDiagRadioAutoAck(false);
    }

exit:
    appendErrorResult(error);
    return error;
}

// *****************************************************************************
// Add more platform specific diagnostic's CLI features here.
// *****************************************************************************
const struct PlatformDiagCommand sCommands[] = {
    {"addr-match", &processAddressMatch},
    {"auto-ack", &processAutoAck},
};

otError otPlatDiagProcess(otInstance *aInstance, uint8_t aArgsLength, char *aArgs[])
{
    otError error = OT_ERROR_INVALID_COMMAND;
    size_t  i;

    for (i = 0; i < otARRAY_LENGTH(sCommands); i++)
    {
        if (strcmp(aArgs[0], sCommands[i].mName) == 0)
        {
            error = sCommands[i].mCommand(aInstance, aArgsLength - 1, aArgsLength > 1 ? &aArgs[1] : NULL);
            break;
        }
    }

    return error;
}

// *****************************************************************************
// Implement platform specific diagnostic's APIs.
// *****************************************************************************

void otPlatDiagSetOutputCallback(otInstance *aInstance, otPlatDiagOutputCallback aCallback, void *aContext)
{
    OT_UNUSED_VARIABLE(aInstance);

    sDiagOutputCallback  = aCallback;
    sDiagCallbackContext = aContext;
}

void otPlatDiagModeSet(bool aMode)
{
    sDiagMode = aMode;
}

bool otPlatDiagModeGet()
{
    return sDiagMode;
}

static sl_rail_status_t startTxStream(sl_rail_stream_mode_t aMode)
{
    uint16_t         txChannel;
    sl_rail_status_t status;

    SuccessOrExit(status = sli_ot_radio_interface_rail_get_channel(&txChannel));
    sl_rail_tx_options_t txOptions = SL_RAIL_TX_OPTIONS_DEFAULT;

#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
    // Translate Tx antenna diversity mode into RAIL Tx Antenna options:
    // If enabled, use the currently-selected antenna, otherwise leave
    // both options 0 so Tx antenna tracks Rx antenna.
    if (sl_rail_util_ant_div_get_tx_antenna_mode() != SL_RAIL_UTIL_ANTENNA_MODE_DISABLED)
    {
        txOptions |= ((sl_rail_util_ant_div_get_tx_antenna_selected() == SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA1)
                          ? SL_RAIL_TX_OPTION_ANTENNA_0
                          : SL_RAIL_TX_OPTION_ANTENNA_1);
    }
#endif
    status = sli_ot_radio_interface_rail_start_tx_stream(txChannel, aMode, txOptions);
exit:
    return status;
}

static sl_rail_status_t stopTxStream(void)
{
    sl_rail_status_t         status;
    uint16_t                 currentChannel;
    sl_rail_scheduler_info_t rxSchedulerInfo = {
        .priority = SL_802154_RADIO_PRIO_BACKGROUND_RX_VALUE,
    };

    SuccessOrExit(status = sli_ot_radio_interface_rail_stop_tx_stream());
    // Since start transmit stream turn off the radio state,
    // call the sl_rail_start_rx to turn on radio
    IgnoreError(sli_ot_radio_interface_rail_get_channel(&currentChannel));

    status = sli_ot_radio_interface_rail_start_rx(currentChannel, &rxSchedulerInfo);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

exit:
    return status;
}

otError otPlatDiagRadioTransmitCarrier(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);

    sl_rail_status_t status;

    if (aEnable)
    {
        otLogInfoPlat("Diag CARRIER-WAVE/Tone start");
        status = startTxStream(SL_RAIL_STREAM_CARRIER_WAVE);
    }
    else
    {
        otLogInfoPlat("Diag CARRIER-WAVE/Tone stop");
        status = stopTxStream();
    }
    return (status != SL_RAIL_STATUS_NO_ERROR ? OT_ERROR_FAILED : OT_ERROR_NONE);
}

otError otPlatDiagRadioTransmitStream(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);

    sl_rail_status_t status;

    if (aEnable)
    {
        otLogInfoPlat("Diag Stream PN9 start");
        status = startTxStream(SL_RAIL_STREAM_PN9_STREAM);
    }
    else
    {
        otLogInfoPlat("Diag Stream stop");
        status = stopTxStream();
    }
    return (status != SL_RAIL_STATUS_NO_ERROR ? OT_ERROR_FAILED : OT_ERROR_NONE);
}

otError otPlatDiagRadioAddressMatch(bool aEnable)
{
    sl_rail_status_t status;

    otLogInfoPlat("Diag address-match %s", aEnable ? "enable" : "disable");

    status = sli_ot_radio_interface_rail_set_promiscuous_mode(!aEnable);
    return (status != SL_RAIL_STATUS_NO_ERROR ? OT_ERROR_FAILED : OT_ERROR_NONE);
}

otError otPlatDiagRadioAutoAck(bool aAutoAckEnabled)
{
    otLogInfoPlat("Diag auto-ack %s", aAutoAckEnabled ? "enable" : "disable");

    sli_ot_radio_interface_rail_pause_rx_auto_ack(!aAutoAckEnabled);

    return OT_ERROR_NONE;
}

void otPlatDiagChannelSet(uint8_t aChannel)
{
    otError          error = OT_ERROR_NONE;
    sl_rail_status_t status;

    sl_rail_scheduler_info_t bgRxSchedulerInfo = {
        .priority = SL_802154_RADIO_PRIO_BACKGROUND_RX_VALUE,
        // sliptime/transaction time is not used for bg rx
    };

    error = sli_ot_radio_interface_load_channel_config(aChannel, sTxPower);
    OT_ASSERT(error == OT_ERROR_NONE);

    status = sli_ot_radio_interface_rail_start_rx(aChannel, &bgRxSchedulerInfo);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
}

void otPlatDiagTxPowerSet(int8_t aTxPower)
{
    sl_rail_status_t status;

    // sl_rail_set_tx_power_dbm() takes power in units of deci-dBm (0.1dBm)
    // Multiply by 10 because aPower is supposed be in units dBm
    status = sli_ot_radio_interface_rail_set_tx_power_dbm(((sl_rail_tx_power_t)aTxPower) * 10);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
    sTxPower = aTxPower;
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

static otError getGpioPortAndPin(uint32_t aGpio, uint16_t *aPort, uint16_t *aPin)
{
    otError error = OT_ERROR_NONE;

    *aPort = GET_GPIO_PORT(aGpio);
    *aPin  = GET_GPIO_PIN(aGpio);

#if defined(SL_CATALOG_GPIO_PRESENT)
    if (*aPort > SL_HAL_GPIO_PORT_MAX || *aPin > SL_HAL_GPIO_PIN_MAX)
#else
    if (*aPort > GPIO_PORT_MAX || *aPin > GPIO_PIN_MAX)
#endif
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }

exit:
    return error;
}

otError otPlatDiagGpioSet(uint32_t aGpio, bool aValue)
{
    otError  error = OT_ERROR_NONE;
    uint16_t port;
    uint16_t pin;

    SuccessOrExit(error = getGpioPortAndPin(aGpio, &port, &pin));

    sl_gpio_t gpio;

    gpio.port = (uint8_t)port;
    gpio.pin  = (uint8_t)pin;

    if (aValue)
    {
        VerifyOrExit(sl_gpio_set_pin(&gpio) == SL_STATUS_OK, error = OT_ERROR_INVALID_ARGS);
    }
    else
    {
        VerifyOrExit(sl_gpio_clear_pin(&gpio) == SL_STATUS_OK, error = OT_ERROR_INVALID_ARGS);
    }

exit:
    return error;
}

otError otPlatDiagGpioGet(uint32_t aGpio, bool *aValue)
{
    otError  error = OT_ERROR_NONE;
    uint16_t port;
    uint16_t pin;

    SuccessOrExit(error = getGpioPortAndPin(aGpio, &port, &pin));

    sl_gpio_t gpio;

    gpio.port = (uint8_t)port;
    gpio.pin  = (uint8_t)pin;

    VerifyOrExit(sl_gpio_get_pin_input(&gpio, aValue) == SL_STATUS_OK, error = OT_ERROR_INVALID_ARGS);

exit:
    return error;
}

otError otPlatDiagGpioSetMode(uint32_t aGpio, otGpioMode aMode)
{
    otError        error = OT_ERROR_NONE;
    uint16_t       port;
    uint16_t       pin;
    sl_gpio_mode_t mode;

    SuccessOrExit(error = getGpioPortAndPin(aGpio, &port, &pin));

    mode = (aMode == OT_GPIO_MODE_INPUT) ? SL_GPIO_MODE_INPUT : SL_GPIO_MODE_PUSH_PULL;

    sl_gpio_t gpio;

    gpio.port = (uint8_t)port;
    gpio.pin  = (uint8_t)pin;

    error = sl_gpio_set_pin_mode(&gpio, mode, 0 /*out*/);

    // Convert to otError.
    VerifyOrExit(error == SL_STATUS_OK,
                 error = (error == SL_STATUS_INVALID_STATE ? OT_ERROR_INVALID_STATE : OT_ERROR_INVALID_ARGS));

exit:
    return error;
}

otError otPlatDiagGpioGetMode(uint32_t aGpio, otGpioMode *aMode)
{
    otError        error = OT_ERROR_NONE;
    uint16_t       port;
    uint16_t       pin;
    sl_gpio_mode_t mode;

    SuccessOrExit(error = getGpioPortAndPin(aGpio, &port, &pin));

    sl_gpio_t            gpio;
    sl_gpio_pin_config_t pin_config;

    gpio.port = (uint8_t)port;
    gpio.pin  = (uint8_t)pin;

    VerifyOrExit(sl_gpio_get_pin_config(&gpio, &pin_config) == SL_STATUS_OK, error = OT_ERROR_INVALID_ARGS);
    mode = pin_config.mode;

    *aMode = (mode == SL_GPIO_MODE_INPUT) ? OT_GPIO_MODE_INPUT : OT_GPIO_MODE_OUTPUT;

exit:
    return error;
}
#endif // OPENTHREAD_CONFIG_DIAG_ENABLE
