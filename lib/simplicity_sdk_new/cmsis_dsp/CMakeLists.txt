# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

# This CMakeLists.txt in the root of the repository ensures that, when another
# project fetches CMSIS-DSP by calling FetchContent_MakeAvailable(), the call
# automatically adds CMSIS-DSP as a subdirectory. To maintain compatibility with
# existing projects that assume CMSIS-DSP's entry point to be Source/, we only
# add a redirection in the root CMakeLists.txt.

cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 17)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/module")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(sled)

project(cmsis_dsp)

# Use a custom configuration type name to avoid getting a default optimization
# setting from the toolchain. We want to manually specify -Ofast later.
set(CMAKE_CONFIGURATION_TYPES fast)

#############
# CMSIS-DSP #
#############

# TODO: IAR build fails for fp16, disable it for now
if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
  set(DISABLEFLOAT16 ON)
endif()

add_subdirectory(Source)
# Link DSP library with CMSIS-Core
find_package(SLCC MODULE REQUIRED COMPONENTS cmsis_core)
target_link_libraries(CMSISDSP PRIVATE slc::cmsis_core)

# Optimize for speed
target_compile_options(CMSISDSP PRIVATE
  $<$<C_COMPILER_ID:GNU>:-Ofast>
  $<$<C_COMPILER_ID:Clang>:-O3 -ffast-math>
  $<$<C_COMPILER_ID:IAR>:-Ohs>
)

# Enable fp16 for GCC
target_compile_options(CMSISDSP PRIVATE
  $<$<C_COMPILER_ID:GNU>:-mfp16-format=ieee>
)

install(TARGETS CMSISDSP OPTIONAL ARCHIVE DESTINATION ".")
sled_target_set_slc_component_metadata(CMSISDSP
  EXPORT TRUE
  COMPONENT cmsis_dsp
  INSTALL_DIR "."
)
