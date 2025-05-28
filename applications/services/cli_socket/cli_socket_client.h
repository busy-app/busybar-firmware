#pragma once

#include <furi.h>
#include <lwip/tcp.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts a client thread
 * @param[in] client_socket lwIP client socket handle
 */
void cli_socket_client_start(struct tcp_pcb* client_socket);

#ifdef __cplusplus
}
#endif
