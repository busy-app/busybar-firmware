#
#  # License
#  <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
# 
#  SPDX-License-Identifier: Zlib
# 
#  The licensor of this software is Silicon Laboratories Inc.
# 
#  This software is provided 'as-is', without any express or implied
#  warranty. In no event will the authors be held liable for any damages
#  arising from the use of this software.
# 
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
# 
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#  2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
#  3. This notice may not be removed or altered from any source distribution.
#

include(${CMAKE_CURRENT_LIST_DIR}/posix_vendor_api.cmake)

set(CLI_SOURCES "")

option(SL_OT_ANTENNA_CLI "Enable support for antenna diversity cli commands" ON)
option(SL_OT_COEX_CLI    "Enable support for coexistence cli commands" ON)
option(SL_OT_TEST_CLI    "Enable support for test cli commands" OFF)
option(SL_OT_EFR32_CLI   "Enable support for efr32/platform cli commands" OFF)
option(SL_OT_MEMORY_CLI  "Enable support for memory usage cli commands" ON)

if(SL_OT_ANTENNA_CLI)
    list (APPEND CLI_SOURCES ${CLI_SRC_DIR}/antenna_diversity_cli.c)
    target_compile_definitions(ot-config INTERFACE "SL_OPENTHREAD_ANT_DIV_CLI_ENABLE=1")
endif()

if(SL_OT_COEX_CLI)
    list (APPEND CLI_SOURCES ${CLI_SRC_DIR}/coexistence_cli.c)
    target_compile_definitions(ot-config INTERFACE "SL_OPENTHREAD_COEX_CLI_ENABLE=1")
endif()

if(SL_OT_TEST_CLI)
    list (APPEND CLI_SOURCES ${CLI_SRC_DIR}/test_cli.c)
    target_compile_definitions(ot-config INTERFACE "SL_OPENTHREAD_TEST_CLI_ENABLE=1")
endif()

if(SL_OT_EFR32_CLI)
    list (APPEND CLI_SOURCES ${CLI_SRC_DIR}/efr32_cli.c)
    target_compile_definitions(ot-config INTERFACE "SL_OPENTHREAD_EFR32_CLI_ENABLE=1")
endif()

if(SL_OT_MEMORY_CLI)
    list (APPEND CLI_SOURCES ${CLI_SRC_DIR}/memory_usage_cli.c)
    target_compile_definitions(ot-config INTERFACE "SL_OPENTHREAD_MEMORY_USAGE_CLI_ENABLE=1")
endif()

if(NOT CLI_SOURCES STREQUAL "")
    target_compile_definitions(ot-config INTERFACE "OPENTHREAD_CONFIG_CLI_MAX_USER_CMD_ENTRIES=2")

    add_library(ot-vendor-posix-cli
        ${CLI_SRC_DIR}/cli_utils.c
        ${POSIX_SRC_DIR}/posix_cli_user_commands.cpp
        ${CLI_SOURCES}
    )

    target_link_libraries(ot-vendor-posix-cli
        PUBLIC
            ot-vendor-posix-api
            openthread-cli-ftd
            ot-posix-config
            ot-config
    )

    target_compile_definitions(ot-vendor-posix-cli
        PRIVATE
            OPENTHREAD_FTD=1
    )

    target_include_directories(ot-vendor-posix-cli
        PRIVATE
            ${PAL_INC_DIR}
            ${CLI_INC_DIR}
            ${UTIL_INC_DIR}
    )

    set(OT_CLI_VENDOR_TARGET ot-vendor-posix-cli)
endif()
