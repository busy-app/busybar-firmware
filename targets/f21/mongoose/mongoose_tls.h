/**
 * @file mongoose_tls.h
 * @brief Custom TLS connection flow implementation for the Mongoose library.
 *
 * This module replaces the implementation for the @p mg_tls_init function
 * extending it with hardware crypto-based mTLS capability.
 */
#pragma once

#include <mongoose.h>

#include <toolbox/tls_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise the TLS layer for the given Mongoose connection.
 *
 * @param[in,out] conn pointer to the Mongoose connection to be acted upon
 * @param[in] url pointer to a zero-terminated string containing the remote server URL
 * @param[in] tls_config pointer to a TLS configuration structure
 *
 * @returns @c true on success, @c false otherwise
 */
bool mongoose_tls_init(struct mg_connection* conn, const char* url, const TlsConfig* tls_config);

#ifdef __cplusplus
}
#endif
