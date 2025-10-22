#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool updater_nwp_session_is_current_version(const char* nwp_rps_path);

#ifdef __cplusplus
}
#endif
