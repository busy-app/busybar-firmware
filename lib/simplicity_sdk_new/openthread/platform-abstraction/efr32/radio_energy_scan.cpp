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
 *     derived from this software without specific or written permission.
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
 *   This file implements the energy scan functionality for the EFR32 radio platform.
 */

#include "radio_energy_scan.hpp"

#include <assert.h>
#include <string.h>

#include <openthread-system.h>
#include <openthread/logging.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/time.h>

#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/logging.hpp"
#include "utils/code_utils.h"

#include "platform-efr32.h"
#include "radio_instance.h"
#include "radio_interface.h"
#include "radio_power_manager.h"
#include "radio_state.h"

#include "sl_rail.h"
#include "sl_rail_ieee802154.h"
#include "sl_rail_types.h"
#include "sl_status.h"

#include "platform-band.h"

// Constants
static constexpr int8_t  ENERGY_SCAN_INVALID_RESULT = -128; // Invalid/uninitialized scan result
static constexpr int8_t  ENERGY_SCAN_MIN_RSSI       = -128; // Minimum possible RSSI value
static constexpr uint8_t SYMBOLS_PER_ENERGY_READING = 8;    // we take a reading every 8 symbols
static constexpr uint8_t QUARTER_DBM_IN_DBM         = 4;

/**
 * Energy scan status enumeration
 */
enum class EnergyScanStatus
{
    Idle,
    InProgress,
    Completed
};

// Timer for energy scan operations
static sl_rail_multi_timer_t rail_timer;

/**
 * Multi-instance state for energy scan operations.
 * Tracks which instance is active and the scan result.
 */
struct InstanceState
{
    volatile EnergyScanStatus status;
    volatile int8_t           resultDbm;
    instanceIndex_t           instanceIndex;
    bool                      isAsync;
};

/**
 * Execution state for the current energy scan operation.
 * Tracks timing and measurement progress.
 */
struct ExecutionState
{
    uint32_t frameCounter;
    uint32_t frameCounterMax;
    int8_t   maxReading;
};

/**
 * Created when scan starts, destroyed when scan completes.
 * Provides automatic cleanup and self-contained state management.
 */
class EnergyScan
{
private:
    InstanceState  mInstance;
    ExecutionState mExecution;
    uint16_t       mChannel;

public:
    // Constructors
    EnergyScan(otInstance *instance, uint16_t channel, sl_rail_time_t duration, bool isAsync);
    ~EnergyScan();

    // State queries
    bool isCompleted() const;
    bool isInProgress() const;
    bool isBlockingReceive() const;

    // Instance management
    otInstance     *getInstance() const;
    instanceIndex_t getInstanceIndex() const;
    bool            isAsynchronous() const;

    // Result management
    int8_t getResult() const;

    // Execution state queries
    uint32_t getFrameCounter() const;
    uint32_t getFrameCounterMax() const;
    bool     hasMoreReadings() const;

    // Core scan operations
    sl_status_t start();
    void        processEnergyReading();
    void        incrementFrameCounter();
    void        completeScan();
};

// Per-instance scan storage
static EnergyScan *sEnergyScans[RADIO_INTERFACE_COUNT] = {nullptr};

//------------------------------------------------------------------------------
// Energy Scan Management Functions
// Internal functions for managing energy scan lifecycle

namespace {
// Forward declarations for functions that are only used within this namespace
sl_status_t startEnergyScan(EnergyScan *scan);

// Internal helper functions
bool        isScanInProgress(EnergyScan *scan);
bool        isBlockingReceive(EnergyScan *scan);
sl_status_t waitForSyncScanCompletion(EnergyScan *scan, int8_t *result);
void        processCompletion(EnergyScan *scan);

// Helper functions
uint16_t         getSymbolDurationUs(void);
sl_rail_status_t scheduleNextReading(EnergyScan *scan, uint16_t symbols);
void             timer_handler(struct sl_rail_multi_timer *tmr, sl_rail_time_t expectedTimeOfEvent, void *cbArg);

// Clear a single scan slot and reset pointer
void clearScan(EnergyScan *&scan)
{
    if (scan != nullptr)
    {
        delete scan;
        scan = nullptr;
    }
}

// Clear all scan slots
void clearAllScans()
{
    for (auto &scan : sEnergyScans)
    {
        clearScan(scan);
    }
}

// Check if a scan is in progress
bool isScanInProgress(EnergyScan *scan)
{
    return (scan != nullptr && scan->isInProgress());
}

// Check if scan is blocking receive
bool isBlockingReceive(EnergyScan *scan)
{
    return (scan != nullptr && scan->isBlockingReceive());
}

// Wait for sync scan completion and get result
sl_status_t waitForSyncScanCompletion(EnergyScan *scan, int8_t *result)
{
    sl_status_t status = SL_STATUS_OK;

    otEXPECT_ACTION(result != nullptr, status = SL_STATUS_NULL_POINTER);
    otEXPECT_ACTION(scan != nullptr, status = SL_STATUS_FAIL);

    // Wait for scan to complete naturally based on scan duration
    while (scan->isInProgress())
    {
#ifdef TESTING
        Testing::Radio::EnergyScanTest::AdvanceToTimerEvent(scan);
#endif
    }

    *result = scan->getResult();

exit:
    return status;
}

// Process scan completion
void processCompletion(EnergyScan *scan)
{
    otInstance     *instance = nullptr;
    instanceIndex_t index;

    otEXPECT(scan != nullptr);
    otEXPECT(scan->isCompleted());

    index = scan->getInstanceIndex();

    if (scan->isAsynchronous())
    {
        // Async: get result first, then cleanup and report
        instance      = scan->getInstance();
        int8_t result = scan->getResult();

        clearScan(sEnergyScans[index]);

        otPlatRadioEnergyScanDone(instance, result);
        otSysEventSignalPending();
    }
    else
    {
        // Sync: nothing to notify; result already stored for caller
        clearScan(sEnergyScans[index]);
    }

exit:
    return;
}

// Helper function implementations
uint16_t getSymbolDurationUs(void)
{
    uint32_t symbolRate = sli_ot_radio_interface_get_symbol_rate();
    if (symbolRate)
    {
        symbolRate = (uint16_t)(1000000 / symbolRate);
    }
    return (symbolRate > 0) ? (uint16_t)symbolRate : 1;
}

sl_rail_status_t scheduleNextReading(EnergyScan *scan, uint16_t symbols)
{
    sli_ot_radio_interface_cancel_timer(&rail_timer);
    return sli_ot_radio_interface_set_timer(&rail_timer,
                                            symbols * getSymbolDurationUs(),
                                            SL_RAIL_TIME_DELAY,
                                            timer_handler,
                                            scan);
}

// Common implementation for both sync and async scans
sl_status_t startEnergyScan(EnergyScan *scan)
{
    sl_status_t status;

    otEXPECT_ACTION(scan != nullptr, status = SL_STATUS_NULL_POINTER);

    // Start the scan
    status = scan->start();

exit:
    return status;
}

// Finalize the energy scan when all readings are complete.
// Reports the result and re-enables frame detection.
void finalizeEnergyScan(EnergyScan *scan)
{
    sl_rail_status_t status;

    otEXPECT(scan != nullptr);

    // Energy scan complete, report RSSI value
    scan->completeScan();

    // Report completion for async scans only
    // Sync scans will call this after extracting the result
    if (scan->isAsynchronous())
    {
        processCompletion(scan);
    }

    // Re-enable frame detection
    status =
        sli_ot_radio_interface_config_rx_options(SL_RAIL_RX_OPTION_DISABLE_FRAME_DETECTION, SL_RAIL_RX_OPTIONS_NONE);
    otEXPECT(status == SL_RAIL_STATUS_NO_ERROR);

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    // Note: Debug counters for energy scan are tracked via radio_interface
#endif

exit:
    return;
}

// Timer handler for energy scan
void timer_handler(struct sl_rail_multi_timer *tmr, sl_rail_time_t expectedTimeOfEvent, void *cbArg)
{
    OT_UNUSED_VARIABLE(tmr);
    OT_UNUSED_VARIABLE(expectedTimeOfEvent);

    EnergyScan *scan = static_cast<EnergyScan *>(cbArg);
    otEXPECT(scan != nullptr);

    // Process the current energy reading
    scan->processEnergyReading();

    // Increment frame counter after processing this reading
    scan->incrementFrameCounter();

    // Determine next action based on remaining readings
    if (scan->hasMoreReadings())
    {
        // More readings needed - schedule next reading
        scheduleNextReading(scan, SYMBOLS_PER_ENERGY_READING);
    }
    else
    {
        // All readings complete - finalize scan
        finalizeEnergyScan(scan);
    }

exit:
    return;
}
} // namespace

//------------------------------------------------------------------------------
// EnergyScan Method Implementations

EnergyScan::EnergyScan(otInstance *instance, uint16_t channel, sl_rail_time_t duration, bool isAsync)
    : mChannel(channel)
{
    // Initialize instance state
    mInstance.status        = EnergyScanStatus::Idle;
    mInstance.resultDbm     = ENERGY_SCAN_INVALID_RESULT;
    mInstance.instanceIndex = sli_ot_radio_instance_get_index(instance);
    mInstance.isAsync       = isAsync;
    OT_ASSERT(mInstance.instanceIndex < RADIO_INTERFACE_COUNT);

    // Initialize execution state
    mExecution.frameCounter    = 0;
    mExecution.frameCounterMax = duration / (getSymbolDurationUs() * SYMBOLS_PER_ENERGY_READING);
    mExecution.maxReading      = ENERGY_SCAN_MIN_RSSI;

    // Make sure we perform at least one reading
    if (mExecution.frameCounterMax == 0)
    {
        mExecution.frameCounterMax = 1;
    }
}

EnergyScan::~EnergyScan()
{
    // Automatic cleanup - cancel any ongoing timer
    if (mInstance.status == EnergyScanStatus::InProgress)
    {
        sli_ot_radio_interface_cancel_timer(&rail_timer);
    }
}

// State queries
bool EnergyScan::isCompleted() const
{
    return mInstance.status == EnergyScanStatus::Completed;
}

bool EnergyScan::isInProgress() const
{
    return mInstance.status == EnergyScanStatus::InProgress;
}

bool EnergyScan::isBlockingReceive() const
{
    return isInProgress();
}

// Instance management
otInstance *EnergyScan::getInstance() const
{
    return sli_ot_radio_instance_get(mInstance.instanceIndex);
}

instanceIndex_t EnergyScan::getInstanceIndex() const
{
    return mInstance.instanceIndex;
}

bool EnergyScan::isAsynchronous() const
{
    return mInstance.isAsync;
}

// Result management
int8_t EnergyScan::getResult() const
{
    return mInstance.resultDbm;
}

// Execution state queries
uint32_t EnergyScan::getFrameCounter() const
{
    return mExecution.frameCounter;
}

uint32_t EnergyScan::getFrameCounterMax() const
{
    return mExecution.frameCounterMax;
}

bool EnergyScan::hasMoreReadings() const
{
    return getFrameCounter() < getFrameCounterMax();
}

// Core scan operations
sl_status_t EnergyScan::start()
{
    sl_rail_status_t status        = SL_RAIL_STATUS_NO_ERROR;
    sl_status_t      result        = SL_STATUS_OK;
    efr32BandConfig *config        = nullptr;
    efr32BandConfig *currentConfig = nullptr;

    // Set radio to idle before starting scan
    sli_ot_radio_state_set_idle();

    // Get and load the appropriate band configuration for the channel
    config = sli_ot_radio_interface_get_band_config(static_cast<uint8_t>(mChannel));
    otEXPECT_ACTION(config != nullptr, result = SL_STATUS_INVALID_PARAMETER);

    currentConfig = sli_ot_radio_interface_get_current_band_config();
    if (currentConfig != config)
    {
        sli_ot_radio_interface_load_rail_config(config, SL_INVALID_TX_POWER);
    }

    // Disable frame detection during energy scan
    status = sli_ot_radio_interface_config_rx_options(SL_RAIL_RX_OPTION_DISABLE_FRAME_DETECTION,
                                                      SL_RAIL_RX_OPTION_DISABLE_FRAME_DETECTION);
    otEXPECT_ACTION(status == SL_RAIL_STATUS_NO_ERROR, result = SL_STATUS_FAIL);

    otEXPECT_ACTION(sli_ot_radio_interface_set_rx((uint8_t)mChannel) == OT_ERROR_NONE, result = SL_STATUS_FAIL);

    // Schedule initial timer for first sample:
    // sync scan finishes after this single sample
    // async scan reschedules in timer_handler
    status = scheduleNextReading(this, SYMBOLS_PER_ENERGY_READING);
    otEXPECT_ACTION(status == SL_RAIL_STATUS_NO_ERROR, result = SL_STATUS_FAIL);

    // Mark scan as in progress only after successful initialization
    mInstance.status = EnergyScanStatus::InProgress;

exit:
    if (result != SL_STATUS_OK)
    {
        mInstance.status = EnergyScanStatus::Idle;
    }

    return result;
}

void EnergyScan::processEnergyReading()
{
    bool    waitForRSSI           = (mExecution.frameCounter == mExecution.frameCounterMax - 1);
    int16_t currentRSSIQuarterdBm = sli_ot_radio_interface_get_rssi(waitForRSSI);
    int8_t  currentRSSI;

    if (currentRSSIQuarterdBm == SL_RAIL_RSSI_INVALID)
    {
        mExecution.maxReading = ENERGY_SCAN_INVALID_RESULT;
    }
    else
    {
        currentRSSI = (int8_t)(currentRSSIQuarterdBm / QUARTER_DBM_IN_DBM);

        if (mExecution.maxReading < currentRSSI)
        {
            mExecution.maxReading = currentRSSI;
        }
    }
}

void EnergyScan::incrementFrameCounter()
{
    mExecution.frameCounter++;
}

void EnergyScan::completeScan()
{
    mInstance.resultDbm = mExecution.maxReading;
    mInstance.status    = EnergyScanStatus::Completed;
}

//------------------------------------------------------------------------------
// Energy Scan Management Function Implementations

//------------------------------------------------------------------------------
// Helper Functions

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Helper Functions

//------------------------------------------------------------------------------
// Public API Functions
// Core energy scan functionality for OpenThread platform abstraction

/**
 * Initialize energy scan module.
 * No initialization needed for per-scan objects.
 */
void sli_ot_energy_scan_init(void)
{
    // Initialize the timer to zero
    memset(&rail_timer, 0, sizeof(rail_timer));

    // Clean up any ongoing energy scans
    clearAllScans();
}

/**
 * Deinitialize energy scan module.
 * Cancels any ongoing scan and cleans up.
 */
void sli_ot_energy_scan_deinit(void)
{
    // Clean up any ongoing energy scans
    clearAllScans();
}

/**
 * Deinitialize energy scan for a specific instance.
 * Cancels the scan and cleans up.
 */
void sli_ot_energy_scan_deinit_instance(otInstance *instance)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(instance);
    OT_ASSERT(index < RADIO_INTERFACE_COUNT);

    clearScan(sEnergyScans[index]);
}

bool sli_ot_energy_scan_is_in_progress(void)
{
    // Check if any instance has a scan in progress
    for (instanceIndex_t i = 0; i < RADIO_INTERFACE_COUNT; i++)
    {
        if (isScanInProgress(sEnergyScans[i]))
        {
            return true;
        }
    }
    return false;
}

bool sli_ot_energy_scan_is_blocking_receive(otInstance *aInstance)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    OT_ASSERT(index < RADIO_INTERFACE_COUNT);

    return isBlockingReceive(sEnergyScans[index]);
}

sl_status_t sli_ot_energy_scan(otInstance    *aInstance,
                               uint16_t       aChannel,
                               sl_rail_time_t aAveragingTimeUs,
                               int8_t        *aResult)
{
    sl_status_t     status;
    EnergyScan     *scan  = nullptr;
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    OT_ASSERT(index < RADIO_INTERFACE_COUNT);

    otEXPECT_ACTION(aResult != nullptr, status = SL_STATUS_NULL_POINTER);
    otEXPECT_ACTION(sEnergyScans[index] == nullptr, status = SL_STATUS_BUSY);

    // Create and start synchronous scan for the specified instance
    scan   = new EnergyScan(aInstance, aChannel, aAveragingTimeUs, false);
    status = startEnergyScan(scan);
    otEXPECT(status == SL_STATUS_OK);

    sEnergyScans[index] = scan;

    // Wait for completion and get the result
    status = waitForSyncScanCompletion(scan, aResult);
    otEXPECT(status == SL_STATUS_OK);

    // Report completion
    processCompletion(scan);

exit:
    if (status != SL_STATUS_OK)
    {
        sli_ot_energy_scan_deinit_instance(aInstance);
    }

    return status;
}

sl_status_t sli_ot_energy_scan_async(otInstance *aInstance, uint16_t aChannel, sl_rail_time_t aAveragingTimeUs)
{
    sl_status_t     status;
    EnergyScan     *scan  = nullptr;
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    OT_ASSERT(index < RADIO_INTERFACE_COUNT);

    otEXPECT_ACTION(sEnergyScans[index] == nullptr, status = SL_STATUS_BUSY);

    // Create and start asynchronous scan
    // Only set per-instance pointer when start is successful
    scan   = new EnergyScan(aInstance, aChannel, aAveragingTimeUs, true);
    status = startEnergyScan(scan);
    otEXPECT(status == SL_STATUS_OK);

    sEnergyScans[index] = scan;

exit:
    if (status != SL_STATUS_OK)
    {
        sli_ot_energy_scan_deinit_instance(aInstance);
    }

    return status;
}

void sli_ot_energy_scan_process(otInstance *aInstance)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    OT_ASSERT(index < RADIO_INTERFACE_COUNT);

    EnergyScan *scan = sEnergyScans[index];

    otEXPECT(scan != nullptr);
    otEXPECT(scan->isCompleted());

    // Report completion
    processCompletion(scan);

exit:
    return;
}

//------------------------------------------------------------------------------
// Energy Scan State Management Functions
// Functions to manage energy scan state from external modules

otError sli_ot_energy_scan_status_to_ot_error(sl_status_t status)
{
    otError error;

    switch (status)
    {
    case SL_STATUS_OK:
        error = OT_ERROR_NONE;
        break;
    case SL_STATUS_BUSY:
        error = OT_ERROR_BUSY;
        break;
    case SL_STATUS_INVALID_PARAMETER:
    case SL_STATUS_NULL_POINTER:
        error = OT_ERROR_INVALID_ARGS;
        break;
    case SL_STATUS_FAIL:
    default:
        error = OT_ERROR_FAILED;
        break;
    }

    return error;
}

#ifdef TESTING
namespace Testing {
namespace Radio {
namespace EnergyScanTest {
void AdvanceToTimerEvent(EnergyScan *scan)
{
    // Test helper: advances timer for the specified scan
    timer_handler(nullptr, 0, scan);
}

EnergyScan *GetScanForInstance(otInstance *instance)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(instance);
    return sEnergyScans[index];
}
} // namespace EnergyScanTest
} // namespace Radio
} // namespace Testing
#endif
