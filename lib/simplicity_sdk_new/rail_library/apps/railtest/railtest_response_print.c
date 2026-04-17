/***************************************************************************//**
 * @file
 * @brief Internal version of response_print.c guarded by mutexes to be consumed
 *    by RTOS based apps.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>

#include "app_task_init.h"
#include "railtest_response_print_mapping.h"

// -----------------------------------------------------------------------------
// Configuration Macros
// -----------------------------------------------------------------------------
// Pull in correct config file:
#ifdef RESPONSE_PRINT_USE_LOCAL_CONFIG_HEADER
  #include "response_print_config.h" // component-level config file (new method)
#else // !defined(RESPONSE_PRINT_USE_LOCAL_CONFIG_HEADER)
  #ifdef CONFIGURATION_HEADER
    #include CONFIGURATION_HEADER // application-level config file (old method)
  #endif

  #ifdef MAX_FORMAT_STRING_SIZE
    #define RESPONSE_PRINT_FORMAT_STR_SIZE_MAX MAX_FORMAT_STRING_SIZE
  #else
    #define RESPONSE_PRINT_FORMAT_STR_SIZE_MAX 256U
  #endif
#endif // defined(RESPONSE_PRINT_USE_LOCAL_CONFIG_HEADER)

#define TAG_VALUE_OVERHEAD 3  // '{', '}', and '\0'

#define RESPONSE_PRINT_RETURN_IF_DISABLED \
  do {                                    \
    if (!responsePrintEnabled) {          \
      return true;                        \
    }                                     \
  } while (false)

// -----------------------------------------------------------------------------
// Structures and Types
// -----------------------------------------------------------------------------
/**
 * @enum StripMode_t
 * This enum allows you to specify a mode to determine whether the tag or value
 * format string are stripped when processing.
 */
typedef enum StripMode {
  STRIP_NONE,  /**< Do not strip either tag or value format strings */
  STRIP_TAG,   /**< Strip the 'tag:' protion of the string */
  STRIP_VALUE, /**< Strip the ':valueFormat' portion of the string */
} StripMode_t;

// -----------------------------------------------------------------------------
// Static Function Prototypes
// -----------------------------------------------------------------------------
static int32_t appendToFormatString(char *dest,
                                    char *src,
                                    uint32_t size,
                                    uint32_t capacity,
                                    StripMode_t stripMode);

static int responsePrintInternal(StripMode_t stripMode,
                                 char *formatString,
                                 va_list args,
                                 bool finalize);

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static volatile bool responsePrintEnabled = true;
static osPriority_t ogPriority;
static uint8_t railapp_mutex_counter = 0U;

// -----------------------------------------------------------------------------
// Response Print Private Functions
// -----------------------------------------------------------------------------

/**
 * Append the given tag:valueFormat pair to the format string while validating
 * it and adding any necessary framing characters. It can optionally strip off
 * either the tag or value from the string
 * @param dest String to add the output to
 * @param src The string to take the tag:valueFormat from
 * @param size The number of characters to read from src
 * @param capacity The space remaining in dest
 * @param stripMode Determines whether to strip the tag or valueFormat
 * components from the string.
 * @return Returns the number of characters written on success and something
 * less than 0 on failure.
 */
static int32_t appendToFormatString(char *dest,
                                    char *src,
                                    uint32_t size,
                                    uint32_t capacity,
                                    StripMode_t stripMode)
{
  char *delim;
  int32_t offset = 0;

  // Make sure there's enough room for this formatter and any overhead
  if (capacity < (size + TAG_VALUE_OVERHEAD)) {
    return -11;
  }

  // If the first byte of the format string is \n, insert that before '{'
  while ((size > 0) && (*src == '\n')) {
    dest[offset++] = *src++;
    size--;
  }

  // Always insert the leading '{'
  dest[offset++] = '{';

  // Copy the data from the format string into our temporary space
  memcpy(dest + offset, src, size);
  offset += size;

  // Insert the trailing '}' and null terminator
  dest[offset++] = '}';
  dest[offset]   = '\0'; // Don't count the null terminator in the size

  // Make sure that the format string is of the form tag:valueFormat
  delim = strstr(dest, ":");
  if (delim == NULL && stripMode == STRIP_NONE) {
    // We don't have a delimiter but one is expected
    return -12;
  } else if (delim == NULL) {
    // There is only one tag/value piece and we only need one so return success
    return offset;
  }

  // Make sure there isn't a second ':' character
  if (strstr(delim + 1, ":") != NULL) {
    return -13;
  }

  // Remove either the tag or value if we're supposed to
  if (stripMode == STRIP_TAG) {
    offset = offset - (delim - dest);
    // Copy the value string over the tag (including the terminating '\0')
    memmove(dest + 1, delim + 1, offset);
  } else if (stripMode == STRIP_VALUE) {
    *delim = '}'; // Just terminate the destination string early
    *(delim + 1) = '\0';
    offset = delim - dest + 1;
  }

  return offset;
}

/**
 * Take in a response print format string and convert it to the appropriate
 * printf-style string. Then pass this string to vprintf() along with any
 * supplied arguments for display.
 * @param stripMode Specifies whether the tag or value format strings should be
 * stripped from the formatString if present.
 * @param formatString This is the format string to process.
 * @param args The va_list object representing the arguments passed on the stack
 * for this print command.
 * @param finalize Whether or not to print the closing '}'
 * @return
 */

static char buffer[RESPONSE_PRINT_FORMAT_STR_SIZE_MAX];

static int responsePrintInternal(StripMode_t stripMode,
                                 char *formatString,
                                 va_list args,
                                 bool finalize)
{
  char *end = formatString, *start = formatString;
  uint32_t offset = 0;
  int rval = 0;

  // Take the input string and convert it into valid response format
  while (end != NULL) {
    uint32_t size;

    // Get the next token in the input buffer to find the tag name
    end = strpbrk(start, ",");

    // If we didn't find an end token then just process the rest of the string
    if (end != NULL) {
      size = end - start;
    } else {
      size = strlen(start);
    }

    // Convert and validate the given format string
    rval = appendToFormatString(buffer + offset,
                                start,
                                size,
                                (RESPONSE_PRINT_FORMAT_STR_SIZE_MAX - offset),
                                stripMode);
    if (rval < 0) {
      break;
    }
    // Move past all the currently written characters
    offset += rval;

    // Advance the start pointer
    start = end + 1;
  }
  // Print out the parsed format buffer
  vprintf(buffer, args);

  // Print out the error code if there is one
  if (rval < 0) {
    printf(" {internal_error:%d}", -rval);
  } else {
    rval = 0;
  }
  if (finalize) {
    printf("}\n");
  }

  return rval;
}

/**
 * Performs the operation of responsePrintStart
 * @param command The name of the command that's being executed.
 * @param useMutex Boolean to determine lock has to be acquired before printing
 * @return Returns true on success and false on failure.
 */
static bool responsePrintStartInternal(char *command, bool useMutex)
{
  if (useMutex) {
    railapp_mutex_acquire();
  }
  RESPONSE_PRINT_RETURN_IF_DISABLED;
  // Print the start of command standard formatting
  printf("{");
  if (command != NULL) {
    printf("{(%s)}", command);
  }
  return true;
}

// -----------------------------------------------------------------------------
// Response Print Public Functions
// -----------------------------------------------------------------------------

void railapp_mutex_acquire(void)
{
  if (osKernelGetState() == osKernelRunning) {
    assert(osMutexAcquire(railapp_mtx, osWaitForever) == osOK);
    if (railapp_mutex_counter == 0U) {
      ogPriority = osThreadGetPriority(osThreadGetId()); // Store priority only for the first time
      assert(osThreadSetPriority(osThreadGetId(), osPriorityNormal2) == osOK);
    }
    railapp_mutex_counter++;
  }
}

void railapp_mutex_release(void)
{
  if (osKernelGetState() == osKernelRunning) {
    assert(osMutexRelease(railapp_mtx) == osOK);
    railapp_mutex_counter--;
    if (railapp_mutex_counter == 0U) {
      assert(osThreadSetPriority(osThreadGetId(), ogPriority) == osOK);
    }
  }
}

void railtest_responsePrintEnable(bool enable)
{
  responsePrintEnabled = enable;
}

bool railtest_responsePrintHeader(char *command, char *formatString, ...)
{
  railapp_mutex_acquire();
  RESPONSE_PRINT_RETURN_IF_DISABLED;
  va_list ap;

  va_start(ap, formatString);
  printf("#");  // Header strings start with a '#'
  responsePrintStartInternal(command, false);
  bool success = (responsePrintInternal(STRIP_VALUE, formatString, ap, true) == 0);
  va_end(ap);
  railapp_mutex_release();
  return success;
}

bool railtest_responsePrintMulti(char *formatString, ...)
{
  railapp_mutex_acquire();
  RESPONSE_PRINT_RETURN_IF_DISABLED;
  va_list ap;

  va_start(ap, formatString);
  responsePrintStartInternal(NULL, false);
  bool success = (responsePrintInternal(STRIP_TAG, formatString, ap, true) == 0);
  va_end(ap);
  railapp_mutex_release();
  return success;
}

bool railtest_responsePrint(char *command, char *formatString, ...)
{
  railapp_mutex_acquire();
  RESPONSE_PRINT_RETURN_IF_DISABLED;
  va_list ap;

  va_start(ap, formatString);
  responsePrintStartInternal(command, false);
  bool success = (responsePrintInternal(STRIP_NONE, formatString, ap, true) == 0);
  va_end(ap);
  railapp_mutex_release();
  return success;
}

bool railtest_responsePrintStart(char *command)
{
  responsePrintStartInternal(command, true);
  return true;
}

bool railtest_responsePrintContinue(char *formatString, ...)
{
  RESPONSE_PRINT_RETURN_IF_DISABLED;
  va_list ap;

  va_start(ap, formatString);
  bool success = (responsePrintInternal(STRIP_NONE, formatString, ap, false) == 0);
  va_end(ap);
  return success;
}

bool railtest_responsePrintEnd(char *formatString, ...)
{
  RESPONSE_PRINT_RETURN_IF_DISABLED;
  va_list ap;
  bool success = true;
  va_start(ap, formatString);
  if (formatString[0] == '}') {
    printf("%s\n", formatString);
  } else {
    success = (responsePrintInternal(STRIP_NONE, formatString, ap, true) == 0);
    va_end(ap);
  }
  railapp_mutex_release();
  return success;
}

bool railtest_responsePrintError(char *command, uint8_t code, char *formatString, ...)
{
  railapp_mutex_acquire();
  RESPONSE_PRINT_RETURN_IF_DISABLED;
  va_list ap;

  va_start(ap, formatString);

  // Print the command name
  printf("{");
  if (command != NULL) {
    printf("{(%s)}", command);
  }

  // Print the formatted error string.
  // @todo: Add validation of the formatString
  printf("{error:");
  vprintf(formatString, ap);
  printf("}");

  // Print the error code if it was specified
  printf("{errorCode:%d}", code);

  // Terminate the response
  printf("}\n");

  va_end(ap);
  railapp_mutex_release();
  return true;
}
