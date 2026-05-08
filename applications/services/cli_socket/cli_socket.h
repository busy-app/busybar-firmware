#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Update the runtime WiFi CLI enable state.
 *
 * Called by the sysctl command after persisting the new value.
 * Safe to call from any thread.
 */
void cli_socket_set_wifi_enabled(bool enabled);

#ifdef __cplusplus
}
#endif
