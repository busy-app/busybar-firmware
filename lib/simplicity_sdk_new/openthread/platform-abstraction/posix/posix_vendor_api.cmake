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

set(OT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

set(POSIX_SRC_DIR ${OT_DIR}/platform-abstraction/posix)
set(PAL_INC_DIR   ${OT_DIR}/platform-abstraction/include)
set(CLI_SRC_DIR   ${OT_DIR}/src/cli)
set(CLI_INC_DIR   ${OT_DIR}/include)
set(UTIL_SRC_DIR  ${OT_DIR}/src/util)
set(UTIL_INC_DIR  ${OT_DIR}/include/util)
set(MEMORY_SRC_DIR ${OT_DIR}/src/util/)
set(MEMORY_INC_DIR ${OT_DIR}/include/util/)

add_library(ot-vendor-posix-api
    ${POSIX_SRC_DIR}/posix_vendor_spinel_interface.cpp
    ${POSIX_SRC_DIR}/radio_extension.cpp
)

target_link_libraries(ot-vendor-posix-api
    PUBLIC
        openthread-ftd
        openthread-posix
        ot-posix-config
        ot-config
)

target_compile_definitions(ot-vendor-posix-api
    PRIVATE
        OPENTHREAD_FTD=1
)

target_include_directories(ot-vendor-posix-api
    PUBLIC
        ${POSIX_SRC_DIR}
    PRIVATE
        ${PAL_INC_DIR}
)
