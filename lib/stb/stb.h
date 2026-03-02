/**
 * @file stb.h
 * @brief Includes and configures parts of STB that we need
 */

#pragma once

#pragma GCC diagnostic push
// stb_ds.h:523:7: error: "__clang__" is not defined, evaluates to 0 [-Werror=undef]
//   523 |   #if __clang__
//       |       ^~~~~~~~~
#pragma GCC diagnostic ignored "-Wundef"

// Data Structures
#define STBDS_NO_SHORT_NAMES
#include "stb_repo/stb_ds.h"

#pragma GCC diagnostic pop
