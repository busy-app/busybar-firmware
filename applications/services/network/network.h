/**
 * @file network.h
 * @brief Network TCP/IP stack API.
 */
#pragma once

/**
 * @brief The string key for Network instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_NETWORK)`
 */
#define RECORD_NETWORK "network"

#ifdef __cplusplus
extern "C" {
#endif

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
