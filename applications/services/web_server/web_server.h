/**
 * @file web_server.h
 * @brief API for controlling HTTP web server.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the HTTP API version string.
 *
 * @param[out] version A string to write the version into. The string should be initialized by the caller. 
 */
void web_server_get_api_version(FuriString* version);

#ifdef __cplusplus
}
#endif
