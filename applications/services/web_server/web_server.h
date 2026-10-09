/**
 * @file web_server.h
 * @brief API for controlling HTTP web server.
 */
#pragma once

#include <toolbox/semver.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the HTTP API version.
 *
 * @param[out] version Pointer to a SemVer structure to hold the value (must be allocated).
 */
void web_server_get_api_version(SemVer* version);

/**
 * @brief Get the HTTP API version string.
 *
 * @param[out] version_string A string to write the version into. The string should be initialized by the caller.
 */
void web_server_get_api_version_string(FuriString* version_string);

#ifdef __cplusplus
}
#endif
