/**
 * @file js_app_common.h
 * @brief Common definitions regarding JavaScript apps.
 *
 * @see @ref js_app.h for more info on JavaScript apps.
 */
#pragma once
#include <stdbool.h>

#define JS_APP_ID_LEN_MAX 32

/**
 * @brief Check if an application ID is valid.
 *
 * Valid application IDs correspond to the following regular expression:
 * ^[a-zA-Z0-9_\-][a-zA-Z0-9_\-.]{0,31}$
 *
 * @param[in] app_id zero-terminated string to be validated
 * @return true if app_id is a valid application ID
 */
bool js_app_registry_is_valid_app_id(const char* app_id);
