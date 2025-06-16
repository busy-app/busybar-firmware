/**
 * @file pipe_util.h
 * @brief Pipe utilities
 */

#pragma once

#include "pipe.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Copies data from one pipe to another until a specific sequence is met.
 * 
 * @param[in] source     Pipe to copy data from
 * @param[in] dest       Pipe to copy data into. May be NULL
 * @param[in] terminator Terminator sequence
 * 
 * @returns `true` if terminator sequence found, `false` if one of the pipes was
 *          closed prematurely.
 */
bool pipe_copy_until(PipeSide* source, PipeSide* dest, const char* terminator);

#ifdef __cplusplus
}
#endif
