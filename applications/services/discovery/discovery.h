/**
 * @file discovery.h
 * Facilitates discovery of this device on the local network using mDNS
 */

#pragma once

#include <lwip/api.h>
#include <lwip/netif.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Discovery Discovery;

#define RECORD_DISCOVERY "discovery"

typedef struct DiscoveryRequest DiscoveryRequest;

typedef void (*DiscoveryDynamicInfo)(DiscoveryRequest* request, void* context);

typedef enum {
    DiscoveryTransportTcp,
    DiscoveryTransportUdp,
} DiscoveryTransport;

typedef struct {
    const char* name;
    const char* service;
    DiscoveryTransport transport;
    uint16_t port;
    DiscoveryDynamicInfo txt;
} DiscoveryInfo;

// ================
// API for services
// ================

/**
 * @brief Adds a service to be announced to the local network
 * 
 * @param[inout] discovery Discovery service
 * @param[in] info Service info to be announced
 * @param[inout] context Context for `DynamicInfo` callback. May be NULL
 */
void discovery_service_add(Discovery* discovery, const DiscoveryInfo* info, void* context);

/**
 * @brief Adds an additional TXT record attached to a service
 * 
 * @warning Only call inside of a `DynamicInfo` callback. You can't get a
 * `Request` pointer otherwise anyway.
 * 
 * @param[inout] request Opaque request identifier
 * @param[in] txt TXT record contents
 */
void discovery_request_feed_txt(DiscoveryRequest* request, const char* txt);

#ifdef __cplusplus
}
#endif
