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
 *   This file implements the radio power manager for the EFR32 platform.
 */

#include <openthread-core-config.h>

#include "platform-band.h"
#include "radio_channel_switching.h"
#include "radio_instance.h"
#include "radio_interface.h"
#include "radio_multi_channel.h"
#include "radio_power_manager.h"

extern "C" {
#include "rail_config.h"
#include "sl_rail_ieee802154.h"

#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
#include "sl_rail_mux_rename.h"
#endif // SL_CATALOG_RAIL_MULTIPLEXER_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT
#include "sl_rail_util_ieee802154_fast_channel_switching_config.h"
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT

} // extern "C"

constexpr size_t RADIO_POWER_MANAGER_MAX_INSTANCES = RADIO_INTERFACE_COUNT;

#if !OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE

static int8_t sli_max_channel_power[RADIO_POWER_MANAGER_MAX_INSTANCES][SL_MAX_CHANNELS_SUPPORTED];
static int8_t sli_default_tx_power[RADIO_POWER_MANAGER_MAX_INSTANCES];

/**
 * This function gets the lowest value for the max_tx_power for a channel, from the max_tx_powerTable set
 * across all interfaces. It also gets the highest default_tx_power set across all interfaces.
 *
 * @param[out]  default_tx_power    A pointer to update the derived default_tx_power across all IIDs.
 * @param[out]  tx_power_from_table A pointer to update the Tx Power derived from the MaxChannelPowerTable.
 * @param[in]   channel             Channel of interest
 *
 */
static void sli_get_default_and_max_powers_across_instances(int8_t  *default_tx_power,
                                                            int8_t  *tx_power_from_table,
                                                            uint16_t channel)
{
    OT_ASSERT(tx_power_from_table != NULL);
    OT_ASSERT(default_tx_power != NULL);

    for (uint8_t i = 0U; i < RADIO_POWER_MANAGER_MAX_INSTANCES; i++)
    {
        // Obtain the minimum Tx power set by different instances, for `channel`
        // If there is an interface using lower Tx power than the one we have
        // in tx_power_from_table..
        // Update tx_power_from_table.
        *tx_power_from_table = SL_MIN(*tx_power_from_table, sli_max_channel_power[i][channel - SL_CHANNEL_MIN]);

        // If the default Tx Power set is not invalid..
        if (sli_default_tx_power[i] != SL_INVALID_TX_POWER)
        {
            // Obtain the Max value between local default_tx_power and sli_default_tx_power.
            // If selected default Tx Power is Invalid, initialise it to sli_default_tx_power.
            // We have already validated that sli_default_tx_power holds a valid value.
            *default_tx_power = (*default_tx_power == SL_INVALID_TX_POWER)
                                    ? sli_default_tx_power[i]
                                    : SL_MAX(*default_tx_power, sli_default_tx_power[i]);
        }
    }
}

/**
 * This function returns the tx power to be used based on the default and max tx power table, for a given channel.
 *
 * @param[in]   channel   Channel of interest
 *
 * @returns The radio Tx Power for the given channel, in dBm.
 *
 */
static int8_t sli_get_max_tx_power_across_instances(uint16_t channel)
{
    int8_t max_channel_tx_power = SL_INVALID_TX_POWER;
    int8_t max_default_tx_power = SL_INVALID_TX_POWER;
    int8_t selected_tx_power    = SL_INVALID_TX_POWER;

#if FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

    if (sli_ot_radio_channel_switching_is_multi_channel_enabled())
    {
        sl_rail_ieee802154_rx_channel_switching_cfg_t channel_switching_cfg;

        // Get switching config
        sl_get_channel_switching_cfg(&channel_switching_cfg);

        // Find the max_channel_tx_power, to be minimum of Max channel power for the
        // channels infast channel config, accross all instances. This is because, if instance_1
        // sets the max tx power of the channel to be less than the max tx power set by
        // instance_2, we will need to work with the lower tx power to be compliant on both
        // interfaces.

        // Find the max_default_tx_power, to be maximum of the default Tx power accross all
        // the interfaces.

        for (uint8_t i = 0U; i < SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS; i++)
        {
            channel = channel_switching_cfg.channels[i];
            sli_get_default_and_max_powers_across_instances(&max_default_tx_power, &max_channel_tx_power, channel);
        }
    }
    else
#endif
    {
        sli_get_default_and_max_powers_across_instances(&max_default_tx_power, &max_channel_tx_power, channel);
    }

    // Return the minimum of max_channel_tx_power and max_default_tx_power.
    selected_tx_power = SL_MIN(max_channel_tx_power, max_default_tx_power);
    return (selected_tx_power == SL_INVALID_TX_POWER) ? OPENTHREAD_CONFIG_DEFAULT_TRANSMIT_POWER : selected_tx_power;
}

#endif //! OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE

void sli_set_tx_power_in_rail(int8_t power_in_dbm)
{
    sl_rail_status_t status;

    // sl_rail_set_tx_power_dbm() takes power in units of deci-dBm (0.1dBm)
    // Multiply by 10 because power_in_dbm is supposed be in units dBm
    status = sli_ot_radio_interface_rail_set_tx_power_dbm(((sl_rail_tx_power_t)power_in_dbm) * 10);

    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
}

void sli_init_power_manager(void)
{
#if !OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE
    memset(sli_max_channel_power, SL_INVALID_TX_POWER, sizeof(sli_max_channel_power));
    memset(sli_default_tx_power, SL_INVALID_TX_POWER, sizeof(sli_default_tx_power));
#endif //! OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE
}

void sli_update_tx_power_after_config_update(sl_rail_tx_pa_mode_t pa_mode, int8_t tx_power)
{
    sl_rail_status_t     status;
    sl_rail_tx_pa_mode_t current_pa_mode;
    sl_rail_tx_power_t   tx_power_dbm = tx_power * 10;

    current_pa_mode = sli_ot_radio_interface_rail_get_tx_pa_mode();

    // Always need to re-establish tx power dbm after rail_config_tx_power
    // First need to get existing power setting and reassert value after config

    if (current_pa_mode != SL_RAIL_TX_PA_MODE_INVALID)
    {
        tx_power_dbm = sli_ot_radio_interface_rail_get_tx_power_dbm();
    }

    status = sli_ot_radio_interface_rail_config_tx_power(pa_mode);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

    status = sli_ot_radio_interface_rail_set_tx_power_dbm(tx_power_dbm);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
}

otError sli_set_channel_max_tx_power(otInstance *aInstance, uint8_t channel, int8_t max_power)
{
    otError error = OT_ERROR_NONE;

#if !OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE
    int8_t          tx_power;
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);

    otEXPECT_ACTION(channel >= SL_CHANNEL_MIN && channel <= SL_CHANNEL_MAX, error = OT_ERROR_INVALID_ARGS);

    sli_max_channel_power[index][channel - SL_CHANNEL_MIN] = max_power;
    tx_power                                               = sl_get_tx_power_for_current_channel(aInstance);
    sli_set_tx_power_in_rail(tx_power);

exit:
#else
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(channel);
    OT_UNUSED_VARIABLE(max_power);
    error = OT_ERROR_NOT_IMPLEMENTED;
#endif
    return error;
}

otError sli_set_default_tx_power(otInstance *aInstance, int8_t tx_power)
{
    otError error = OT_ERROR_NONE;

#if !OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE
    int8_t          max_tx_power;
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);

    sli_default_tx_power[index] = tx_power;
    max_tx_power                = sl_get_tx_power_for_current_channel(aInstance);

    sli_set_tx_power_in_rail(max_tx_power);
#else
    OT_UNUSED_VARIABLE(instance);
    OT_UNUSED_VARIABLE(tx_power);
    error = OT_ERROR_NOT_IMPLEMENTED;
#endif

    return error;
}

int8_t sl_get_tx_power_for_current_channel(otInstance *instance)
{
    int8_t   tx_power;
    uint16_t channel;

    sli_ot_radio_interface_rail_get_channel_ptr(&channel);

#if OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE
    uint8_t  raw_power_calibration[SL_OPENTHREAD_RAW_POWER_CALIBRATION_LENGTH];
    uint8_t  fem_setting[SL_OPENTHREAD_FEM_SETTING_LENGTH];
    uint16_t raw_calibration_length = SL_OPENTHREAD_RAW_POWER_CALIBRATION_LENGTH;
    uint16_t fem_setting_length     = SL_OPENTHREAD_FEM_SETTING_LENGTH;
    otError  error;

    error = otPlatRadioGetRawPowerSetting(instance, channel, raw_power_calibration, &raw_calibration_length);

    error = sl_parse_raw_power_calibration_cb(raw_power_calibration,
                                              raw_calibration_length,
                                              &tx_power,
                                              fem_setting,
                                              &fem_setting_length);
    OT_ASSERT(error == OT_ERROR_NONE);

    sl_configure_fem_cb(fem_setting, fem_setting_length);

#else
    OT_UNUSED_VARIABLE(instance);
    tx_power = sli_get_max_tx_power_across_instances(channel);
#endif

    return tx_power;
}

#if OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE
SL_WEAK otError sl_parse_raw_power_calibration_cb(uint8_t  *raw_power_calibration,
                                                  uint16_t  raw_setting_length,
                                                  int8_t   *radio_power,
                                                  uint8_t  *fem_setting,
                                                  uint16_t *fem_setting_length)
{
    OT_ASSERT(raw_power_calibration != NULL);
    OT_ASSERT(radio_power != NULL);
    OT_UNUSED_VARIABLE(raw_setting_length);
    OT_UNUSED_VARIABLE(fem_setting);
    OT_UNUSED_VARIABLE(fem_setting_length);

    *radio_power = raw_power_calibration[0];
    return OT_ERROR_NONE;
}

SL_WEAK void sl_configure_fem_cb(uint8_t *fem_setting, uint16_t fem_setting_length)
{
    OT_UNUSED_VARIABLE(fem_setting);
    OT_UNUSED_VARIABLE(fem_setting_length);
}
#endif // OPENTHREAD_CONFIG_POWER_CALIBRATION_ENABLE
