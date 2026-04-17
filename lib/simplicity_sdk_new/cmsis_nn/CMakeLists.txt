cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 17)

include(sled)

project(cmsis_nn)

# Use a custom configuration type name to avoid getting a default optimization
# setting from the toolchain. We want to manually specify -Ofast later.
set(CMAKE_CONFIGURATION_TYPES fast)

############
# CMSIS-NN #
############

add_library(cmsis-nn STATIC)

target_compile_options(cmsis-nn PRIVATE -Ofast)

target_include_directories(cmsis-nn PUBLIC "Include")

add_subdirectory(Source)

# Optimize for speed
if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
# cmsis-nn target sets -Ofast, replace it with -Ohs for IAR
set_target_properties(cmsis-nn PROPERTIES COMPILE_OPTIONS "-Ohs")
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
# cmsis-nn target sets -Ofast, replace it with -3 -ffast-math for LLVM
set_target_properties(cmsis-nn PROPERTIES COMPILE_OPTIONS "-O3;-ffast-math")
endif()

install(TARGETS cmsis-nn OPTIONAL ARCHIVE DESTINATION ".")
sled_target_set_slc_component_metadata(cmsis-nn
  EXPORT TRUE
  COMPONENT cmsis_nn
  INSTALL_DIR "."
)
