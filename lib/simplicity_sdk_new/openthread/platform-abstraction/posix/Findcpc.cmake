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

#[=======================================================================[.rst:
Findcpc
-------

Finds the cpc library.
For our vendor implementation, this module resolves the symbols to build
and link against the CPC library using a vendor agnostic CMake interface.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``cpc::cpc``
  CPC library and header files

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``cpc_FOUND``
  True if the system has the CPC resources.
``cpc_INCLUDE_DIRS``
  Include directories needed to use CPC.
``cpc_LIBRARIES``
  Libraries needed to link to CPC.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be used by the module if set:

``Cpc_INCLUDE_DIR``
  The directory containing ``sl_cpc.h``.
``Cpc_LIBRARY``
  The path to the cpc library.

#]=======================================================================]

include(FindPackageHandleStandardArgs)

find_package(PkgConfig)
pkg_check_modules(Libcpc QUIET IMPORTED_TARGET libcpc)

find_library(Cpc_LIBRARY
    NAMES cpc
    PATHS ${Libcpc_LIBRARY_DIRS}
)

find_path(Cpc_INCLUDE_DIR
    NAMES sl_cpc.h
    PATHS ${Libcpc_INCLUDE_DIRS}
)

find_package_handle_standard_args(cpc
    FOUND_VAR cpc_FOUND
    REQUIRED_VARS Cpc_LIBRARY Cpc_INCLUDE_DIR
)

if(cpc_FOUND AND NOT TARGET cpc::cpc)
    set(cpc_LIBRARIES ${Cpc_LIBRARY})
    set(cpc_INCLUDE_DIRS ${Cpc_INCLUDE_DIR})

    mark_as_advanced(
        Cpc_LIBRARY
        Cpc_INCLUDE_DIR
    )

    add_library(cpc::cpc UNKNOWN IMPORTED)
    set_target_properties(cpc::cpc PROPERTIES
        IMPORTED_LOCATION "${cpc_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${cpc_INCLUDE_DIRS}"
    )

endif()
