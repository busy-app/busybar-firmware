/**
 * @file desktop.h
 *
 * @brief Functions for controlling the currently running application.
 *
 * Normally, only one application can be run and visible on the displays (except for services, which
 * may or may not draw something on the display(s)). Desktop service responds to the rotary switch position,
 * additionally, it allows overriding the current app programmatically (e.g. starting from the Apps menu).
 */
#pragma once

#include <stdbool.h>

/**
 * @brief Record key to get the Desktop instance.
 */
#define RECORD_DESKTOP "desktop"

/**
 * @brief Opaque declaration for the Desktop type.
 */
typedef struct Desktop Desktop;

/**
 * @brief Request the Desktop service to replace the currently running app.
 *
 * Calling this function will merely schedule the request, not actually start the
 * application. It may be overridden at any point in time by the rotary switch or
 * further calls to this function.
 *
 * @warning The memory passed by the arg pointer MUST be valid at least
 *          till the start of the application (exact lifetime is determined
 *          by the application logic).
 *
 * @param[in,out] instance pointer to the Desktop instance
 * @param[in] name app name or ID to replace the current app with
 * @param[in,out] arg pointer to an arbitrary object
 * @returns true if the request has been scheduled, false otherwise
 */
bool desktop_replace_current_app(Desktop* instance, const char* name, void* arg);
