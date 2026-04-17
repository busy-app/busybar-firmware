/*******************************************************************************
 * @file
 * @brief Provides definitions required to support CLI in both SoC and RCP
 * builds
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

#include <openthread/cli.h>

extern otCliCommand sl_ot_custom_commands[];
extern const uint8_t sl_ot_custom_commands_count;

/**
 * Updates the list of OT CLI user commands with command lists registered
 * in sl_ot_custom_commands.
 */

void sl_ot_custom_cli_init(void);

/**
 * Iterates over list of commands in @p aCommands to match the name
 * of the command referred to in @p aArgs and execute the handler
 * function if found.
 *
 * @param[in] *aContext	        Pointer to openthread instance
 * @param[in]  aArgsLength      Number of arguments in @p aArgs
 * @param[in] *aArgs[]          Array of arguments to pass to handler
 * @param[in]  aCommandsLength  Number of subcommands in @p aCommands
 * @param[in]  aCommands[]      List of subcommands registered by cli module
 *
 * @returns The result of the executed handler, or OT_ERROR_INVALID_ARGS if command was not found.
 */

otError processCommand(void *aContext,
                       uint8_t aArgsLength,
                       char *aArgs[],
                       uint8_t aCommandsLength,
                       const otCliCommand aCommands[]);

/**
 * Print all commands in @p commands
 *
 * @param[in]  commands         list of commands
 * @param[in]  commandCount     number of commands in @p commands
 *
 */
void printCommands(otCliCommand commands[], size_t commandCount);
