/**
 * @file get_line.h
 * @brief read a line from stdin (or a pipe)
 */
#pragma once

#include <furi/core/string.h>
#include <containers/pipe.h>

typedef enum GetLineError {
    GetLineErrorNone,
    GetLineErrorTooLong,
    GetLineErrorInterrupt,
    GetLineErrorEOF,
} GetLineError;

typedef struct GetLineResult {
    GetLineError error;
    FuriString* line;
} GetLineResult;

/**
 * @brief Read a line from stdin.
 *
 * Only lines up to max_length characters (including the newline) are read and returned.
 * Longer lines are discarded.
 *
 * @param max_length maximal length of the string (CR and LF characters are trimmed).
 * @return result of the operation. If error is GetLineErrorNone, line contains a valid string.
 */
GetLineResult get_line(size_t max_length);

/**
 * @brief Read a line from a pipe.
 *
 * Only lines up to max_length characters (including the newline) are read and returned.
 * Longer lines are discarded.
 *
 * @param pipe pipe to read from.
 * @param max_length maximal length of the string (CR and LF characters are trimmed).
 * @return result of the operation. If error is GetLineErrorNone, line contains a valid string.
 */
GetLineResult get_line_pipe(PipeSide* pipe, size_t max_length);
