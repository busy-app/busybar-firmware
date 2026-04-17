/*
 *  Copyright (c) 2025, The OpenThread Authors.
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
 *   This file defines the energy scan functionality for the EFR32 radio platform.
 */

#ifndef RADIO_ENERGY_SCAN_H
#define RADIO_ENERGY_SCAN_H

#include <openthread-core-config.h>

#include <stdbool.h>
#include <stdint.h>

#include <openthread/instance.h>
#include <openthread/platform/radio.h>

#include "radio_instance.h"
#include "sl_rail_types.h"
#include "sl_status.h"

/**
 * Initialize the energy scan module.
 */
void sli_ot_energy_scan_init(void);

/**
 * Deinitialize the energy scan module.
 *
 * Cancels ongoing scans if any and cleans up resources.
 */
void sli_ot_energy_scan_deinit(void);

/**
 * Deinitialize energy scan for a specific instance.
 *
 * @param[in] aInstance  The OpenThread instance.
 */
void sli_ot_energy_scan_deinit_instance(otInstance *aInstance);

/**
 * Perform a synchronous energy scan on the specified channel.
 * This function blocks until the scan is complete and returns the result.
 * The scan will run for exactly the specified averaging time.
 *
 * @param[in] aInstance       The OpenThread instance.
 * @param[in] aChannel        The channel to scan.
 * @param[in] aAveragingTimeUs The averaging time in microseconds.
 * @param[out] aResult        Pointer to store the energy scan result in dBm.
 *
 * @retval SL_STATUS_OK              Energy scan completed successfully.
 * @retval SL_STATUS_BUSY            Energy scan is already in progress.
 * @retval SL_STATUS_FAIL            Failed to start or complete energy scan.
 * @retval SL_STATUS_NULL_POINTER    aResult is nullptr.
 */
sl_status_t sli_ot_energy_scan(otInstance    *aInstance,
                               uint16_t       aChannel,
                               sl_rail_time_t aAveragingTimeUs,
                               int8_t        *aResult);

/**
 * Start an asynchronous energy scan on the specified channel.
 *
 * @param[in] aInstance      The OpenThread instance.
 * @param[in] aChannel       The channel to scan.
 * @param[in] aAveragingTimeUs The averaging time in microseconds.
 *
 * @retval SL_STATUS_OK      Successfully started energy scan.
 * @retval SL_STATUS_BUSY    Energy scan is already in progress.
 * @retval SL_STATUS_FAIL    Failed to start energy scan.
 */
sl_status_t sli_ot_energy_scan_async(otInstance *aInstance, uint16_t aChannel, sl_rail_time_t aAveragingTimeUs);

/**
 * Process completed energy scan.
 *
 * @param[in] aInstance  The OpenThread instance.
 */
void sli_ot_energy_scan_process(otInstance *aInstance);

/**
 * Check if energy scan is in progress.
 *
 * @retval true  Energy scan is in progress.
 * @retval false Energy scan is not in progress.
 */
bool sli_ot_energy_scan_is_in_progress(void);

/**
 * Check if energy scan is blocking radio receive operations.
 *
 * @param[in] aInstance  The OpenThread instance (may be nullptr for global check).
 *
 * @retval true  Energy scan is blocking receive.
 * @retval false Energy scan is not blocking receive.
 */
bool sli_ot_energy_scan_is_blocking_receive(otInstance *aInstance);

/**
 * Convert sl_status_t to otError for OpenThread compatibility.
 *
 * @param[in] status  The sl_status_t value.
 *
 * @retval The corresponding otError value.
 */
otError sli_ot_energy_scan_status_to_ot_error(sl_status_t status);

class EnergyScan;

// Energy scan parameters structure (needed for external API compatibility)
typedef struct EnergyScanParams
{
    uint8_t  scanChannel;  ///< Energy scan channel
    uint16_t scanDuration; ///< Energy scan duration
} EnergyScanParams;

// Testing helper functions
#ifdef TESTING
namespace Testing {
namespace Radio {
namespace EnergyScanTest {
void        AdvanceToTimerEvent(EnergyScan *scan);
EnergyScan *GetScanForInstance(otInstance *instance);
} // namespace EnergyScanTest
} // namespace Radio
} // namespace Testing
#endif

#endif // RADIO_ENERGY_SCAN_H
