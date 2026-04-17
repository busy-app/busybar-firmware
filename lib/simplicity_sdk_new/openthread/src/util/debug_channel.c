/*******************************************************************************
 * @file
 * @brief This file implements the binary backchannel debug functionality
 * that allows printing to the PTI in Simplicity Studio
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#if defined(SL_CATALOG_OT_DEBUG_CHANNEL_PRESENT)

#include "byte_util.h"
#include "debug_channel.h"
#include <sl_iostream.h>
#include <sl_iostream_debug.h>
#include <sl_iostream_swo_itm_8.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//------------------------------------------------------------------------------
// Forward declarations.

static void        sl_debug_internal_binary_printf(sl_iostream_swo_itm_8_msg_type_t debugType,
                                                   const char                      *formatString,
                                                   va_list                          args);
static void        startMessage(sl_iostream_swo_itm_8_msg_type_t debugType, uint8_t *buffer, size_t bufferSize);
static sl_status_t putBytes(uint8_t *bytes, size_t count);
static sl_status_t endMessage(void);

//------------------------------------------------------------------------------
// Static functions

/**
 * @brief Builds and prints a message on the debug channel using a SL-style format string
 *
 * @param[in]   debugType       the debug message type
 * @param[in]   formatString    SL-style format string
 * @param[in]   args            arguments for \ref formatString
 */
static void sl_debug_internal_binary_printf(sl_iostream_swo_itm_8_msg_type_t debugType,
                                            const char                      *formatString,
                                            va_list                          args)
{
    // Output longer than 128 bytes (with debug framing added) is split into
    // multiple messages.
    uint8_t buffer[128];
    uint8_t pointerLength = 0;

    startMessage(debugType, buffer, sizeof(buffer));
    if (formatString != NULL)
    {
        for (; *formatString; formatString++)
        {
            if (*formatString >= '0' && *formatString <= '9')
            {
                pointerLength = 10 * pointerLength + (*formatString - '0');
                continue;
            }
            switch (*formatString)
            {
            case 'B':
            case 'W':
            case 'D':
            {
                uint8_t data[4];
                uint8_t count;
                if (*formatString == 'B')
                {
                    slStoreLowHighInt32u(data, va_arg(args, unsigned int));
                    count = 1;
                }
                else if (*formatString == 'W')
                {
                    slStoreLowHighInt32u(data, va_arg(args, unsigned int));
                    count = 2;
                }
                else
                {
                    slStoreLowHighInt32u(data, va_arg(args, uint32_t));
                    count = 4;
                }
                putBytes(data, count);
            }
            break;
            case 'l':
                pointerLength = BYTE_0(va_arg(args, unsigned int));
                break;
            case 'p':
            case 'f':
            case 'F':
            {
                // The AVR required special handling for const char * but all current
                // platforms can treat const char * as (uint8_t *).
                uint8_t *data = va_arg(args, uint8_t *);
                if (*formatString == 'F')
                {
                    pointerLength = 0;
                    while (pointerLength < 0xFF && data[pointerLength] != '\0')
                    {
                        pointerLength++;
                    }
                }
                if (putBytes(data, pointerLength) != SL_STATUS_OK)
                {
                    goto fail;
                }
                pointerLength = 0;
            }
            break;
            case 'b':
                // TODO: UNSUPPORTED until I find a replacement for the legacy buffer-management code
                {
                    unsigned int data = va_arg(args, unsigned int);
                    (void)(data);
                }
                break;
            default:
            {
                uint8_t data = *formatString;
                putBytes(&data, 1);
            }
            break;
            }
        }
    }
fail:
    endMessage();
}

static uint8_t *txBuffer;
static size_t   txBufferSize;
static size_t   txBufferLength;

static void startMessage(sl_iostream_swo_itm_8_msg_type_t debugType, uint8_t *buffer, size_t bufferSize)
{
    sl_iostream_set_debug_type(debugType);
    txBuffer       = buffer;
    txBufferSize   = bufferSize;
    txBufferLength = 0;
}

static sl_status_t putBytes(uint8_t *bytes, size_t count)
{
    sl_status_t status    = SL_STATUS_OK;
    size_t      remaining = count;
    while (remaining > 0)
    {
        // Copy largest possible chunk of data that will fit into buffer
        size_t chunk = txBufferSize - txBufferLength;
        if (remaining < chunk)
        {
            chunk = remaining;
        }

        memmove(txBuffer + txBufferLength, bytes, chunk);
        txBufferLength += chunk;
        remaining -= chunk;
        bytes += chunk;

        // Flush txBuffer if there are still bytes that need to be added to the message
        if (remaining > 0)
        {
            status = endMessage();
        }
    }
    return status;
}

static sl_status_t endMessage(void)
{
    sl_status_t status = SL_STATUS_OK;
    status             = sl_iostream_write(sl_iostream_debug_handle, txBuffer, txBufferLength);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }

    txBufferLength = 0;

exit:
    return status;
}

//------------------------------------------------------------------------------
// Public functions

void sl_ot_debug_channel_init(void)
{
    sl_iostream_debug_init();
}

void sl_debug_binary_format(sl_iostream_swo_itm_8_msg_type_t debugType, const char *formatString, ...)
{
    va_list args;
    va_start(args, formatString);
    sl_debug_internal_binary_printf(debugType, formatString, args);
    va_end(args);
}

void sl_debug_printf(const char *formatString, ...)
{
    va_list args;
    va_start(args, formatString);
    sl_iostream_printf(sl_iostream_debug_handle, formatString, args);
    va_end(args);
}

void sl_debug_binary_write(sl_iostream_swo_itm_8_msg_type_t debugType, const void *buffer, size_t buffer_length)
{
    sl_iostream_set_debug_type(debugType);
    sl_iostream_write(sl_iostream_debug_handle, buffer, buffer_length);
}

#endif // SL_CATALOG_OT_DEBUG_CHANNEL_PRESENT
