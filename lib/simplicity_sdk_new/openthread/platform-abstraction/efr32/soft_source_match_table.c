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
 *   This file implements a software Source Match table, for radios that don't have
 *   such hardware acceleration.
 *
 */
#include "soft_source_match_table.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <openthread/logging.h>
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
#include <openthread/platform/multipan.h>
#endif
#include "radio_instance.h"
#include "common/debug.hpp"
#include "utils/code_utils.h"

// Print entire source match tables when
#ifndef PRINT_MULTIPAN_SOURCE_MATCH_TABLES
#define PRINT_MULTIPAN_SOURCE_MATCH_TABLES 0
#endif

#if RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM || RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM
static uint16_t sPanId[RADIO_CONFIG_SRC_MATCH_PANID_NUM] = {0};

#if PRINT_MULTIPAN_SOURCE_MATCH_TABLES
static void printPanIdTable(void)
{
    for (panIndex_t panIndex = 0; panIndex < RADIO_CONFIG_SRC_MATCH_PANID_NUM; panIndex++)
    {
        otLogDebgPlat("sPanId[panIndex=%d] = 0x%04x", panIndex, sPanId[panIndex]);
    }
}
#else
#define printPanIdTable()
#endif

void utilsSoftSrcMatchSetPanId(otInstance *aInstance, uint16_t aPanId)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    sPanId[panIndex] = aPanId;
    otLogInfoPlat("Setting panIndex=%d to 0x%04x", panIndex, aPanId);

    printPanIdTable();
    return;
}
#endif // RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM || RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM

#if RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM
typedef struct srcMatchShortEntry
{
    uint16_t checksum;
    bool     allocated;
} sSrcMatchShortEntry;

static sSrcMatchShortEntry srcMatchShortEntry[RADIO_CONFIG_SRC_MATCH_PANID_NUM][RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM];

#if PRINT_MULTIPAN_SOURCE_MATCH_TABLES
static void printShortEntryTable(otInstance *aInstance)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    otLogDebgPlat("================================|============|===========");
    otLogDebgPlat("ShortEntry[panIndex][entry]     | .allocated | .checksum ");
    otLogDebgPlat("================================|============|===========");
    for (int16_t i = 0; i < RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM; i++)
    {
        otLogDebgPlat("ShortEntry[panIndex=%d][entry=%d] | %d          | 0x%04x",
                      panIndex,
                      i,
                      srcMatchShortEntry[panIndex][i].allocated,
                      srcMatchShortEntry[panIndex][i].checksum);
    }
    otLogDebgPlat("================================|============|===========");
}
#else
#define printShortEntryTable(aInstance)
#endif

int16_t utilsSoftSrcMatchShortFindEntry(otInstance *aInstance, uint16_t aShortAddress)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);
    int16_t          entry    = -1;

    uint16_t checksum = aShortAddress + sPanId[panIndex];

    for (int16_t i = 0; i < RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM; i++)
    {
        if (checksum == srcMatchShortEntry[panIndex][i].checksum && srcMatchShortEntry[panIndex][i].allocated)
        {
            entry = i;
            break;
        }
    }

    return entry;
}

static int16_t findSrcMatchShortAvailEntry(otInstance *aInstance)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);
    int16_t          entry    = -1;

    for (int16_t i = 0; i < RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM; i++)
    {
        if (!srcMatchShortEntry[panIndex][i].allocated)
        {
            entry = i;
            break;
        }
    }

    return entry;
}

static inline void addToSrcMatchShortIndirect(otInstance *aInstance, uint16_t entry, uint16_t aShortAddress)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);
    uint16_t         checksum = aShortAddress + sPanId[panIndex];

    srcMatchShortEntry[panIndex][entry].checksum  = checksum;
    srcMatchShortEntry[panIndex][entry].allocated = true;

    printShortEntryTable(aInstance);
}

static inline void removeFromSrcMatchShortIndirect(otInstance *aInstance, uint16_t entry)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    srcMatchShortEntry[panIndex][entry].allocated = false;
    srcMatchShortEntry[panIndex][entry].checksum  = 0;

    printShortEntryTable(aInstance);
}

otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance, uint16_t aShortAddress)
{
    otError error = OT_ERROR_NONE;

    int16_t entry = utilsSoftSrcMatchShortFindEntry(aInstance, aShortAddress);
    // Prevent duplicate entries.
    otEXPECT(!(entry >= 0 && entry < RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM));

    entry = findSrcMatchShortAvailEntry(aInstance);

    otLogDebgPlat("Add ShortAddr: entry=%d, addr=0x%04x", entry, aShortAddress);

    otEXPECT_ACTION(entry >= 0 && entry < RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM, error = OT_ERROR_NO_BUFS);

    addToSrcMatchShortIndirect(aInstance, (uint16_t)entry, aShortAddress);

exit:
    return error;
}

otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance, uint16_t aShortAddress)
{
    otError error = OT_ERROR_NONE;

    int16_t entry = utilsSoftSrcMatchShortFindEntry(aInstance, aShortAddress);

    otLogDebgPlat("Clear ShortAddr: entry=%d, addr=0x%04x", entry, aShortAddress);

    otEXPECT_ACTION(entry >= 0 && entry < RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM, error = OT_ERROR_NO_ADDRESS);

    removeFromSrcMatchShortIndirect(aInstance, (uint16_t)entry);

exit:
    return error;
}

void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    otLogDebgPlat("Clear ShortAddr entries");

    memset(srcMatchShortEntry[panIndex], 0, sizeof(srcMatchShortEntry[panIndex]));

    printShortEntryTable(aInstance);
}
#endif // RADIO_CONFIG_SRC_MATCH_SHORT_ENTRY_NUM

#if RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM
typedef struct srcMatchExtEntry
{
    uint16_t checksum;
    bool     allocated;
} sSrcMatchExtEntry;

static sSrcMatchExtEntry srcMatchExtEntry[RADIO_CONFIG_SRC_MATCH_PANID_NUM][RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM];

#if PRINT_MULTIPAN_SOURCE_MATCH_TABLES
static void printExtEntryTable(otInstance *aInstance)
{
    const panIndex_t panIndex = getPanInde(aInstance);

    otLogDebgPlat("==============================|============|===========");
    otLogDebgPlat("ExtEntry[panIndex][entry]     | .allocated | .checksum ");
    otLogDebgPlat("==============================|============|===========");
    for (int16_t i = 0; i < RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM; i++)
    {
        otLogDebgPlat("ExtEntry[panIndex=%d][entry=%d] | %d          | 0x%04x",
                      panIndex,
                      i,
                      srcMatchExtEntry[panIndex][i].allocated,
                      srcMatchExtEntry[panIndex][i].checksum);
    }
    otLogDebgPlat("==============================|============|===========");
}
#else
#define printExtEntryTable(aInstance)
#endif

int16_t utilsSoftSrcMatchExtFindEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);
    int16_t          entry    = -1;
    uint16_t         checksum = sPanId[panIndex];

    checksum += (uint16_t)aExtAddress->m8[0] | (uint16_t)(aExtAddress->m8[1] << 8);
    checksum += (uint16_t)aExtAddress->m8[2] | (uint16_t)(aExtAddress->m8[3] << 8);
    checksum += (uint16_t)aExtAddress->m8[4] | (uint16_t)(aExtAddress->m8[5] << 8);
    checksum += (uint16_t)aExtAddress->m8[6] | (uint16_t)(aExtAddress->m8[7] << 8);

    for (int16_t i = 0; i < RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM; i++)
    {
        if (checksum == srcMatchExtEntry[panIndex][i].checksum && srcMatchExtEntry[panIndex][i].allocated)
        {
            entry = i;
            break;
        }
    }

    return entry;
}

static int16_t findSrcMatchExtAvailEntry(otInstance *aInstance)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);
    int16_t          entry    = -1;

    for (int16_t i = 0; i < RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM; i++)
    {
        if (!srcMatchExtEntry[panIndex][i].allocated)
        {
            entry = i;
            break;
        }
    }

    return entry;
}

static inline void addToSrcMatchExtIndirect(otInstance *aInstance, uint16_t entry, const otExtAddress *aExtAddress)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);
    uint16_t         checksum = sPanId[panIndex];

    checksum += (uint16_t)aExtAddress->m8[0] | (uint16_t)(aExtAddress->m8[1] << 8);
    checksum += (uint16_t)aExtAddress->m8[2] | (uint16_t)(aExtAddress->m8[3] << 8);
    checksum += (uint16_t)aExtAddress->m8[4] | (uint16_t)(aExtAddress->m8[5] << 8);
    checksum += (uint16_t)aExtAddress->m8[6] | (uint16_t)(aExtAddress->m8[7] << 8);

    srcMatchExtEntry[panIndex][entry].checksum  = checksum;
    srcMatchExtEntry[panIndex][entry].allocated = true;

    printExtEntryTable(aInstance);
}

static inline void removeFromSrcMatchExtIndirect(otInstance *aInstance, uint16_t entry)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    srcMatchExtEntry[panIndex][entry].allocated = false;
    srcMatchExtEntry[panIndex][entry].checksum  = 0;

    printExtEntryTable(aInstance);
}

otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    otError error = OT_ERROR_NONE;

    int16_t entry = utilsSoftSrcMatchExtFindEntry(aInstance, aExtAddress);
    // Prevent duplicate entries.
    otEXPECT(!(entry >= 0 && entry < RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM));

    entry = findSrcMatchExtAvailEntry(aInstance);

    otLogDebgPlat("Add ExtAddr: entry=%d, addr %p", entry, (void *)aExtAddress->m8);

    otEXPECT_ACTION(entry >= 0 && entry < RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM, error = OT_ERROR_NO_BUFS);

    addToSrcMatchExtIndirect(aInstance, (uint16_t)entry, aExtAddress);

exit:
    return error;
}

otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    otError error = OT_ERROR_NONE;
    int16_t entry = utilsSoftSrcMatchExtFindEntry(aInstance, aExtAddress);

    otLogDebgPlat("Clear ExtAddr: entry=%d", entry);

    otEXPECT_ACTION(entry >= 0 && entry < RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM, error = OT_ERROR_NO_ADDRESS);

    removeFromSrcMatchExtIndirect(aInstance, (uint16_t)entry);

exit:
    return error;
}

void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance)
{
    const panIndex_t panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    otLogDebgPlat("Clear ExtAddr entries");

    memset(srcMatchExtEntry[panIndex], 0, sizeof(srcMatchExtEntry[panIndex]));

    printExtEntryTable(aInstance);
}
#endif // RADIO_CONFIG_SRC_MATCH_EXT_ENTRY_NUM
