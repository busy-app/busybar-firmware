/**
 * @file discovery.h
 * @brief mDNS network discovery service API.
 */
#pragma once

#include <core/string.h>

/**
 * @brief The string key for Discovery instance access.
 */
#define RECORD_DISCOVERY "discovery"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Discovery service opaque type.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_DISCOVERY)`
 */
typedef struct Discovery Discovery;

/**
 * @brief Called to request one DNS-SD TXT record
 *
 * @param[in] index Index of the record that this function should return
 * @param[out] txt_out pointer to the output buffer string
 * @param[in,out] context pointer to the user-specific object (may be @c NULL)
 * 
 * @returns `true` if data was given and iteration should continue,
 *          `false` if no data was given and iteration should stop
 */
typedef bool (*DiscoveryTxtCallback)(size_t index, FuriString* txt_out, void* context);

/**
 * @brief Enumeration of supported transport protocol types.
 */
typedef enum {
    DiscoveryTransportTypeTcp, /**< TCP as transport protocol */
    DiscoveryTransportTypeUdp, /**< UDP as transport protocol */
} DiscoveryTransportType;

/**
 * @brief Announced service information structure.
 */
typedef struct {
    const char* name; /**< The service's name string (e.g. "httpd") */
    const char* service; /**< The service type string (e.g. "_http") */
    DiscoveryTxtCallback txt_callback; /**< Pointer to the TXT callback function */
    DiscoveryTransportType transport_type; /**< Transport protocol type */
    uint16_t port; /**< Port number the service is listening on (e.g. 80) */
} DiscoveryServiceInfo;

/**
 * @brief Add a service to be announced to the local network.
 *
 * @warning The data pointed to by @p info and @c context must live forever
 *          (i.e. must NOT be deleted after the call to this function).
 *
 * @param[in,out] discovery Discovery service
 * @param[in] info Service info to be announced
 * @param[in,out] context Context for @p txt_callback (may be @c NULL)
 *
 * @returns @c true on success, @c false otherwise
 */
bool discovery_add_service(Discovery* discovery, const DiscoveryServiceInfo* info, void* context);

#ifdef __cplusplus
}
#endif
