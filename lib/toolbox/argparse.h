/**
 * @file argparse.h
 * @brief Command line argument parsing library.
 *
 * Supports a getopt-like syntax with single-character options and positional arguments.
 * Options may have a required argument or be argument-less.
 *
 * Strings containing space characters must be enclosed in single or double  quotes.
 * If quotes need to be present inside of a quoted string, they must be of opposite type, e.g.
 * "String with 'quotes'" or 'String with "quotes"'.
 *
 * Escaping is not supported.
 */
#pragma once

#include <core/string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Option callback function type.
 *
 * If @p opt is a zero (NUL) character, the parsed option is a positional argument.
 *
 * If an option argument was not required, @p optarg will be @c NULL.
 *
 * @param[in] opt option character from the parsed argument string
 * @param[in] optarg option argument (if required and present, may be @c NULL)
 * @param[in,out] context pointer to a user-specified object from @c parse_args invocation
 */
typedef void (*OptionCallback)(char opt, const char* optarg, void* context);

/**
 * @brief Parse command options from an argument string.
 *
 * @p opts must be a string containing all of the options to be looked for, e.g. `"abcDEF"`.
 *
 * To make an option require an argument, `:` must be added after the option character, i.e.
 * `"a:b:c"` means that `-a` and `-b` options require an argument, while `-c` does not.
 *
 * If only positional arguments are needed, @p opts can be @c NULL or an empty string.
 *
 * The following situations are treated as a success:
 * - Input string contains options that match the @p opts option specifier string
 * - Input string contains only positional arguments regardless of @p opts contents
 * - Input string is empty or contains only space characters
 *
 * The following situations are treated as errors:
 * - An option is found in the input string that is not in the option specifier string
 * - An option argument is required, but is not present in the input string
 * - No option specifier is provided, but options are present in the input string
 * - A non-paired quote was found in the input string
 *
 * @param[in,out] args pointer to a @c FuriString containing the argument string
 * @param[in] opts pointer to a zero-terminated string containing option specifiers (may be @c NULL)
 * @param[in] callback pointer to a function to be called for each option or positional argument
 * @param[in,out] context pointer to a user-specified object (will be passed to the callback)
 * @returns @c true on success, @c false otherwise
 */
bool parse_args(FuriString* args, const char* opts, OptionCallback callback, void* context);

#ifdef __cplusplus
}
#endif
