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
 * @file logging_backchannel.c
 *  This file implements the OpenThread platform abstraction for logging using the backchannel interface.
 *
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include <openthread-core-config.h>
#include <openthread/config.h>

#include <utils/code_utils.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/logging.h>

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_OT_BACKCHANNEL_LOG_PRESENT

#include "debug_channel.h"

#if (OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED)
static bool sLogInitialized = false;

#ifndef LOG_PARSE_BUFFER_SIZE
#define LOG_PARSE_BUFFER_SIZE (19 /* Timestamp */ + OPENTHREAD_CONFIG_LOG_MAX_SIZE + 1 /* \n */)
#endif

#if (LOG_TIMESTAMP_ENABLE == 1)
static inline int logTimestamp(char *aLogString, uint16_t aMaxSize)
{
    long unsigned int now = otPlatAlarmMilliGetNow();
    return snprintf(aLogString, (size_t)aMaxSize, "[%010lu]", now);
}
#endif

static void utilsLogBackchannelOutput(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, va_list ap)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);

    size_t length = 0;
    int    charsWritten;
    char   logString[LOG_PARSE_BUFFER_SIZE + 1];

    otEXPECT(sLogInitialized == true);

#if (LOG_TIMESTAMP_ENABLE == 1)
    length += logTimestamp(logString, LOG_PARSE_BUFFER_SIZE);
#endif

    charsWritten = vsnprintf(&logString[length], (size_t)(LOG_PARSE_BUFFER_SIZE - length), aFormat, ap);
    otEXPECT(charsWritten >= 0);
    length += charsWritten;

    if (length > LOG_PARSE_BUFFER_SIZE)
    {
        length = LOG_PARSE_BUFFER_SIZE;
    }

    logString[length++] = '\n';

    sl_debug_binary_write(EM_DEBUG_PRINTF, logString, length);

exit:
    return;
}

void efr32LogInit(void)
{
    sLogInitialized = true;
}

void efr32LogDeinit(void)
{
    sLogInitialized = false;
}

void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    va_list ap;

    va_start(ap, aFormat);

    utilsLogBackchannelOutput(aLogLevel, aLogRegion, aFormat, ap);

    va_end(ap);
}
#endif

#endif // SL_CATALOG_OT_BACKCHANNEL_LOG_PRESENT
