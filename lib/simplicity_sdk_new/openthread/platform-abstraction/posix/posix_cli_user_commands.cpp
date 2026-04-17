/*******************************************************************************
 * @file
 * @brief This file defines the vendor posix cli command lists that are registered
 *        with the OpenThread CLI library as user commands.
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
#include "common/code_utils.hpp"

#ifdef SL_OPENTHREAD_ANT_DIV_CLI_ENABLE
extern "C" otError antennaCommand(void *aContext, uint8_t aArgsLength, char *aArgs[]);
#endif // SL_OPENTHREAD_ANT_DIV_CLI_ENABLE

#ifdef SL_OPENTHREAD_COEX_CLI_ENABLE
extern "C" otError coexCommand(void *aContext, uint8_t aArgsLength, char *aArgs[]);
#endif // SL_OPENTHREAD_COEX_CLI_ENABLE

#ifdef SL_OPENTHREAD_TEST_CLI_ENABLE
extern "C" otError testCommand(void *aContext, uint8_t aArgsLength, char *aArgs[]);
#endif

#ifdef SL_OPENTHREAD_EFR32_CLI_ENABLE
extern "C" otError efr32Command(void *aContext, uint8_t aArgsLength, char *aArgs[]);
#endif

#ifdef SL_OPENTHREAD_MEMORY_USAGE_CLI_ENABLE
extern "C" otError memoryUsageHandler(void *aContext, uint8_t aArgsLength, char *aArgs[]);
#endif // SL_OPENTHREAD_MEMORY_USAGE_CLI_ENABLE

//-----------------------------------------------------------------------------
// Extern CLI symbol definitions
//-----------------------------------------------------------------------------
extern const otCliCommand sl_ot_custom_commands[] = {
#ifdef SL_OPENTHREAD_ANT_DIV_CLI_ENABLE
    {"antenna", antennaCommand},
#endif // SL_OPENTHREAD_ANT_DIV_CLI_ENABLE
#ifdef SL_OPENTHREAD_COEX_CLI_ENABLE
    {"coexistence", coexCommand},
#endif // SL_OPENTHREAD_COEX_CLI_ENABLE
#ifdef SL_OPENTHREAD_TEST_CLI_ENABLE
    {"test", testCommand},
#endif // SL_OPENTHREAD_TEST_CLI_ENABLE
#ifdef SL_OPENTHREAD_EFR32_CLI_ENABLE
    {"efr32", efr32Command},
#endif // SL_OPENTHREAD_EFR32_CLI_ENABLE
#ifdef SL_OPENTHREAD_MEMORY_USAGE_CLI_ENABLE
    {"memoryusage", memoryUsageHandler},
#endif // SL_OPENTHREAD_MEMORY_USAGE_CLI_ENABLE
};

uint8_t sl_ot_custom_commands_count = OT_ARRAY_LENGTH(sl_ot_custom_commands);

void otCliVendorSetUserCommands(void)
{
    if (sl_ot_custom_commands_count > 0)
    {
        IgnoreError(otCliSetUserCommands(sl_ot_custom_commands, sl_ot_custom_commands_count, nullptr));
    }
}
