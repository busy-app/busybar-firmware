/**
 * @file network.h
 */
#pragma once

#define RECORD_NETWORK "network"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Network Network;

void network_init_current_thread(Network* instance);

void network_deinit_current_thread(Network* instance);

#ifdef __cplusplus
}
#endif
