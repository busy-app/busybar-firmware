/*******************************************************************************
 * @file
 * @brief Utility functions for handling CLI list of CLI commands
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

#include "sl_ot_custom_cli.h"
#include <string.h>
#include "common/code_utils.hpp"

otError processCommand(void              *aContext,
                       uint8_t            aArgsLength,
                       char              *aArgs[],
                       uint8_t            aCommandsLength,
                       const otCliCommand aCommands[])
{
    otError error = OT_ERROR_INVALID_COMMAND;

    VerifyOrExit(aArgs && aArgsLength != 0 && aCommands && aCommandsLength != 0);

    for (size_t i = 0; i < aCommandsLength; i++)
    {
        if (strcmp(aArgs[0], aCommands[i].mName) == 0)
        {
            // Command found, call command handler
            error = (aCommands[i].mCommand)(aContext, aArgsLength - 1, (aArgsLength > 1) ? &aArgs[1] : NULL);
            ExitNow();
        }
    }

exit:
    return error;
}

void printCommands(otCliCommand commands[], size_t commandCount)
{
    for (size_t i = 0; i < commandCount; i++)
    {
        otCliOutputFormat("%s\r\n", commands[i].mName);
    }
}
