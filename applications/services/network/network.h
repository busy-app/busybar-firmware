/**
 * @file network.h
 * @brief Network TCP/IP stack API.
 */
#pragma once

#include <lwip/netif.h>

/**
 * @brief The string key for Network instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_NETWORK)`
 */
#define RECORD_NETWORK "network"

/** Identifies a logical network interface. */
typedef enum {
    NetworkNetifWifi, /**< Wi-Fi (WL) interface */
    NetworkNetifUsb, /**< USB-NCM (EX) interface */
    NetworkNetifCount, /**< Sentinel — not a valid interface */
} NetworkNetif;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Find a netif by its logical interface identifier.
 *
 * Must be called with the lwIP core lock held — either from within the lwIP thread
 * or between LOCK_TCPIP_CORE / UNLOCK_TCPIP_CORE.
 *
 * @param netif  Logical interface to find.
 * @return First matching netif, or NULL if not found.
 */
struct netif* network_find_netif(NetworkNetif netif);

/**
 * @brief Assign the lwIP 2-character name to a netif.
 *
 * Encapsulates the interface-to-name mapping; call from netif init callbacks.
 *
 * @param netif  The lwIP netif to name.
 * @param id     Logical interface identifier.
 */
void network_netif_assign_name(struct netif* netif, NetworkNetif id);

/** Opaque Network type declaration. */
typedef struct Network Network;

/**
 * @brief Initialise the calling thread for use with the TCP/IP stack.
 *
 * Every thread that uses APIs from the following files:
 *
 * - lwip/api.h
 * - lwip/netbuf.h
 * - lwip/netifapi.h
 * - lwip/sockets.h
 *
 * MUST first register itself by calling this function
 * from inside of its execution context.
 *
 * @param[in,out] instance pointer to the Network instance
 */
void network_init_current_thread(Network* instance);

/**
 * @brief Deinitialise the calling thread
 *
 * Needs to be called before exiting from a thread initialised with the
 * network_init_current_thread() call.
 *
 * @param[in,out] instance pointer to the Network instance
 */
void network_deinit_current_thread(Network* instance);

#ifdef __cplusplus
}
#endif
