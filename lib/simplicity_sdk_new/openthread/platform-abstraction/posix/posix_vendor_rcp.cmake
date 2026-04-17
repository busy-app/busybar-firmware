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

set(CPCD_SOURCE_DIR "" CACHE PATH "path to directory containing cpcd CMakeLists.txt")

set(OT_POSIX_CONFIG_RCP_VENDOR_TARGET "cpc-interface")
set(CPC_LIB_TARGET "cpc")

if (NOT OT_PLATFORM_CONFIG)
    message(WARNING "OT_PLATFORM_CONFIG file which defines OT_VENDOR_RADIO_URL_HELP_BUS missing")
endif()

add_library(${OT_POSIX_CONFIG_RCP_VENDOR_TARGET} INTERFACE)

# If cpc target isn't already defined, run find_package to build
# library as subdirectory or locate pre-installed resources
# and add imported targets to cpc interface target
if (NOT TARGET cpc)
    if (CPCD_SOURCE_DIR)
        option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

        # Use cpc (lib) target directly and query header path from cpcd project
        add_subdirectory(${CPCD_SOURCE_DIR} build)
        get_target_property(CPC_HEADER cpc PUBLIC_HEADER)
        get_filename_component(CPC_INC ${CPC_HEADER} DIRECTORY)
        target_include_directories(${OT_POSIX_CONFIG_RCP_VENDOR_TARGET} INTERFACE ${CPC_INC})
    else()
        list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
        find_package(cpc REQUIRED)
        set(CPC_LIB_TARGET "cpc::cpc")
    endif()
endif()

target_link_libraries(${OT_POSIX_CONFIG_RCP_VENDOR_TARGET} 
    INTERFACE
        ${CPC_LIB_TARGET}
        ot-posix-config
)

target_include_directories(${OT_POSIX_CONFIG_RCP_VENDOR_TARGET}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/core
        ${PROJECT_SOURCE_DIR}/src/posix/platform/
        ${PROJECT_SOURCE_DIR}/src/posix/platform/include
)
